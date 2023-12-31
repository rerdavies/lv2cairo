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

#include "StandardDialogTestPage.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cButtonElement.hpp"
#include "lv2c/Lv2cMessageDialog.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c_ui/Lv2FileDialog.hpp"

using namespace lv2c;
using namespace lv2c::ui;


class CustomMessageDialog: public Lv2cMessageDialog {
    public:
    using self=CustomMessageDialog;
    using super=Lv2cMessageDialog;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }

    CustomMessageDialog()
    {
        DefaultSize(Lv2cSize(480,0)); // adjust the width. Leave the height at 0 in order to fit to size.
        PrimaryButtonStyle().Width(100);  // widen thebuttons a bit.
        SecondaryButtonStyle().Width(100);
        IconStyle().Width(48).Height(48); // make the icon larger.


        Title("Custom Dialog");
        Text("Are you sure you want to wash your socks? This action may be irreversable.");
        PrimaryButtonText("Wash them!");

        PrimaryButtonColor(Lv2cColor("#E04040"));
        SecondaryButtonText("Cancel");
        IconSource("laundry.svg");
        IconTintColor(Lv2cColor("#7070A0"));


    }
};

static Lv2cButtonElement::ptr MakeButton(const std::string&text)
{
    auto result = Lv2cButtonElement::Create();
    result->Variant(Lv2cButtonVariant::Dialog);
    result->Style().Margin({0,0,0,16});
    result->Text(text);
    result->Style().Width(140);
    return result;
}
class MessageBoxTestElement: public Lv2cContainerElement {
public:
    using self = MessageBoxTestElement;
    using super = Lv2cContainerElement;
    using ptr = std::shared_ptr<self>;

    static ptr Create() { return std::make_shared<self>(); }

    MessageBoxTestElement()
    {
        auto container = Lv2cVerticalStackElement::Create();
        container->Style().HorizontalAlignment(Lv2cAlignment::Start);
        {
            auto button = MakeButton("INFO");

            button->Clicked.AddListener([this](const Lv2cMouseEventArgs&)
            {
                Window()->MessageBox(
                    Lv2cMessageDialogType::Info,
                    "Info",
                    "The capital of Wisconsin is Madison."
                );
                return true;
            });
            container->AddChild(button);

        } 
        {
            auto button = MakeButton("WARNING");

            warningEventHandle = button->Clicked.AddListener([this](const Lv2cMouseEventArgs&)
            {
                Window()->MessageBox(
                    Lv2cMessageDialogType::Warning,
                    "Warning",
                    "Do not set yourself on fire."
                );
                return true;
            });
            container->AddChild(button);

        } 
        {
            auto button = MakeButton("ERROR");

            button->Clicked.AddListener([this](const Lv2cMouseEventArgs&)
            {
                Window()->MessageBox(
                    Lv2cMessageDialogType::Error,
                    "Error",
                    "<b>Nam</b> <s>libero</s> <sub>tempore</sub>, <sup>cum</sup> <span color='#FF8080'>soluta</span> <i>nobis</i> "
                    "<span variant='small-caps'>est</span> <tt>eligendi</tt> optio, cumque <u>nihil</u> impedit, quo minus id, "
                    "quod maxime placeat facere possimus, omnis voluptas assumenda est, omnis dolor repellendaus.\n\n"
                    "— Cicero\n"
                    "\nSee https://docs.gtk.org/Pango/pango_markup.html 🍁"

                );
                return true;
            });
            container->AddChild(button);

        } 
        {
            auto button = MakeButton("Custom");

            button->Clicked.AddListener([this](const Lv2cMouseEventArgs&)
            {
                auto dlg = CustomMessageDialog::Create();
                dlg->Show(Window());
                return true;
            });
            container->AddChild(button);

        } 
        {
            auto button = MakeButton("File dialog");

            button->Clicked.AddListener([this](const Lv2cMouseEventArgs&)
            {
                auto dlg = Lv2FileDialog::Create("Open","TestFileDialog");
                std::vector<Lv2FileFilter> fileTypes
                {
                    {"All files",{},{}},
                    {"Audio files",{},{ "audio/*"}},
                    {"WAV files",{},{ "audio/x-wav"}},
                    {"WAV or FLAC files",{},{ "audio/x-wav", "audio/x-flac"}},
                    {"Video files",{},{ "video/*"}},
                    {"NAM Files (*.nam)",{".nam"},{}},
                    {"C++ files",{".cpp",".hpp",".cc",".h",".c"},{}}
                };
                dlg->FileTypes(
                    fileTypes
                );
                dlg->Show(Window());
                return true;
            });
            container->AddChild(button);

        } 

        AddChild(container);

    }
private:
    EventHandle warningEventHandle;

};

Lv2cElement::ptr StandardDialogTestPage::CreatePageView(Lv2cTheme::ptr theme)
{
    auto main = Lv2cFlexGridElement::Create();
    main->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .FlexWrap(Lv2cFlexWrap::Wrap)
        .FlexDirection(Lv2cFlexDirection::Column)
        .Background(theme->paper)
        .Padding({24, 16, 24, 16});
    {
        auto element = MessageBoxTestElement::Create();
        main->AddChild(element);
    }
    return main;
}
