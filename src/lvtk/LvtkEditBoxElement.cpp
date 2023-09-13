// Copyright (c) 2023 Robin Davies
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

#include "lvtk/LvtkEditBoxElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "lvtk/LvtkLog.hpp"
#include "lvtk/LvtkPangoContext.hpp"

#include "pango/pangocairo.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cmath>
#include "lvtk/LvtkPangoContext.hpp"
#include "Utf8Utils.hpp"

#define XK_MISCELLANY
#define XK_LATIN1
#include "X11/keysymdef.h"
#include <codecvt>

using namespace lvtk;

using namespace lvtk;

using SelectionRange = LvtkEditBoxElement::SelectionRange;

bool LvtkEditBoxElement::WillDraw() const
{
    return this->Text().length() != 0 || super::WillDraw();
}

void LvtkEditBoxElement::OnTextChanged(const std::string &text)
{
    this->textChanged = true;
    this->markupChanged = true;
    Invalidate();
}
void LvtkEditBoxElement::OnSelectionChanged(const SelectionRange&selection)
{
    Invalidate();
}

LvtkEditBoxElement &LvtkEditBoxElement::Text(const std::string &text)
{
    this->TextProperty.set(text);

    return *this;
}

static std::vector<std::string> splitFamilies(const std::string &text, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream s(text);
    std::string line;
    while (std::getline(s, line, delimiter))
    {
        size_t start = 0;
        while (start < line.length() && line[start] == ' ')
            ++start;
        size_t end;
        for (end = line.length(); end > start && line[end - 1] == ' '; --end)
        {
        }
        std::string t = line.substr(start, end - start);
        if (t.length() != 0)
        {
            result.push_back(t);
        }
    }
    return result;
}
const std::string &LvtkEditBoxElement::Text()  const { return TextProperty.get(); }

std::string LvtkEditBoxElement::GetFontFamily()
{
    std::string fontFamily;
    for (LvtkElement *element = this; element != nullptr; element = element->Parent())
    {
        fontFamily = element->Style().FontFamily();
        if (fontFamily.length() != 0)
        {
            break;
        }
    }

    std::vector<std::string> familyNames = splitFamilies(fontFamily, ',');

    const std::set<std::string> &installedFamilies = gPangoContext.FontFamilies();
    std::string result;
    for (const std::string &familyName : familyNames)
    {
        if (installedFamilies.contains(familyName))
        {
            result = familyName;
            break;
        }
    }
    if (result.length() == 0)
    {
        result = "Serif";
    }
    return result;
}

LvtkEditBoxElement::LvtkEditBoxElement()
{
    this->Style().HorizontalAlignment(LvtkAlignment::Start);
    TextProperty.SetElement(this,&LvtkEditBoxElement::OnTextChanged);
}

LvtkEditBoxElement::~LvtkEditBoxElement()
{
    if (pangoLayout)
    {
        g_object_unref(pangoLayout);
    }
}

LvtkSize LvtkEditBoxElement::MeasureClient(LvtkSize constraint, LvtkSize available, LvtkDrawingContext &context)
{
    if (pangoLayout == nullptr)
    {

        // pangoLayout = pango_cairo_create_layout(context.get());
        pangoLayout = pango_layout_new(GetPangoContext());
        std::string text = selectionMarkup(Text());
    }
    PangoFontDescription *desc = GetFontDescription();
    pango_layout_set_font_description(pangoLayout, desc);

    auto pangoContext = this->Window()->GetPangoContext();
    PangoFontMetrics *metrics = pango_context_get_metrics(pangoContext,desc,nullptr);
    double metricHeight = (pango_font_metrics_get_ascent(metrics) + pango_font_metrics_get_descent(metrics))/PANGO_SCALE;
    (void)metricHeight;

    pango_font_description_free(desc);


    double height = constraint.Height();
    if (height == 0)
    {
        // measure text so we can calcuate the extent for the current typeface.

        if (singleLine)
        {
            // prepare layout so we can calcuate our height.
            pango_layout_set_width(pangoLayout, -1);
            pango_layout_set_height(pangoLayout, -1);
            pango_layout_set_ellipsize(pangoLayout, PangoEllipsizeMode::PANGO_ELLIPSIZE_NONE);
            pango_layout_set_alignment(pangoLayout, PangoAlignment::PANGO_ALIGN_LEFT);
        }
        else
        {
            throw std::runtime_error("Not implemented.");
            // pango_layout_set_ellipsize(pangoLayout,PangoEllipsizeMode::PANGO_ELLIPSIZE_NONE);
            // pango_layout_set_width(pangoLayout, ((int)std::floor(constraint.getWidth())) * PANGO_SCALE);
            // pango_layout_set_height(pangoLayout, -5000); // max 5000 lines. That should be enough/
        }

        pango_layout_set_text(pangoLayout, "X",1);

        pango_cairo_update_layout(context.get(), pangoLayout);

        int x, y;
        pango_layout_get_size(pangoLayout, &x, &y);
        height = y / PANGO_SCALE;
        this->fontHeight = height;
        //std::cout << "Pango height: " << height << std::endl;



    }

    double width = constraint.Width();
    if (width == 0)
    {
        width = 120; // default width if it hasn't been set.
    }
    if (Style().HorizontalAlignment() == LvtkAlignment::Stretch && available.Width() != 0)
    {
        width = available.Width();
    }
    if (Style().VerticalAlignment() == LvtkAlignment::Stretch && available.Height() != 0)
    {
        height = available.Height();
    }
    if (constraint.Width() != 0)
    {
        width = constraint.Width();
    }
    //std::cout << "EditBox height: " << height << std::endl;
    return LvtkSize(width, height);
}
LvtkSize LvtkEditBoxElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{
    return available;
}

void LvtkEditBoxElement::OnDraw(LvtkDrawingContext &dc)
{
    super::OnDraw(dc);

    markupText = selectionMarkup(Text());
    pango_layout_set_markup(pangoLayout,markupText.c_str(),markupText.length());
    
    if (this->selectionChanged || this->textChanged)
    {
        this->UpdateSelectionRects();
        cursorBlink = true;
        cursorBlinkHold = 1;
    }
    pango_cairo_update_layout(dc.get(), pangoLayout);

    LvtkSize size = this->ClientSize();
    
    PangoRectangle pangoRect;
    pango_layout_get_extents(pangoLayout,nullptr,&pangoRect);
    double measuredWidth = std::ceil(pangoRect.width/PANGO_SCALE);
    if (measuredWidth > size.Width())
    {
        double maxOffset = measuredWidth-size.Width()+1;
        if (this->cursorRect.Right()-scrollOffset > size.Width()*0.9)
        {
            this->scrollOffset = std::ceil(this->cursorRect.Right()-size.Width()*0.9);
            if (scrollOffset > maxOffset)
            {
                this->scrollOffset = maxOffset;
            }
        } 
        if (this->cursorRect.Left()-scrollOffset < size.Width()*0.1)
        {
            this->scrollOffset = std::floor(this->cursorRect.Left()-size.Width()*0.1);
            if (this->scrollOffset < 0) this->scrollOffset = 0;
        }
        if (scrollOffset > maxOffset)
        {
            this->scrollOffset = maxOffset;
        }

    } else {
        scrollOffset = 0;
        auto textAlign = Style().TextAlign();
        if (textAlign == LvtkTextAlign::End)
        {
            scrollOffset = std::floor(-(size.Width()-measuredWidth));
        }
        if (textAlign == LvtkTextAlign::Center)
        {
            scrollOffset = std::floor(-(size.Width()-measuredWidth)/2);
        }
    } 




    this->selectionChanged = false;
    this->textChanged = false;
    this->markupChanged = false;

    auto source = Style().Color();
    if (source)
    {
        cairo_save(dc.get());
        dc.set_source(source);

        LvtkPoint pt {-scrollOffset,0};
        pt = dc.round_to_device(pt);
        dc.move_to(-scrollOffset, 0);
        pango_cairo_show_layout(dc.get(), pangoLayout);

        if (cursorBlink && Focused())
        {
            dc.set_source(textCursorColor);
            dc.rectangle(cursorRect.translate({-scrollOffset,0}));
            dc.fill();
        }

        cairo_restore(dc.get());
    }
}

LvtkEditBoxElement &LvtkEditBoxElement::SingleLine(bool value)
{
    if (singleLine != value)
    {
        singleLine = value;
        Invalidate();
        InvalidateLayout();
    }
    return *this;
}
bool LvtkEditBoxElement::SingleLine()
{
    return singleLine;
}

LvtkStyle::ptr LvtkEditBoxElement::GetVariantStyle()
{
    if (!this->IsMounted())
    {
        return nullptr;
    }
    const auto &theme = this->Theme();

    switch (variant)
    {
    default:
    case EditBoxVariant::Underline:
        return theme.editBoxUnderlineStyle;
    case EditBoxVariant::Frame:
        return theme.editBoxFrameStyle;
    }
}

LvtkEditBoxElement &LvtkEditBoxElement::Variant(EditBoxVariant variant)
{
    if (this->variant != variant)
    {
        this->variant = variant;

        RemoveClass(errorStyle);
        RemoveClass(variantStyle);
        variantStyle = GetVariantStyle();
        AddClass(variantStyle);
        UpdateErrorStyle();
        InvalidateLayout();
    }
    return *this;
}
EditBoxVariant LvtkEditBoxElement::Variant() const
{
    return variant;
}

void LvtkEditBoxElement::OnUnmount()
{
    UnicodeEntry(false);
    StopBlinkTimer();
    super::OnUnmount();
}
void LvtkEditBoxElement::OnMount()
{
    super::OnMount();
    UnicodeEntry(false);
    RemoveClass(errorStyle);
    auto& theme = this->Theme(); 
    errorStyle = theme.editBoxErrorStyle;
    
    RemoveClass(variantStyle);
    variantStyle = GetVariantStyle();
    this->Classes(variantStyle);
    UpdateErrorStyle();

    primaryTextColor = theme.primaryTextColor;
    secondaryTextColor = theme.secondaryTextColor;

    textSelectionColor = theme.textSelectionColor;
    textCursorColor = theme.textCursorColor;
    textSelectionWebColor = textSelectionColor.toPangoString();

    this->Style().Color(secondaryTextColor);

}

PangoFontDescription *LvtkEditBoxElement::GetFontDescription()
{
    std::string fontFamily = GetFontFamily();

    PangoFontDescription *desc = pango_font_description_new();

    pango_font_description_set_family(desc, fontFamily.c_str());
    double fontSize = this->Style().FontSize().PixelValue();
    if (fontSize == 0)
    {
        fontSize = 12;
    }
    pango_font_description_set_size(
        desc,
        (gint)(fontSize * 72.0 / 96 * PANGO_SCALE));

    LvtkStyle &style = Style();
    auto variant = style.FontVariant();
    if (variant.has_value())
    {
        // cast enum class LvtkFontVariant to enum PangoVariant!
        PangoVariant pangoVariant = (PangoVariant)(int)(variant.value());
        pango_font_description_set_variant(desc, pangoVariant);
    }
    auto vWeight = style.FontWeight();
    if (vWeight.has_value())
    {
        // cast enum class LvtkFontWeight to enum PangoWeight!
        PangoWeight pangoWeight = (PangoWeight)(int)(vWeight.value());
        pango_font_description_set_weight(desc, pangoWeight);
    }
    auto vStyle = style.FontStyle();
    if (vStyle.has_value())
    {
        // cast enum class LvtkFontStyle to enum PangoStyle.
        PangoStyle pangoStyle = (PangoStyle)(int)(vStyle.value());
        pango_font_description_set_style(desc, pangoStyle);
    }
    auto vStretch = style.FontStretch();
    if (vStretch.has_value())
    {
        // cast enum class LvtkFontStretch to enum PangoStretch.
        PangoStretch pangoStretch = (PangoStretch)(int)(vStretch.value());
        pango_font_description_set_stretch(desc, pangoStretch);
    }
    return desc;
}

size_t LvtkEditBoxElement::GetCharacterFromPoint(LvtkPoint point)
{
    if (pangoLayout == nullptr) return 0;
    // I'm sure there's a better way to do this, but this works pretty well.
    // enumerate through each cursor position and pick the closts to the mouse.
    size_t bestPosition = 0;


    double bestDistance = std::numeric_limits<double>::max();

    const std::string&text = Text();
    double pangoX = (point.x+this->scrollOffset)*PANGO_SCALE;
    for (size_t position = 0; /***/; position = Utf8Increment(position,text)) {
        PangoRectangle pstrongPos, pweakPos;
        pango_layout_get_cursor_pos(pangoLayout, position, &pstrongPos, &pweakPos);

        double error = std::abs(pstrongPos.x-pangoX);
        if (error < bestDistance)
        {
            bestPosition = position;
            bestDistance = error;
        }
        if (position >= text.size()) 
        {
            break;
        }
    }
    return bestPosition;
}

bool LvtkEditBoxElement::OnMouseDown(LvtkMouseEventArgs &event)
{
    if (!this->Focused())
    {
        Focus();
    } else {
        size_t position = GetCharacterFromPoint(event.point);
        this->mouseStartPosition = position;
        Selection({position});
        this->CaptureMouse();
    }
    return true;
}
bool LvtkEditBoxElement::OnMouseMove(LvtkMouseEventArgs &event)
{
    if (this->Capture() == this)
    {
        size_t position = GetCharacterFromPoint(event.point);
        Selection({mouseStartPosition,position});
        return true;
    }
    return false;
}

bool LvtkEditBoxElement::OnMouseUp(LvtkMouseEventArgs &event)
{
    this->ReleaseCapture();
    return true;
}

bool LvtkEditBoxElement::OnFocus(const LvtkFocusEventArgs &eventArgs)
{
    super::OnFocus(eventArgs);
    StartBlinkTimer();
    this->Style().Color(primaryTextColor);
    Invalidate();
    this->markupChanged = true;
    this->selectionChanged = true;
    return false;
}
bool LvtkEditBoxElement::OnLostFocus(const LvtkFocusEventArgs &eventArgs)
{
    StopBlinkTimer();
    super::OnLostFocus(eventArgs);
    UnicodeEntry(false);
    this->Style().Color(secondaryTextColor);

    this->markupChanged = true;
    this->selectionChanged = true;

    Invalidate();
    return false;
}

bool LvtkEditBoxElement::SelectionRange::operator==(const SelectionRange &other) const
{
    return this->start == other.start && this->end == other.end;
}

LvtkEditBoxElement::SelectionRange::SelectionRange() : start(0), end(0)
{
}
LvtkEditBoxElement::SelectionRange::SelectionRange(size_t position) : start(position), end(position)
{
}
LvtkEditBoxElement::SelectionRange::SelectionRange(size_t start, size_t end)
    : start(start),
      end(end)
{
}

LvtkEditBoxElement::SelectionRange LvtkEditBoxElement::Selection() const
{
    return selection;
}
LvtkEditBoxElement &LvtkEditBoxElement::Selection(SelectionRange selection)
{
    if (this->selection != selection)
    {
        this->markupChanged |= this->selection.HasMarkup() || selection.HasMarkup();
        this->selectionChanged = true;
        this->selection = selection;
        OnSelectionChanged(selection);
    }
    return *this;
}

static LvtkRectangle ConvertPangoRect(PangoRectangle &pangoRect)
{
    double left = std::floor(pangoRect.x / PANGO_SCALE);
    // double right = std::ceil((pangoRect.x+pangoRect.width)/PANGO_SCALE);
    double top = std::floor(pangoRect.y / PANGO_SCALE);
    double bottom = std::ceil((pangoRect.y + pangoRect.height) / PANGO_SCALE);

    return LvtkRectangle(
        left, top, 1, bottom - top);
}
void LvtkEditBoxElement::GetCursorRects(int cursorPosition, LvtkRectangle *strongPos, LvtkRectangle *weakPos)
{
    PangoRectangle pstrongPos, pweakPos;
    pango_layout_get_cursor_pos(pangoLayout, cursorPosition, &pstrongPos, &pweakPos);

    *strongPos = ConvertPangoRect(pstrongPos);
    *weakPos = ConvertPangoRect(pweakPos);
}

std::string LvtkEditBoxElement::selectionMarkup(const std::string &text)
{
    std::stringstream input{text};
    std::stringstream output;

    size_t start = std::min(selection.start, selection.end);
    size_t end = std::max(selection.start, selection.end);

    if (UnicodeEntry())
    {
        start = end;
    } else {
        if (start == end || !this->Focused())
        {
            start = std::numeric_limits<size_t>::max();
            end = std::numeric_limits<size_t>::max();
        }
    }

    size_t i = 0;
    while (true)
    {
        if (i == start)
        {
            if (UnicodeEntry())
            {
                output << "<span underline='single'>u" << this->unicodeEntryString
                << "</span>"
                ;

            } else {
                output << "<span background='" << this->textSelectionWebColor << "'>";
            }
        }
        if (i == end)
        {
            if (!UnicodeEntry())
            {
                output << "</span>";
            }
        }
        auto c = input.get();
        if (c == EOF)
            break;

        if (c == '<')
        {
            output << "&#60;";
        }
        else if (c == '&')
        {
            output << "&#38;";
        }
        else
        {
            output << (char)c;
        }
        ++i;
    }

    return output.str();
}
void LvtkEditBoxElement::UpdateSelectionRects()
{
    if (pangoLayout == nullptr)
        return;
    LvtkRectangle strongRect, weakRect;
    if (UnicodeEntry())
    {
        GetCursorRects(selection.end +1 + this->unicodeEntryString.length(), &strongRect, &weakRect);
    } else {
        GetCursorRects(selection.end, &strongRect, &weakRect);
    }
    if (strongRect.Height() < fontHeight)
    {
        strongRect.Height(fontHeight);
    }
    this->cursorRect = strongRect;
}

static constexpr uint32_t BLINK_TIME_MS = 530;
void LvtkEditBoxElement::OnBlinkTimer()
{
    if (cursorBlinkHold != 0)
    {
        --cursorBlinkHold;
    } else {
        this->cursorBlink = !this->cursorBlink;
        InvalidateClientRect(this->cursorRect);
    }
    this->blinkTimerHandle = this->Window()->PostDelayed(BLINK_TIME_MS, [this]()
                                                            { OnBlinkTimer(); });
}

void LvtkEditBoxElement::StartBlinkTimer()
{
    StopBlinkTimer();
    cursorBlink = true;
    cursorBlinkHold = 1;

    this->blinkTimerHandle = this->Window()->PostDelayed(BLINK_TIME_MS, [this]()
                                                            { OnBlinkTimer(); });
}
void LvtkEditBoxElement::StopBlinkTimer()
{
    if (blinkTimerHandle.isValid())
    {
        this->cursorBlink = false;
        this->Window()->CancelPostDelayed(this->blinkTimerHandle);
        this->blinkTimerHandle = AnimationHandle::InvalidHandle;
    }
}

bool LvtkEditBoxElement::InsertText(const std::string &insertText)
{
    if (insertText.length() == 0)
        return false;

    const std::string &text = Text();
    size_t ux0 = std::min(this->selection.start, this->selection.end);
    size_t ux1 = std::max(this->selection.start, this->selection.end);
    std::string result;
    result.reserve(ux0 + insertText.length() + text.length() - ux1);
    result.append(text.begin(), text.begin() + ux0);
    result.append(insertText.begin(), insertText.end());
    result.append(text.begin() + ux1, text.end());

    size_t cursor = ux0 + insertText.length();
    Text(result);
    Selection(SelectionRange(cursor, cursor));
    return true;
}

static std::string RemoveSelection(const std::string &text, SelectionRange selection)
{
    size_t ux0 = std::min(selection.start, selection.end);
    size_t ux1 = std::max(selection.start, selection.end);
    if (ux1 > text.length())
        ux1 = text.length();
    std::string result;
    result.reserve(ux0 + text.length() - ux1);
    result.append(text.begin(), text.begin() + ux0);
    result.append(text.begin() + ux1, text.end());
    return result;
}

size_t LvtkEditBoxElement::CursorPrevious(size_t position)
{
    if (position == 0) return 0;
    return Utf8Decrement(position,Text());
}

size_t LvtkEditBoxElement::CursorNext(size_t position)
{
    if (position >= Text().size()) return Text().size();
    return Utf8Increment(position,Text());
}
bool LvtkEditBoxElement::HandleKeysym(ModifierState modifiers, unsigned int keyCode)
{
    if (modifiers == ModifierState::Empty)
    {
        switch (keyCode)
        {
        case XK_BackSpace:
        {
            SelectionRange currentSelection = Selection();
            if (currentSelection.start != currentSelection.end)
            {
                std::string result = RemoveSelection(Text(), Selection());
                auto cursor = std::min(currentSelection.start, currentSelection.end);
                SetUndo(Text(),Selection());
                Text(result);
                Selection({cursor});
            }
            else
            {
                if (currentSelection.end == 0)
                    return true;
                auto cursor = CursorPrevious(currentSelection.end);
                std::string result = RemoveSelection(Text(), {cursor, currentSelection.end});

                SetUndo(Text(),Selection());
                Text(result);
                Selection({cursor});
            }
            return true;
        }
        case XK_Delete:
        case XK_KP_Delete:
        {
            SelectionRange currentSelection = Selection();
            if (currentSelection.start != currentSelection.end)
            {
                std::string result = RemoveSelection(Text(), Selection());
                auto cursor = std::min(currentSelection.start, currentSelection.end);

                SetUndo(Text(),Selection());
                Text(result);
                Selection({cursor});
            }
            else
            {
                if (currentSelection.end == Text().length())
                    return true;
                auto cursor = currentSelection.end;
                std::string result = RemoveSelection(Text(), {cursor, CursorNext(cursor)});

                SetUndo(Text(),Selection());
                Text(result);
                Selection({cursor});
            }
            return true;
        }
        case XK_Left:
        case XK_KP_Left:
        {
            SelectionRange currentSelection = Selection();
            size_t cursor = CursorPrevious(currentSelection.end);
            Selection({cursor});

            return true;
        }
        case XK_Right:
        case XK_KP_Right:
        {
            SelectionRange currentSelection = Selection();
            auto cursor = CursorNext(currentSelection.end);
            Selection({cursor});

            return true;
        }
        case XK_Begin:
        case XK_Home:
        case XK_KP_Begin:
        case XK_KP_Home:
        {
            Selection({0});
            return true;
        }
        case XK_End:
        case XK_KP_End:
        {
            Selection({Text().size()});
            return true;
        }
        case XK_Undo:
        case XK_Redo:
        {
            return Undo();
        }
        case XK_Insert:
        case XK_KP_Insert:
        {
            return true;
        }
        }
    } else if (modifiers == ModifierState::Shift)
    {
        switch (keyCode)
        {
            case XK_Left:
            case XK_KP_Left:
            {
                SelectionRange currentSelection = Selection();
                if (currentSelection.end > 0)
                {
                    Selection({currentSelection.start,CursorPrevious(currentSelection.end)});
                }
                return true;
            }
            case XK_Right:
            case XK_KP_Right:
            {
                SelectionRange currentSelection = Selection();
                if (currentSelection.end < Text().size())
                {
                    Selection({currentSelection.start,CursorNext(currentSelection.end)});
                }
                return true;
            }
            case XK_Begin:
            case XK_Home:
            case XK_KP_Begin:
            case XK_KP_Home:
            {
                SelectionRange currentSelection = Selection();
                Selection({currentSelection.start,0});
                return true;
            }
        case XK_End:
        case XK_KP_End:
        {
            SelectionRange currentSelection = Selection();
            Selection({currentSelection.start,Text().length()});
            return true;
        }
        }
    } else if (modifiers == ModifierState::Control)
    {
        switch (keyCode)
        {

        case XK_z:
        case XK_y:
            return Undo();
        case XK_a:
        {
            Selection({0,Text().length()});
            return true;
        }
        }
    } else if (modifiers == (ModifierState::Control + ModifierState::Shift))
    {
        switch (keyCode)
        {
            case XK_U:
                UnicodeEntry(true);
                break;
        }
    }

    return false;
}

static bool isControlKey(const LvtkKeyboardEventArgs &event)
{
    const char *text = event.text;
    return event.textValid && event.keysymValid && 
        (
            (text[0] < 0x20 && text[0] >= 1) 
            || text[0] == 127 /*del*/
        )
        && text[1] == 0;
}


void LvtkEditBoxElement::AppendUnicodeEntryText()
{
    uint64_t value = 0;
    constexpr uint64_t REPLACEMENT_CHARACTER = 0xFFFDu;
    for (char c : unicodeEntryString)
    {
        uint64_t v;
        if (c >= '0' && c <= '9')
        {
            v = c-'0';
        } else if (c >= 'A' && c <= 'F')
        {
            v = c-'A'+10;
        } else if (c >= 'a' && c <= 'f')
        {
            v = c-'a'+10;
        } else {
            LogError("Invalid unicode character.");
            return;
        }
        value = value*16+v;
    }
    if (value < 0x20) return; // ascii control codes.
    if (value >= 0x80 && value < 0xA0) return; // latin1 control codes.
    if (value >= 0xFFF0 && value <= 0xFFFF) return; // specials.
    if (value > 0x10FFFFu)
    {
        value = REPLACEMENT_CHARACTER;
    } else if (value >= 0xD800u && value <= 0xDFFFu)
    {
        value = REPLACEMENT_CHARACTER;
    }
    std::string utfText = Utf8FromUtf32((char32_t)value);

    ShowError(false);
    auto previousText = this->Text();
    auto previousSelection = Selection();
    if (InsertText(utfText))
    {
        SetUndo(previousText,previousSelection);
    }
}

bool LvtkEditBoxElement::UnicodeEntryKeyDown(const LvtkKeyboardEventArgs &event)
{
    if (event.textValid && !isControlKey(event))
    {
        if (strlen(event.text) != 1) return true;
        char c = event.text[0];
        if (c >= '0' && c <= '9') 
        {
            // accept.
        } else if (c >= 'A' && c <= 'F')
        {
            // accept
        } else if (c >= 'a' && c <= 'f')
        {
            // accept
        } else {
            return true; // ignore it.
        }
        this->unicodeEntryString.append(1,c);
        this->markupChanged = true;
        this->selectionChanged = true;
        Invalidate();
        return true;
    } else if (event.keysymValid)
    {
        if (event.modifierState == ModifierState::Empty)
        {
            switch (event.keysym)
            {
            case XK_Return:
            case XK_KP_Enter:
            case XK_Escape:

                AppendUnicodeEntryText();
                UnicodeEntry(false);
                return true;
            case XK_BackSpace:
                if (unicodeEntryString.length() != 0)
                {
                    unicodeEntryString.resize(unicodeEntryString.length()-1);
                    this->markupChanged = true;
                    this->selectionChanged = true;
                    Invalidate();
                }
                return true;
            default:
                break;
            }
            return true;
        }
    }
    return false;
}

bool LvtkEditBoxElement::OnKeyDown(const LvtkKeyboardEventArgs &event)
{
    if (UnicodeEntry())
    {
        return UnicodeEntryKeyDown(event);
    }
    if (event.textValid && !isControlKey(event))
    {
        ShowError(false);
        auto previousText = this->Text();
        auto previousSelection = Selection();
        if (InsertText(event.text))
        {
            SetUndo(previousText,previousSelection);
        }
        return true;
    }
    else if (event.keysymValid)
    {
        return HandleKeysym(event.modifierState, event.keysym);
    }
    return false;
}

bool LvtkEditBoxElement::Undo()
{
    if (!hasUndo) return true;
    std::string currentText = Text();
    SelectionRange currentSelection = Selection();
    Text(undoText);
    Selection(undoSelection);
    undoText = currentText;
    undoSelection = currentSelection;
    return true;
}

void LvtkEditBoxElement::SetUndo(const std::string &text, SelectionRange selection)
{
    hasUndo = true;
    undoText = text;
    undoSelection = selection;
}

void LvtkEditBoxElement::UpdateErrorStyle()
{
    if (hasErrorStyle != showError)
    {
        hasErrorStyle = showError;
        RemoveClass(errorStyle);
        if (showError)
        {
            AddClass(errorStyle);
        }
        Invalidate();
    }
}

LvtkEditBoxElement &LvtkEditBoxElement::ShowError(bool value)
{
    if (showError != value)
    {
        showError = value;
        UpdateErrorStyle();
        Invalidate();
    }
    return *this;
}
bool LvtkEditBoxElement::ShowError() const
{
    return showError;
}


void LvtkEditBoxElement::SelectAll()
{
    Selection({Text().length(),0});
}
void LvtkEditBoxElement::SelectEnd()
{
    Selection({Text().length(),Text().length()});
}

LvtkEditBoxElement&LvtkEditBoxElement::UnicodeEntry(bool value)
{
    if (value != this->unicodeEntry)
    {
        if (value)
        {
            unicodeEntryString = "";
        }
        unicodeEntry = value;
        markupChanged = true;
        this->selectionChanged = true;
        Invalidate();
    }
    return *this;
}
bool LvtkEditBoxElement::UnicodeEntry() const
{
    return unicodeEntry;
}


bool LvtkEditBoxElement::WantsFocus() const
{
    if (Style().Visibility() == LvtkVisibility::Collapsed)
    {
        return false;
    }
    return true;

}