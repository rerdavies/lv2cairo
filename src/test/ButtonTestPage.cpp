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

#include "ButtonTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cTypographyElement.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cSvgElement.hpp"
#include "lv2c/Lv2cDialog.hpp"

using namespace lvtk;

class MyDialog : public Lv2cDialog
{
public:
    using self = MyDialog;
    using super = Lv2cDialog;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }

    MyDialog()
    {
        (*this)
            .Title("Hello Dialog")
            .DefaultSize(Lv2cSize(320, 0))
            .Positioning(Lv2cWindowPositioning::CenterOnParent)
            .SettingsKey("HelloDialog");
    }
    ~MyDialog()
    {
    }

    Lv2cElement::ptr Render() override
    {
        auto container = Lv2cVerticalStackElement::Create();
        container->Style()
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .VerticalAlignment(Lv2cAlignment::Start)
            .Background(Theme().paper)
            .Padding({24, 16, 24, 16});
        {
            auto body = Lv2cFlexGridElement::Create();
            body->Style()
                .HorizontalAlignment(Lv2cAlignment::Stretch)
                .VerticalAlignment(Lv2cAlignment::Start)
                .FlexAlignItems(Lv2cAlignment::Start)
                .FlexDirection(Lv2cFlexDirection::Row)
                .FlexWrap(Lv2cFlexWrap::NoWrap);
            {
                auto icon = Lv2cSvgElement::Create();
                icon->Source("info.svg");
                icon->Style()
                    .Width(24)
                    .Height(24)
                    .TintColor(Theme().secondaryTextColor)
                    .MarginEnd({16});
                body->AddChild(icon);
            }
            {
                auto typography = Lv2cTypographyElement::Create();
                typography->Text("Hello world! Cancel OK").Variant(Lv2cTypographyVariant::BodySecondary);
                typography->Style()
                    .HorizontalAlignment(Lv2cAlignment::Stretch)
                    .VerticalAlignment(Lv2cAlignment::Start)
                    .MarginBottom({24})
                    .SingleLine(false);
                body->AddChild(typography);
            }
            container->AddChild(body);
        }
        {
            auto flexGrid = Lv2cFlexGridElement::Create();
            flexGrid->Style()
                .HorizontalAlignment(Lv2cAlignment::Stretch)
                .VerticalAlignment(Lv2cAlignment::Start)
                .FlexJustification(Lv2cFlexJustification::End)
                .FlexDirection(Lv2cFlexDirection::Row)
                .FlexWrap(Lv2cFlexWrap::NoWrap)
                .FlexAlignItems(Lv2cAlignment::Center);
            {
                Lv2cButtonElement::ptr cancelButton = Lv2cButtonElement::Create();
                this->cancelButton = cancelButton;
                cancelButton->Variant(Lv2cButtonVariant::BorderButton);
                cancelButton->Style()
                    .Width(80)
                    .Margin({0, 0, 16, 0});
                cancelButton->Text("Cancel");
                flexGrid->AddChild(cancelButton);
            }
            {
                Lv2cButtonElement::ptr okButton = Lv2cButtonElement::Create();
                this->okButton = okButton;
                okButton->Variant(Lv2cButtonVariant::BorderButtonDefault);
                okButton->Style().Width(80);
                okButton->Text("OK");
                flexGrid->AddChild(okButton);
            }
            container->AddChild(flexGrid);

            okEventHandle = okButton->Clicked.AddListener([this](const Lv2cMouseEventArgs &args)
                                                {
                OnOk();
                return true; });

            cancelEventHandle = cancelButton->Clicked.AddListener([this](const Lv2cMouseEventArgs &)
                                                        {
                OnCancel();
                return true; });
        }
        return container;
    }
    void OnMount() override
    {
        okButton->Focus();
    }
    void OnOk()
    {
        Close();
    }
    void OnCancel()
    {
        Close();
    }
    virtual void OnClosing() override
    {
        super::OnClosing();
        okEventHandle = EventHandle::InvalidHandle;
        cancelEventHandle = EventHandle::InvalidHandle;
    }
    EventHandle okEventHandle, cancelEventHandle;
    Lv2cButtonElement::ptr okButton, cancelButton;
};

class TestDialog : public Lv2cContainerElement
{
public:
    using self = TestDialog;
    using super = Lv2cElement;
    using ptr = std::shared_ptr<self>;

    static ptr Create(Lv2cButtonBaseElement::ptr button)
    {
        return std::make_shared<self>(button);
    }
    TestDialog(Lv2cButtonBaseElement::ptr button)
        : button(button)
    {
        this->AddChild(button);
        onClickHandle = button->Clicked.AddListener([this](const Lv2cMouseEventArgs &args)
                                          {
            OnClick();
            return true; });
    }
    virtual ~TestDialog()
    {
    }

protected:
    virtual void OnUnmount() override
    {
        button->Clicked.RemoveListener(onClickHandle);
        onClickHandle = EventHandle::InvalidHandle;
    }

private:
    Lv2cButtonBaseElement::ptr button;
    EventHandle onClickHandle;

    void OnClick()
    {
        auto dlg = MyDialog::Create();
        dlg->Show(this->Window());
    }
};

Lv2cElement::ptr ButtonTestPage::CreatePageView(Lv2cTheme::ptr theme)
{

    Lv2cVerticalStackElement::ptr main = Lv2cVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16});
    {
        // Lv2cTypographyElement::ptr title = Lv2cTypographyElement::Create();
        // title->Variant(TypographyVariant::Title).Text("Button Test");
        // title->Style()
        //     .Padding({4})
        //     .Margin({0,0,0,16})
        //     .BorderWidth({0,0,0,1})
        //     .HorizontalAlignment(Lv2cAlignment::Stretch)
        //     .BorderColor(Lv2cColor(1,1,1,0.25))
        //     ;

        // main->AddChild(title);
    } {
        auto flexGrid = Lv2cFlexGridElement::Create();
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(Lv2cAlignment::Center)
            .FlexJustification(Lv2cFlexJustification::Start)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .MarginBottom(16);
        main->AddChild(flexGrid);
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(TestDialog::Create(button));
            button->Variant(Lv2cButtonVariant::Dialog);
            button->Text("dialog");
            button->Style().Width(120);
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::DialogSecondary);
            button->Text("OK");
            button->Style().Width(120);
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::DialogPrimary);
            button->Text("Primary");
            button->Style().Width(120);
        }
    }
    {
        auto flexGrid = Lv2cFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(Lv2cAlignment::Center)
            .FlexJustification(Lv2cFlexJustification::Start)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .MarginBottom(16);
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild((button));
            button->Variant(Lv2cButtonVariant::Dialog);
            button->Text("home").Icon("FileDialog/home.svg");
            button->Style().Width(120);
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::DialogSecondary);
            button->Text("about").Icon("info.svg");
            button->Style().Width(120);
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::DialogPrimary);
            button->Text("undo").Icon("undo.svg");
            button->Style().Width(120);
        }
    }
    if (true ){
        auto flexGrid = Lv2cFlexGridElement::Create();
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(Lv2cAlignment::Center)
            .FlexJustification(Lv2cFlexJustification::Start)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .MarginBottom(16);
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::BorderButton);
            button->Icon("FileDialog/home.svg");
            button->Text("Home");
            button->Style().Width(120);
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::BorderButtonDefault);
            button->Icon("info.svg");
            button->Text("About");
            button->Style().Width(120);
        }

        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::BorderButtonPrimary);
            button->Icon("undo.svg");
            button->Text("Undo");
            button->Style().Width(120);
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::BorderButtonSecondary);
            button->Icon("redo.svg");
            button->Text("Redo\u00A0\u00A0");
            button->Style().Width(0);
        }
        main->AddChild(flexGrid);
    }
    if (true ){
        auto flexGrid = Lv2cFlexGridElement::Create();
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(Lv2cAlignment::Center)
            .FlexJustification(Lv2cFlexJustification::Start)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .MarginBottom(16);
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::BorderButton);
            button->Text("Border button");
            button->Style().Width(120);
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::BorderButtonDefault);
            button->Text("Default");
            button->Style().Width(120);
        }

        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::BorderButtonPrimary);
            button->Text("Primary");
            button->Style().Width(120);
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::BorderButtonSecondary);
            button->Text("Secondary");
            button->Style().Width(180);
        }
        main->AddChild(flexGrid);
    }
    {
        auto element = Lv2cFlexGridElement::Create();
        main->AddChild(element);
        element->Style()
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .FlexWrap(Lv2cFlexWrap::Wrap)
            .FlexJustification(Lv2cFlexJustification::End)
            .VerticalAlignment(Lv2cAlignment::Start)
            .HorizontalAlignment(Lv2cAlignment::Stretch);
        {
            auto row = Lv2cFlexGridElement::Create();
            element->AddChild(row);
            row->Style()
                .FlexWrap(Lv2cFlexWrap::Wrap)
                .FlexJustification(Lv2cFlexJustification::End)
                .VerticalAlignment(Lv2cAlignment::Start)
                .HorizontalAlignment(Lv2cAlignment::Stretch);
            {
                auto button = Lv2cButtonElement::Create();
                row->AddChild(button);
                button->Variant(Lv2cButtonVariant::BorderButtonSecondary);
                button->Text("Cancel");
                button->Style().Margin({4, 0, 4, 0}).Width(120);
            }
            {
                auto button = Lv2cButtonElement::Create();
                row->AddChild(button);
                button->Variant(Lv2cButtonVariant::BorderButtonPrimary);
                button->Text("OK");
                button->Style().Margin({4, 0, 4, 0}).Width(120);
            }
        }
    }
    {
        auto flexGrid = Lv2cFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(Lv2cAlignment::Center)
            .FlexJustification(Lv2cFlexJustification::Start)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .MarginBottom(24);
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild((button));
            button->Variant(Lv2cButtonVariant::Dialog);
            button->Icon("FileDialog/home.svg");
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::DialogSecondary);
            button->Icon("info.svg");
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::DialogPrimary);
            button->Icon("undo.svg");
        }
    }
    {
        auto flexGrid = Lv2cFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .ColumnGap(24)
            .FlexAlignItems(Lv2cAlignment::Center)
            .FlexJustification(Lv2cFlexJustification::Start)
            .FlexWrap(Lv2cFlexWrap::NoWrap)
            .MarginBottom(24);
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild((button));
            button->Variant(Lv2cButtonVariant::ImageButton);
            button->Icon("FileDialog/home.svg");
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::ImageButtonSecondary);
            button->Icon("info.svg");
        }
        {
            auto button = Lv2cButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(Lv2cButtonVariant::ImageButtonPrimary);
            button->Icon("undo.svg");
        }
    }

    return std::static_pointer_cast<Lv2cElement>(main);
}
