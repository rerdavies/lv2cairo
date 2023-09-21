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

#pragma once

#include "lvtk_ui/PiPedalUI.hpp"
#include <lilv/lilv.h>
#include <filesystem>


#define PIPEDAL_UI "http://github.com/rerdavies/pipedal/ui"
#define PIPEDAL_UI_PREFIX PIPEDAL_UI "#"

#define PIPEDAL_UI__ui PIPEDAL_UI_PREFIX "ui"

#define PIPEDAL_UI__fileProperties PIPEDAL_UI_PREFIX "fileProperties"

#define PIPEDAL_UI__fileProperty PIPEDAL_UI_PREFIX "fileProperty"
#define PIPEDAL_UI__patchProperty  PIPEDAL_UI_PREFIX "patchProperty"
#define PIPEDAL_UI__directory  PIPEDAL_UI_PREFIX "directory"
#define PIPEDAL_UI__fileTypes  PIPEDAL_UI_PREFIX "fileTypes"

#define PIPEDAL_UI__fileType  PIPEDAL_UI_PREFIX "fileType"
#define PIPEDAL_UI__fileExtension  PIPEDAL_UI_PREFIX "fileExtension"
#define PIPEDAL_UI__mimeType  PIPEDAL_UI_PREFIX "mimeType"

#define PIPEDAL_UI__outputPorts  PIPEDAL_UI_PREFIX "outputPorts"
#define PIPEDAL_UI__text  PIPEDAL_UI_PREFIX "text"

#define PIPEDAL_UI__frequencyPlot PIPEDAL_UI_PREFIX "frequencyPlot"
#define PIPEDAL_UI__xLeft PIPEDAL_UI_PREFIX "xLeft"
#define PIPEDAL_UI__xRight PIPEDAL_UI_PREFIX "xRight"
#define PIPEDAL_UI__xLog PIPEDAL_UI_PREFIX "xLog"
#define PIPEDAL_UI__yTop PIPEDAL_UI_PREFIX "yTop"
#define PIPEDAL_UI__yBottom PIPEDAL_UI_PREFIX "yBottom"
#define PIPEDAL_UI__yDb PIPEDAL_UI_PREFIX "yDb"

#define PIPEDAL_UI__width PIPEDAL_UI_PREFIX "width"




namespace lvtk::ui
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