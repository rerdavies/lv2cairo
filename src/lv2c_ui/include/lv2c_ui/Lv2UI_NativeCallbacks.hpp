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


#include "lv2/ui/ui.h"
#include "lv2/log/log.h"


namespace lvtk::ui {
    /// @brief Lv2 native callbacks. For private use only.
    class Lv2NativeCallbacks
    {
    public:
        // LV2 callback handlers.
        virtual bool instantiate(
            const char *plugin_ui_uri,
            const char *plugin_uri,
            const char *bundle_path,
            LV2UI_Write_Function write_function,
            LV2UI_Controller controller,
            LV2UI_Widget *widget,
            const LV2_Feature *const *features) = 0;

        virtual void ui_port_event(
            uint32_t port_index,
            uint32_t buffer_size,
            uint32_t format,
            const void *buffer) = 0;
        virtual int ui_show() = 0;
        virtual int ui_hide() = 0;
        virtual int ui_idle() = 0;
        virtual void ui_delete() = 0;
        virtual int ui_resize(int width, int height) = 0;
    };
}