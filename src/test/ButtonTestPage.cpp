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
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkButtonElement.hpp"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkVerticalStackElement.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "lvtk/LvtkSvgElement.hpp"
#include "lvtk/LvtkDialog.hpp"

using namespace lvtk;

class MyDialog : public LvtkDialog
{
public:
    using self = MyDialog;
    using super = LvtkDialog;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }

    MyDialog()
    {
        (*this)
            .Title("Hello Dialog")
            .DefaultSize(LvtkSize(320, 0))
            .Positioning(LvtkWindowPositioning::CenterOnParent)
            .SettingsKey("HelloDialog");
    }
    ~MyDialog()
    {
    }

    LvtkElement::ptr Render() override
    {
        auto container = LvtkVerticalStackElement::Create();
        container->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .VerticalAlignment(LvtkAlignment::Start)
            .Background(Theme().paper)
            .Padding({24, 16, 24, 16});
        {
            auto body = LvtkFlexGridElement::Create();
            body->Style()
                .HorizontalAlignment(LvtkAlignment::Stretch)
                .VerticalAlignment(LvtkAlignment::Start)
                .FlexAlignItems(LvtkAlignment::Start)
                .FlexDirection(LvtkFlexDirection::Row)
                .FlexWrap(LvtkFlexWrap::NoWrap);
            {
                auto icon = LvtkSvgElement::Create();
                icon->Source("info.svg");
                icon->Style()
                    .Width(24)
                    .Height(24)
                    .TintColor(Theme().secondaryTextColor)
                    .MarginEnd({16});
                body->AddChild(icon);
            }
            {
                auto typography = LvtkTypographyElement::Create();
                typography->Text("Hello world! Cancel OK").Variant(LvtkTypographyVariant::BodySecondary);
                typography->Style()
                    .HorizontalAlignment(LvtkAlignment::Stretch)
                    .VerticalAlignment(LvtkAlignment::Start)
                    .MarginBottom({24})
                    .SingleLine(false);
                body->AddChild(typography);
            }
            container->AddChild(body);
        }
        {
            auto flexGrid = LvtkFlexGridElement::Create();
            flexGrid->Style()
                .HorizontalAlignment(LvtkAlignment::Stretch)
                .VerticalAlignment(LvtkAlignment::Start)
                .FlexJustification(LvtkFlexJustification::End)
                .FlexDirection(LvtkFlexDirection::Row)
                .FlexWrap(LvtkFlexWrap::NoWrap)
                .FlexAlignItems(LvtkAlignment::Center);
            {
                LvtkButtonElement::ptr cancelButton = LvtkButtonElement::Create();
                this->cancelButton = cancelButton;
                cancelButton->Variant(LvtkButtonVariant::BorderButton);
                cancelButton->Style()
                    .Width(80)
                    .Margin({0, 0, 16, 0});
                cancelButton->Text("Cancel");
                flexGrid->AddChild(cancelButton);
            }
            {
                LvtkButtonElement::ptr okButton = LvtkButtonElement::Create();
                this->okButton = okButton;
                okButton->Variant(LvtkButtonVariant::BorderButtonDefault);
                okButton->Style().Width(80);
                okButton->Text("OK");
                flexGrid->AddChild(okButton);
            }
            container->AddChild(flexGrid);

            okEventHandle = okButton->Clicked.AddListener([this](const LvtkMouseEventArgs &args)
                                                {
                OnOk();
                return true; });

            cancelEventHandle = cancelButton->Clicked.AddListener([this](const LvtkMouseEventArgs &)
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
    LvtkButtonElement::ptr okButton, cancelButton;
};

class TestDialog : public LvtkContainerElement
{
public:
    using self = TestDialog;
    using super = LvtkElement;
    using ptr = std::shared_ptr<self>;

    static ptr Create(LvtkButtonBaseElement::ptr button)
    {
        return std::make_shared<self>(button);
    }
    TestDialog(LvtkButtonBaseElement::ptr button)
        : button(button)
    {
        this->AddChild(button);
        onClickHandle = button->Clicked.AddListener([this](const LvtkMouseEventArgs &args)
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
    LvtkButtonBaseElement::ptr button;
    EventHandle onClickHandle;

    void OnClick()
    {
        auto dlg = MyDialog::Create();
        dlg->Show(this->Window());
    }
};

LvtkElement::ptr ButtonTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    LvtkVerticalStackElement::ptr main = LvtkVerticalStackElement::Create();
    main->Style().Background(theme->paper).Padding({24, 16, 24, 16});
    {
        // LvtkTypographyElement::ptr title = LvtkTypographyElement::Create();
        // title->Variant(TypographyVariant::Title).Text("Button Test");
        // title->Style()
        //     .Padding({4})
        //     .Margin({0,0,0,16})
        //     .BorderWidth({0,0,0,1})
        //     .HorizontalAlignment(LvtkAlignment::Stretch)
        //     .BorderColor(LvtkColor(1,1,1,0.25))
        //     ;

        // main->AddChild(title);
    } {
        auto flexGrid = LvtkFlexGridElement::Create();
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexJustification(LvtkFlexJustification::Start)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .MarginBottom(16);
        main->AddChild(flexGrid);
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(TestDialog::Create(button));
            button->Variant(LvtkButtonVariant::Dialog);
            button->Text("dialog");
            button->Style().Width(120);
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::DialogSecondary);
            button->Text("OK");
            button->Style().Width(120);
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::DialogPrimary);
            button->Text("Primary");
            button->Style().Width(120);
        }
    }
    {
        auto flexGrid = LvtkFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexJustification(LvtkFlexJustification::Start)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .MarginBottom(16);
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild((button));
            button->Variant(LvtkButtonVariant::Dialog);
            button->Text("home").Icon("FileDialog/home.svg");
            button->Style().Width(120);
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::DialogSecondary);
            button->Text("about").Icon("info.svg");
            button->Style().Width(120);
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::DialogPrimary);
            button->Text("undo").Icon("undo.svg");
            button->Style().Width(120);
        }
    }
    if (true ){
        auto flexGrid = LvtkFlexGridElement::Create();
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexJustification(LvtkFlexJustification::Start)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .MarginBottom(16);
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::BorderButton);
            button->Icon("FileDialog/home.svg");
            button->Text("Home");
            button->Style().Width(120);
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::BorderButtonDefault);
            button->Icon("info.svg");
            button->Text("About");
            button->Style().Width(120);
        }

        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::BorderButtonPrimary);
            button->Icon("undo.svg");
            button->Text("Undo");
            button->Style().Width(120);
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::BorderButtonSecondary);
            button->Icon("redo.svg");
            button->Text("Redo\u00A0\u00A0");
            button->Style().Width(0);
        }
        main->AddChild(flexGrid);
    }
    if (true ){
        auto flexGrid = LvtkFlexGridElement::Create();
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexJustification(LvtkFlexJustification::Start)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .MarginBottom(16);
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::BorderButton);
            button->Text("Border button");
            button->Style().Width(120);
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::BorderButtonDefault);
            button->Text("Default");
            button->Style().Width(120);
        }

        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::BorderButtonPrimary);
            button->Text("Primary");
            button->Style().Width(120);
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::BorderButtonSecondary);
            button->Text("Secondary");
            button->Style().Width(180);
        }
        main->AddChild(flexGrid);
    }
    {
        auto element = LvtkFlexGridElement::Create();
        main->AddChild(element);
        element->Style()
            .HorizontalAlignment(LvtkAlignment::Stretch)
            .FlexWrap(LvtkFlexWrap::Wrap)
            .FlexJustification(LvtkFlexJustification::End)
            .VerticalAlignment(LvtkAlignment::Start)
            .HorizontalAlignment(LvtkAlignment::Stretch);
        {
            auto row = LvtkFlexGridElement::Create();
            element->AddChild(row);
            row->Style()
                .FlexWrap(LvtkFlexWrap::Wrap)
                .FlexJustification(LvtkFlexJustification::End)
                .VerticalAlignment(LvtkAlignment::Start)
                .HorizontalAlignment(LvtkAlignment::Stretch);
            {
                auto button = LvtkButtonElement::Create();
                row->AddChild(button);
                button->Variant(LvtkButtonVariant::BorderButtonSecondary);
                button->Text("Cancel");
                button->Style().Margin({4, 0, 4, 0}).Width(120);
            }
            {
                auto button = LvtkButtonElement::Create();
                row->AddChild(button);
                button->Variant(LvtkButtonVariant::BorderButtonPrimary);
                button->Text("OK");
                button->Style().Margin({4, 0, 4, 0}).Width(120);
            }
        }
    }
    {
        auto flexGrid = LvtkFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .ColumnGap(8)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexJustification(LvtkFlexJustification::Start)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .MarginBottom(24);
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild((button));
            button->Variant(LvtkButtonVariant::Dialog);
            button->Icon("FileDialog/home.svg");
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::DialogSecondary);
            button->Icon("info.svg");
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::DialogPrimary);
            button->Icon("undo.svg");
        }
    }
    {
        auto flexGrid = LvtkFlexGridElement::Create();
        main->AddChild(flexGrid);
        flexGrid->Style()
            .ColumnGap(24)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexJustification(LvtkFlexJustification::Start)
            .FlexWrap(LvtkFlexWrap::NoWrap)
            .MarginBottom(24);
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild((button));
            button->Variant(LvtkButtonVariant::ImageButton);
            button->Icon("FileDialog/home.svg");
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::ImageButtonSecondary);
            button->Icon("info.svg");
        }
        {
            auto button = LvtkButtonElement::Create();
            flexGrid->AddChild(button);
            button->Variant(LvtkButtonVariant::ImageButtonPrimary);
            button->Icon("undo.svg");
        }
    }

    return std::static_pointer_cast<LvtkElement>(main);
}
