// Copyright (c) 2023 Robin E. R. Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once
#include "LvtkElement.hpp"
#include <set>
#include <string>
#include "pango/pangocairo.h"
#include "LvtkBindingProperty.hpp"
#include "LvtkButtonBaseElement.hpp"

typedef struct _PangoLayout PangoLayout;
typedef struct _PangoFontDescriptor PangoFontDescriptor;

namespace lvtk
{

    class LvtkPangoContext;

    

    enum class EditBoxVariant {
        Underline,
        Frame,
    };

    class LvtkEditBoxElement : public LvtkButtonBaseElement
    {
    public:
        virtual const char* Tag() const override { return "EditBox";}
        using self = LvtkEditBoxElement;
        using super=LvtkButtonBaseElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        using text_property_t = LvtkBindingProperty<std::string>;


        LvtkEditBoxElement();
        virtual ~LvtkEditBoxElement() override;

        LvtkEditBoxElement &ShowError(bool value);
        bool ShowError() const;

        LvtkEditBoxElement& Variant(EditBoxVariant variant);
        EditBoxVariant Variant() const;

        struct SelectionRange {
            SelectionRange();
            SelectionRange(size_t position);
            SelectionRange(size_t start, size_t end);

            bool HasMarkup() { return start != end; }

            bool operator==(const SelectionRange &other) const;

            size_t start;
            size_t end;
        };

        LvtkBindingProperty<std::string> TextProperty;
        LvtkEditBoxElement &Text(const std::string &text);
        const std::string &Text() const;


        void SelectAll();
        void SelectEnd();

        LvtkEditBoxElement &SingleLine(bool value);
        bool SingleLine();

        virtual bool WillDraw() const override;

        SelectionRange Selection() const;
        LvtkEditBoxElement& Selection(SelectionRange selection);

    protected:
        virtual bool WantsFocus() const override;
        virtual bool OnMouseDown(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseMove(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseUp(LvtkMouseEventArgs &event) override;
        virtual bool OnFocus(const LvtkFocusEventArgs &eventArgs) override;
        virtual bool OnLostFocus(const LvtkFocusEventArgs &eventArgs) override;

        virtual bool OnKeyDown(const LvtkKeyboardEventArgs&event) override;

        virtual void OnTextChanged(const std::string&text);
        virtual void OnSelectionChanged(const SelectionRange&text);

        virtual LvtkSize MeasureClient(LvtkSize constraint, LvtkSize maxAvailable,LvtkDrawingContext &context) override;
        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context) override;

        virtual void OnDraw(LvtkDrawingContext &dc) override;

        virtual void OnMount() override;
        virtual void OnUnmount() override;

    private:
        void AppendUnicodeEntryText();
        bool UnicodeEntryKeyDown(const LvtkKeyboardEventArgs &event);
        LvtkEditBoxElement&UnicodeEntry(bool value);
        bool UnicodeEntry() const;
        void SetUndo(const std::string &text, SelectionRange selection);
        bool Undo();
        bool InsertText(const std::string &text);
        bool HandleKeysym(ModifierState modifiers,unsigned int keyCode);

        void UpdateErrorStyle();
    private:
        size_t GetCharacterFromPoint(LvtkPoint point);

        size_t CursorPrevious(size_t position);
        size_t CursorNext(size_t position);
        std::string selectionMarkup(const std::string& text);
        void UpdateSelectionRects();
        void GetCursorRects(int cursorPosition, LvtkRectangle *strongPos, LvtkRectangle*weakPos);
        LvtkColor textSelectionColor, textCursorColor;
        std::string textSelectionWebColor;
        LvtkRectangle cursorRect;
        double fontHeight;

        bool cursorBlink = true;
        int cursorBlinkHold = 0;
        AnimationHandle blinkTimerHandle;
        void OnBlinkTimer();
        void StartBlinkTimer();
        void StopBlinkTimer();

        LvtkColor primaryTextColor,secondaryTextColor;

        LvtkSize clientMeasure;
        LvtkStyle::ptr GetVariantStyle();
        SelectionRange selection;
        PangoFontDescription*GetFontDescription();

        PangoLayout *pangoLayout = nullptr;
        PangoFontDescriptor *fontDescriptor = nullptr;
        std::string GetFontFamily();

        bool singleLine = true;

        EditBoxVariant variant = EditBoxVariant::Underline;
        LvtkStyle::ptr variantStyle;
        LvtkStyle::ptr errorStyle;

        observer_handle_t textObserverHandle;

        std::string markupText;
        bool showError = false;
        bool hasErrorStyle = false;


        bool hasUndo = false;
        std::string undoText;
        SelectionRange undoSelection;

        bool textChanged = false;
        bool markupChanged = false;
        bool selectionChanged = false;
        double scrollOffset = 0;
        bool unicodeEntry = false;
        std::string unicodeEntryString;
        size_t mouseStartPosition;
    };

} // namespace