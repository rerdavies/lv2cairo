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

#include "lv2c/Lv2cDropdownElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cSvgElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cPangoContext.hpp"
#include "lv2c/Lv2cDropdownItemElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "pango/pangocairo.h"
#include "lv2c/Lv2cPangoContext.hpp"
#include "lv2c/Lv2cSlideInOutAnimationElement.hpp"
#include "lv2c/Lv2cDropShadowElement.hpp"

#define XK_MISCELLANY
#define XK_LATIN1
#include "X11/keysymdef.h"

#include <chrono>

using namespace lvtk;

namespace lvtk::implementation
{
    class DropdownItemLayoutElement : public Lv2cContainerElement
    {
    public:
        using self = DropdownItemLayoutElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

    protected:
        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &context) override;
        virtual Lv2cSize Arrange(Lv2cSize available, Lv2cDrawingContext &context) override;
    private:
        std::vector<double> columnWidths;
        std::vector<size_t> columnCounts;
    };

    class AnimatedDropdownElement : public Lv2cDropShadowElement
    {
        using clock_t = std::chrono::steady_clock;
        static constexpr int ANIMATION_DURATION = 200;
        // static constexpr int ANIMATION_DURATION = 2000;
    public:
        using self = AnimatedDropdownElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;

        static ptr Create(
            const Lv2cTheme &theme,
            const std::vector<Lv2cDropdownItemElement::ptr> &itemElements)
        {
            return std::make_shared<self>(theme, itemElements);
        }

        AnimatedDropdownElement(const Lv2cTheme &theme, const std::vector<Lv2cDropdownItemElement::ptr> &itemElements)
        {
            this->DropShadow(theme.menuDropShadow);
            this->Style()
                .HorizontalAlignment(Lv2cAlignment::Start);
            this->AddClass(theme.dropdownItemContainerStyle);

            {
                this->slideElement = Lv2cSlideInOutAnimationElement::Create();

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
        void SetAnchor(Lv2cElement *anchor) { this->anchor = anchor; }
        BINDING_PROPERTY(SelectedId, selection_id_t, -1)
        // BINDING_PROPERTY(SelectionColor,Lv2cColor,Lv2cColor(1,0.5,0.5))
    protected:
        bool wrapElements = false;
        virtual void OnMount() override
        {
            super::OnMount();
        }

    private:
        Lv2cSlideInOutAnimationElement::ptr slideElement;
        bool AnimateUpward() const;
        void OnSelectedIdChanged(selection_id_t selection);

        bool hasAnimated = false;
        virtual void OnLayoutComplete() override;

        Lv2cElement *anchor = nullptr;
        std::vector<Lv2cDropdownItemElement::ptr> itemElements;
    };

    void AnimatedDropdownElement::OnLayoutComplete()
    {
        if (!hasAnimated)
        {
            hasAnimated = true;

            if (AnimateUpward())
            {
                slideElement->EasingFunction(Lv2cEasingFunction::EaseInQuad);
                slideElement->StartAnimation(Lv2cSlideAnimationType::SlideInBottom, ANIMATION_DURATION);
            }
            else
            {
                slideElement->EasingFunction(Lv2cEasingFunction::EaseInQuad);
                slideElement->StartAnimation(Lv2cSlideAnimationType::SlideInTop, ANIMATION_DURATION);
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
                item->HoverState(item->HoverState() + Lv2cHoverState::Pressed);
            }
            else
            {
                item->HoverState(item->HoverState() - Lv2cHoverState::Pressed);
            }
        }
    }
}

using namespace lvtk::implementation;

Lv2cSize DropdownItemLayoutElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &context)
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
        Lv2cSize childSize = child->MeasuredSize();
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
    return Lv2cSize(width,height);
}
Lv2cSize DropdownItemLayoutElement::Arrange(Lv2cSize available, Lv2cDrawingContext &context)
{
    double x = 0;
    size_t childIx = 0;
    for (size_t column = 0; column < columnCounts.size(); ++column)
    {
        double y = 0;
        double columnWidth = columnWidths[column];
        for (size_t i = 0; i < columnCounts[column]; ++i)
        {
            Lv2cElement::ptr child = Child(childIx++);
            Lv2cSize size = child->MeasuredSize();
            size.Width(columnWidth);
            child->Arrange(size,context);
            Lv2cRectangle rc { x,y,size.Width(),size.Height()};
            child->Layout(rc);
            y += size.Height();
        }
        x += columnWidth;
    }

    return available;
}

Lv2cSize Lv2cDropdownElement::MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable, Lv2cDrawingContext &context)
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
        Lv2cRectangle iconMeasure = icon->MeasuredSize();

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
Lv2cDropdownElement::~Lv2cDropdownElement() noexcept
{
    if (pangoLayout)
    {
        g_object_unref(pangoLayout);
    }
}

Lv2cDropdownElement::Lv2cDropdownElement()
{
    SelectedId(INVALID_SELECTION_ID);
    auto flexGrid = Lv2cFlexGridElement::Create();
    this->AddChild(flexGrid);
    flexGrid->Style()
        .FlexWrap(Lv2cFlexWrap::NoWrap)
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .FlexAlignItems(Lv2cAlignment::Center);

    typography = Lv2cTypographyElement::Create();
    flexGrid->AddChild(typography);
    typography->Style().HorizontalAlignment(Lv2cAlignment::Stretch).Ellipsize(Lv2cEllipsizeMode::End).SingleLine(true);

    auto icon = Lv2cSvgElement::Create();
    this->icon = icon;
    icon->Source("ic_dropdown.svg");
    icon->Style()
        .Width(18)
        .Height(18)
        .Margin({4, 0, 0, 0});

    flexGrid->AddChild(icon);

    TextProperty.Bind(typography->TextProperty);

    UnselectedTextProperty.SetElement(this, &Lv2cDropdownElement::OnUnselectedTextChanged);
    SelectedIdProperty.SetElement(this, &Lv2cDropdownElement::OnSelectedIdChanged);
    DropdownItemsProperty.SetElement(this, &Lv2cDropdownElement::OnDropdownItemsChanged);

    TextProperty.SetElement(this, Lv2cBindingFlags::InvalidateOnChanged);
}

const std::string &Lv2cDropdownElement::Text() const
{

    return TextProperty.get();
}
Lv2cDropdownElement &Lv2cDropdownElement::Text(const std::string &text)
{
    TextProperty.set(text);
    return *this;
}

void Lv2cDropdownElement::OnUnmount()
{
    CloseDropdown();
}
void Lv2cDropdownElement::OnMount()
{
    auto &theme = Theme();
    
    this->ClearClasses();
    super::OnMount();

    this->hoverTextColors = theme.hoverTextColors;
    this->AddClass(theme.dropdownUnderlineStyle);

    UpdateColors();
}

bool Lv2cDropdownElement::WantsFocus() const
{
    if (Style().Visibility() != Lv2cVisibility::Visible)
    {
        return false;
    }
    return true;
}

bool Lv2cDropdownElement::OnFocus(const Lv2cFocusEventArgs &eventArgs)
{
    super::OnFocus(eventArgs);
    return false;
}
bool Lv2cDropdownElement::OnLostFocus(const Lv2cFocusEventArgs &eventArgs)
{
    CloseDropdown();
    super::OnLostFocus(eventArgs);
    return false;
}

void Lv2cDropdownElement::UpdateColors()
{
    Lv2cColor color = hoverTextColors.GetColor(HoverState());
    if (selectionValid)
    {
        this->typography->Style()
            .Color(color)
            .FontStyle(Lv2cFontStyle::Normal);
    }
    else
    {
        Lv2cColor transparentColor = Lv2cColor(color, 0.5);
        this->typography->Style()
            .Color(transparentColor)
            .FontStyle(Lv2cFontStyle::Italic);
    }
    this->icon->Style().TintColor(color);
}
void Lv2cDropdownElement::OnHoverStateChanged(Lv2cHoverState hoverState)
{
    UpdateColors();
    this->Invalidate();
}

bool Lv2cDropdownElement::OnClick(Lv2cMouseEventArgs &event)
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

bool Lv2cDropdownItem::operator==(const Lv2cDropdownItem &other) const
{
    return itemId == other.itemId && separator == other.separator && svgIcon == other.svgIcon && text == other.text;
}

lvtk::Lv2cDropdownItem::Lv2cDropdownItem() : itemId(INVALID_SELECTION_ID), separator(false) {}

lvtk::Lv2cDropdownItem::Lv2cDropdownItem(selection_id_t itemId, const std::string &text)
    : itemId(itemId), text(text), separator(false)
{
}

lvtk::Lv2cDropdownItem::Lv2cDropdownItem(selection_id_t itemId, const std::string &text, const std::string &svgIcon)
    : itemId(itemId), text(text), svgIcon(svgIcon), separator(false)
{
}

Lv2cDropdownItem Lv2cDropdownItem::Separator()
{
    Lv2cDropdownItem item;
    item.separator = true;
    return item;
}

selection_id_t Lv2cDropdownItem::ItemId() const { return itemId; }
Lv2cDropdownItem &Lv2cDropdownItem::ItemId(selection_id_t value)
{
    itemId = value;
    return *this;
}
const std::string &Lv2cDropdownItem::SvgIcon() const { return svgIcon; }
Lv2cDropdownItem &Lv2cDropdownItem::SvgIcon(const std::string &value)
{
    svgIcon = value;
    return *this;
}
const std::string &Lv2cDropdownItem::Text() const { return text; }
Lv2cDropdownItem &Lv2cDropdownItem::Text(const std::string &value)
{
    text = value;
    return *this;
}

Lv2cDropdownItem &Lv2cDropdownItem::IsSeparator(bool value)
{
    separator = value;
    return *this;
}
bool Lv2cDropdownItem::IsSeparator() const { return separator; }

AnimatedDropdownElement::ptr Lv2cDropdownElement::RenderDropdown()
{
    Lv2cDropShadow dropShadowSettings;
    dropShadowSettings.variant = Lv2cDropShadowVariant::DropShadow;
    dropShadowSettings.opacity = 0.75;
    dropShadowSettings.radius = 8;
    dropShadowSettings.xOffset = 0;
    dropShadowSettings.yOffset = 2;

    auto dropShadow = Lv2cDropShadowElement::Create();

    dropShadow->DropShadow(dropShadowSettings);

    {
        bool hasIcon = false;
        for (auto &dropdownItem : DropdownItems())
        {
            hasIcon |= dropdownItem.SvgIcon().length() != 0;
        }

        dropdownItemEventHandles.resize(0);
        dropdownItemEventHandles.reserve(DropdownItems().size());

        std::vector<Lv2cDropdownItemElement::ptr> dropdownItemElements;
        for (auto &dropdownItem : DropdownItems())
        {
            Lv2cDropdownItemElement::ptr item;
            if (hasIcon)
            {
                item = Lv2cDropdownItemElement::Create(dropdownItem.ItemId(), dropdownItem.Text(), dropdownItem.SvgIcon());
            }
            else
            {
                item = Lv2cDropdownItemElement::Create(dropdownItem.ItemId(), dropdownItem.Text());
            }
            dropdownItemElements.push_back(item);
            auto itemId = dropdownItem.ItemId();
            dropdownItemEventHandles.push_back(item->Clicked.AddListener([this, itemId](const Lv2cMouseEventArgs &e)
                                                                         {
                    this->FireItemClick(itemId);
                    return true; }));
        }
        auto &theme = Theme();
        auto frame = AnimatedDropdownElement::Create(theme, dropdownItemElements);
        // add a spacer to eforce miminimum width.
        double minWidth = this->ClientBounds().Width() - 8;
        auto spacer = Lv2cElement::Create();
        frame->AddChild(spacer);
        spacer->Style().Width(minWidth).Height(0);

        frame->SelectedId(this->SelectedId());

        this->dropdownElement = frame;
    }
    return dropdownElement;
}

void Lv2cDropdownElement::FireItemClick(selection_id_t itemId)
{
    Window()->PostDelayed(0, [this, itemId]()
                          {
        CloseDropdown();
        SelectedId(itemId); });
}

void Lv2cDropdownElement::OpenDropdown()
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
bool Lv2cDropdownElement::DropdownOpen() const
{
    return this->dropdownElement != nullptr;
}

void Lv2cDropdownElement::CloseDropdown()
{
    if (this->dropdownElement)
    {
        auto t = dropdownElement;
        dropdownElement = nullptr;
        this->Window()->GetRootElement()->RemoveChild(t);
    }
}
void Lv2cDropdownElement::ReleaseDropdownElements()
{
    dropdownItemEventHandles.resize(0);
}

selection_id_t Lv2cDropdownElement::SelectedId() const
{
    return SelectedIdProperty.get();
}
Lv2cDropdownElement &Lv2cDropdownElement::SelectedId(selection_id_t selectedId)
{
    SelectedIdProperty.set(selectedId);
    return *this;
}

const std::string &Lv2cDropdownElement::UnselectedText() const
{
    return UnselectedTextProperty.get();
}
Lv2cDropdownElement &Lv2cDropdownElement::UnselectedText(const std::string &text)
{
    UnselectedTextProperty.set(text);
    return *this;
}

void Lv2cDropdownElement::OnSelectedIdChanged(selection_id_t value)
{
    UpdateText();
}
void Lv2cDropdownElement::OnUnselectedTextChanged(const std::string &value)
{

    UpdateText();
}

void Lv2cDropdownElement::OnDropdownItemsChanged(const items_t &value)
{
    UpdateText();
}

void Lv2cDropdownElement::UpdateText()
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

int64_t Lv2cDropdownElement::SelectedIndex(selection_id_t selectedId) const
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

void Lv2cDropdownElement::SelectNext()
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
void Lv2cDropdownElement::SelectPrevious()
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

bool Lv2cDropdownElement::OnKeyDown(const Lv2cKeyboardEventArgs &event)
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
