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

#include <string>

#include <concepts>
#include <memory>

#include "Lv2UI_NativeCallbacks.hpp"
#include "lvtk/IcuString.hpp"
#include "Lv2PluginInfo.hpp"
#include "lvtk/LvtkElement.hpp"
#include "lvtk/LvtkContainerElement.hpp"
#include "lvtk/LvtkBindingProperty.hpp"
#include "lvtk/LvtkWindow.hpp"

#include <unordered_map>

namespace lvtk
{
    class LvtkWindow;
    class LvtkSettingsFile;
}
namespace lvtk::ui
{
    class Lv2PortViewFactory;

    class Lv2UI : public Lv2NativeCallbacks
    {
    private:
        Lv2UI(const Lv2UI &) = delete;
        Lv2UI(Lv2UI &&) = delete;

    public:
        Lv2UI(std::shared_ptr<Lv2PluginInfo> pluginInfo, LvtkSize defaultWindowSize);
        Lv2UI(std::shared_ptr<Lv2PluginInfo> pluginInfo, const LvtkCreateWindowParameters& windowParameters);
        virtual ~Lv2UI();

        Lv2UI& Theme(LvtkTheme::ptr theme);
        LvtkTheme::ptr Theme();

        Lv2UI& PortViewFactory(std::shared_ptr<Lv2PortViewFactory> value);
        Lv2PortViewFactory&PortViewFactory() const;
        
        const std::string &PluginUiUri() const;
        const std::string &PluginUri() const;
        const std::string &BundlePath() const;

        LV2_URID GetUrid(const char *urid);
        const char *UridToString(LV2_URID);

        void VLogError(const char *format, ...);
        void LogError(const char *message);
        void LogError(const std::string &message);

        void VLogNote(const char *format, ...);
        void LogNote(const char *message);
        void LogNote(const std::string &message);

        void VLogTrace(const char *format, ...);
        void LogTrace(const char *message);
        void LogTrace(const std::string &message);

        void VLogWarning(const char *format, ...);
        void LogWarning(const char *message);
        void LogWarning(const std::string &message);

        const Lv2PluginInfo &PluginInfo() const { return *(pluginInfo.get()); }


        virtual LvtkContainerElement::ptr Render();
        virtual LvtkContainerElement::ptr RenderControls();

        Lv2UI&SetControlValue(const std::string&key, double value);
        double GetControlValue(const std::string&key) const;


    protected:
        LvtkBindingProperty<double>&GetControlProperty(const std::string&key);
        const LvtkBindingProperty<double>&GetControlProperty(const std::string&key) const;


        virtual void AddRenderControls(LvtkContainerElement::ptr container);
        virtual LvtkElement::ptr RenderControl(LvtkBindingProperty<double>&value,const Lv2PortInfo&portInfo);
        virtual LvtkElement::ptr RenderStereoControl(
            const std::string&label,
            LvtkBindingProperty<double>&leftValue,const Lv2PortInfo&leftPortInfo,
            LvtkBindingProperty<double>&rightValue,const Lv2PortInfo&rightPortInfo
            );

        virtual void OnPatchPropertyReceived(LV2_URID type, const void*data);
        
    private:
        IcuString::Ptr icuInstance; // lifetime managment for Icu libraries.
        float scaleFactor = 1.0;

        bool IsVuMeterPair(size_t portIndex);

        std::shared_ptr<Lv2PortViewFactory> portViewFactory;
        void SetCreateWindowDefaults();
        std::shared_ptr<LvtkSettingsFile> settingsFile;

        LvtkTheme::ptr theme;
        LvtkCreateWindowParameters createWindowParameters;
        std::vector<LvtkBindingProperty<double> *> bindingSites;
        std::vector<Observable<double>::handle_t> bindingSiteObserverHandles;
        std::vector<double> currentHostPortValues;

        void OnPortValueChanged(int32_t portIndex, double value);
        std::unordered_map<std::string,LvtkBindingProperty<double>*> bindingSiteMap;

        std::shared_ptr<Lv2PluginInfo> pluginInfo;
        void InitUrids();

        struct Urids
        {
            LV2_URID log__Error;
            LV2_URID log__Note;
            LV2_URID log__Warning;
            LV2_URID log__Trace;
            LV2_URID atom__Float;
            LV2_URID atom__eventTransfer;
            LV2_URID atom__Object;
            LV2_URID atom__Resource;
            LV2_URID atom__Blank;
            LV2_URID patch__Set;
            LV2_URID patch__property;
            LV2_URID patch__value;
        };
        Urids urids;
        // LV2 callback handlers.
        virtual bool instantiate(
            const char *plugin_ui_uri,
            const char *plugin_uri,
            const char *bundle_path,
            LV2UI_Write_Function write_function,
            LV2UI_Controller controller,
            LV2UI_Widget *widget,
            const LV2_Feature *const *features) override;

        virtual void ui_port_event(
            uint32_t port_index,
            uint32_t buffer_size,
            uint32_t format,
            const void *buffer) override;
        virtual int ui_show() override;
        virtual int ui_hide() override;
        virtual int ui_idle() override;
        virtual void ui_delete() override;
        virtual int ui_resize(int width, int height) override;

        std::string pluginUiUri;
        std::string pluginUri;
        std::string bundlePath;
        LV2UI_Write_Function writeFunction = nullptr;
        LV2UI_Controller controller = nullptr;
        LV2UI_Widget *widget = nullptr;
        void *parentWindow = nullptr;

        LV2_Log_Log *log = nullptr;
        LV2_URID_Map *map = nullptr;
        LV2_URID_Unmap *unmap = nullptr;
        LV2UI_Resize *resize = nullptr;
        LV2UI_Request_Value *requestValue = nullptr;

        std::shared_ptr<LvtkWindow> cairoWindow;
    };

    class Lv2UIRegistrationBase
    {
    protected:
        Lv2UIRegistrationBase(const char *pluginUri);
        virtual ~Lv2UIRegistrationBase();

    public:
        const std::string &PluginUri() const { return pluginUri; }
        virtual Lv2UI *Create() = 0;

    private:
        std::string pluginUri;
    };

    template <class T>
        requires std::derived_from<T, Lv2UI>
    class Lv2UIRegistration : public Lv2UIRegistrationBase
    {
    public:
        Lv2UIRegistration(const char *plugin_uri) : Lv2UIRegistrationBase(plugin_uri) {}

        virtual Lv2UI *Create() override { return new T(); }
    };
}