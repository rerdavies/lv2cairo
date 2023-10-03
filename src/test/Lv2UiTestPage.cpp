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
#include "lvtk_ui/Lv2UI.hpp"
#include "lvtk/LvtkScrollContainerElement.hpp"

using namespace lvtk::ui;

namespace lvtk
{

    class SamplePluginUI : public Lv2UI
    {
    public:
        SamplePluginUI()
            : Lv2UI(SamplePluginInfo::Create(), LvtkCreateWindowParameters())
        {
        }
    };

    class UiContainerElement : public LvtkContainerElement
    {
    public:
        using self = UiContainerElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create(LvtkTheme::ptr theme) { return std::make_shared<self>(theme); }
        UiContainerElement(LvtkTheme::ptr theme);

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

        LvtkElement::ptr CreatePageView(LvtkTheme::ptr theme) override
        {

            return UiContainerElement::Create(theme);
        }
    };

    Lv2UiTestPage::ptr Lv2UiTestPage::Create()
    {
        return Lv2UiTestPageImpl::Create();
    }
};

UiContainerElement::UiContainerElement(LvtkTheme::ptr theme)
{
    this->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch);

    {
        pluginUi.Theme(theme);
        pluginUi.SetControlValue("vu", -4.0f);
        pluginUi.SetControlValue("lfoOut", -0.3f);

        pluginUi.SetControlValue("vuOutL", -14.0f);
        pluginUi.SetControlValue("vuOutR", -5.0f);
        AddChild(pluginUi.Render());
    }
}
