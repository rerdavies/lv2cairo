#include "lv2c_ui/Lv2PortViewFactory.hpp"

#include "lv2c_ui/Lv2PortView.hpp"

#include "lv2c/Lv2cContainerElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cDropdownElement.hpp"
#include "lv2c/Lv2cButtonBaseElement.hpp"
#include "lv2c/Lv2cNumericEditBoxElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cDialElement.hpp"
#include "lv2c/Lv2cSwitchElement.hpp"
#include "lv2c/Lv2cOnOffSwitchElement.hpp"
#include "lv2c/Lv2cDialElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "lv2c_ui/Lv2ControlConstants.hpp"
#include "lv2c_ui/Lv2MomentaryButtonElement.hpp"
#include "lv2c_ui/Lv2TextOutputElement.hpp"
#include "lv2c/Lv2cVuElement.hpp"
#include "lv2c/Lv2cProgressElement.hpp"
#include "lv2c/Lv2cDbVuElement.hpp"
#include "lv2c/Lv2cLampElement.hpp"
#include "lv2c/Lv2cStatusTextElement.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cTheme.hpp"
#include "lv2c_ui/Lv2TunerElement.hpp"

#define XK_MISCELLANY
#include "X11/keysymdef.h"

using namespace lv2c;
using namespace lv2c::ui;

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
Lv2cContainerElement::ptr Lv2PortViewFactory::CreateContainer(Lv2PortViewController *viewController)
{
    double width = GetControlWidth(viewController, Lv2ControlWidth);
    width = 0;

    auto stackElement = CreateContainer();
    stackElement->Style()
        .Width(width);
    return stackElement;
}

Lv2cContainerElement::ptr Lv2PortViewFactory::CreateContainer()
{
    auto stackElement = Lv2cVerticalStackElement::Create();
    stackElement->Style()
        .Height(Lv2ControlHeight)
        .HorizontalAlignment(Lv2cAlignment::Start)
        .VerticalAlignment(Lv2cAlignment::Start)
        //.Background(Lv2cColor(1,0.5,0.5,0.1))
        .Margin({4, 0, 4, 0});
    return stackElement;

}

Lv2cElement::ptr Lv2PortViewFactory::CreateCaption(const std::string&title,Lv2cAlignment alignment)
{
    auto caption = Lv2cTypographyElement::Create();
    caption->Variant(Lv2cTypographyVariant::Caption);
    caption->Style()
        .Ellipsize(Lv2cEllipsizeMode::End)
        .SingleLine(true)
        .HorizontalAlignment(alignment)
        .VerticalAlignment(Lv2cAlignment::Start);
    caption->Text(title);
    return caption;


}


Lv2cElement::ptr Lv2PortViewFactory::CreateCaption(Lv2PortViewController *viewController)
{
    Lv2PortViewType viewType = viewController->GetViewType();
    Lv2cAlignment alignment = Lv2cAlignment::Center;
    if (viewType == Lv2PortViewType::Dropdown || viewType == Lv2PortViewType::StatusOutputMessage)
    {
        alignment = Lv2cAlignment::Start;
    }
    switch (viewController->GetViewType())
    {
    case Lv2PortViewType::Trigger:
    case Lv2PortViewType::Momentary:
    case Lv2PortViewType::MomentaryOnByDefault:
        return CreateCaption(" ",alignment);

    default:
        return CreateCaption(viewController->Caption(),alignment);
    }
}

Lv2cElement::ptr Lv2PortViewFactory::CreateControl(Lv2PortViewController *viewController)
{
    Lv2PortViewType viewType = viewController->GetViewType();
    switch (viewType)
    {
    case Lv2PortViewType::Dial:
        return CreateDial(viewController);
    case Lv2PortViewType::Dropdown:
    {
        std::vector<Lv2cDropdownItem> items;
        selection_id_t nextId = 0;
        for (auto &scalePoint : viewController->PortInfo().scale_points())
        {
            Lv2cDropdownItem item{nextId++, scalePoint.label()};
            items.push_back(std::move(item));
        }
        return CreateDropdown(viewController, items);
    }
    case Lv2PortViewType::Trigger: 
        return CreateMomentary(viewController);
    case Lv2PortViewType::Momentary:
        return CreateMomentary(viewController);
    case Lv2PortViewType::MomentaryOnByDefault:
        return CreateMomentary(viewController);

    case Lv2PortViewType::Tuner:
        return CreateTuner(viewController);
    case Lv2PortViewType::OnOff:
        return CreateOnOff(viewController);
    case Lv2PortViewType::Toggle:
        return CreateToggle(viewController);

    case Lv2PortViewType::Progress:
        return CreateProgressBar(viewController);
    case Lv2PortViewType::VuMeter:
        return CreateVuMeter(viewController);
    case Lv2PortViewType::StereoVuMeter:
        return CreateStereoVuMeter(viewController);
    case Lv2PortViewType::LED:
        return CreateLed(viewController);

    case Lv2PortViewType::StatusOutputMessage:
    {
        std::vector<Lv2cDropdownItem> items;
        selection_id_t nextId = 0;
        for (auto &scalePoint : viewController->PortInfo().scale_points())
        {
            Lv2cDropdownItem item{nextId++, scalePoint.label()};
            items.push_back(std::move(item));
        }

        return CreateStatusMessage(viewController, items);
    }
    case Lv2PortViewType::TextOutput:
    {
        return CreateTextOutputMessage(viewController);
    }   
    default:
        return Lv2cElement::Create();
    }
}

Lv2cElement::ptr Lv2PortViewFactory::CreateTuner(Lv2PortViewController *viewController)
{
    auto element = Lv2TunerElement::Create();
    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center);

    element->ValueIsMidiNote(viewController->PortInfo().units() == Lv2Units::midiNote);
    viewController->PortValueProperty.Bind(element->ValueProperty);

    return element;
}

Lv2cElement::ptr Lv2PortViewFactory::CreateMomentary(
    Lv2PortViewController *viewController)
{
    auto element = Lv2MomentaryButtonElement::Create();
    auto viewType = viewController->GetViewType();

    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center);

    element->MinValue(viewController->PortInfo().min_value());
    element->MaxValue(viewController->PortInfo().max_value());
    Lv2MomentaryButtonElement::ButtonType buttonType;
    double defaultValue = viewController->PortInfo().min_value();
    switch (viewType)
    {   
        case Lv2PortViewType::Momentary:
        buttonType = Lv2MomentaryButtonElement::ButtonType::Momentary;
        break;
    case Lv2PortViewType::MomentaryOnByDefault:
        buttonType = Lv2MomentaryButtonElement::ButtonType::MomentaryOnByDefault;
        defaultValue = viewController->PortInfo().max_value();
        break;
    case Lv2PortViewType::Trigger:
        buttonType = Lv2MomentaryButtonElement::ButtonType::Trigger;
        break;
    default:
        throw std::runtime_error("Invalid momentary button type.");
    }
    element->SetButtonType(buttonType);
    element->Value(defaultValue);
    element->Text(viewController->Caption());

    viewController->PortValueProperty.Bind(element->ValueProperty);

    return element;
}

Lv2cElement::ptr Lv2PortViewFactory::CreateToggle(Lv2PortViewController *viewController)
{
    auto element = Lv2cSwitchElement::Create();

    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center);

    viewController->PortValueProperty.Bind(element->ValueProperty);

    return element;
}
Lv2cElement::ptr Lv2PortViewFactory::CreateOnOff(Lv2PortViewController *viewController)
{
    auto element = Lv2cOnOffSwitchElement::Create();
    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center);
    viewController->PortValueProperty.Bind(element->ValueProperty);
    return element;
}

Lv2cElement::ptr Lv2PortViewFactory::CreateProgressBar(Lv2PortViewController *viewController)
{
    auto element = Lv2cProgressElement::Create();
    element->MaxValue(viewController->MaxValue());
    element->MinValue(viewController->MinValue());
    element->Style()
        .VerticalAlignment(Lv2cAlignment::Center)
        .HorizontalAlignment(Lv2cAlignment::Center)
        .Margin({0, 8, 0, 8});

    viewController->PortValueProperty.Bind(element->ValueProperty);

    return element;
}

Lv2cElement::ptr Lv2PortViewFactory::CreateVuMeter(Lv2PortViewController *viewController)
{
    if (viewController->Units() == Lv2Units::db)
    {
        auto element = Lv2cDbVuElement::Create();
        element->MaxValue(viewController->MaxValue());
        element->MinValue(viewController->MinValue());
        element->Style()
            .Height(0)
            .VerticalAlignment(Lv2cAlignment::Stretch)
            .HorizontalAlignment(Lv2cAlignment::Center)
            .Margin({0, 8, 0, 8});

        viewController->PortValueProperty.Bind(element->ValueProperty);
        return element;
    }
    else
    {
        auto element = Lv2cVuElement::Create();
        element->MaxValue(viewController->MaxValue());
        element->MinValue(viewController->MinValue());
        element->Style()
            .Height(0)
            .VerticalAlignment(Lv2cAlignment::Stretch)
            .HorizontalAlignment(Lv2cAlignment::Center)
            .Margin({0, 8, 0, 8});

        viewController->PortValueProperty.Bind(element->ValueProperty);

        return element;
    }
}
Lv2cElement::ptr Lv2PortViewFactory::CreateStereoVuMeter(Lv2PortViewController *viewController)
{

    if (viewController->Units() == Lv2Units::db)
    {
        auto element = Lv2cStereoDbVuElement::Create();
        element->MaxValue(viewController->MaxValue());
        element->MinValue(viewController->MinValue());
        element->Style()
            .Height(0)
            .VerticalAlignment(Lv2cAlignment::Stretch)
            .HorizontalAlignment(Lv2cAlignment::Center)
            .Margin({0, 8, 0, 8});

        viewController->PortValueProperty.Bind(element->ValueProperty);
        viewController->RightPortValueProperty.Bind(element->RightValueProperty);
        return element;
    }
    else
    {
        auto element = Lv2cStereoVuElement::Create();
        element->MaxValue(viewController->MaxValue());
        element->MinValue(viewController->MinValue());
        element->Style()
            .Height(0)
            .VerticalAlignment(Lv2cAlignment::Stretch)
            .HorizontalAlignment(Lv2cAlignment::Center)
            .Margin({0, 8, 0, 8});

        viewController->PortValueProperty.Bind(element->ValueProperty);
        viewController->RightPortValueProperty.Bind(element->RightValueProperty);

        return element;
    }
}

Lv2cElement::ptr Lv2PortViewFactory::CreateDial(Lv2PortViewController *viewController)
{
    auto dial = Lv2cDialElement::Create();

    // center the dial in the avilable space.
    dial->Style()
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center)
        .Width(48)
        .Height(48);

    // connect the value.
    viewController->DialValueProperty.Bind(dial->ValueProperty);
    viewController->IsDraggingProperty.Bind(dial->IsDraggingProperty);
    return dial;
}

Lv2cElement::ptr Lv2PortViewFactory::CreateTextOutputMessage(Lv2PortViewController *viewController)
{
    auto element = Lv2TextOutputElement::Create(&viewController->PortInfo());
    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Start)
        .VerticalAlignment(Lv2cAlignment::Center);
    viewController->DisplayValueProperty.Bind(element->DisplayValueProperty);
    return element;
}

Lv2cElement::ptr Lv2PortViewFactory::CreateStatusMessage(Lv2PortViewController *viewController, const std::vector<Lv2cDropdownItem> &items)
{
    auto element = Lv2cStatusTextElement::Create();
    element->DropdownItems(items);
    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Start)
        .VerticalAlignment(Lv2cAlignment::Center);
    viewController->DropdownValueProperty.Bind(element->SelectedIdProperty);
    return element;
}

Lv2cElement::ptr Lv2PortViewFactory::CreateDropdown(
    Lv2PortViewController *viewController,
    const std::vector<Lv2cDropdownItem> &dropdownItems)
{
    // occupy the entire center portion of the control.
    auto dropdown = Lv2cDropdownElement::Create();
    dropdown->DropdownItems(dropdownItems);
    // center the dial in the avilable space.
    dropdown->Style()
        .HorizontalAlignment(Lv2cAlignment::Start)
        .VerticalAlignment(Lv2cAlignment::Center);

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

namespace lv2c::implementation
{
    class EditContainerElement : public Lv2cContainerElement
    {
    public:
        using self = EditContainerElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create(Lv2PortViewController *viewController) { return std::make_shared<self>(viewController); }

        EditContainerElement(Lv2PortViewController *viewController);
        virtual ~EditContainerElement() noexcept;

    protected:
        virtual bool OnKeyDown(const Lv2cKeyboardEventArgs &event) override;
        virtual void OnMount() override;

    private:
        virtual void OnEditBoxFocusLost();
        virtual void OnEditBoxFocus();

        bool displayEdit = false;
        void DisplayEdit(bool value);

        Lv2cButtonBaseElement::ptr buttonBase;
        Lv2cTypographyElement::ptr displayValue;
        Lv2cNumericEditBoxElement::ptr editBox;

        EventHandle clickHandle;
        EventHandle editTextLostFocusHandle;
        EventHandle editTextFocusHandle;
    };
    class LabelContainerElement : public Lv2cContainerElement
    {
    public:
        using self = LabelContainerElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create(Lv2PortViewController *viewController) { return std::make_shared<self>(viewController); }
        LabelContainerElement(Lv2PortViewController *viewController);

    protected:
        Lv2cElement::ptr label;
        virtual void OnMount() override;
    };

    void LabelContainerElement::OnMount()
    {
        super::OnMount();
        label->Style().BorderColor(
            Lv2cColor(Theme().secondaryTextColor, 0.25));
    }

    LabelContainerElement::LabelContainerElement(Lv2PortViewController *viewController)
    {
        auto displayValue = Lv2cTypographyElement::Create();
        label = displayValue;
        displayValue->Variant(Lv2cTypographyVariant::BodySecondary);
        displayValue->Style()
            .HorizontalAlignment(Lv2cAlignment::Center)
            .Width(Lv2ControlWidth)
            .TextAlign(Lv2cTextAlign::Center)
            .VerticalAlignment(Lv2cAlignment::Center)
            .Padding({0, 4, 0, 4})
            .Margin({0, 2, 0, 2})
            .BorderWidth({0, 0, 0, 1});
        viewController->DisplayValueProperty.Bind(displayValue->TextProperty);
        AddChild(label);
    }
}

using namespace lv2c::implementation;
Lv2cElement::ptr Lv2PortViewFactory::CreateEditArea(Lv2PortViewController *viewController)
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
        auto element = Lv2cElement::Create();
        element->Style()
            .Width(1)
            .Height(this->EditControlHeight());
        return element;
    }
    case ValueControlStyle::Collapsed:
    {
        auto element = Lv2cElement::Create();
        element->Style()
            .Width(0)
            .Height(0)
            .HorizontalAlignment(Lv2cAlignment::Center)
            .VerticalAlignment(Lv2cAlignment::Start);
        return element;
    }
    default:
        throw std::runtime_error("Unhandled case.");
    }
}
EditContainerElement::EditContainerElement(Lv2PortViewController *viewController)
{

    auto buttonBase = Lv2cButtonBaseElement::Create();
    this->AddChild(buttonBase);
    {
        this->buttonBase = buttonBase;

        this->clickHandle = buttonBase->Clicked.AddListener(
            [this](const Lv2cMouseEventArgs &args)
            {
                this->DisplayEdit(true);
                return true;
            });
        {

            auto displayValue = Lv2cTypographyElement::Create();
            this->displayValue = displayValue;
            buttonBase->AddChild(displayValue);
            buttonBase->Style().RoundCorners({4, 8, 0, 0});

            displayValue->Variant(Lv2cTypographyVariant::BodySecondary);
            displayValue->Style()
                .HorizontalAlignment(Lv2cAlignment::Center)
                .Width(Lv2ControlWidth)
                .TextAlign(Lv2cTextAlign::Center)
                .VerticalAlignment(Lv2cAlignment::Center)
                .Padding({0, 4, 0, 4})
                .Margin({0, 2, 0, 2})
                .BorderWidth({0, 0, 0, 1});
            viewController->DisplayValueProperty.Bind(displayValue->TextProperty);
        }

        {
            auto editbox = Lv2cNumericEditBoxElement::Create();
            this->editBox = editbox;
            AddChild(editbox);
            editBox->MaxValue(viewController->MaxValue());
            editBox->MinValue(viewController->MinValue());

            editBox->ValueType(viewController->IsInteger() ? Lv2cValueType::Int64 : Lv2cValueType::Double);
            editTextLostFocusHandle = editbox->LostFocusEvent.AddListener(
                [this](const Lv2cFocusEventArgs &args)
                {
                    this->OnEditBoxFocusLost();
                    return false;
                }

            );

            editTextFocusHandle = editbox->FocusEvent.AddListener(
                [this](const Lv2cFocusEventArgs &args)
                {
                    this->OnEditBoxFocus();
                    return false;
                }

            );

            editbox->Variant(EditBoxVariant::Underline);
            // displayArea->AddChild(editbox);

            editbox->Style()
                .HorizontalAlignment(Lv2cAlignment::Center)
                .Width(Lv2ControlWidth)
                .TextAlign(Lv2cTextAlign::Center)
                .VerticalAlignment(Lv2cAlignment::Center)
                .Visibility(Lv2cVisibility::Hidden);

            viewController->PortValueProperty.Bind(editBox->ValueProperty);
        }
    }
}

void EditContainerElement::OnMount()
{
    displayValue->Style().BorderColor(
        Lv2cColor(Theme().secondaryTextColor, 0.25));
}

void EditContainerElement::OnEditBoxFocusLost()
{
    this->DisplayEdit(false);
}
void EditContainerElement::OnEditBoxFocus()
{
    this->DisplayEdit(true);
}

bool EditContainerElement::OnKeyDown(const Lv2cKeyboardEventArgs &event)
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
            this->editBox->Style().Visibility(Lv2cVisibility::Visible);
            this->buttonBase->Style().Visibility(Lv2cVisibility::Hidden);
            this->editBox->Focus();

            this->editBox->SelectAll();
        }
        else
        {
            this->editBox->Style().Visibility(Lv2cVisibility::Hidden);
            this->buttonBase->Style().Visibility(Lv2cVisibility::Visible);
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

Lv2cElement::ptr Lv2PortViewFactory::CreateLed(Lv2PortViewController *viewController)
{
    auto element = Lv2cLampElement::Create();
    element->Variant(Lv2cLampVariant::OnOff);
    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Center)
        .VerticalAlignment(Lv2cAlignment::Center);
    viewController->PortValueProperty.Bind(element->ValueProperty);
    auto ledColor = viewController->PortInfo().pipedal_ledColor();
    if (ledColor.length() > 0)
    {
        if (ledColor == "red") {
            element->SetLampColor(Lv2cColor(1.0f, 0.0f, 0.0f)); 
        } else if (ledColor == "green") {
            element->SetLampColor(Lv2cColor(0.0f, 1.0f, 0.0f)); 
        }
    }
    return element;
}
Lv2cContainerElement::ptr Lv2PortViewFactory::CreatePage()
{

    auto element = Lv2cFlexGridElement::Create();
    element->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .Padding({24, 16, 24, 0})
        .RowGap(16)
        .ColumnGap(8)
        .FlexAlignItems(Lv2cAlignment::Center)
        .Background(Theme().paper)
        .FlexWrap(Lv2cFlexWrap::Wrap)
        // .BorderWidth({1})
        // .BorderColor(Lv2cColor("#FF8080"))
        ;
    return element;
}

Lv2cTheme &Lv2PortViewFactory::Theme()
{
    return *(theme.get());
}
Lv2PortViewFactory &Lv2PortViewFactory::Theme(std::shared_ptr<Lv2cTheme> theme)
{
    this->theme = theme;
    return *this;
}

Lv2cSize Lv2PortViewFactory::DefaultSize() const
{
    return Lv2cSize(Lv2ControlWidth ,Lv2ControlHeight);

}
