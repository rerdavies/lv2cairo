// Copyright (c) 2023 Robin Davies
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

#include "SamplePluginInfo.hpp"
#include "lvtk_ui/Lv2UI.hpp"

using namespace lvtk::ui;
using namespace lvtk;

class SamplePlugin: public Lv2UI {
public:
    using super=Lv2UI;
    SamplePlugin();
};



SamplePlugin::SamplePlugin() : super(
    SamplePluginInfo::Create(),
    LvtkSize(490,360) // default window size.
    )
{
    LvtkTheme::ptr theme = LvtkTheme::Create(true); // start with dark theme.
    theme->paper = LvtkColor("#080818"); // something dark.
    this->Theme(theme);
}

static Lv2UIRegistration<SamplePlugin> registration { PLUGIN_UI_URI};




