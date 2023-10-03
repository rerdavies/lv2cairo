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
#pragma once
/*////////////////////////////////////////////////////////////////////////////////////////////
SECURITY WARNING: Not suitable for use on Internet-facing user input.

This code may have a potential DDOS vulnerability if used on patterns supplied by remote
users.  Perfectly fine for local user input though.

See analysis at the end of this file.
//////////////////////////////////////////////////////////////////////////////////////////*/
#include <vector>
#include <memory>
#include <string>

#ifndef NDEBUG
#define ENABLE_GLOBMATCHER_UNIT_TEST
#endif
class GlobExpression;

namespace lvtk::ui
{
    /// @brief Matches strings against patterns with '*' or '?' wildcards.
    /// Call SetPattern to prepare the pattern for matching. Call Matches() to 
    /// find out if the pattern matches.
    /// Not threadsafe.
    class GlobMatcher
    {

    public:
        static constexpr uint64_t MAX_BACKTRACKING_ATTEMPTS = 10000; // see not at head of file.

        GlobMatcher();
        GlobMatcher(const std::string &pattern);

        void SetPattern(const std::string &pattern);
        bool Matches(const std::string &text);

    private:
        void PushRun(std::string &run);

        std::vector<std::shared_ptr<GlobExpression>> expressions;
    };

#ifdef ENABLE_GLOBMATCHER_UNIT_TEST

    extern void GlobMatcherTest();
#endif

} // namespace

/*////////////////////////////////////////////////////////////////////////////////////////////
The issue: Worst-case performance for the pattern matcher is N^P (n = target length, P = number of
* patterns), when faced with patterns that require backtracking to match. The simplest vulnerability
is:

   *?*?*?*?*?*?*?*?*?*?*?*X

which is solvable, but variations of `*[!X]*[!Y*[!Z]*` are more difficult to deal with.

The current implementation limits the maimum number of backtracking operations
and throws an exception if the limit is exeeded. CPU use for a pathological
case: < 1 ms.

However, it's not absolutely clear that this fully addresses the problem. 10,000 backtracks
x 1,000 *'s applied to five hundred targets? ....

The fix: remove [] functionality, and merge *?*?*?*?* nodes to a new "at least 4 characters"
node, which would produce O 2^N execution time! (So also limit the maximum complexity of patterns).

Perfectly fine for local use. If a user wants to construct a pathological pattern, so be it. There
are easier ways to pin a CPU.

///////////////////////////////////////////////////////////////////////////////////////////*/
