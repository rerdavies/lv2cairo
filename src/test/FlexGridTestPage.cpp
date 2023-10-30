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

#include "FlexGridTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cDropdownElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cBindingProperty.hpp"
#include "lv2c/Lv2cNumericEditBoxElement.hpp"

using namespace lv2c;

static std::vector<Lv2cElement::ptr> MakeFlexChildren()
{
    std::vector<Lv2cElement::ptr> result;
    std::vector<std::string> data{"Sed", "ut", "perspiciatis", "unde",
                                  "omnis", "iste", "natus", "error", "sit",
                                  "voluptatem", "accusantium", "doloremque", "laudantium"};

    auto style = Lv2cStyle::Create();
    style->FontSize(Lv2cMeasurement::Point(15))
        .Margin({1})
        .BorderWidth({1})
        .BorderColor(Lv2cColor(1, 1, 1, 0.3))
        .Padding({4})
        .Color(Lv2cColor(1, 1, 1))
        .TextAlign(Lv2cTextAlign::Start);

    for (auto &word : data)
    {
        auto typography = Lv2cTypographyElement::Create();
        typography->Text(word).AddClass(style);
        typography->Style()
            .Margin({1})
            .BorderWidth({1})
            .BorderColor(Lv2cColor(1, 1, 1, 0.3))
            .Padding({4});
        result.push_back(typography);
    }
    return result;
}

template <typename T>
class VariantDropdownElement : public Lv2cDropdownElement
{
public:
    using self = VariantDropdownElement<T>;
    using super = Lv2cDropdownElement;
    using ptr = std::shared_ptr<self>;
    static ptr Create(const std::vector<Lv2cDropdownItem> &items)
    {
        auto result = std::make_shared<self>();
        result->DropdownItems(items);
        return result;
    }
    VariantDropdownElement()
    {
        ValueProperty.SetElement(this, &VariantDropdownElement<T>::OnValueChanged);
    }



    BINDING_PROPERTY(Value, T, ((T)0))
protected:
    virtual void OnDropdownItemsChanged(const items_t&value) override
    {
        super::OnDropdownItemsChanged(value);
        SelectedId(value[0].ItemId());

    }

    virtual void OnSelectedIdChanged(selection_id_t value) override
    {
        super::OnSelectedIdChanged(value);
        Value((T)value);
    }
    virtual void OnValueChanged(T value)
    {
        SelectedId((selection_id_t)value);
    }
};

#define V_DROPDOWN_ENTRY(VARIANT, MEMBER) \
    Lv2cDropdownItem((int64_t)(VARIANT::MEMBER), (#MEMBER))

class FlexTestElement : public Lv2cContainerElement
{
public:
    using self = FlexTestElement;
    using super = Lv2cContainerElement;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }
    FlexTestElement();

private:
    Lv2cFlexGridElement::ptr MakeGrid();
    Lv2cFlexGridElement::ptr grid;
    std::vector<observer_handle_t> observerHandles;
};

static Lv2cElement::ptr LabeledControl(const std::string &label_, Lv2cElement::ptr control)
{
    auto container = Lv2cVerticalStackElement::Create();
    container->Style().HorizontalAlignment(Lv2cAlignment::Start);
    auto label = Lv2cTypographyElement::Create();
    label->Text(label_);
    label->Style().FontSize(11)
        .Margin({6,0,0,0});
    container->AddChild(label);
    container->AddChild(control);
    container->Style().Margin({4});
    return container;
}
static Lv2cElement::ptr LabeledEditControl(const std::string &label_, Lv2cElement::ptr control)
{
    auto container = Lv2cVerticalStackElement::Create();
    container->Style().HorizontalAlignment(Lv2cAlignment::Start);
    auto label = Lv2cTypographyElement::Create();
    label->Text(label_);
    label->Style().FontSize(11)
        .Margin({4,0,0,8});
    container->AddChild(label);
    container->AddChild(control);
    container->Style().Margin({4});
    return container;
}
FlexTestElement::FlexTestElement()
{
    auto stack = Lv2cFlexGridElement::Create();
    stack->Style().FlexDirection(Lv2cFlexDirection::Column).FlexWrap(Lv2cFlexWrap::NoWrap).HorizontalAlignment(Lv2cAlignment::Stretch).VerticalAlignment(Lv2cAlignment::Stretch);
    {
        this->grid = MakeGrid();
        stack->AddChild(grid);

        {
            auto element = Lv2cElement::Create();
            element->Style()
                .Width(Lv2cMeasurement::Percent(100))
                .Height(1)
                .Margin({0,8,0,0})
                .Background(Lv2cColor(0.3,0.3,0.3));
            stack->AddChild(element);
        }

        auto controlGrid = Lv2cFlexGridElement::Create();
        controlGrid->Style()
            .VerticalAlignment(Lv2cAlignment::End)
            .FlexWrap(Lv2cFlexWrap::Wrap);
        {
            auto element = VariantDropdownElement<Lv2cFlexDirection>::Create({V_DROPDOWN_ENTRY(Lv2cFlexDirection, Row),
                                                                               V_DROPDOWN_ENTRY(Lv2cFlexDirection, Column)});
            this->observerHandles.push_back(
                element->ValueProperty.addObserver([this](Lv2cFlexDirection value)
                                                   {
                grid->Style().FlexDirection(value);
                grid->InvalidateLayout(); }));
            element->Style().Margin({8});
            element->Value(Lv2cFlexDirection::Row);
            controlGrid->AddChild(LabeledControl("FlexDirection", element));
        }
        {
            auto element = VariantDropdownElement<Lv2cFlexWrap>::Create(
                {V_DROPDOWN_ENTRY(Lv2cFlexWrap, Wrap),
                 V_DROPDOWN_ENTRY(Lv2cFlexWrap, NoWrap)});
            this->observerHandles.push_back(
                element->ValueProperty.addObserver([this](Lv2cFlexWrap value)
                                                   {
                grid->Style().FlexWrap(value);
                grid->InvalidateLayout(); }));
            element->Style().Margin({8});
            controlGrid->AddChild(LabeledControl("FlexWrap", element));
            element->Value(Lv2cFlexWrap::Wrap);
        }
        {
            auto element = VariantDropdownElement<Lv2cFlexJustification>::Create(
                {
                    V_DROPDOWN_ENTRY(Lv2cFlexJustification, Start),
                    V_DROPDOWN_ENTRY(Lv2cFlexJustification, End),
                    V_DROPDOWN_ENTRY(Lv2cFlexJustification, Center),
                    V_DROPDOWN_ENTRY(Lv2cFlexJustification, SpaceBetween),
                    V_DROPDOWN_ENTRY(Lv2cFlexJustification, SpaceAround),
                });
            this->observerHandles.push_back(
                element->ValueProperty.addObserver([this](Lv2cFlexJustification value)
                                                   {
                grid->Style().FlexJustification(value);
                grid->InvalidateLayout(); }));
            element->Style().Margin({8});
            controlGrid->AddChild(LabeledControl("FlexJustification", element));
            element->Value(Lv2cFlexJustification::Start);
        }
        {
            auto element = VariantDropdownElement<Lv2cAlignment>::Create(
                {
                    V_DROPDOWN_ENTRY(Lv2cAlignment, Start),
                    V_DROPDOWN_ENTRY(Lv2cAlignment, End),
                    V_DROPDOWN_ENTRY(Lv2cAlignment, Center),
                });
            this->observerHandles.push_back(
                element->ValueProperty.addObserver([this](Lv2cAlignment value)
                                                   {
                grid->Style().FlexAlignItems(value);
                grid->InvalidateLayout(); }));
            element->Style().Margin({8});
            controlGrid->AddChild(LabeledControl("FlexAlignItems", element));
            element->Value(Lv2cAlignment::Start);
        }
        {
            auto element = Lv2cNumericEditBoxElement::Create();
            element->ValueType(Lv2cValueType::Int16)
                .MinValue(0)
                .MaxValue(20000);
            element->Value(0);
            element->Style().Width(100);
            observerHandles.push_back(
            element->ValueProperty.addObserver(
                [this](double value)
                {
                    grid->Style().RowGap(value);
                    grid->InvalidateLayout();
                })
            );
            controlGrid->AddChild(LabeledEditControl("RowGap", element));
        }
        {
            auto element = Lv2cNumericEditBoxElement::Create();
            element->ValueType(Lv2cValueType::Int16)
                .MinValue(0)
                .MaxValue(20000);
            element->Style().Width(100);
            element->Value(0);
            observerHandles.push_back(
            element->ValueProperty.addObserver(
                [this](double value)
                {
                    grid->Style().ColumnGap(value);
                    grid->InvalidateLayout();
                })
            );
            controlGrid->AddChild(LabeledEditControl("ColumnGap", element));
        }

        stack->AddChild(controlGrid);
    }
    AddChild(stack);
}

Lv2cFlexGridElement::ptr FlexTestElement::MakeGrid()
{

    Style().HorizontalAlignment(Lv2cAlignment::Stretch);
    Style().VerticalAlignment(Lv2cAlignment::Stretch);

    auto element = Lv2cFlexGridElement::Create();
    auto &style = element->Style();
    style
        .FlexWrap(Lv2cFlexWrap::Wrap)
        .FlexDirection(Lv2cFlexDirection::Row)
        .FlexJustification(Lv2cFlexJustification::Start)
        .FlexAlignItems(Lv2cAlignment::Start)
        .BorderWidth({1})
        .BorderColor(Lv2cColor(1, 1, 1, 0.2))
        .Padding({1})
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .HorizontalAlignment(Lv2cAlignment::Stretch);

    std::vector<Lv2cElement::ptr> flexChildren = MakeFlexChildren();
    flexChildren[9]->Style().FontSize(Lv2cMeasurement::Point(17));

    for (auto &child : flexChildren)
    {
        element->AddChild(child);
    }
    return element;
}

Lv2cElement::ptr FlexGridTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    Lv2cVerticalStackElement::ptr main = Lv2cVerticalStackElement::Create();

    main->Style()
        .HorizontalAlignment(Lv2cAlignment::Start)
        .Background(theme->paper)
        .Padding({16, 8, 16, 8})
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .HorizontalAlignment(Lv2cAlignment::Stretch);

    {
        Lv2cTypographyElement::ptr title = Lv2cTypographyElement::Create();
        title->Variant(Lv2cTypographyVariant::Title).Text("Flex-Grid Test");

        title->Style().Padding({0, 8, 0, 8});
        main->AddChild(title);
    }
    {
        auto element = FlexTestElement::Create();

        main->AddChild(element);
    }

    return main;
}
