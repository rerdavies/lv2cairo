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

#pragma GCC diagnostic ignored "-Wrestrict" // GCC 12 bug.

#include "lv2c/Lv2cMarkdownElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include <sstream>
#include <fstream>

using namespace lv2c;

static std::string monoFontFamily = "Lucida Console,Consolas,Liberation Mono,Monaco,Courier,monospace";

namespace lv2c::implementation
{
    class MarkdownTypographyElement : public Lv2cTypographyElement
    {
    public:
        using self = MarkdownTypographyElement;
        using super = Lv2cTypographyElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

    protected:
        virtual void OnMount() override
        {
            super::OnMount();
            Style().BorderColor(Theme().dividerColor);
        }
    };
    class MarkdownRuleElement : public Lv2cElement
    {
    public:
        using self = MarkdownRuleElement;
        using super = Lv2cElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        MarkdownRuleElement()
        {
            Style()
                .Height(1)
                .MarginTop(16)
                .MarginBottom(16)
                .HorizontalAlignment(Lv2cAlignment::Stretch);
        }

    protected:
        virtual void OnMount() override
        {
            super::OnMount();
            Style().Background(Theme().dividerColor);
        }
    };
}

using namespace lv2c::implementation;

Lv2cMarkdownElement::Lv2cMarkdownElement()
{
    Style().HorizontalAlignment(Lv2cAlignment::Stretch);
}
void Lv2cMarkdownElement::SetMarkdown(const std::string &text)
{
    std::stringstream s(text);
    SetMarkdown(s);
}
void Lv2cMarkdownElement::SetMarkdown(std::istream &s)
{
    std::string line;
    while (s && !s.eof())
    {
        std::getline(s, line);
        AddMarkdownLine(line);
    }
    FlushMarkdown();
}
void Lv2cMarkdownElement::FlushMarkdown()
{
    if (lineBuffer.length() == 0)
        return;

    auto element = MarkdownTypographyElement::Create();
    element->Variant(textVariant);

    element->Text(lineBuffer);
    
    element->Style()
        .SingleLine(false);
    lineBuffer.resize(0);

    if (hangingText.length() != 0)
    {
        constexpr double HANGING_INDENT_WIDTH = 18;
        auto hangingElement = Lv2cTypographyElement::Create();

        hangingElement->Variant(textVariant).Text(hangingText);
        hangingElement->Style()
            .SingleLine(false)
            .Width(HANGING_INDENT_WIDTH);
        auto grid = Lv2cFlexGridElement::Create();
        grid->Style()
            .FlexDirection(Lv2cFlexDirection::Row)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .FlexAlignItems(Lv2cAlignment::Start)
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .MarginLeft(leftMargin)
            .MarginBottom(16);
        grid->AddChild(hangingElement);
        grid->AddChild(element);
        this->leftMargin += HANGING_INDENT_WIDTH;
        AddChild(grid);
        this->hangingText = "";
    }
    else
    {
        element->Style().MarginLeft(leftMargin).MarginBottom(12);
        if (lineBreak)
        {
            element->Style()
                .MarginBottom(0)
                .PaddingBottom(4);
        } 
        if (markdownVariant != MarkdownVariant::Paragraph)
        {
            element->Style()
                .MarginTop(24)
                ;
            switch (markdownVariant)
            {
                case MarkdownVariant::H1:
                    element->Style()
                        
                        .FontWeight(Lv2cFontWeight::Bold)
                        .FontSize(titleSize)
                        .MarginBottom(16)
                        ;
                    break;
                case MarkdownVariant::H2:
                    element->Style()
                        .FontWeight(Lv2cFontWeight::Bold)
                        .FontSize(headingSize)
                        .MarginBottom(16)
                        ;
                    break;
                case MarkdownVariant::H3:
                    element->Style()
                        .FontWeight(Lv2cFontWeight::Bold)
                        .MarginBottom(16)
                        ;
                    break;
                case MarkdownVariant::H4:
                    element->Style()
                        .FontWeight(Lv2cFontWeight::Bold)
                        .MarginBottom(16)
                        ;
                    break;
                default:
                    break;
            }
        }
        
        if (!lineBreak) {
            markdownVariant = MarkdownVariant::Paragraph;
        }

        AddChild(element);
    }
    lineBreak = false;
}

static bool GetHangingIndentText(const std::string &line, std::string &hangingText, std::string &lineOut, size_t &hangingIndentSpaces)
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
    }
    if (hangingIndentPosition == std::string::npos)
    {
        return false;
    }
    hangingText = std::string(line.begin(), line.begin() + hangingIndentPosition);
    if (hangingText == "-")
    {
        hangingText = "•";
    }
    size_t i = hangingIndentPosition;
    while (i < line.size() && line[i] == ' ')
    {
        ++i;
    }
    lineOut = std::string(line.begin() + i, line.end());
    hangingIndentSpaces = i;
    return true;
}

static bool IsRule(const std::string&text)
{
    if (!text.starts_with("---")) return false;

    for (auto i = text.begin()+3; i != text.end(); ++i)
    {
        if (*i != '-') return false;
    }
    return true;
}

static bool GetTitleChars(const std::string&text, int &titleChars,std::string&lineOut)
{
    titleChars = 0;
    if (!text.starts_with("#")) return false;

    auto i = text.begin();
    while (i != text.end() && *i == '#')
    {
        ++i;
        ++titleChars;
    }
    while (i != text.end() && *i == ' ')
    {
        ++i;
    }
    lineOut = std::string(i,text.end());
    return true;
}
void Lv2cMarkdownElement::AddMarkdownLine(const std::string &text_)
{
    std::string text = text_;
    if (this->markdownVariant == MarkdownVariant::PreFormated)
    {
        if (text == "```")
        {
            FlushMarkdown();
            this->markdownVariant = MarkdownVariant::Paragraph;
            // fall through to add one more (empty) line.
            text = "";
        }
        auto element = Lv2cTypographyElement::Create();
        element->Variant(TextVariant());
        element->Text(text);
        element->Style()
            .SingleLine(true)
            .MarginLeft(32)
            .FontFamily(monoFontFamily);
            ;
        this->AddChild(element);
        return;
    }
    if (text.length() == 0)
    {
        FlushMarkdown();
        return;
    }
    if (text == "```")
    {
        FlushMarkdown();
        this->markdownVariant = MarkdownVariant::PreFormated;
        return;
    }
    if (IsRule(text))
    {
        FlushMarkdown();
        AddChild(MarkdownRuleElement::Create());
        return;
    }
    if (lineBuffer.length() == 0)
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
            auto &stackTop = hangingIndentStack[hangingIndentStack.size() - 1];
            this->hangingIndentChars = stackTop.indentCharacters;
            this->leftMargin = stackTop.indentMargin;
            hangingIndentStack.pop_back();
            FlushMarkdown();
        }

        std::string lineOut;
        size_t hangingIndentSpaces;
        int titleChars;
        if (GetTitleChars(text,titleChars, lineOut))
        {
            switch (titleChars)
            {
            case 3:
                markdownVariant = MarkdownVariant::H1;
                break;
            case 4:
                markdownVariant = MarkdownVariant::H2;
                break;
            case 5:
                markdownVariant = MarkdownVariant::H3;
            case 6:
            default:
                markdownVariant = MarkdownVariant::H4;
                break;
            }
            text = lineOut;
        } else if (GetHangingIndentText(text, this->hangingText, lineOut, hangingIndentSpaces))
        {
            lineBuffer = lineOut;
            hangingIndentStack.push_back({this->leftMargin, this->hangingIndentChars});
            this->hangingIndentChars = hangingIndentSpaces;
            return;
        }
    }

    if (text.ends_with("  "))
    {
        std::string t = std::string(text.begin(), text.end() - 2);
        lineBreak = true;
        if (t.length() == 0 && lineBuffer.length() == 0)
        {
            lineBuffer = " ";
        }
        lineBuffer += t;
        FlushMarkdown();
        return;
    }
    size_t ws = 0;
    while (ws < text.length() && text[ws] == ' ')
        ++ws;
    if (ws != 0)
    {
        text = std::string(text.begin() + ws, text.end());
    }
    if (lineBuffer.length() != 0 && !lineBuffer.ends_with(' '))
    {
        lineBuffer += ' ';
    }
    lineBuffer += text;
}

Lv2cMarkdownElement &Lv2cMarkdownElement::TextVariant(Lv2cTypographyVariant textVariant)
{
    this->textVariant = textVariant;
    this->markdownVariant = MarkdownVariant::Paragraph;
    return *this;
}
Lv2cTypographyVariant Lv2cMarkdownElement::TextVariant() const
{
    return this->textVariant;
}

void Lv2cMarkdownElement::AddMarkdownFile(const std::filesystem::path &path)
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
