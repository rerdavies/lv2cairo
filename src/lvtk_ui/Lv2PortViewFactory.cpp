#include "lvtk_ui/Lv2PortViewFactory.hpp"

#include "lvtk_ui/Lv2PortView.hpp"

#include "lvtk/LvtkContainerElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkDropdownElement.hpp"
#include "lvtk/LvtkButtonBaseElement.hpp"
#include "lvtk/LvtkNumericEditBoxElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkDialElement.hpp"
#include "lvtk/LvtkSwitchElement.hpp"
#include "lvtk/LvtkOnOffSwitchElement.hpp"
#include "lvtk/LvtkDialElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "lvtk_ui/Lv2ControlConstants.hpp"
#include "lvtk/LvtkVuElement.hpp"
#include "lvtk/LvtkDbVuElement.hpp"
#include "lvtk/LvtkLampElement.hpp"
#include "lvtk/LvtkStatusTextElement.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkTheme.hpp"
#include "lvtk_ui/Lv2TunerElement.hpp"

#define XK_MISCELLANY
#include "X11/keysymdef.h"

using namespace lvtk;
using namespace lvtk::ui;

double Lv2PortViewFactory::EditControlHeight() const
{
    constexpr double EDIT_CONTROL_HEIGHT = 31;
    return EDIT_CONTROL_HEIGHT;
}

double Lv2PortViewFactory::GetControlWidth(Lv2PortViewController *viewController, double defaultControlWidth)
{
    Lv2PortViewType viewType = viewController->GetViewType();
    double width = defaultControlWidth;
    if (viewType == Lv2PortViewType::Tuner)
    {
        width  *= 2;
    } else if (viewType == Lv2PortViewType::Dropdown || viewType == Lv2PortViewType::StatusOutputMessage)
    {
        width *= 2;
    }
    else if (viewType == Lv2PortViewType::VuMeter && viewController->Caption().length() <= 1)
    {
        width /= 4;
    }
    return width;
}
LvtkContainerElement::ptr Lv2PortViewFactory::CreateContainer(Lv2PortViewController *viewController)
{
    double width = GetControlWidth(viewController, Lv2ControlWidth);
    width = 0;

    auto stackElement = CreateContainer();
    stackElement->Style()
        .Width(width);
    return stackElement;
}

LvtkContainerElement::ptr Lv2PortViewFactory::CreateContainer()
{
    auto stackElement = LvtkVerticalStackElement::Create();
    stackElement->Style()
        .Height(Lv2ControlHeight)
        .HorizontalAlignment(LvtkAlignment::Start)
        .VerticalAlignment(LvtkAlignment::Start)
        //.Background(LvtkColor(1,0.5,0.5,0.1))
        .Margin({4, 0, 4, 0});
    return stackElement;

}

LvtkElement::ptr Lv2PortViewFactory::CreateCaption(const std::string&title,LvtkAlignment alignment)
{
    auto caption = LvtkTypographyElement::Create();
    caption->Variant(LvtkTypographyVariant::Caption);
    caption->Style()
        .Ellipsize(LvtkEllipsizeMode::End)
        .SingleLine(true)
        .HorizontalAlignment(alignment)
        .VerticalAlignment(LvtkAlignment::Start);
    caption->Text(title);
    return caption;


}


LvtkElement::ptr Lv2PortViewFactory::CreateCaption(Lv2PortViewController *viewController)
{
    Lv2PortViewType viewType = viewController->GetViewType();
    LvtkAlignment alignment = LvtkAlignment::Center;
    if (viewType == Lv2PortViewType::Dropdown || viewType == Lv2PortViewType::StatusOutputMessage)
    {
        alignment = LvtkAlignment::Start;
    }
    return CreateCaption(viewController->Caption(),alignment);
}

LvtkElement::ptr Lv2PortViewFactory::CreateControl(Lv2PortViewController *viewController)
{
    Lv2PortViewType viewType = viewController->GetViewType();
    switch (viewType)
    {
    case Lv2PortViewType::Dial:
        return CreateDial(viewController);
    case Lv2PortViewType::Dropdown:
    {
        std::vector<LvtkDropdownItem> items;
        selection_id_t nextId = 0;
        for (auto &scalePoint : viewController->PortInfo().scale_points())
        {
            LvtkDropdownItem item{nextId++, scalePoint.label()};
            items.push_back(std::move(item));
        }
        return CreateDropdown(viewController, items);
    }
    case Lv2PortViewType::Tuner:
        return CreateTuner(viewController);
    case Lv2PortViewType::OnOff:
        return CreateOnOff(viewController);
    case Lv2PortViewType::Toggle:
        return CreateToggle(viewController);

    case Lv2PortViewType::VuMeter:
        return CreateVuMeter(viewController);
    case Lv2PortViewType::StereoVuMeter:
        return CreateStereoVuMeter(viewController);
    case Lv2PortViewType::LED:
        return CreateLed(viewController);

    case Lv2PortViewType::StatusOutputMessage:
    {
        std::vector<LvtkDropdownItem> items;
        selection_id_t nextId = 0;
        for (auto &scalePoint : viewController->PortInfo().scale_points())
        {
            LvtkDropdownItem item{nextId++, scalePoint.label()};
            items.push_back(std::move(item));
        }

        return CreateStatusMessage(viewController, items);
    }
    default:
        return LvtkElement::Create();
    }
}

LvtkElement::ptr Lv2PortViewFactory::CreateTuner(Lv2PortViewController *viewController)
{
    auto element = Lv2TunerElement::Create();
    element->Style()
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center);

    element->ValueIsMidiNote(viewController->PortInfo().units() == Lv2Units::midiNote);
    viewController->PortValueProperty.Bind(element->ValueProperty);

    return element;
}

LvtkElement::ptr Lv2PortViewFactory::CreateToggle(Lv2PortViewController *viewController)
{
    auto element = LvtkSwitchElement::Create();

    element->Style()
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center);

    viewController->PortValueProperty.Bind(element->ValueProperty);

    return element;
}
LvtkElement::ptr Lv2PortViewFactory::CreateOnOff(Lv2PortViewController *viewController)
{
    auto element = LvtkOnOffSwitchElement::Create();
    element->Style()
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center);
    viewController->PortValueProperty.Bind(element->ValueProperty);
    return element;
}

LvtkElement::ptr Lv2PortViewFactory::CreateVuMeter(Lv2PortViewController *viewController)
{

    if (viewController->Units() == Lv2Units::db)
    {
        auto element = LvtkDbVuElement::Create();
        element->MaxValue(viewController->MaxValue());
        element->MinValue(viewController->MinValue());
        element->Style()
            .Height(0)
            .VerticalAlignment(LvtkAlignment::Stretch)
            .HorizontalAlignment(LvtkAlignment::Center)
            .Margin({0, 8, 0, 8});

        viewController->PortValueProperty.Bind(element->ValueProperty);
        return element;
    }
    else
    {
        auto element = LvtkVuElement::Create();
        element->MaxValue(viewController->MaxValue());
        element->MinValue(viewController->MinValue());
        element->Style()
            .Height(0)
            .VerticalAlignment(LvtkAlignment::Stretch)
            .HorizontalAlignment(LvtkAlignment::Center)
            .Margin({0, 8, 0, 8});

        viewController->PortValueProperty.Bind(element->ValueProperty);

        return element;
    }
}
LvtkElement::ptr Lv2PortViewFactory::CreateStereoVuMeter(Lv2PortViewController *viewController)
{

    if (viewController->Units() == Lv2Units::db)
    {
        auto element = LvtkStereoDbVuElement::Create();
        element->MaxValue(viewController->MaxValue());
        element->MinValue(viewController->MinValue());
        element->Style()
            .Height(0)
            .VerticalAlignment(LvtkAlignment::Stretch)
            .HorizontalAlignment(LvtkAlignment::Center)
            .Margin({0, 8, 0, 8});

        viewController->PortValueProperty.Bind(element->ValueProperty);
        viewController->RightPortValueProperty.Bind(element->RightValueProperty);
        return element;
    }
    else
    {
        auto element = LvtkStereoVuElement::Create();
        element->MaxValue(viewController->MaxValue());
        element->MinValue(viewController->MinValue());
        element->Style()
            .Height(0)
            .VerticalAlignment(LvtkAlignment::Stretch)
            .HorizontalAlignment(LvtkAlignment::Center)
            .Margin({0, 8, 0, 8});

        viewController->PortValueProperty.Bind(element->ValueProperty);
        viewController->RightPortValueProperty.Bind(element->RightValueProperty);

        return element;
    }
}

LvtkElement::ptr Lv2PortViewFactory::CreateDial(Lv2PortViewController *viewController)
{
    auto dial = LvtkDialElement::Create();

    // center the dial in the avilable space.
    dial->Style()
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center)
        .Width(48)
        .Height(48);

    // connect the value.
    viewController->DialValueProperty.Bind(dial->ValueProperty);
    viewController->IsDraggingProperty.Bind(dial->IsDraggingProperty);
    return dial;
}

LvtkElement::ptr Lv2PortViewFactory::CreateStatusMessage(Lv2PortViewController *viewController, const std::vector<LvtkDropdownItem> &items)
{
    auto element = LvtkStatusTextElement::Create();
    element->DropdownItems(items);
    element->Style()
        .HorizontalAlignment(LvtkAlignment::Start)
        .VerticalAlignment(LvtkAlignment::Center);
    viewController->DropdownValueProperty.Bind(element->SelectedIdProperty);
    return element;
}

LvtkElement::ptr Lv2PortViewFactory::CreateDropdown(
    Lv2PortViewController *viewController,
    const std::vector<LvtkDropdownItem> &dropdownItems)
{
    // occupy the entire center portion of the control.
    auto dropdown = LvtkDropdownElement::Create();
    dropdown->DropdownItems(dropdownItems);
    // center the dial in the avilable space.
    dropdown->Style()
        .HorizontalAlignment(LvtkAlignment::Start)
        .VerticalAlignment(LvtkAlignment::Center);

    // connect the value.
    viewController->DropdownValueProperty.Bind(dropdown->SelectedIdProperty);
    return dropdown;
}

Lv2PortViewFactory::ValueControlStyle Lv2PortViewFactory::GetValueControlStyle(Lv2PortViewController *viewController)
{
    auto viewType = viewController->GetViewType();
    switch (viewType)
    {
    case Lv2PortViewType::Dial:
        return ValueControlStyle::EditBox;
    case Lv2PortViewType::Toggle:
        return ValueControlStyle::Label;
    case Lv2PortViewType::VuMeter:
    case Lv2PortViewType::StereoVuMeter:
        return ValueControlStyle::Collapsed;
    default:
        return ValueControlStyle::Hidden; // makes it easier to control alignment.
    }
}

namespace lvtk::implementation
{
    class EditContainerElement : public LvtkContainerElement
    {
    public:
        using self = EditContainerElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create(Lv2PortViewController *viewController) { return std::make_shared<self>(viewController); }

        EditContainerElement(Lv2PortViewController *viewController);
        virtual ~EditContainerElement() noexcept;

    protected:
        virtual bool OnKeyDown(const LvtkKeyboardEventArgs &event) override;
        virtual void OnMount() override;

    private:
        virtual void OnEditBoxFocusLost();
        virtual void OnEditBoxFocus();

        bool displayEdit = false;
        void DisplayEdit(bool value);

        LvtkButtonBaseElement::ptr buttonBase;
        LvtkTypographyElement::ptr displayValue;
        LvtkNumericEditBoxElement::ptr editBox;

        EventHandle clickHandle;
        EventHandle editTextLostFocusHandle;
        EventHandle editTextFocusHandle;
    };
    class LabelContainerElement : public LvtkContainerElement
    {
    public:
        using self = LabelContainerElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create(Lv2PortViewController *viewController) { return std::make_shared<self>(viewController); }
        LabelContainerElement(Lv2PortViewController *viewController);

    protected:
        LvtkElement::ptr label;
        virtual void OnMount() override;
    };

    void LabelContainerElement::OnMount()
    {
        super::OnMount();
        label->Style().BorderColor(
            LvtkColor(Theme().secondaryTextColor, 0.25));
    }

    LabelContainerElement::LabelContainerElement(Lv2PortViewController *viewController)
    {
        auto displayValue = LvtkTypographyElement::Create();
        label = displayValue;
        displayValue->Variant(LvtkTypographyVariant::BodySecondary);
        displayValue->Style()
            .HorizontalAlignment(LvtkAlignment::Center)
            .Width(Lv2ControlWidth)
            .TextAlign(LvtkTextAlign::Center)
            .VerticalAlignment(LvtkAlignment::Center)
            .Padding({0, 4, 0, 4})
            .Margin({0, 2, 0, 2})
            .BorderWidth({0, 0, 0, 1});
        viewController->DisplayValueProperty.Bind(displayValue->TextProperty);
        AddChild(label);
    }
}

using namespace lvtk::implementation;
LvtkElement::ptr Lv2PortViewFactory::CreateEditArea(Lv2PortViewController *viewController)
{
    auto controlStyle = GetValueControlStyle(viewController);
    switch (controlStyle)
    {
    case ValueControlStyle::EditBox:
    {
        auto result = EditContainerElement::Create(viewController);
        result->Style().Height(this->EditControlHeight());
        return result;
    }
    case ValueControlStyle::Label:
    {
        auto result = LabelContainerElement::Create(viewController);
        result->Style().Height(this->EditControlHeight());
        return result;
    }
    case ValueControlStyle::Hidden:
    {
        auto element = LvtkElement::Create();
        element->Style()
            .Width(1)
            .Height(this->EditControlHeight());
        return element;
    }
    case ValueControlStyle::Collapsed:
    {
        auto element = LvtkElement::Create();
        element->Style()
            .Width(0)
            .Height(0)
            .HorizontalAlignment(LvtkAlignment::Center)
            .VerticalAlignment(LvtkAlignment::Start);
        return element;
    }
    default:
        throw std::runtime_error("Unhandled case.");
    }
}
EditContainerElement::EditContainerElement(Lv2PortViewController *viewController)
{

    auto buttonBase = LvtkButtonBaseElement::Create();
    this->AddChild(buttonBase);
    {
        this->buttonBase = buttonBase;

        this->clickHandle = buttonBase->Clicked.AddListener(
            [this](const LvtkMouseEventArgs &args)
            {
                this->DisplayEdit(true);
                return true;
            });
        {

            auto displayValue = LvtkTypographyElement::Create();
            this->displayValue = displayValue;
            buttonBase->AddChild(displayValue);
            buttonBase->Style().RoundCorners({4, 8, 0, 0});

            displayValue->Variant(LvtkTypographyVariant::BodySecondary);
            displayValue->Style()
                .HorizontalAlignment(LvtkAlignment::Center)
                .Width(Lv2ControlWidth)
                .TextAlign(LvtkTextAlign::Center)
                .VerticalAlignment(LvtkAlignment::Center)
                .Padding({0, 4, 0, 4})
                .Margin({0, 2, 0, 2})
                .BorderWidth({0, 0, 0, 1});
            viewController->DisplayValueProperty.Bind(displayValue->TextProperty);
        }

        {
            auto editbox = LvtkNumericEditBoxElement::Create();
            this->editBox = editbox;
            AddChild(editbox);
            editBox->MaxValue(viewController->MaxValue());
            editBox->MinValue(viewController->MinValue());

            editBox->ValueType(viewController->IsInteger() ? LvtkValueType::Int64 : LvtkValueType::Double);
            editTextLostFocusHandle = editbox->LostFocusEvent.AddListener(
                [this](const LvtkFocusEventArgs &args)
                {
                    this->OnEditBoxFocusLost();
                    return false;
                }

            );

            editTextFocusHandle = editbox->FocusEvent.AddListener(
                [this](const LvtkFocusEventArgs &args)
                {
                    this->OnEditBoxFocus();
                    return false;
                }

            );

            editbox->Variant(EditBoxVariant::Underline);
            // displayArea->AddChild(editbox);

            editbox->Style()
                .HorizontalAlignment(LvtkAlignment::Center)
                .Width(Lv2ControlWidth)
                .TextAlign(LvtkTextAlign::Center)
                .VerticalAlignment(LvtkAlignment::Center)
                .Visibility(LvtkVisibility::Hidden);

            viewController->PortValueProperty.Bind(editBox->ValueProperty);
        }
    }
}

void EditContainerElement::OnMount()
{
    displayValue->Style().BorderColor(
        LvtkColor(Theme().secondaryTextColor, 0.25));
}

void EditContainerElement::OnEditBoxFocusLost()
{
    this->DisplayEdit(false);
}
void EditContainerElement::OnEditBoxFocus()
{
    this->DisplayEdit(true);
}

bool EditContainerElement::OnKeyDown(const LvtkKeyboardEventArgs &event)
{
    if (event.keysymValid && event.modifierState == ModifierState::Empty)
    {
        switch (event.keysym)
        {
        case XK_KP_Enter:
        case XK_Return:
        case XK_Escape:
        case XK_Cancel:
            this->Window()->Focus(nullptr);
            return true;
        default:
            break;
        }
    }
    return false;
}

void EditContainerElement::DisplayEdit(bool value)
{
    if (this->displayEdit != value)
    {
        this->displayEdit = value;
        if (displayEdit)
        {
            this->editBox->Style().Visibility(LvtkVisibility::Visible);
            this->buttonBase->Style().Visibility(LvtkVisibility::Hidden);
            this->editBox->Focus();

            this->editBox->SelectAll();
        }
        else
        {
            this->editBox->Style().Visibility(LvtkVisibility::Hidden);
            this->buttonBase->Style().Visibility(LvtkVisibility::Visible);
            this->editBox->ReleaseFocus();
        }
        this->editBox->Invalidate();
        this->displayValue->Invalidate();
    }
}

EditContainerElement::~EditContainerElement() noexcept
{
    if (editBox && editTextFocusHandle)
    {
        editBox->LostFocusEvent.RemoveListener(editTextLostFocusHandle);
        editBox->FocusEvent.RemoveListener(editTextFocusHandle);
        editTextLostFocusHandle = EventHandle::InvalidHandle;
        editTextFocusHandle = EventHandle::InvalidHandle;
    }

    if (buttonBase && clickHandle)
    {
        buttonBase->Clicked.RemoveListener(clickHandle);
        clickHandle = EventHandle::InvalidHandle;
    }
}

LvtkElement::ptr Lv2PortViewFactory::CreateLed(Lv2PortViewController *viewController)
{
    auto element = LvtkLampElement::Create();
    element->Variant(LvtkLampVariant::OnOff);
    element->Style()
        .HorizontalAlignment(LvtkAlignment::Center)
        .VerticalAlignment(LvtkAlignment::Center);
    viewController->PortValueProperty.Bind(element->ValueProperty);
    return element;
}
LvtkContainerElement::ptr Lv2PortViewFactory::CreatePage()
{

    auto element = LvtkFlexGridElement::Create();
    element->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch)
        .Padding({24, 16, 24, 0})
        .RowGap(16)
        .ColumnGap(8)
        .FlexAlignItems(LvtkAlignment::Center)
        .Background(Theme().paper)
        .FlexWrap(LvtkFlexWrap::Wrap)
        // .BorderWidth({1})
        // .BorderColor(LvtkColor("#FF8080"))
        ;
    return element;
}

LvtkTheme &Lv2PortViewFactory::Theme()
{
    return *(theme.get());
}
Lv2PortViewFactory &Lv2PortViewFactory::Theme(std::shared_ptr<LvtkTheme> theme)
{
    this->theme = theme;
    return *this;
}

LvtkSize Lv2PortViewFactory::DefaultSize() const
{
    return LvtkSize(Lv2ControlWidth ,Lv2ControlHeight);

}
