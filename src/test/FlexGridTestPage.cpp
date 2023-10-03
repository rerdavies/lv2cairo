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
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkDropdownElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkBindingProperty.hpp"
#include "lvtk/LvtkNumericEditBoxElement.hpp"

using namespace lvtk;

static std::vector<LvtkElement::ptr> MakeFlexChildren()
{
    std::vector<LvtkElement::ptr> result;
    std::vector<std::string> data{"Sed", "ut", "perspiciatis", "unde",
                                  "omnis", "iste", "natus", "error", "sit",
                                  "voluptatem", "accusantium", "doloremque", "laudantium"};

    auto style = LvtkStyle::Create();
    style->FontSize(LvtkMeasurement::Point(15))
        .Margin({1})
        .BorderWidth({1})
        .BorderColor(LvtkColor(1, 1, 1, 0.3))
        .Padding({4})
        .Color(LvtkColor(1, 1, 1))
        .TextAlign(LvtkTextAlign::Start);

    for (auto &word : data)
    {
        auto typography = LvtkTypographyElement::Create();
        typography->Text(word).AddClass(style);
        typography->Style()
            .Margin({1})
            .BorderWidth({1})
            .BorderColor(LvtkColor(1, 1, 1, 0.3))
            .Padding({4});
        result.push_back(typography);
    }
    return result;
}

template <typename T>
class VariantDropdownElement : public LvtkDropdownElement
{
public:
    using self = VariantDropdownElement<T>;
    using super = LvtkDropdownElement;
    using ptr = std::shared_ptr<self>;
    static ptr Create(const std::vector<LvtkDropdownItem> &items)
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
    LvtkDropdownItem((int64_t)(VARIANT::MEMBER), (#MEMBER))

class FlexTestElement : public LvtkContainerElement
{
public:
    using self = FlexTestElement;
    using super = LvtkContainerElement;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }
    FlexTestElement();

private:
    LvtkFlexGridElement::ptr MakeGrid();
    LvtkFlexGridElement::ptr grid;
    std::vector<observer_handle_t> observerHandles;
};

static LvtkElement::ptr LabeledControl(const std::string &label_, LvtkElement::ptr control)
{
    auto container = LvtkVerticalStackElement::Create();
    container->Style().HorizontalAlignment(LvtkAlignment::Start);
    auto label = LvtkTypographyElement::Create();
    label->Text(label_);
    label->Style().FontSize(11)
        .Margin({6,0,0,0});
    container->AddChild(label);
    container->AddChild(control);
    container->Style().Margin({4});
    return container;
}
static LvtkElement::ptr LabeledEditControl(const std::string &label_, LvtkElement::ptr control)
{
    auto container = LvtkVerticalStackElement::Create();
    container->Style().HorizontalAlignment(LvtkAlignment::Start);
    auto label = LvtkTypographyElement::Create();
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
    auto stack = LvtkFlexGridElement::Create();
    stack->Style().FlexDirection(LvtkFlexDirection::Column).FlexWrap(LvtkFlexWrap::NoWrap).HorizontalAlignment(LvtkAlignment::Stretch).VerticalAlignment(LvtkAlignment::Stretch);
    {
        this->grid = MakeGrid();
        stack->AddChild(grid);

        {
            auto element = LvtkElement::Create();
            element->Style()
                .Width(LvtkMeasurement::Percent(100))
                .Height(1)
                .Margin({0,8,0,0})
                .Background(LvtkColor(0.3,0.3,0.3));
            stack->AddChild(element);
        }

        auto controlGrid = LvtkFlexGridElement::Create();
        controlGrid->Style()
            .VerticalAlignment(LvtkAlignment::End)
            .FlexWrap(LvtkFlexWrap::Wrap);
        {
            auto element = VariantDropdownElement<LvtkFlexDirection>::Create({V_DROPDOWN_ENTRY(LvtkFlexDirection, Row),
                                                                               V_DROPDOWN_ENTRY(LvtkFlexDirection, Column)});
            this->observerHandles.push_back(
                element->ValueProperty.addObserver([this](LvtkFlexDirection value)
                                                   {
                grid->Style().FlexDirection(value);
                grid->InvalidateLayout(); }));
            element->Style().Margin({8});
            element->Value(LvtkFlexDirection::Row);
            controlGrid->AddChild(LabeledControl("FlexDirection", element));
        }
        {
            auto element = VariantDropdownElement<LvtkFlexWrap>::Create(
                {V_DROPDOWN_ENTRY(LvtkFlexWrap, Wrap),
                 V_DROPDOWN_ENTRY(LvtkFlexWrap, NoWrap)});
            this->observerHandles.push_back(
                element->ValueProperty.addObserver([this](LvtkFlexWrap value)
                                                   {
                grid->Style().FlexWrap(value);
                grid->InvalidateLayout(); }));
            element->Style().Margin({8});
            controlGrid->AddChild(LabeledControl("FlexWrap", element));
            element->Value(LvtkFlexWrap::Wrap);
        }
        {
            auto element = VariantDropdownElement<LvtkFlexJustification>::Create(
                {
                    V_DROPDOWN_ENTRY(LvtkFlexJustification, Start),
                    V_DROPDOWN_ENTRY(LvtkFlexJustification, End),
                    V_DROPDOWN_ENTRY(LvtkFlexJustification, Center),
                    V_DROPDOWN_ENTRY(LvtkFlexJustification, SpaceBetween),
                    V_DROPDOWN_ENTRY(LvtkFlexJustification, SpaceAround),
                });
            this->observerHandles.push_back(
                element->ValueProperty.addObserver([this](LvtkFlexJustification value)
                                                   {
                grid->Style().FlexJustification(value);
                grid->InvalidateLayout(); }));
            element->Style().Margin({8});
            controlGrid->AddChild(LabeledControl("FlexJustification", element));
            element->Value(LvtkFlexJustification::Start);
        }
        {
            auto element = VariantDropdownElement<LvtkAlignment>::Create(
                {
                    V_DROPDOWN_ENTRY(LvtkAlignment, Start),
                    V_DROPDOWN_ENTRY(LvtkAlignment, End),
                    V_DROPDOWN_ENTRY(LvtkAlignment, Center),
                });
            this->observerHandles.push_back(
                element->ValueProperty.addObserver([this](LvtkAlignment value)
                                                   {
                grid->Style().FlexAlignItems(value);
                grid->InvalidateLayout(); }));
            element->Style().Margin({8});
            controlGrid->AddChild(LabeledControl("FlexAlignItems", element));
            element->Value(LvtkAlignment::Start);
        }
        {
            auto element = LvtkNumericEditBoxElement::Create();
            element->ValueType(LvtkValueType::Int16)
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
            auto element = LvtkNumericEditBoxElement::Create();
            element->ValueType(LvtkValueType::Int16)
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

LvtkFlexGridElement::ptr FlexTestElement::MakeGrid()
{

    Style().HorizontalAlignment(LvtkAlignment::Stretch);
    Style().VerticalAlignment(LvtkAlignment::Stretch);

    auto element = LvtkFlexGridElement::Create();
    auto &style = element->Style();
    style
        .FlexWrap(LvtkFlexWrap::Wrap)
        .FlexDirection(LvtkFlexDirection::Row)
        .FlexJustification(LvtkFlexJustification::Start)
        .FlexAlignItems(LvtkAlignment::Start)
        .BorderWidth({1})
        .BorderColor(LvtkColor(1, 1, 1, 0.2))
        .Padding({1})
        .VerticalAlignment(LvtkAlignment::Stretch)
        .HorizontalAlignment(LvtkAlignment::Stretch);

    std::vector<LvtkElement::ptr> flexChildren = MakeFlexChildren();
    flexChildren[9]->Style().FontSize(LvtkMeasurement::Point(17));

    for (auto &child : flexChildren)
    {
        element->AddChild(child);
    }
    return element;
}

LvtkElement::ptr FlexGridTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    LvtkVerticalStackElement::ptr main = LvtkVerticalStackElement::Create();

    main->Style()
        .HorizontalAlignment(LvtkAlignment::Start)
        .Background(theme->paper)
        .Padding({16, 8, 16, 8})
        .VerticalAlignment(LvtkAlignment::Stretch)
        .HorizontalAlignment(LvtkAlignment::Stretch);

    {
        LvtkTypographyElement::ptr title = LvtkTypographyElement::Create();
        title->Variant(LvtkTypographyVariant::Title).Text("Flex-Grid Test");

        title->Style().Padding({0, 8, 0, 8});
        main->AddChild(title);
    }
    {
        auto element = FlexTestElement::Create();

        main->AddChild(element);
    }

    return main;
}
