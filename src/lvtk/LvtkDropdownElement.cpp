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

#include "lvtk/LvtkDropdownElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkSvgElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkPangoContext.hpp"
#include "lvtk/LvtkDropdownItemElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "pango/pangocairo.h"
#include "lvtk/LvtkPangoContext.hpp"
#include "lvtk/LvtkSlideInOutAnimationElement.hpp"
#include "lvtk/LvtkDropShadowElement.hpp"

#define XK_MISCELLANY
#define XK_LATIN1
#include "X11/keysymdef.h"

#include <chrono>

using namespace lvtk;

namespace lvtk::implementation
{
    class DropdownItemLayoutElement : public LvtkContainerElement
    {
    public:
        using self = DropdownItemLayoutElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

    protected:
        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable, LvtkDrawingContext &context) override;
        virtual LvtkSize Arrange(LvtkSize available, LvtkDrawingContext &context) override;
    private:
        std::vector<double> columnWidths;
        std::vector<size_t> columnCounts;
    };

    class AnimatedDropdownElement : public LvtkDropShadowElement
    {
        using clock_t = std::chrono::steady_clock;
        static constexpr int ANIMATION_DURATION = 200;
        // static constexpr int ANIMATION_DURATION = 2000;
    public:
        using self = AnimatedDropdownElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create(
            const LvtkTheme &theme,
            const std::vector<LvtkDropdownItemElement::ptr> &itemElements)
        {
            return std::make_shared<self>(theme, itemElements);
        }

        AnimatedDropdownElement(const LvtkTheme &theme, const std::vector<LvtkDropdownItemElement::ptr> &itemElements)
        {
            this->DropShadow(theme.menuDropShadow);
            this->Style()
                .HorizontalAlignment(LvtkAlignment::Start);
            this->AddClass(theme.dropdownItemContainerStyle);

            {
                this->slideElement = LvtkSlideInOutAnimationElement::Create();

                auto stack = DropdownItemLayoutElement::Create();

                slideElement->AddChild(stack);

                for (auto &item : itemElements)
                {
                    stack->AddChild(item);
                }
                this->itemElements = itemElements;
                SelectedIdProperty.SetElement(this, &AnimatedDropdownElement::OnSelectedIdChanged);
            }
            this->AddChild(slideElement);
        }
        void SetAnchor(LvtkElement *anchor) { this->anchor = anchor; }
        BINDING_PROPERTY(SelectedId, selection_id_t, -1)
        // BINDING_PROPERTY(SelectionColor,LvtkColor,LvtkColor(1,0.5,0.5))
    protected:
        bool wrapElements = false;
        virtual void OnMount() override
        {
            super::OnMount();
        }

    private:
        LvtkSlideInOutAnimationElement::ptr slideElement;
        bool AnimateUpward() const;
        void OnSelectedIdChanged(selection_id_t selection);

        bool hasAnimated = false;
        virtual void OnLayoutComplete() override;

        LvtkElement *anchor = nullptr;
        std::vector<LvtkDropdownItemElement::ptr> itemElements;
    };

    void AnimatedDropdownElement::OnLayoutComplete()
    {
        if (!hasAnimated)
        {
            hasAnimated = true;

            if (AnimateUpward())
            {
                slideElement->EasingFunction(LvtkEasingFunction::EaseInQuad);
                slideElement->StartAnimation(LvtkSlideAnimationType::SlideInBottom, ANIMATION_DURATION);
            }
            else
            {
                slideElement->EasingFunction(LvtkEasingFunction::EaseInQuad);
                slideElement->StartAnimation(LvtkSlideAnimationType::SlideInTop, ANIMATION_DURATION);
            }
        }
    }

    bool AnimatedDropdownElement::AnimateUpward() const
    {
        return this->ScreenBounds().Top() < this->anchor->ScreenBounds().Top();
    }
    void AnimatedDropdownElement::OnSelectedIdChanged(selection_id_t selection)
    {
        for (auto &item : itemElements)
        {
            if (item->SelectionId() == selection)
            {
                item->HoverState(item->HoverState() + LvtkHoverState::Pressed);
            }
            else
            {
                item->HoverState(item->HoverState() - LvtkHoverState::Pressed);
            }
        }
    }
}

using namespace lvtk::implementation;

LvtkSize DropdownItemLayoutElement::MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable, LvtkDrawingContext &context)
{
    columnCounts.resize(0);
    columnWidths.resize(0);
    double width = 0;
    double height = 0;

    double x = 0; double y = 0;
    double columnWidth = 0;
    size_t columnCount = 0;
    for (auto & child: Children())
    {
        child->Measure(clientConstraint,clientAvailable,context);
        LvtkSize childSize = child->MeasuredSize();
        if (y + childSize.Height() >= clientAvailable.Height() && columnCount != 0)
        {
            columnCounts.push_back(columnCount);
            columnWidths.push_back(columnWidth);
            if (y > height)
            {
                height = y;
            }
            x += columnWidth;
            y = 0;
            columnCount = 0;
            columnWidth = 0;
        }
        if (childSize.Width() > columnWidth)
        {
            columnWidth = childSize.Width();
        }
        y += childSize.Height();
        ++columnCount;
    }
    if (columnCount != 0)
    {
        columnCounts.push_back(columnCount);
        columnWidths.push_back(columnWidth);
        if (y > height) height = y;
        x += columnWidth;
    }
    width = x;
    return LvtkSize(width,height);
}
LvtkSize DropdownItemLayoutElement::Arrange(LvtkSize available, LvtkDrawingContext &context)
{
    double x = 0;
    size_t childIx = 0;
    for (size_t column = 0; column < columnCounts.size(); ++column)
    {
        double y = 0;
        double columnWidth = columnWidths[column];
        for (size_t i = 0; i < columnCounts[column]; ++i)
        {
            LvtkElement::ptr child = Child(childIx++);
            LvtkSize size = child->MeasuredSize();
            size.Width(columnWidth);
            child->Arrange(size,context);
            LvtkRectangle rc { x,y,size.Width(),size.Height()};
            child->Layout(rc);
            y += size.Height();
        }
        x += columnWidth;
    }

    return available;
}

LvtkSize LvtkDropdownElement::MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable, LvtkDrawingContext &context)
{
    if (clientConstraint.Width() != 0)
    {
        // fixed width: use standard measure.
        return super::MeasureClient(clientConstraint, clientAvailable, context);
    }
    else
    {
        // variable width: measure all the dropdown items to determine our width.
        icon->Measure(clientConstraint, clientAvailable, context);
        LvtkRectangle iconMeasure = icon->MeasuredSize();

        if (pangoLayout == nullptr)
        {
            pangoLayout = pango_layout_new(GetPangoContext());
        }
        PangoFontDescription *desc = gPangoContext.GetFontDescription(Style());

        pango_layout_set_font_description(pangoLayout, desc);

        double maxWidth = 20;
        for (auto &dropdownItem : this->DropdownItems())
        {
            pango_layout_set_text(pangoLayout, dropdownItem.Text().c_str(), dropdownItem.Text().length());
            pango_cairo_update_layout(context.get(), pangoLayout);

            PangoRectangle pangoExtent;
            pango_layout_get_extents(pangoLayout, nullptr, &pangoExtent);
            double width = std::ceil(pangoExtent.width / PANGO_SCALE);
            if (width > maxWidth)
                maxWidth = width;
        }
        maxWidth += 4; // for good luck.

        pango_font_description_free(desc);

        clientConstraint.Width(maxWidth + iconMeasure.Width());

        return super::MeasureClient(clientConstraint, clientAvailable, context);
    }
}
LvtkDropdownElement::~LvtkDropdownElement() noexcept
{
    if (pangoLayout)
    {
        g_object_unref(pangoLayout);
    }
}

LvtkDropdownElement::LvtkDropdownElement()
{
    SelectedId(INVALID_SELECTION_ID);
    auto flexGrid = LvtkFlexGridElement::Create();
    this->AddChild(flexGrid);
    flexGrid->Style()
        .FlexWrap(LvtkFlexWrap::NoWrap)
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .FlexAlignItems(LvtkAlignment::Center);

    typography = LvtkTypographyElement::Create();
    flexGrid->AddChild(typography);
    typography->Style().HorizontalAlignment(LvtkAlignment::Stretch).Ellipsize(LvtkEllipsizeMode::End).SingleLine(true);

    auto icon = LvtkSvgElement::Create();
    this->icon = icon;
    icon->Source("ic_dropdown.svg");
    icon->Style()
        .Width(18)
        .Height(18)
        .Margin({4, 0, 0, 0});

    flexGrid->AddChild(icon);

    TextProperty.Bind(typography->TextProperty);

    UnselectedTextProperty.SetElement(this, &LvtkDropdownElement::OnUnselectedTextChanged);
    SelectedIdProperty.SetElement(this, &LvtkDropdownElement::OnSelectedIdChanged);
    DropdownItemsProperty.SetElement(this, &LvtkDropdownElement::OnDropdownItemsChanged);

    TextProperty.SetElement(this, LvtkBindingFlags::InvalidateOnChanged);
}

const std::string &LvtkDropdownElement::Text() const
{

    return TextProperty.get();
}
LvtkDropdownElement &LvtkDropdownElement::Text(const std::string &text)
{
    TextProperty.set(text);
    return *this;
}

void LvtkDropdownElement::OnUnmount()
{
    CloseDropdown();
}
void LvtkDropdownElement::OnMount()
{
    auto &theme = Theme();
    this->hoverTextColors = theme.hoverTextColors;
    this->AddClass(theme.dropdownUnderlineStyle);

    UpdateColors();
}

bool LvtkDropdownElement::WantsFocus() const
{
    if (Style().Visibility() != LvtkVisibility::Visible)
    {
        return false;
    }
    return true;
}

bool LvtkDropdownElement::OnFocus(const LvtkFocusEventArgs &eventArgs)
{
    super::OnFocus(eventArgs);
    return false;
}
bool LvtkDropdownElement::OnLostFocus(const LvtkFocusEventArgs &eventArgs)
{
    CloseDropdown();
    super::OnLostFocus(eventArgs);
    return false;
}

void LvtkDropdownElement::UpdateColors()
{
    LvtkColor color = hoverTextColors.GetColor(HoverState());
    if (selectionValid)
    {
        this->typography->Style()
            .Color(color)
            .FontStyle(LvtkFontStyle::Normal);
    }
    else
    {
        LvtkColor transparentColor = LvtkColor(color, 0.5);
        this->typography->Style()
            .Color(transparentColor)
            .FontStyle(LvtkFontStyle::Italic);
    }
    this->icon->Style().TintColor(color);
}
void LvtkDropdownElement::OnHoverStateChanged(LvtkHoverState hoverState)
{
    UpdateColors();
    this->Invalidate();
}

bool LvtkDropdownElement::OnClick(LvtkMouseEventArgs &event)
{
    if (super::OnClick(event))
    {
        return true;
    }
    if (this->DropdownItems().size() == 0)
    {
        return true;
    }

    CloseDropdown();

    OpenDropdown();

    return true;
}

bool LvtkDropdownItem::operator==(const LvtkDropdownItem &other) const
{
    return itemId == other.itemId && separator == other.separator && svgIcon == other.svgIcon && text == other.text;
}

lvtk::LvtkDropdownItem::LvtkDropdownItem() : itemId(INVALID_SELECTION_ID), separator(false) {}

lvtk::LvtkDropdownItem::LvtkDropdownItem(selection_id_t itemId, const std::string &text)
    : itemId(itemId), text(text), separator(false)
{
}

lvtk::LvtkDropdownItem::LvtkDropdownItem(selection_id_t itemId, const std::string &text, const std::string &svgIcon)
    : itemId(itemId), text(text), svgIcon(svgIcon), separator(false)
{
}

LvtkDropdownItem LvtkDropdownItem::Separator()
{
    LvtkDropdownItem item;
    item.separator = true;
    return item;
}

selection_id_t LvtkDropdownItem::ItemId() const { return itemId; }
LvtkDropdownItem &LvtkDropdownItem::ItemId(selection_id_t value)
{
    itemId = value;
    return *this;
}
const std::string &LvtkDropdownItem::SvgIcon() const { return svgIcon; }
LvtkDropdownItem &LvtkDropdownItem::SvgIcon(const std::string &value)
{
    svgIcon = value;
    return *this;
}
const std::string &LvtkDropdownItem::Text() const { return text; }
LvtkDropdownItem &LvtkDropdownItem::Text(const std::string &value)
{
    text = value;
    return *this;
}

LvtkDropdownItem &LvtkDropdownItem::IsSeparator(bool value)
{
    separator = value;
    return *this;
}
bool LvtkDropdownItem::IsSeparator() const { return separator; }

AnimatedDropdownElement::ptr LvtkDropdownElement::RenderDropdown()
{
    LvtkDropShadow dropShadowSettings;
    dropShadowSettings.variant = LvtkDropShadowVariant::DropShadow;
    dropShadowSettings.opacity = 0.75;
    dropShadowSettings.radius = 8;
    dropShadowSettings.xOffset = 0;
    dropShadowSettings.yOffset = 2;

    auto dropShadow = LvtkDropShadowElement::Create();

    dropShadow->DropShadow(dropShadowSettings);

    {
        bool hasIcon = false;
        for (auto &dropdownItem : DropdownItems())
        {
            hasIcon |= dropdownItem.SvgIcon().length() != 0;
        }

        dropdownItemEventHandles.resize(0);
        dropdownItemEventHandles.reserve(DropdownItems().size());

        std::vector<LvtkDropdownItemElement::ptr> dropdownItemElements;
        for (auto &dropdownItem : DropdownItems())
        {
            LvtkDropdownItemElement::ptr item;
            if (hasIcon)
            {
                item = LvtkDropdownItemElement::Create(dropdownItem.ItemId(), dropdownItem.Text(), dropdownItem.SvgIcon());
            }
            else
            {
                item = LvtkDropdownItemElement::Create(dropdownItem.ItemId(), dropdownItem.Text());
            }
            dropdownItemElements.push_back(item);
            auto itemId = dropdownItem.ItemId();
            dropdownItemEventHandles.push_back(item->Clicked.AddListener([this, itemId](const LvtkMouseEventArgs &e)
                                                                         {
                    this->FireItemClick(itemId);
                    return true; }));
        }
        auto &theme = Theme();
        auto frame = AnimatedDropdownElement::Create(theme, dropdownItemElements);
        // add a spacer to eforce miminimum width.
        double minWidth = this->ClientBounds().Width() - 8;
        auto spacer = LvtkElement::Create();
        frame->AddChild(spacer);
        spacer->Style().Width(minWidth).Height(0);

        frame->SelectedId(this->SelectedId());

        this->dropdownElement = frame;
    }
    return dropdownElement;
}

void LvtkDropdownElement::FireItemClick(selection_id_t itemId)
{
    Window()->PostDelayed(0, [this, itemId]()
                          {
        CloseDropdown();
        SelectedId(itemId); });
}

void LvtkDropdownElement::OpenDropdown()
{
    CloseDropdown();

    Focus();

    auto dropdown = RenderDropdown();
    this->dropdownElement = dropdown;
    this->Window()->GetRootElement()->AddPopup(
        dropdown, this, [this]()
        { this->ReleaseDropdownElements(); });
    dropdown->SetAnchor(this);
}
bool LvtkDropdownElement::DropdownOpen() const
{
    return this->dropdownElement != nullptr;
}

void LvtkDropdownElement::CloseDropdown()
{
    if (this->dropdownElement)
    {
        auto t = dropdownElement;
        dropdownElement = nullptr;
        this->Window()->GetRootElement()->RemoveChild(t);
    }
}
void LvtkDropdownElement::ReleaseDropdownElements()
{
    dropdownItemEventHandles.resize(0);
}

selection_id_t LvtkDropdownElement::SelectedId() const
{
    return SelectedIdProperty.get();
}
LvtkDropdownElement &LvtkDropdownElement::SelectedId(selection_id_t selectedId)
{
    SelectedIdProperty.set(selectedId);
    return *this;
}

const std::string &LvtkDropdownElement::UnselectedText() const
{
    return UnselectedTextProperty.get();
}
LvtkDropdownElement &LvtkDropdownElement::UnselectedText(const std::string &text)
{
    UnselectedTextProperty.set(text);
    return *this;
}

void LvtkDropdownElement::OnSelectedIdChanged(selection_id_t value)
{
    UpdateText();
}
void LvtkDropdownElement::OnUnselectedTextChanged(const std::string &value)
{

    UpdateText();
}

void LvtkDropdownElement::OnDropdownItemsChanged(const items_t &value)
{
    UpdateText();
}

void LvtkDropdownElement::UpdateText()
{
    auto selectedId = SelectedId();
    bool hasSelection = false;
    for (auto &dropdownItem : this->DropdownItems())
    {
        if (selectedId == dropdownItem.ItemId())
        {
            Text(dropdownItem.Text());
            hasSelection = true;
        }
    }
    if (!hasSelection)
    {
        Text(this->UnselectedText());
    }
    if (selectionValid != hasSelection)
    {
        this->selectionValid = hasSelection;
        UpdateColors();
        Invalidate();
    }
}

int64_t LvtkDropdownElement::SelectedIndex(selection_id_t selectedId) const
{
    int64_t result = -1;
    const auto &items = DropdownItems();
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (items[i].ItemId() == selectedId)
        {
            result = (int64_t)i;
            break;
        }
    }
    return result;
}

void LvtkDropdownElement::SelectNext()
{
    if (this->dropdownElement)
    {
        int64_t selected = SelectedIndex(dropdownElement->SelectedId());
        if (selected < 0)
        {
            if (DropdownItems().size() == 0)
            {
                return;
            }
            selected = 0;
        }
        else
        {
            ++selected;
            if (selected >= (int64_t)(DropdownItems().size()))
            {
                selected = 0;
            }
        }
        dropdownElement->SelectedId(DropdownItems()[selected].ItemId());
    }
    else
    {
        int64_t selected = SelectedIndex(SelectedId());
        if (selected < 0)
        {
            if (DropdownItems().size() == 0)
            {
                return;
            }
            selected = 0;
        }
        else
        {
            ++selected;
            if (selected >= (int64_t)(DropdownItems().size()))
            {
                selected = 0;
            }
        }
        SelectedId(DropdownItems()[selected].ItemId());
    }
}
void LvtkDropdownElement::SelectPrevious()
{
    if (dropdownElement)
    {
        int64_t selected = SelectedIndex(dropdownElement->SelectedId());
        if (selected < 0)
        {
            if (DropdownItems().size() == 0)
            {
                return;
            }
            selected = (int64_t)(DropdownItems().size() - 1);
        }
        else
        {
            --selected;
            if (selected < 0)
            {
                selected = (int64_t)(DropdownItems().size() - 1);
            }
        }
        dropdownElement->SelectedId(DropdownItems()[selected].ItemId());
    }
    else
    {
        int64_t selected = SelectedIndex(SelectedId());
        if (selected < 0)
        {
            if (DropdownItems().size() == 0)
            {
                return;
            }
            selected = (int64_t)(DropdownItems().size() - 1);
        }
        else
        {
            --selected;
            if (selected < 0)
            {
                selected = (int64_t)(DropdownItems().size() - 1);
            }
        }
        SelectedId(DropdownItems()[selected].ItemId());
    }
}

bool LvtkDropdownElement::OnKeyDown(const LvtkKeyboardEventArgs &event)
{
    if (event.keysymValid)
    {
        if (event.modifierState == ModifierState::Empty)
        {
            switch (event.keysym)
            {
            case XK_Up:
            case XK_KP_Up:
                SelectPrevious();
                return true;
            case XK_Down:
            case XK_KP_Down:
                SelectNext();
                return true;
            case XK_Escape:
                CloseDropdown();
                return true;
            case XK_space:
                if (!DropdownOpen())
                {
                    OpenDropdown();
                }
                return true;
            case XK_Return:
            case XK_KP_Enter:
                if (DropdownOpen())
                {
                    SelectedId(this->dropdownElement->SelectedId());
                    CloseDropdown();
                }
                else
                {
                    OpenDropdown();
                }
                return true;
            default:
                break;
            }
        }
    }
    return false;
}
