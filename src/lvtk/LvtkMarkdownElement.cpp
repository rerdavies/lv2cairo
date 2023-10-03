/*
Copyright (c) 2023 Robin E. R. Davies

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "lvtk/LvtkMarkdownElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include <sstream>
#include <fstream>

using namespace lvtk;

namespace lvtk::implementation
{
    class MarkdownTypographyElement : public LvtkTypographyElement
    {
    public:
        using self = MarkdownTypographyElement;
        using super = LvtkTypographyElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

    protected:
        virtual void OnMount() override
        {
            super::OnMount();
            Style().BorderColor(Theme().dividerColor);
        }
    };
    class MarkdownRuleElement : public LvtkElement
    {
    public:
        using self = MarkdownRuleElement;
        using super = LvtkElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        MarkdownRuleElement()
        {
            Style()
                .Height(1)
                .HorizontalAlignment(LvtkAlignment::Stretch);
        }

    protected:
        virtual void OnMount() override
        {
            super::OnMount();
            Style().Background(Theme().dividerColor);
        }
    };
}

using namespace lvtk::implementation;

LvtkMarkdownElement::LvtkMarkdownElement()
{
    Style().HorizontalAlignment(LvtkAlignment::Stretch);
}
void LvtkMarkdownElement::SetMarkdown(const std::string &text)
{
    std::stringstream s(text);
    SetMarkdown(s);
}
void LvtkMarkdownElement::SetMarkdown(std::istream &s)
{
    std::string line;
    while (s)
    {
        std::getline(s, line);
        AddMarkdownLine(line);
    }
    FlushMarkdown();
}
void LvtkMarkdownElement::FlushMarkdown()
{
    if (lineBuffer.length() == 0)
        return;

    auto element = MarkdownTypographyElement::Create();
    element->Variant(textVariant)
        .Text(lineBuffer);
    element->Style()
        .SingleLine(false);
    lineBuffer.resize(0);

    if (lineBreak)
    {
        lineBreak = false;
        element->Style()
            .MarginBottom(0)
            .PaddingBottom(8);
    }
    if (hangingText.length() != 0)
    {
        constexpr double HANGING_INDENT_WIDTH = 32;
        auto hangingElement = LvtkTypographyElement::Create();

        hangingElement->Variant(textVariant).Text(hangingText);
        hangingElement->Style()
            .SingleLine(false)
            .Width(HANGING_INDENT_WIDTH);
        auto grid = LvtkFlexGridElement::Create();
        grid->Style()
            .FlexDirection(LvtkFlexDirection::Row)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .FlexAlignItems(LvtkAlignment::Start)
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .MarginLeft(leftMargin)
            .MarginBottom(8);
        grid->AddChild(hangingElement);
        grid->AddChild(element);
        this->leftMargin += HANGING_INDENT_WIDTH;
    }
    else
    {
        element->Style().MarginLeft(leftMargin).MarginBottom(8);
        AddChild(element);
    }
}

static bool StripHangingIndent(const std::string &line, std::string &hangingText, std::string &lineOut, size_t &hangingIndentSpaces)
{
    hangingIndentSpaces = 0;
    size_t hangingIndentPosition = std::string::npos;
    if (line.starts_with("- "))
    {
        hangingIndentPosition = 1;
    }
    else
    {
        size_t i = 0;
        while (i < line.size() && ((line[i] >= '0' && line[i] <= '9') || line[i] == '.'))
        {
            ++i;
        }
        if (i != 0)
        {
            if (line[i - 1] == '.' && i < line.length() && line[i] == ' ')
            {
                hangingIndentPosition = i;
            }
        }
        if (hangingIndentPosition == std::string::npos)
        {
            size_t i = 0;
            while (i < line.size() && ((line[i] >= 'a' && line[i] <= 'z')))
            {
                ++i;
            }
            if (i != 0)
            {
                if (i + 2 < line.length())
                {
                    if (line[i] == ')' && line[i + 1] == ' ')
                    {
                        hangingIndentPosition = i + 1;
                    }
                    else if (line[i] == '.' && line[i + 1] == ' ')
                    {
                        hangingIndentPosition = i + 1;
                    }
                }
            }
        }
        if (hangingIndentPosition == std::string::npos)
        {
            return false;
        }
        hangingText = std::string(line.begin(), line.end() + i);
        while (i < line.size() && line[i] == ' ')
        {
            ++i;
        }
        lineOut = std::string(line.begin() + i, line.end());
        hangingIndentSpaces = i;
        return true;
    }
    return false;
}

void LvtkMarkdownElement::AddMarkdownLine(const std::string &text_)
{
    std::string text = text_;
    if (text.length() == 0)
    {
        FlushMarkdown();
        return;
    }
    if (text == "---")
    {
        FlushMarkdown();
        AddChild(MarkdownRuleElement::Create());
        return;
    }
    if (lineBuffer.length() == 0)
    {

        std::string lineOut;
        size_t hangingIndentSpaces;
        if (StripHangingIndent(text, this->hangingText, lineOut, hangingIndentSpaces))
        {
            lineBuffer = lineOut;
            hangingIndentStack.push_back({this->leftMargin, this->hangingIndentChars});
            this->hangingIndentChars = hangingIndentSpaces;
            return;
        }
    }
    else
    {

        while (hangingIndentStack.size() != 0)
        {
            bool hasIndent = true;
            for (size_t i = 0; i < hangingIndentChars; ++i)
            {
                if (i >= text.length() || text[i] != ' ')
                {
                    hasIndent = false;
                    break;
                }
            }
            if (hasIndent)
            {
                text = std::string(text.begin() + hangingIndentChars, text.end());
                break;
            }
            FlushMarkdown();
        }
    }
    size_t ws = 0;
    while (ws < text.length() && text[ws] == ' ')
        ++ws;
    if (ws != 0)
    {
        text = std::string(text.begin() + ws, text.end());
    }

    if (text.ends_with("  "))
    {
        std::string t = std::string(text.begin(), text.end() - 2);
        lineBreak = true;
        lineBuffer += t;
        FlushMarkdown();
        return;
    }
    if (lineBuffer.length() != 0 && !lineBuffer.ends_with(' '))
    {
        lineBuffer += ' ';
    }
    lineBuffer += text;
}

LvtkMarkdownElement &LvtkMarkdownElement::TextVariant(LvtkTypographyVariant textVariant)
{
    this->textVariant = textVariant;
    return *this;
}
LvtkTypographyVariant LvtkMarkdownElement::TextVariant() const
{
    return this->textVariant;
}

void LvtkMarkdownElement::AddMarkdownFile(const std::filesystem::path &path)
{
    std::ifstream f;
    f.open(path);
    std::string line;
    while (f)
    {
        getline(f, line);
        AddMarkdownLine(line);
    }
    FlushMarkdown();
}
