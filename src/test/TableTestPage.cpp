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

#include "TableTestPage.hpp"
#include "lvtk/LvtkTableElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkDropdownElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkBindingProperty.hpp"
#include "lvtk/LvtkNumericEditBoxElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"

using namespace lvtk;

constexpr size_t TABLE_ROWS = 5;

static std::vector<LvtkElement::ptr> MakeTableChildren()
{
    std::vector<LvtkElement::ptr> result;

    auto style = LvtkStyle::Create();

    for (size_t r = 0; r < TABLE_ROWS; ++r)
    {
        LvtkElement::ptr element;

        element = LvtkElement::Create();
        element->Style()
            .Width(60 + ((r & 1) ? 20 : 0))
            .Height(20)
            .Background(LvtkColor("#FF8080"));
        result.push_back(element);

        element = LvtkElement::Create();
        element->Style()
            .Width(60.0+ ((r % 3) * 20.0))
            .Height(20)
            .Background(LvtkColor("#80FF80"));
        result.push_back(element);

        element = LvtkElement::Create();
        element->Style()
            .Width(60.0 + ((r % 4) *10.0))
            .Height(20)
            .Background(LvtkColor("#8080FF"));
        result.push_back(element);

        element = LvtkElement::Create();
        element->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .Height(r == 0 ? 40 : 20)
            .Background(LvtkColor("#804040"));
        result.push_back(element);

        element = LvtkElement::Create();
        element->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .Height(20.0 + ((r & 1) ? 30.0f : 0.0f))
            .Background(LvtkColor("#408040"));
        result.push_back(element);
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
    virtual void OnDropdownItemsChanged(const items_t &value) override
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

class TableTestElement : public LvtkContainerElement
{
public:
    using self = TableTestElement;
    using super = LvtkContainerElement;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }
    TableTestElement();

private:
    LvtkTableElement::ptr MakeTable();
    LvtkTableElement::ptr table;
    std::vector<observer_handle_t> observerHandles;
};

#pragma GCC diagnostic ignored "-Wunused-function"
static LvtkElement::ptr LabeledControl(const std::string &label_, LvtkElement::ptr control)
{
    auto container = LvtkVerticalStackElement::Create();
    container->Style().HorizontalAlignment(LvtkAlignment::Start);
    auto label = LvtkTypographyElement::Create();
    label->Text(label_);
    label->Style().FontSize(11).Margin({6, 0, 0, 0});
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
    label->Style().FontSize(11).Margin({4, 0, 0, 8});
    container->AddChild(label);
    container->AddChild(control);
    container->Style().Margin({4});
    return container;
}
TableTestElement::TableTestElement()
{
    auto stack = LvtkVerticalStackElement::Create();

    Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch)
        ;
    stack->Style()
        .FlexDirection(LvtkFlexDirection::Column)
        .FlexWrap(LvtkFlexWrap::NoWrap)
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch);
    {
        {
            auto element = LvtkElement::Create();
            element->Style()
                .Width(LvtkMeasurement::Percent(100))
                .Height(1)
                .Margin({0, 8, 0, 0})
                .Background(LvtkColor(0.3, 0.3, 0.3));
            stack->AddChild(element);
        }


        this->table = MakeTable();
        table->Style()
            .Margin({0,16,0,16})
            ;
        stack->AddChild(table);


        auto controlGrid = LvtkFlexGridElement::Create();
        // {
        //     auto element = VariantDropdownElement<LvtkAlignment>::Create(
        //         {
        //             V_DROPDOWN_ENTRY(LvtkAlignment, Start),
        //             V_DROPDOWN_ENTRY(LvtkAlignment, End),
        //             V_DROPDOWN_ENTRY(LvtkAlignment, Center),
        //         });
        //     this->observerHandles.push_back(
        //         element->ValueProperty.addObserver([this](LvtkAlignment value)
        //                                            {
        //         table->Style().FlexAlignItems(value);
        //         table->InvalidateLayout(); }));
        //     element->Style().Margin({8});
        //     controlGrid->AddChild(LabeledControl("FlexAlignItems", element));
        //     element->Value(LvtkAlignment::Start);
        // }
        {
            auto element = LvtkNumericEditBoxElement::Create();
            element->ValueType(LvtkValueType::Int16)
                .MinValue(0)
                .MaxValue(20000);
            element->Value(1);
            element->Style().Width(100);
            observerHandles.push_back(
                element->ValueProperty.addObserver(
                    [this](double value)
                    {
                        table->Style().BorderWidth({value});
                        table->InvalidateLayout();
                    }));
            controlGrid->AddChild(LabeledEditControl("Border Width", element));
        }
        {
            auto element = LvtkNumericEditBoxElement::Create();
            element->ValueType(LvtkValueType::Int16)
                .MinValue(0)
                .MaxValue(20000);
            element->Style().Width(100);
            element->Value(2);
            observerHandles.push_back(
                element->ValueProperty.addObserver(
                    [this](double value)
                    {
                        table->Style().CellPadding({value});
                        table->InvalidateLayout();
                    }));
            controlGrid->AddChild(LabeledEditControl("Cell Padding", element));
        }

        stack->AddChild(controlGrid);
    }
    AddChild(stack);
}

LvtkTableElement::ptr TableTestElement::MakeTable()
{

    
    auto element = LvtkTableElement::Create();
    auto &style = element->Style();
    style
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .BorderWidth(1)
        .BorderColor(LvtkColor("#808080"))
        .CellPadding({2})
        ;

    std::vector<LvtkElement::ptr> tableChildren = MakeTableChildren();

    element->Children(tableChildren);
    element->ColumnDefinitions({
        {LvtkAlignment::Start, LvtkAlignment::Start, 150},
        {LvtkAlignment::Center, LvtkAlignment::Center, 0},
        {LvtkAlignment::End, LvtkAlignment::End, 0},
        {LvtkAlignment::Start, LvtkAlignment::Stretch, 2},
        {LvtkAlignment::End, LvtkAlignment::Stretch, 3},
    });

    return element;
}

LvtkElement::ptr TableTestPage::CreatePageView(LvtkTheme::ptr theme)
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
        title->Variant(LvtkTypographyVariant::Title).Text("Table Test");

        title->Style().Padding({0, 8, 0, 8});
        main->AddChild(title);
    }
    {
        auto element = TableTestElement::Create();

        main->AddChild(element);
    }

    return main;
}
