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

#include "lv2c_ui/Lv2UI.hpp"

#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/atom/util.h"
#include "lv2/core/lv2.h"
#include "lv2/core/lv2_util.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/ui/ui.h"
#include "lv2/urid/urid.h"

#include <vector>
/*
   Tie LV2UI to LV2 API.

*/

using namespace lv2c::ui;

constexpr size_t MAX_REGISTRATION = 100;
static bool gMaxRegistrationsExceeded = false;
static size_t gRegistrationCount = 0;
static Lv2UIRegistrationBase * gRegistrations[MAX_REGISTRATION];


std::vector<LV2UI_Descriptor> descriptors;

static LV2UI_Handle
lv2_instantiate(const LV2UI_Descriptor *descriptor,
                  const char *plugin_uri,
                  const char *bundle_path,
                  LV2UI_Write_Function write_function,
                  LV2UI_Controller controller,
                  LV2UI_Widget *widget,
                  const LV2_Feature *const *features)
{
    Lv2NativeCallbacks *lv2UI = nullptr;

    for (size_t i = 0; i < gRegistrationCount; ++i)
    {
        auto registration = gRegistrations[i];
        if (strcmp(registration->PluginUri().c_str(), descriptor->URI) == 0)
        {
            lv2UI = registration->Create();
            break;
        }
    }
    if (lv2UI != nullptr)
    {
        if (!lv2UI->instantiate(
                descriptor->URI,
                plugin_uri,
                bundle_path,
                write_function,
                controller,
                widget,
                features))
        {
            lv2UI->ui_delete();
            lv2UI = nullptr;
        }
    }

    return (LV2UI_Handle)(lv2UI);
}

static void
lv2_cleanup(LV2UI_Handle handle)
{
    Lv2NativeCallbacks *ui = (Lv2NativeCallbacks *)handle;
    ui->ui_delete();
}

/* Optional non-embedded UI show interface. */
static int
ui_show(LV2UI_Handle handle)
{
    Lv2NativeCallbacks *ui = (Lv2NativeCallbacks *)handle;
    return ui->ui_show();
}
static int
ui_hide(LV2UI_Handle handle)
{
    Lv2NativeCallbacks *ui = (Lv2NativeCallbacks *)handle;
    return ui->ui_hide();
}
static int
ui_idle(LV2UI_Handle handle)
{
    Lv2NativeCallbacks *ui = (Lv2NativeCallbacks *)handle;
    return ui->ui_idle();
}

static int ui_resize(
    LV2UI_Feature_Handle handle, 
    int width, 
    int height)
{
    Lv2NativeCallbacks *ui = (Lv2NativeCallbacks *)handle;
    return ui->ui_resize(width,height);

}


static const void *
lv2_extension_data(const char *uri)
{
    static const LV2UI_Show_Interface show = {ui_show, ui_hide};
    static const LV2UI_Idle_Interface idle = {ui_idle};
    static const LV2UI_Resize resize = {nullptr,ui_resize};

    if (!strcmp(uri, LV2_UI__showInterface))
    {
        return &show;
    }
    if (!strcmp(uri, LV2_UI__idleInterface))
    {
        return &idle;
    }
    if (!strcmp(uri, LV2_UI__resize))
    {
        return &resize;
    }

    return NULL;
}


static void
lv2_port_event(LV2UI_Handle handle,
           uint32_t port_index,
           uint32_t buffer_size,
           uint32_t format,
           const void *buffer)
{
    Lv2NativeCallbacks *ui = (Lv2NativeCallbacks *)handle;
    ui->ui_port_event(port_index, buffer_size, format, buffer);
}

Lv2UIRegistrationBase::Lv2UIRegistrationBase(const char *pluginUri)
    : pluginUri(pluginUri)
{

    //fprintf(stderr,"Registering %s\n",pluginUri);
    
    if (gRegistrationCount >= MAX_REGISTRATION)
    {
        gMaxRegistrationsExceeded = true; // i/o hasn't been brought up yet. give an error later.
        return;
    }
    gRegistrations[gRegistrationCount++] = this;
}

Lv2UIRegistrationBase::~Lv2UIRegistrationBase()
{
}


// The main LV2 entry point.
LV2_SYMBOL_EXPORT
const LV2UI_Descriptor *
lv2ui_descriptor(uint32_t index)
{
    if (descriptors.size() == 0)
    {

        for (size_t i = 0; i < gRegistrationCount; ++i)
        {
            auto registration = gRegistrations[i];
            LV2UI_Descriptor descriptor{
                registration->PluginUri().c_str(),
                lv2_instantiate,
                lv2_cleanup,
                lv2_port_event,
                lv2_extension_data};
            descriptors.push_back(descriptor);
        }
    }

    return (index < descriptors.size()) ? &(descriptors[index]) : nullptr;
}
