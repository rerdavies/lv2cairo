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

#include <stdexcept>
#include <iostream>
#include <memory.h>
#include <filesystem>

#include <stdio.h>
#include <stdlib.h>
#include "lv2c/Lv2cSettingsFile.hpp"


#include "TunerTestPage.hpp"
#include "TableTestPage.hpp"
#include "PaletteTestPage.hpp"
#include "StandardDialogTestPage.hpp"
#include "DialTestPage.hpp"
#include "VerticalStackTest.hpp"
#include "Lv2ControlTestPage.hpp"
#include "TypographyTestPage.hpp"
#include "FlexGridTestPage.hpp"
#include "EditBoxTestPage.hpp"
#include "ButtonTestPage.hpp"
#include "SvgTestPage.hpp"
#include "PngTestPage.hpp"
#include "DropdownTestPage.hpp"
#include "DropShadowTestPage.hpp"
#include "ScrollBarTestPage.hpp"
#include "MotionBlurTestPage.hpp"
#include "Lv2UiTestPage.hpp"




#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cDropdownElement.hpp"
#include "lv2c/Lv2cLog.hpp"
#include "lv2c/Lv2cVerticalStackElement.hpp"
#include "lv2c/Lv2cElement.hpp"
#include "lv2c/Lv2cDropShadowElement.hpp"

using namespace std;
using namespace lvtk;



static void SetResourceDirectories(const char*argv0)
{
    std::filesystem::path t = argv0;
    std::filesystem::path executableDirectory = t.parent_path();
    executableDirectory = std::filesystem::weakly_canonical(executableDirectory);

    std::vector<filesystem::path> resourceDirectories;
    resourceDirectories.push_back(executableDirectory / "resources");

    resourceDirectories.push_back(executableDirectory);
    char *env = getenv("RESOURCEDIR");
    if (env)
    {
        resourceDirectories.push_back(env);
    }
    Lv2cWindow::SetResourceDirectories(resourceDirectories);
}

void SelectPageView(Lv2cContainerElement* pageElements,size_t selectionId)
{
    for (size_t i = 0; i < pageElements->ChildCount(); ++i)
    {
        auto child = pageElements->Child(i);
        child->Style().Visibility(i == selectionId ? Lv2cVisibility::Visible : Lv2cVisibility::Collapsed);
    }
    pageElements->InvalidateLayout();

}



void RequestRerender(); // forward declaration.


class TestWindow : public Lv2cWindow
{
public:
    using self = TestWindow;
    using super = Lv2cWindow;
    using ptr = std::shared_ptr<self>;
    static ptr Create() { return std::make_shared<self>(); }


    void CreateWindow();

protected:
    void OnDraw(Lv2cDrawingContext &dc)
    {
        super::OnDraw(dc);
    }

    void Render();
private:
    std::vector<std::shared_ptr<TestPage>> pages;

    observer_handle_t selectedIdHandle;
    observer_handle_t selectedModeHandle;
    observer_handle_t selectedScaleHandle;
    Lv2cContainerElement::ptr pageElements;
    Lv2cSettingsFile settings;

};


static TestWindow::ptr myWindow;




void TestWindow::CreateWindow()
{
    settings.Load("Lv2cTestMain");
    bool darkMode = settings.Root()["DarkMode"].as<bool>(true);

    Lv2cTheme::ptr theme = Lv2cTheme::Create(darkMode);

    this->Theme(theme);

    WindowTitle("Lv2cTestMain");
    double windowScale =  settings.Root()["WindowScale"].as<double>(1.0);
    WindowScale(windowScale);


    Lv2cCreateWindowParameters parameters;
    parameters.location = Lv2cPoint(10,20);
    parameters.size = Lv2cSize(800,600);
    parameters.minSize = Lv2cSize(320,200);
    parameters.maxSize = Lv2cSize(4096,4096);
    parameters.title = "Lv2c Demo";
    parameters.settingsKey = "Lv2cTestMain";
    parameters.x11Windowclass = "com.twoplay.lvtk.plugin";
    parameters.positioning = Lv2cWindowPositioning::CenterOnDesktop;
    parameters.settingsObject = settings.Root();
    parameters.backgroundColor = theme->paper;

    super::CreateWindow(parameters);

    Render();



}


void TestWindow::Render()
{    
    


    pages.resize(0);
    pages.push_back(Lv2UiTestPage::Create());
    pages.push_back(Lv2ControlTestPage::Create());
    pages.push_back(StandardDialogTestPage::Create());
    pages.push_back(DialTestPage::Create());
    pages.push_back(PngTestPage::Create());
    pages.push_back(EditBoxTestPage::Create());
    pages.push_back(TypographyTestPage::Create());
    pages.push_back(FlexGridTestPage::Create());
    pages.push_back(TableTestPage::Create());
    pages.push_back(ButtonTestPage::Create());
    pages.push_back(SvgTestPage::Create());
    pages.push_back(ScrollBarTestPage::Create());
    pages.push_back(DropdownTestPage::Create());
    pages.push_back(DropShadowTestPage::Create());
    pages.push_back(MotionBlurTestPage::Create());
    pages.push_back(PaletteTestPage::Create());
    pages.push_back(VerticalStackTestPage::Create());
    pages.push_back(TunerTestPage::Create());
    lvtk::SetLogLevel(Lv2cLogLevel::Debug);


    auto mainElement = Lv2cVerticalStackElement::Create();
    mainElement->Style().Theme(ThemePtr())
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        ;
    int selectionId = 0;


    Lv2cDropdownElement::ptr dropdown;

    mainElement->Style().Background(myWindow->Theme().paper)
    .HorizontalAlignment(Lv2cAlignment::Stretch)
    .VerticalAlignment(Lv2cAlignment::Stretch);
    {
        auto header = Lv2cFlexGridElement::Create();
        auto dropShadow = Lv2cDropShadowElement::Create();
        dropShadow->AddChild(header);
        mainElement->AddChild(dropShadow);

        dropShadow->XOffset(0.0).YOffset(1.0).Radius(3).ShadowOpacity(0.4).Variant(Lv2cDropShadowVariant::DropShadow);
        dropShadow->Style()
            .Background(Theme().paper)
            .MarginBottom(8)
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            ;

        header->Style()
            .Padding({24,16,24,8})
            ;
        {
            dropdown = Lv2cDropdownElement::Create();
            header->AddChild(dropdown);
            std::vector<Lv2cDropdownItem> items;
            int index = 0;
            for (auto &page: pages)
            {
                items.push_back(Lv2cDropdownItem(index++,page->Title()));
            }   
            dropdown->DropdownItems(items);
            dropdown->SelectedId(selectionId);

            selectedIdHandle = dropdown->SelectedIdProperty.addObserver(
                [this](selection_id_t value) mutable
                {
                    SelectPageView(pageElements.get(),value);
                    settings.Root()["page"] = value;
                    settings.Update();

                }
            );
        }
        {
            auto modeDropdown = Lv2cDropdownElement::Create();
            header->AddChild(modeDropdown);
            std::vector<Lv2cDropdownItem> items;

            bool darkMode =  settings.Root()["DarkMode"].as<bool>(true);

            items.push_back(Lv2cDropdownItem(0,"Dark Theme"));
            items.push_back(Lv2cDropdownItem(1,"Light Theme"));
            modeDropdown->DropdownItems(items);
            modeDropdown->SelectedId(darkMode? 0: 1);
            selectedModeHandle = modeDropdown->SelectedIdProperty.addObserver(
                [this,currentSetting = darkMode](selection_id_t value) mutable
                {
                    bool newValue = value == 0;
                    if (newValue != currentSetting)
                    {
                        settings.Root()["DarkMode"] = value == 0;
                        settings.Update();
                        RequestRerender();
                    }
                }
            );
        }
        {
            auto modeDropdown = Lv2cDropdownElement::Create();
            header->AddChild(modeDropdown);
            std::vector<Lv2cDropdownItem> items;

            double windowScale =  settings.Root()["WindowScale"].as<double>(1.0);

            static double windowScales[] { 0.75,1.0,1.5,2};
            items.push_back(Lv2cDropdownItem(0,"0.75x"));
            items.push_back(Lv2cDropdownItem(1,"1x"));
            items.push_back(Lv2cDropdownItem(2,"1.5x"));
            items.push_back(Lv2cDropdownItem(3,"2x"));
            modeDropdown->DropdownItems(items);

            int selectedId = 1;
            for (size_t i = 0; i < 4; ++i)
            {
                if (windowScales[i] == windowScale)
                {
                    selectedId = i;
                    break;
                }
            }
            modeDropdown->SelectedId(selectedId);
            selectedScaleHandle = modeDropdown->SelectedIdProperty.addObserver(
                [this,currentSetting = selectedId](selection_id_t value) mutable
                {
                    if (value != currentSetting)
                    {
                        settings.Root()["WindowScale"] = windowScales[value];
                        settings.Update();
                        RequestRerender();
                    }
                }
            );
        }
    }

    // constexpr float SHADOW_SIZE = 4;
    // constexpr float SHADOW_OFFSET = 2;
    // for (size_t i = 0; i < SHADOW_SIZE+SHADOW_OFFSET; ++i)
    // {
    //     auto div = Lv2cElement::Create();
    //     mainElement->AddChild(div);
    //     float occlusion = (SHADOW_SIZE-i+SHADOW_OFFSET)/(2*SHADOW_SIZE) *0.75f;

    //     div->Style()
    //         .HorizontalAlignment(Lv2cAlignment::Stretch)
    //         .Height(1)
    //         .Background(
    //             Lv2cColor::Blend(occlusion,theme.paper,Lv2cColor(0,0,0))
    //         );
    // }
    {
        pageElements = Lv2cContainerElement::Create();
        mainElement->AddChild(pageElements);
        pageElements->Style()
            .HorizontalAlignment(Lv2cAlignment::Stretch)
            .VerticalAlignment(Lv2cAlignment::Stretch);
        for (auto &page : pages)
        {
            pageElements->AddChild(page->CreatePageView(this->ThemePtr()));
        }
    }



    auto selectedPage = settings.Root()["page"].as<selection_id_t>(0);
    dropdown->SelectedId(selectedPage);
    if (selectedPage == 0)
    {
        SelectPageView(pageElements.get(),selectedPage);
        
    }

    myWindow->GetRootElement()->AddChild(mainElement);
}

static bool rerenderRequested;

void RequestRerender() {
    rerenderRequested = true;
    myWindow->Close();
    // myWindow->PostQuit();
}



int main(int argc, char **argv)
{

    SetResourceDirectories(argv[0]);

    while (true)
    {       
        rerenderRequested = false;
        myWindow = TestWindow::Create();
        myWindow->CreateWindow();

        myWindow->TraceEvents(true);

        myWindow->PumpMessages(true);
        myWindow = nullptr;
        if (!rerenderRequested)
        {
            break;
        }
    }

}
