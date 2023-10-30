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

#include "Lv2UiTestPage.hpp"
#include "SamplePluginInfo.hpp"
#include "lv2c_ui/Lv2UI.hpp"
#include "lv2c/Lv2cScrollContainerElement.hpp"

using namespace lv2c::ui;

namespace lv2c
{

    class SamplePluginUI : public Lv2UI
    {
    public:
        SamplePluginUI()
            : Lv2UI(SamplePluginInfo::Create(), Lv2cCreateWindowParameters())
        {
        }
    };

    class UiContainerElement : public Lv2cContainerElement
    {
    public:
        using self = UiContainerElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create(Lv2cTheme::ptr theme) { return std::make_shared<self>(theme); }
        UiContainerElement(Lv2cTheme::ptr theme);

    private:
        SamplePluginUI pluginUi;
    };

    class Lv2UiTestPageImpl : public Lv2UiTestPage
    {
    public:
        using self = Lv2UiTestPageImpl;
        using super = Lv2UiTestPage;
        using ptr = std::shared_ptr<self>;

        static ptr Create() { return std::make_shared<self>(); }

        Lv2cElement::ptr CreatePageView(Lv2cTheme::ptr theme) override
        {

            return UiContainerElement::Create(theme);
        }
    };

    Lv2UiTestPage::ptr Lv2UiTestPage::Create()
    {
        return Lv2UiTestPageImpl::Create();
    }
}

UiContainerElement::UiContainerElement(Lv2cTheme::ptr theme)
{
    this->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Stretch);

    {
        pluginUi.Theme(theme);
        pluginUi.SetControlValue("vu", -4.0f);
        pluginUi.SetControlValue("lfoOut", -0.3f);

        pluginUi.SetControlValue("vuOutL", -14.0f);
        pluginUi.SetControlValue("vuOutR", -5.0f);
        AddChild(pluginUi.Render());
    }
}
