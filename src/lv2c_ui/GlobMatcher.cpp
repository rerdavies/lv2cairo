// MIT License
//
// Copyright (c) 2023 Robin E. R. Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/*////////////////////////////////////////////////////////////////////////////////////////////
SECURITY WARNING: Please review the security note at the top of GlobMatcher.hpp
                Not suitable for use on internet-facing content.
//////////////////////////////////////////////////////////////////////////////////////////*/


#include "lv2c_ui/GlobMatcher.hpp"
#include <sstream>

using namespace std;
using namespace lv2c::ui;


static bool isEndOfSegment(char c)
{
#ifdef WIN32
    return c == '\0' || c == '/' || c == '\'' || c == ':';
#else
    return c == '\0' || c == '/';
#endif
}

class GlobExpression
{
    friend class GlobMatcher;

public:
    virtual ~GlobExpression() {}

    bool NextMatchesOne(char c) { return next->MatchesOne(c); }
    bool NextMatches(const char *p)
    {
        // guard against "*?*?*?*?*?*[!b]*" DDOSattacks.
        if (++backtrackingAttempts > GlobMatcher::MAX_BACKTRACKING_ATTEMPTS)
        {
            throw std::runtime_error("Maximum backtracking attempts exceeded. Please simplify your pattern.");
        }
        return next->Matches(p);
    }

    virtual bool MatchesOne(char c) = 0;
    virtual bool Matches(const char *p) = 0;

    virtual bool isMatchMany() const { return false; }

    uint64_t backtrackingAttempts = 0;
    GlobExpression *next = nullptr;
};

class MatchManyExpression : public GlobExpression
{
public:
    MatchManyExpression() {}

public:
    virtual bool isMatchMany() const override { return true; }
    using ptr = std::shared_ptr<MatchManyExpression>;
    static ptr Create() { return std::make_shared<MatchManyExpression>(); }

    virtual bool MatchesOne(char c) override { return isEndOfSegment(c); }
    bool Matches(const char *p) override
    {
        while (true)
        {
            if (next->isMatchMany())
            {
                return NextMatches(p); // avoid  "*********" DOS attack. *?*?*?*?*?*?*?* isn't great either.
            }
            if (isEndOfSegment(*p))
            {
                return NextMatches(p);
            }
            if (NextMatchesOne(*p))
            {
                if (NextMatches(p))
                {
                    return true;
                }
            }
            ++p;
        }
    }
};

class MatchEndExpression : public GlobExpression
{
public:
    MatchEndExpression() {}

public:
    using ptr = std::shared_ptr<MatchEndExpression>;
    static ptr Create() { return std::make_shared<MatchEndExpression>(); }
    bool MatchesOne(char c) override
    {
        return isEndOfSegment(c);
    }
    bool Matches(const char *p) override
    {
        return isEndOfSegment(*p);
    }
};

class MatchOneExpression : public GlobExpression
{
public:
    MatchOneExpression() {}

public:
    using ptr = std::shared_ptr<MatchOneExpression>;
    static ptr Create() { return std::make_shared<MatchOneExpression>(); }

    bool MatchesOne(char c) override
    {
        return !isEndOfSegment(c);
    }
    bool Matches(const char *p) override
    {
        if (isEndOfSegment(*p))
            return false;
        ++p;
        return NextMatches(p);
    }
};

class MatchRunExpression : public GlobExpression
{
public:
    MatchRunExpression(const std::string &text) : text(text) {}

public:
    using ptr = std::shared_ptr<MatchRunExpression>;
    static ptr Create(const std::string text) { return std::make_shared<MatchRunExpression>(text); }

    bool MatchesOne(char c) override
    {
        return c == text[0];
    }
    bool Matches(const char *p) override
    {
        const char *match = text.c_str();
        while (true)
        {
            if (*match == '\0')
            {
                return NextMatches(p);
            }
            if (*match != *p)
                return false;
            ++p;
            ++match;
        }
    }

private:
    std::string text;
};

class MatchAlternatesExpression : public GlobExpression
{
public:
    MatchAlternatesExpression(bool inverted, const std::string &alternates) : inverted(inverted), alternates(alternates) {}

public:
    using ptr = std::shared_ptr<MatchAlternatesExpression>;
    static ptr Create(bool inverted, const std::string text) { return std::make_shared<MatchAlternatesExpression>(inverted, text); }

    bool MatchesOne(char c)
    {
        if (isEndOfSegment(c))
            return false; // never allowed to match, even if we parsed it wrong.
        bool match = alternates.find_first_of(c) != string::npos;
        return match != inverted;
    }
    bool Matches(const char *p)
    {
        if (isEndOfSegment(*p))
            return false; // never allowed to match, even if we parsed it wrong.
        bool match = alternates.find_first_of(*p) != string::npos;
        if (match != inverted)
        {
            ++p;
            return NextMatches(p);
        }
        return false;
    }

private:
    bool inverted;
    std::string alternates;
};

GlobMatcher::GlobMatcher()
{
}

GlobMatcher::GlobMatcher(const std::string &pattern)
{
    SetPattern(pattern);
}

void GlobMatcher::PushRun(std::string &run)
{
    if (run.size() != 0)
    {
        expressions.push_back(MatchRunExpression::Create(run));
        run.resize(0);
    }
}
void GlobMatcher::SetPattern(const std::string &pattern)
{
    expressions.resize(0);

    if (pattern == "" || pattern == "*")
    {
        return;
    }
    std::stringstream s(pattern);
    using int_type = std::stringstream::int_type;
    std::string run;
    while (true)
    {
        int_type c = s.get();
        if (c == EOF)
        {
            break;
        }
        if (c == '\\')
        {
            c = s.get();
            if (c == EOF)
            {
                throw std::runtime_error("Invalid pattern.");
            }
            run.push_back((char)c);
        }
        else if (c == '*')
        {
            PushRun(run);
            expressions.push_back(MatchManyExpression::Create());
        }
        else if (c == '?')
        {
            PushRun(run);
            expressions.push_back(MatchOneExpression::Create());
        }
        else if (c == '[')
        {
            PushRun(run);
            std::string alternates;
            bool inverse = false;
            c = s.get();
            if (c == '!')
            {
                inverse = true;
                alternates.push_back((char)c);
                c = s.get();
            }
            while (true)
            {
                if (c == EOF)
                    throw ::runtime_error("Invalid pattern.");
                if (c == ']')
                    break;
                alternates.push_back((char)c);
                c = s.get();
            }
            expressions.push_back(MatchAlternatesExpression::Create(inverse, alternates));
        }
        else
        {
            run.push_back((char)c);
        }
    }
    PushRun(run);
    expressions.push_back(MatchEndExpression::Create());

    for (size_t i = 0; i < expressions.size() - 1; ++i)
    {
        expressions[i]->next = expressions[i + 1].get();
    }
}

bool GlobMatcher::Matches(const std::string &text)
{
    if (expressions.size() == 0)
        return true;
    for (auto &expression : expressions)
    {
        expression->backtrackingAttempts = 0;
    }
    const char *p = text.c_str();
    while (true)
    {
        if (expressions[0]->Matches(p))
        {
            return true;
        }
        while (!isEndOfSegment(*p))
        {
            ++p;
        }
        if (*p == '\0')
        {
            return false;
        }
        ++p;
    }
}

#ifdef ENABLE_GLOBMATCHER_UNIT_TEST

static void TestMatch(const std::string &pattern, const std::string &target, bool expected)
{
    GlobMatcher matcher(pattern);

    bool match = matcher.Matches(target);
    if (match != expected)
    {
        throw std::runtime_error("Test failed.");
    }
}
static void ExpectException(const std::string &pattern, const std::string &target)
{
    bool hadException = false;
    try
    {
        GlobMatcher matcher(pattern);

        bool match = matcher.Matches(target);
        (void)match;
    }
    catch (const std::exception &e)
    {
        hadException = true;
    }
    if (!hadException)
    {
        throw std::runtime_error("Test failed.");
    }
}

#include <iostream>
#include <chrono>
void GlobMatcherTest()
{
    cout << "Running Glob Matcher test " << endl;

    TestMatch("a*c", "axb/axc", true);

    TestMatch("a", "a", true);
    TestMatch("a", "a/b", true);
    TestMatch("a", "b/a", true);
    TestMatch("a", "b/c", false);

    TestMatch("*", "abc", true);
    TestMatch("*c", "abc", true);
    TestMatch("a*c", "a/c", false);
    TestMatch("a*c", "axb/axc", true);

    TestMatch("?", "", false);
    TestMatch("?", "b", true);
    TestMatch("?", "bb", false);
    TestMatch("*?", "", false);
    TestMatch("*?", "b", true);
    TestMatch("*?", "bb", true);
    TestMatch("?b", "bb", true);
    TestMatch("b?", "bb", true);
    TestMatch("*b?b*", "aaaababaaaa", true);
    TestMatch("*b??b*", "aaaababaaaa", false);

    TestMatch("[a]", "a", true);
    TestMatch("[!a]", "a", false);

    TestMatch("[a][!a]", "ab", true);
    TestMatch("[a][!a]", "aa", false);
    TestMatch("[abc][!a]", "cb", true);
    TestMatch("[abc][!a]", "db", false);
    TestMatch("[abc][!a]", "ba", false);
    TestMatch("[abc]*[!a]", "bcccccc", true);

    TestMatch("[]", "a", false);
    TestMatch("[!]", "a", true);

    using clock_t = std::chrono::steady_clock;

    {
        auto start = clock_t::now();
        ExpectException("*[!]*[!]*[!]*[!]x", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");

        auto duration = clock_t::now() - start;
        uint64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        cout << "Worst-case execution time: " << ms << "ms." << endl;
    }

    cout << "Glob Matcher test succeeded. " << endl;
}

#endif