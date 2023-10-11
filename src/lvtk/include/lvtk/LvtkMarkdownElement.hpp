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
#pragma once

#include "LvtkVerticalStackElement.hpp"
#include "LvtkTypographyElement.hpp"
#include <filesystem>
#include <iostream>

namespace lvtk {
    class LvtkMarkdownElement: public LvtkVerticalStackElement {
    public:
        using self=LvtkMarkdownElement;
        using super=LvtkContainerElement;
        using ptr=std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkMarkdownElement();
        LvtkMarkdownElement &TextVariant(LvtkTypographyVariant textVariant);
        LvtkTypographyVariant TextVariant() const;

        void SetMarkdown(const std::string &text);
        void SetMarkdown(std::istream&s);
        void AddMarkdownFile(const std::filesystem::path &path);

        void AddMarkdownLine(const std::string &text);
        void FlushMarkdown();
    private:
        enum class MarkdownVariant {
            H1,
            H2,
            H3,
            H4,
            Paragraph
        };
        struct HangingIndentState {
            double indentMargin;
            size_t indentCharacters;
        };

        std::vector<HangingIndentState> hangingIndentStack;

        MarkdownVariant markdownVariant = MarkdownVariant::Paragraph;

        LvtkTypographyVariant textVariant = LvtkTypographyVariant::BodyPrimary;
        std::string lineBuffer;
        std::string hangingText;
        double leftMargin = 0;
        size_t hangingIndentChars = 0;

        LvtkMeasurement titleSize = LvtkMeasurement::Point(14);
        LvtkMeasurement headingSize = LvtkMeasurement::Point(12);

        bool lineBreak = false;
    };
}