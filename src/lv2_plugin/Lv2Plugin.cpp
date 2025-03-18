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


#include "lv2_plugin/Lv2Plugin.hpp"


#include "lv2/atom/atom.h"
#include "lv2/options/options.h"
#include "lv2/atom/util.h"
#include "lv2/core/lv2.h"
#include "lv2/core/lv2_util.h"
#include "lv2/log/log.h"
#include "lv2/log/logger.h"
#include "lv2/midi/midi.h"
#include "lv2/urid/urid.h"
#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include <iostream>

using namespace lv2c::lv2_plugin;

Lv2LogLevel Lv2Plugin::logLevel = Lv2LogLevel::Note;

void Lv2Plugin::connect_port(LV2_Handle instance, uint32_t port, void *data)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    plugin->ConnectPort(port, data);
}

void Lv2Plugin::activate(LV2_Handle instance)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    plugin->Activate();
}

void Lv2Plugin::run(LV2_Handle instance, uint32_t n_samples)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    plugin->RunOuter(n_samples);
}

void Lv2Plugin::deactivate(LV2_Handle instance)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    plugin->Deactivate();
}
void Lv2Plugin::cleanup(LV2_Handle instance)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    delete plugin;
}

LV2_Worker_Status Lv2Plugin::work_response(LV2_Handle instance, uint32_t size, const void *data)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    return plugin->OnWorkResponse(size, data);
}

LV2_Worker_Status Lv2Plugin::work(
    LV2_Handle instance,
    LV2_Worker_Respond_Function respond,
    LV2_Worker_Respond_Handle handle,
    uint32_t size,
    const void *data)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    return plugin->OnWork(respond, handle, size, data);
}
LV2_State_Status Lv2Plugin::save(
    LV2_Handle instance,
    LV2_State_Store_Function store,
    LV2_State_Handle handle,
    uint32_t flags,
    const LV2_Feature *const *features)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    return plugin->OnSaveLv2State(store, handle, flags, features);
}

LV2_State_Status Lv2Plugin::restore(
    LV2_Handle instance,
    LV2_State_Retrieve_Function retrieve,
    LV2_State_Handle handle,
    uint32_t flags,
    const LV2_Feature *const *features)
{
    Lv2Plugin *plugin = (Lv2Plugin *)instance;
    return plugin->OnRestoreLv2State(retrieve, handle, flags, features);
}

const void *Lv2Plugin::extension_data_with_state(const char *uri)
{
    static const LV2_State_Interface state = {save, restore};
    static const LV2_Worker_Interface worker = {work, work_response, nullptr};
    if (strcmp(uri, LV2_STATE__interface) == 0)
    {
        return &state;
    }
    else if (strcmp(uri, LV2_WORKER__interface) == 0)
    {
        return &worker;
    }
    return nullptr;
    return nullptr;
}

const void *Lv2Plugin::extension_data(const char *uri)
{
    static const LV2_Worker_Interface worker = {work, work_response, nullptr};
    if (strcmp(uri, LV2_WORKER__interface) == 0)
    {
        return &worker;
    }
    return nullptr;
    return nullptr;
}

Lv2Plugin::Lv2Plugin(double rate, const char *_bundle_path, const LV2_Feature *const *features, bool hasState)
    : rate(rate), bundle_path(_bundle_path), hasState(hasState)
{

    this->logger.log = nullptr;
    this->map = nullptr;
    this->unmap = nullptr;
    this->schedule = nullptr;
    this->options = nullptr;
    // Scan host features for URID map
    // clang-format off
    const char* missing = lv2_features_query(
        features,
        LV2_LOG__log, &this->logger.log, false,
        LV2_URID__map, &this->map, true,
        LV2_URID__unmap,&this->unmap,false,
        LV2_WORKER__schedule, &schedule, false,
        LV2_OPTIONS__options, &options, false,
        nullptr);

    lv2_log_logger_set_map(&this->logger, this->map);



    if (missing) {
        lv2_log_error(&this->logger, "Missing feature <%s>\n", missing);
    } else {
        urids.Init(map);
        lv2_atom_forge_init(&this->inputForge, map);
        lv2_atom_forge_init(&this->outputForge, map);
        InitBufSizeOptions();

    }

}

LV2_URID Lv2Plugin::MapURI(const char* uri)
{
    return map->map(map->handle, uri);  
}


const char*Lv2Plugin::UnmapUri(LV2_URID urid)
{
    return unmap->unmap(unmap->handle,urid);
}

void Lv2Plugin::LogError(const char* fmt, ...)
{
    if (logLevel > Lv2LogLevel::Error) return;
    if (logger.log != nullptr)
    {
        va_list va;
        va_start(va,fmt);
        logger.log->vprintf(logger.log->handle, logger.Error, fmt, va);
        va_end(va);
    } else {
        char buffer[256];
        va_list va;
        va_start(va,fmt);
        vsnprintf(buffer,sizeof(buffer),fmt,va);
        buffer[254] = 0;
        size_t len = strlen(buffer);
        if (len != 0 && buffer[len-1] == '\n')
        {
            buffer[len-1] = 0;
        }
        std::cout << "Error: " << buffer << std::endl;
    }

}
void Lv2Plugin::LogWarning(const char* fmt, ...)
{
    if (logLevel > Lv2LogLevel::Warning) return;
    if (logger.log != nullptr)
    {
        va_list va;
        va_start(va, fmt);
        logger.log->vprintf(logger.log->handle, logger.Warning, fmt, va);
        va_end(va);
    } else {
        char buffer[256];
        va_list va;
        va_start(va,fmt);
        vsnprintf(buffer,sizeof(buffer),fmt,va);
        buffer[254] = 0;
        size_t len = strlen(buffer);
        if (len != 0 && buffer[len-1] == '\n')
        {
            buffer[len-1] = 0;
        }
        std::cout << "Warning: " << buffer << std::endl;
    }

}
void Lv2Plugin::LogNote(const char* fmt, ...)
{
    if (logLevel > Lv2LogLevel::Note) return;
    if (logger.log != nullptr)
    {
        va_list va;
        va_start(va, fmt);
        logger.log->vprintf(logger.log->handle, logger.Note, fmt, va);
        va_end(va);
    } else {
        char buffer[256];
        va_list va;
        va_start(va,fmt);
        vsnprintf(buffer,sizeof(buffer),fmt,va);
        buffer[254] = 0;
        size_t len = strlen(buffer);
        if (len != 0 && buffer[len-1] == '\n')
        {
            buffer[len-1] = 0;
        }
        std::cout << "Note: " << buffer << std::endl;

    }

}
void Lv2Plugin::LogTrace(const char* fmt, ...)
{
    if (logLevel > Lv2LogLevel::Trace) return;
    if (logger.log != nullptr)
    {
        va_list va;
        va_start(va, fmt);
        logger.log->vprintf(logger.log->handle, logger.Trace, fmt, va);
        va_end(va);
    } else {
        char buffer[256];
        va_list va;
        va_start(va,fmt);
        vsnprintf(buffer,sizeof(buffer),fmt,va);
        buffer[254] = 0;
        size_t len = strlen(buffer);
        if (len != 0 && buffer[len-1] == '\n')
        {
            buffer[len-1] = 0;
        }
        std::cout << "Trace: " << buffer << std::endl;
    }
}

void Lv2Plugin::HandleEvents(LV2_Atom_Sequence*controlInput)
{
    LV2_ATOM_SEQUENCE_FOREACH (controlInput, ev) {

        /* Update current frame offset to this event's time.  This is stored in
            the instance because it is used for sychronous worker event
            execution.  This allows a sample load event to be executed with
            sample accuracy when running in a non-realtime context (such as
            exporting a session). */
        // frame_offset = ev->time.frames;

        if (lv2_atom_forge_is_object_type(&inputForge, ev->body.type)) {
            const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
            if (obj->body.otype == urids.patch__Set) {
                // Get the property and value of the set message
                const LV2_Atom* property = nullptr;
                const LV2_Atom* value    = nullptr;

                lv2_atom_object_get(
                    obj,
                    urids.patch__property, &property,
                    urids.patch__value,    &value,
                    0);

                if (property && value && property->type == urids.atom__URID)
                {
                    LV2_URID key = ((const LV2_Atom_URID *)property)->body;
                    OnPatchSet(key,value);
                }
            }
            else if (obj->body.otype == urids.patch__Get)
            {
                // Get the property and value of the set message
                // TODO: patch__accept is the correct object property.
                //       delete handling for patch__property once PiPedal has been fixed.
                const LV2_Atom* property = nullptr;
                const LV2_Atom* accept = nullptr;

                lv2_atom_object_get(
                    obj,
                    urids.patch__accept, &accept,
                    urids.patch__property, &property,
                    0);
                if (accept != nullptr && accept->type == urids.atom__URID)
                {
                    LV2_Atom_URID *pVal = (LV2_Atom_URID*)accept;
                    LV2_URID propertyUrid = pVal->body;
                    if (propertyUrid == 0)
                    {
                        OnPatchGetAll();
                    } else {
                        OnPatchGet(propertyUrid);
                    }

                }                    
                else if (property != nullptr && property->type == urids.atom__URID)
                {
                    LV2_Atom_URID *pVal = (LV2_Atom_URID*)property;
                    LV2_URID propertyUrid = pVal->body;
                    if (propertyUrid == 0)
                    {
                        OnPatchGetAll();
                    } else {
                        OnPatchGet(propertyUrid);
                    }

                }

            }
        }
    }
}

void Lv2Plugin::WorkerAction::Request()
{
    if (pPlugin->schedule)
    {
        pPlugin->schedule->schedule_work(
            pPlugin->schedule->handle,
            sizeof(pThis), &pThis); // must be POD!
    }
    else
    {
        // no scheduler. do it synchronously.
        OnWork();
        Response();
    }
}


void Lv2Plugin::WorkerAction::Work(LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle)
{
    OnWork();
    respond(handle, sizeof(pThis), &pThis);
}
void Lv2Plugin::WorkerAction::Response()
{
    OnResponse();
}

void Lv2Plugin::WorkerActionWithCleanup::CleanupWorker::OnWork()
{
    pThis->OnCleanup();
}
void Lv2Plugin::WorkerActionWithCleanup::CleanupWorker::OnResponse()
{
    pThis->OnCleanupComplete();
}

Lv2Plugin::WorkerActionWithCleanup::CleanupWorker::CleanupWorker(Lv2Plugin *plugin, WorkerActionWithCleanup *pThis) : WorkerAction(plugin), pThis(pThis)
{
}



void Lv2Plugin::PutPatchPropertyString(int64_t frameTime,LV2_URID propertyUrid, const char*value)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);
    size_t len = strlen(value);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_string(&outputForge, value, len+1);
    lv2_atom_forge_pop(&outputForge, &frame);

}
void Lv2Plugin::PutPatchPropertyPath(int64_t frameTime,LV2_URID propertyUrid, const char*value)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);
    size_t len = strlen(value);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_path(&outputForge, value, len+1);
    lv2_atom_forge_pop(&outputForge, &frame);

}
void Lv2Plugin::PutPatchPropertyUri(int64_t frameTime,LV2_URID propertyUrid, const char*value)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);
    size_t len = strlen(value);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_uri(&outputForge, value, len+1);
    lv2_atom_forge_pop(&outputForge, &frame);

}



void Lv2Plugin::PutPatchProperty(int64_t frameTime,LV2_URID propertyUrid, float value)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_float(&outputForge, value);
    lv2_atom_forge_pop(&outputForge, &frame);
}

void Lv2Plugin::PutStateChanged(int64_t frameTime)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.state__StateChanged);
    {
        // empty object
    }
    lv2_atom_forge_pop(&outputForge, &frame);
}
void Lv2Plugin::PutPatchProperty(int64_t frameTime,LV2_URID propertyUrid, size_t count, const float *values)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_vector(&outputForge, 
        sizeof(float),
        urids.atom__Float,
        (uint32_t)count,(void*)values);
    lv2_atom_forge_pop(&outputForge, &frame);

}
void Lv2Plugin::PutPatchProperty(int64_t frameTime,LV2_URID propertyUrid, bool value)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_bool(&outputForge, value);
    lv2_atom_forge_pop(&outputForge, &frame);

}

void Lv2Plugin::PutPatchProperty(int64_t frameTime,LV2_URID propertyUrid, double value)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_double(&outputForge, value);
    lv2_atom_forge_pop(&outputForge, &frame);

}
void Lv2Plugin::PutPatchProperty(int64_t frameTime,LV2_URID propertyUrid, int32_t value)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_int(&outputForge, value);
    lv2_atom_forge_pop(&outputForge, &frame);

}
void Lv2Plugin::PutPatchProperty(int64_t frameTime,LV2_URID propertyUrid, int64_t value)
{
    lv2_atom_forge_frame_time(&outputForge, frameTime);

    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(&outputForge, &frame, 0, urids.patch__Set);

    lv2_atom_forge_key(&outputForge, urids.patch__property);
    lv2_atom_forge_urid(&outputForge, propertyUrid);
    lv2_atom_forge_key(&outputForge, urids.patch__value);
    lv2_atom_forge_long(&outputForge, value);
    lv2_atom_forge_pop(&outputForge, &frame);

}
void Lv2Plugin::BeginAtomOutput(LV2_Atom_Sequence *controlOutput)
{
    const uint32_t notify_capacity = controlOutput->atom.size;
    lv2_atom_forge_set_buffer(
        &(this->outputForge), (uint8_t *)(controlOutput), notify_capacity);
    lv2_atom_forge_sequence_head(&this->outputForge, &outputFrame, urids.units__frame);
}


int32_t Lv2Plugin::GetIntOption(const LV2_Options_Option *option)
{
    if (option->type == urids.atom__Float)
    {
        return (int32_t)*(const float*)(option->value);
    }
    if (option->type == urids.atom__Int)
    {
        return *(const int32_t*)(option->value);
    }
    return -1;
}
void Lv2Plugin::InitBufSizeOptions()
{
    if (this->options)
    {
        for (const LV2_Options_Option *option = this->options; option->key != 0 || option->value != 0; ++option)
        {
            if (option->key == urids.buf_size__maxBlockLength)
            {
                bufSizeOptions.maxBlockLength = GetIntOption(option);
            } else if (option->key == urids.buf_size__minBlockLength)
            {
                bufSizeOptions.minBlockLength = GetIntOption(option);
            } else if (option->key == urids.buf_size__nominalBlockLength)
            {
                bufSizeOptions.nominalBlockLength = GetIntOption(option);
            } else if (option->key == urids.buf_size__sequenceSize)
            {
                bufSizeOptions.sequenceSize = GetIntOption(option);
            }
        }
    }
}



// C init before C++ init. We can use these.
size_t gRegistrationCount = 0;
bool gMaxRegistrationsExceeded = false;
PluginRegistrationBase* gRegistrations[100];


PluginRegistrationBase::PluginRegistrationBase(const std::string &pluginUri)
:pluginUri(pluginUri)
{
    if (gRegistrationCount < 100)
    {
        gRegistrations[gRegistrationCount++] = this;
    } else {
        gMaxRegistrationsExceeded = true; // wait until i/o has been brought up.
    }
}

PluginRegistrationBase::~PluginRegistrationBase()
{
    // should't ever get here unless we're tearing down.
}

const std::string&PluginRegistrationBase::getPluginuri() const
{
    return pluginUri;
}


LV2_Handle 
Lv2Plugin::instantiate(const LV2_Descriptor *descriptor,
                       double rate,
                       const char *bundle_path,
                       const LV2_Feature *const *features)
{

    if (gMaxRegistrationsExceeded)
    {
        std::cout << "Error: Lv2Plugin.cpp max registrations exceeded." << std::endl;
    }
    Lv2Plugin *amp = nullptr;

    for (size_t i = 0; i < gRegistrationCount; ++i)
    {
        auto *registration = gRegistrations[i];
        if (strcmp(registration->getPluginuri().c_str(),descriptor->URI) == 0)
        {
            try {
                amp =  registration->Create(rate,bundle_path,features);
            } catch (const std::exception &e)
            {
                // haven't establish a log feature yet. Just log as we can.
                std::cout << "Error creating plugin " << descriptor->URI << ":" << e.what() << std::endl;
            }
            break;

        }
    }
    return (LV2_Handle)amp;
}

std::vector<LV2_Descriptor*> gDescriptors;

LV2_Descriptor **Lv2Plugin::GetLv2GetDescriptors()
{
    if (gDescriptors.size() == 0)
    {
        for (size_t i = 0; i < gRegistrationCount; ++i)
        {
            auto registration = gRegistrations[i];
            gDescriptors.push_back(
                new LV2_Descriptor{
                    registration->getPluginuri().c_str(),
                    Lv2Plugin::instantiate,
                    Lv2Plugin::connect_port,
                    Lv2Plugin::activate,
                    Lv2Plugin::run,
                    Lv2Plugin::deactivate,
                    Lv2Plugin::cleanup,
                    registration->hasState() ? Lv2Plugin::extension_data_with_state: Lv2Plugin::extension_data
                    }
            );
        }
    }
    return &(gDescriptors[0]);
}


// The LV2 Entry point!
extern "C" {
    LV2_SYMBOL_EXPORT
        const LV2_Descriptor*
        lv2_descriptor(uint32_t index)
    {
        auto descriptors = Lv2Plugin::GetLv2GetDescriptors();

        if (index < gRegistrationCount)
        {
            return descriptors[index];
        }
        return NULL;
    }
}




