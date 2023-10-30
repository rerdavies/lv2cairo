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

#pragma once

#include "lv2c_ui/PiPedalUI.hpp"
#include <lilv/lilv.h>
#include <filesystem>
#include "lv2c_ui/PiPedalUiDefs.h"




namespace lv2c::ui
{

    class LilvUiFileType : public UiFileType
    {
    public:
        LilvUiFileType(LilvWorld *pHost, const LilvNode *node);
        static std::vector<UiFileType> GetArray(LilvWorld *pHost, const LilvNode *node, const LilvNode *uri);
    };

    class LilvUiPortNotification: public UiPortNotification {
    public:
            LilvUiPortNotification(LilvWorld*pWorld, const LilvNode*node);
    };

    class LilvUiFileProperty: public UiFileProperty {
    public:
        LilvUiFileProperty(LilvWorld*pWorld, const LilvNode*node, const std::filesystem::path&resourcePath);
    };

    class LilvUiFrequencyPlot: public UiFrequencyPlot {
    public:
        LilvUiFrequencyPlot(LilvWorld*pWorld, const LilvNode*node,
          const std::filesystem::path&resourcePath);

    };
    class LilvPiPedalUI : public PiPedalUI
    {
    public:
        LilvPiPedalUI(LilvWorld*pWorld, const LilvNode*uiNode, const std::filesystem::path&resourcePath);
    };

}