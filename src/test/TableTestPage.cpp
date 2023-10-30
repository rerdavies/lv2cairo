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
#include "lv2c/Lv2cTableElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cDropdownElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cBindingProperty.hpp"
#include "lv2c/Lv2cNumericEditBoxElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"

using namespace lv2c;

constexpr size_t TABLE_ROWS = 5;

static std::vector<Lv2cElement::ptr> MakeTableChildren()
{
    std::vector<Lv2cElement::ptr> result;

    auto style = Lv2cStyle::Create();

    for (size_t r = 0; r < TABLE_ROWS; ++r)
    {
        Lv2cElement::ptr element;

        element = Lv2cElement::Create();
        element->Style()
            .Width(60 + ((r & 1) ? 20 : 0))
            .Height(20)
            .Background(Lv2cColor("#FF8080"));
        result.push_back(element);

        element = Lv2cElement::Create();
        element->Style()
            .Width(60.0+ ((r % 3) * 20.0))
            .Height(20)
            .Background(Lv2cColor("#80FF80"));
        result.push_back(element);

        element = Lv2cElement::Create();
        element->Style()
            .Width(60.0 + ((r % 4) *10.0))
            .Height(20)
            .Background(Lv2cColor("#8080FF"));
        result.push_back(element);

        element = Lv2cElement::Create();
        element->Style()
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .Height(r == 0 ? 40 : 20)
            .Background(Lv2cColor("#804040"));
        result.push_back(element);

        element = Lv2cElement::Create();
        element->Style()
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .Height(20.0 + ((r & 1) ? 30.0f : 0.0f))
            .Background(Lv2cColor("#408040"));
        result.push_back(element);
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
    Lv2cDropdownItem((int64_t)(VARIANT::MEMBER), (#MEMBER))

class TableTestElement : public Lv2cContainerElement
{
public:
    using self = TableTestElement;
    using super = Lv2cContainerElement;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }
    TableTestElement();

private:
    Lv2cTableElement::ptr MakeTable();
    Lv2cTableElement::ptr table;
    std::vector<observer_handle_t> observerHandles;
};

#pragma GCC diagnostic ignored "-Wunused-function"
static Lv2cElement::ptr LabeledControl(const std::string &label_, Lv2cElement::ptr control)
{
    auto container = Lv2cVerticalStackElement::Create();
    container->Style().HorizontalAlignment(Lv2cAlignment::Start);
    auto label = Lv2cTypographyElement::Create();
    label->Text(label_);
    label->Style().FontSize(11).Margin({6, 0, 0, 0});
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
    label->Style().FontSize(11).Margin({4, 0, 0, 8});
    container->AddChild(label);
    container->AddChild(control);
    container->Style().Margin({4});
    return container;
}
TableTestElement::TableTestElement()
{
    auto stack = Lv2cVerticalStackElement::Create();

    Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch)
        ;
    stack->Style()
        .FlexDirection(Lv2cFlexDirection::Column)
        .FlexWrap(Lv2cFlexWrap::NoWrap)
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch);
    {
        {
            auto element = Lv2cElement::Create();
            element->Style()
                .Width(Lv2cMeasurement::Percent(100))
                .Height(1)
                .Margin({0, 8, 0, 0})
                .Background(Lv2cColor(0.3, 0.3, 0.3));
            stack->AddChild(element);
        }


        this->table = MakeTable();
        table->Style()
            .Margin({0,16,0,16})
            ;
        stack->AddChild(table);


        auto controlGrid = Lv2cFlexGridElement::Create();
        // {
        //     auto element = VariantDropdownElement<Lv2cAlignment>::Create(
        //         {
        //             V_DROPDOWN_ENTRY(Lv2cAlignment, Start),
        //             V_DROPDOWN_ENTRY(Lv2cAlignment, End),
        //             V_DROPDOWN_ENTRY(Lv2cAlignment, Center),
        //         });
        //     this->observerHandles.push_back(
        //         element->ValueProperty.addObserver([this](Lv2cAlignment value)
        //                                            {
        //         table->Style().FlexAlignItems(value);
        //         table->InvalidateLayout(); }));
        //     element->Style().Margin({8});
        //     controlGrid->AddChild(LabeledControl("FlexAlignItems", element));
        //     element->Value(Lv2cAlignment::Start);
        // }
        {
            auto element = Lv2cNumericEditBoxElement::Create();
            element->ValueType(Lv2cValueType::Int16)
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
            auto element = Lv2cNumericEditBoxElement::Create();
            element->ValueType(Lv2cValueType::Int16)
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

Lv2cTableElement::ptr TableTestElement::MakeTable()
{

    
    auto element = Lv2cTableElement::Create();
    auto &style = element->Style();
    style
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .BorderWidth(1)
        .BorderColor(Lv2cColor("#808080"))
        .CellPadding({2})
        ;

    std::vector<Lv2cElement::ptr> tableChildren = MakeTableChildren();

    element->Children(tableChildren);
    element->ColumnDefinitions({
        {Lv2cAlignment::Start, Lv2cAlignment::Start, 150},
        {Lv2cAlignment::Center, Lv2cAlignment::Center, 0},
        {Lv2cAlignment::End, Lv2cAlignment::End, 0},
        {Lv2cAlignment::Start, Lv2cAlignment::Stretch, 2},
        {Lv2cAlignment::End, Lv2cAlignment::Stretch, 3},
    });

    return element;
}

Lv2cElement::ptr TableTestPage::CreatePageView(Lv2cTheme::ptr theme)
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
        title->Variant(Lv2cTypographyVariant::Title).Text("Table Test");

        title->Style().Padding({0, 8, 0, 8});
        main->AddChild(title);
    }
    {
        auto element = TableTestElement::Create();

        main->AddChild(element);
    }

    return main;
}
