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
#include "lv2c_ui/Lv2PortView.hpp"
#include "lv2c/Lv2cSettingsFile.hpp"
#include "lv2c_ui/Lv2PortViewFactory.hpp"
#include "lv2c_ui/Lv2FrequencyPlotElement.hpp"
#include "lv2c_ui/Lv2FileElement.hpp"
#include "lv2c_ui/Lv2FileDialog.hpp"
#include "ss.hpp"
#include "Uri.hpp"

#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cFlexGridElement.hpp"
#include "lv2c/Lv2cScrollContainerElement.hpp"
#include "lv2/ui/ui.h"
#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/atom/util.h"
#include "lv2/core/lv2.h"
#include "lv2/core/lv2_util.h"
#include "lv2/patch/patch.h"
#include "lv2/log/log.h"
#include "lv2/midi/midi.h"
#include "lv2/port-groups/port-groups.h"
#include "lv2/urid/urid.h"
#include "lv2/atom/atom.h"
#include "lv2/options/options.h"
#include <stdio.h>
#include <stdarg.h>
#include <cmath>
#include <cassert>

#include <vector>
#include <string.h>

#include "lv2c/Lv2cGroupElement.hpp"

struct LV2_Atom_Forge_ : public LV2_Atom_Forge
{
};
using namespace lv2c::ui;
using namespace lv2c;
using namespace pipedal;

void Lv2UI::SetCreateWindowDefaults()
{
    Lv2cCreateWindowParameters &params = this->createWindowParameters;
    params.positioning = Lv2cWindowPositioning::ChildWindow;
    if (params.positioning == Lv2cWindowPositioning::NotSet)
    {
        params.positioning = Lv2cWindowPositioning::ChildWindow;
    }
    if (params.size == Lv2cSize::Zero)
    {
        params.size = Lv2cSize(800, 600);
    }
    if (params.minSize == Lv2cSize::Zero)
    {
        params.minSize = Lv2cSize(320, std::min(params.size.Height(),200.0));
    }
    if (params.maxSize == Lv2cSize::Zero)
    {
        params.maxSize = Lv2cSize(4096, 4096);
    }

    // Controls name of settings file which contains, window position, and File Dialog parameters (if any).
    // Use the same format as for a GTK Application ID:  "orgname" "-" "plugin_key"
    if (params.settingsKey.length() == 0)
    {
        params.settingsKey = this->pluginInfo->name();
    }

    if (params.x11Windowclass.length() == 0)
    {
        params.x11Windowclass = "lv2c.github.io-plugin"; // Maybe used for settings by Window Managers.
    }
    if (params.gtkApplicationId.length() == 0)
    {
        params.gtkApplicationId = "lv2c.github.io-plugin"; // Connects KDE Plasma and Gnome Destkop to a .desktop file that has the actual icon reference.
    }
    if (params.title.length() == 0)
    {
        params.title = pluginInfo->name();
    }
    if (createWindowParameters.x11WindowName.length() == 0)
    {
        if (pluginInfo->label().length() != 0)
        {
            params.x11WindowName = pluginInfo->label();
        }
        else
        {
            params.x11WindowName = pluginInfo->name();
        }
    }
}
Lv2UI::Lv2UI(std::shared_ptr<Lv2PluginInfo> pluginInfo, Lv2cSize defaultWindowSize)
    : Lv2UI(pluginInfo, Lv2cCreateWindowParameters())
{
    createWindowParameters.size = defaultWindowSize;
    SetCreateWindowDefaults();
}

static std::string MakeSettingsKey(const std::string &pluginUri)
{
    pipedal::uri uri(pluginUri);

    std::filesystem::path path = uri.authority();
    for (size_t i = 0; i < uri.segment_count(); ++i)
    {
        path = path / uri.segment(i);
    }
    return path;
}
Lv2UI::Lv2UI(std::shared_ptr<Lv2PluginInfo> pluginInfo, const Lv2cCreateWindowParameters &createWindowParameters_)
    : pluginInfo(pluginInfo), createWindowParameters(createWindowParameters_),
      icuInstance(lv2c::IcuString::Instance()) // lifetime managment for Icu libraries.
{

    this->createWindowParameters.settingsKey = MakeSettingsKey(pluginInfo->uri());

    this->createWindowParameters.positioning = Lv2cWindowPositioning::ChildWindow;

    SetCreateWindowDefaults();

    settingsFile = std::make_shared<Lv2cSettingsFile>();

    settingsFile = Lv2cSettingsFile::GetSharedFile(createWindowParameters.settingsKey);
    this->createWindowParameters.settingsObject = settingsFile->Root();

    this->bindingSites.resize(pluginInfo->ports().size());
    this->bindingSiteObserverHandles.resize(pluginInfo->ports().size());
    this->currentHostPortValues.resize(pluginInfo->ports().size());

    for (size_t i = 0; i < pluginInfo->ports().size(); ++i)
    {
        auto &port = pluginInfo->ports()[i];
        if (port.is_control_port())
        {
            auto index = port.index();
            if (bindingSites[index] != nullptr)
            {
                std::cout << "Error: Duplicate port index." << std::endl;
            }
            Lv2cBindingProperty<double> *pBinding = new Lv2cBindingProperty<double>();
            bindingSites[index] = pBinding;
            bindingSites[index]->set(port.default_value());
            this->bindingSiteMap[port.symbol()] = bindingSites[index];

            currentHostPortValues[index] = port.default_value();

            if (port.is_input())
            {
                bindingSiteObserverHandles[index] = pBinding->addObserver([this, index](double value)
                                                                          { this->OnPortValueChanged(index, value); });
            }
        } else if (port.is_atom_port() && port.is_input())
        {
            this->inputAtomPort = port.index();
        }

    }

    this->Theme(Lv2cTheme::Create(true));
    this->portViewFactory = Lv2PortViewFactory::Create();
}

Lv2UI::~Lv2UI()
{
    delete forge;

    if (cairoWindow)
    {
        cairoWindow->CloseRootWindow();
        cairoWindow = nullptr;
    }
    bindingSiteObserverHandles.resize(0);

    for (size_t i = 0; i < bindingSites.size(); ++i)
    {
        delete bindingSites[i];
    }
    bindingSites.resize(0);
    bindingSiteMap.clear();
}

// LV2 callback handlers.
bool Lv2UI::instantiate(
    const char *plugin_ui_uri,
    const char *plugin_uri,
    const char *bundle_path,
    LV2UI_Write_Function write_function,
    LV2UI_Controller controller,
    LV2UI_Widget *widget,
    const LV2_Feature *const *features)
{

    if (this->createWindowParameters.backgroundColor.A() == 0)
    {
        this->createWindowParameters.backgroundColor = Theme()->paper;
    }
    this->pluginUiUri = plugin_ui_uri;
    this->pluginUri = plugin_uri;
    this->bundlePath = bundle_path;
    this->writeFunction = write_function;
    this->controller = controller;
    this->widget = widget;

    LV2_Options_Option *options = nullptr;
    const char *missing = lv2_features_query(
        features,
        LV2_LOG__log, &this->log, false,
        LV2_URID__map, &this->map, true,
        LV2_URID__unmap, &this->unmap, false,
        LV2_UI__requestValue, &this->requestValue, false,
        LV2_OPTIONS__options, &options, false,
        NULL);
    if (missing)
    {
        LogError("Missing " LV2_URID__map "feature.");
        return false;
    }

    this->forge = new LV2_Atom_Forge_();

    lv2_atom_forge_init(this->forge, this->map);

    LV2_URID lv2ui_scaleFactor = this->GetUrid(LV2_UI__scaleFactor);
    if (options)
    {
        for (LV2_Options_Option *p = options; p->key != 0; ++p)
        {
            std::string option = this->UridToString(p->key);
            //            std::cout << "Option: " << option << std::endl;
            if (p->subject == lv2ui_scaleFactor)
            {
                if (p->type == this->urids.atom__Float)
                {
                    this->scaleFactor = *(float *)(p->value);
                }
            }
        }
    }
    InitUrids();

    bool parentWindowFound = false;
    for (int i = 0; features[i] != nullptr; ++i)
    {
        // std::cout << "Feature: " << features[i]->URI << std::endl;
        if (!strcmp(features[i]->URI, LV2_UI__parent))
        {
            parentWindowFound = true;
            this->parentWindow = features[i]->data;
        }
        else if (!strcmp(features[i]->URI, LV2_UI__resize))
        {
            resize = (LV2UI_Resize *)features[i]->data;
        }
    }
    if (!parentWindowFound)
    {
        LogError("Parent window not found.");
        return false;
    }

    // Display *XOpenDisplay(nullptr);
    WindowHandle windowHandle{(uint64_t)this->parentWindow};
    cairoWindow = Lv2cWindow::Create();

    if (theme)
    {
        cairoWindow->Theme(theme);
    }
    cairoWindow->SetResourceDirectories(
        {(std::filesystem::path(this->BundlePath()) / "resources").string()});
    cairoWindow->CreateWindow(windowHandle, createWindowParameters);

    WindowHandle x11Handle = cairoWindow->Handle();

    *widget = (LV2UI_Widget)(void *)x11Handle.getHandle();
    auto ui = this->Render();
    cairoWindow->GetRootElement()->AddChild(ui);
    if (this->resize)
    {
        resize->ui_resize(resize->handle, (int)std::ceil(createWindowParameters.size.Width()), (int)std::ceil(createWindowParameters.size.Height()));
    }

    // request initial value of properties we're interested in.
    // for (auto&frequencyPlot: this->pluginInfo->piPedalUI().frequencyPlots())
    // {
    //     LV2_URID urid = GetUrid(frequencyPlot.patchProperty().c_str());
    //     this->RequestPatchProperty(urid);
    // }

    for (auto&fileProperty: this->pluginInfo->piPedalUI().fileProperties())
    {
         LV2_URID urid = GetUrid(fileProperty.patchProperty().c_str());
         this->RequestPatchProperty(urid);
    }
    return true;
}

void Lv2UI::InitUrids()
{
    urids.log__Error = GetUrid(LV2_LOG__Error);
    urids.log__Note = GetUrid(LV2_LOG__Note);
    urids.log__Trace = GetUrid(LV2_LOG__Trace);
    urids.log__Warning = GetUrid(LV2_LOG__Warning);
    urids.atom__Float = GetUrid(LV2_ATOM__Float);
    urids.atom__eventTransfer = GetUrid(LV2_ATOM__eventTransfer);
    urids.atom__Object = GetUrid(LV2_ATOM__Object);
    urids.atom__URID = GetUrid(LV2_ATOM__URID);
    urids.atom__Resource = GetUrid(LV2_ATOM__Resource);
    urids.atom__Blank = GetUrid(LV2_ATOM__Blank);
    urids.patch__Set = GetUrid(LV2_PATCH__Set);
    urids.patch__property = GetUrid(LV2_PATCH__property);
    urids.patch__value = GetUrid(LV2_PATCH__value);
    urids.patch__Get = GetUrid(LV2_PATCH__Get);
    urids.patch__accept = GetUrid(LV2_PATCH__accept);
    urids.atom__Bool = GetUrid(LV2_ATOM__Bool);
    urids.atom__String = GetUrid(LV2_ATOM__String);
    urids.atom__Path = GetUrid(LV2_ATOM__Path);

}

void Lv2UI::ui_port_event(
    uint32_t port_index,
    uint32_t buffer_size,
    uint32_t format,
    const void *buffer)
{
    if (port_index < this->pluginInfo->ports().size())
    {
        if (pluginInfo->ports()[port_index].is_atom_port())
        {
            if (format == urids.atom__eventTransfer)
            {
                const LV2_Atom *atom = (LV2_Atom *)buffer;
                if (atom->type == urids.atom__Object || atom->type == urids.atom__Resource || atom->type == urids.atom__Blank)
                {
                    const LV2_Atom_Object *object = (const LV2_Atom_Object *)atom;
                    if (object->body.otype == urids.patch__Set)
                    {
                        const LV2_Atom *property = nullptr;
                        const LV2_Atom *value = nullptr;
                        lv2_atom_object_get(object,
                                            urids.patch__property, &property,
                                            urids.patch__value, &value,
                                            0);
                        if (property != nullptr && property->type == urids.atom__URID &&  value != nullptr)
                        {
                            const LV2_Atom_URID *atomUrid = (const LV2_Atom_URID*)property;
                            OnPatchPropertyReceived(atomUrid->body, (uint8_t*)value);
                        }
                    }
                }
            }
        }
        else if (pluginInfo->ports()[port_index].is_control_port())
        {
            if (format == 0) // port notification.
            {
                float value = *(float *)buffer;
                if (port_index >= 0 && port_index < bindingSites.size())
                {
                    this->currentHostPortValues[port_index] = value;
                    auto bindingSite = bindingSites[port_index];
                    if (bindingSite)
                    {
                        bindingSite->set(value);
                    }
                }
            }
        }
    }
}

int Lv2UI::ui_show()
{

    return 0;
}
int Lv2UI::ui_hide()
{
    return 0;
}
int Lv2UI::ui_idle()
{
    if (cairoWindow)
    {
        cairoWindow->PumpMessages(false);
    }
    return 0;
}
void Lv2UI::ui_delete()
{
    CloseFileDialog();

    if (cairoWindow)
    {
        cairoWindow->CloseRootWindow();
    }
    cairoWindow = nullptr;
    delete this;
}

const std::string &Lv2UI::PluginUiUri() const { return pluginUiUri; }
const std::string &Lv2UI::PluginUri() const { return pluginUri; }
const std::string &Lv2UI::BundlePath() const { return bundlePath; }

void Lv2UI::VLogError(const char *format, ...)
{
    if (this->log)
    {
        va_list args;
        va_start(args, format);
        log->vprintf(log->handle, urids.log__Error, format, args);
        va_end(args);
    }
    else
    {
        printf("Error: ");
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
    }
}
void Lv2UI::LogError(const char *message)
{
    if (this->log)
    {
        log->printf(log->handle, urids.log__Error, "%s", message);
    }
    else
    {
        printf("Error: ");
        printf("%s", message);
        printf("\n");
    }
}

void Lv2UI::LogError(const std::string &message)
{
    LogError(message.c_str());
}

void Lv2UI::VLogNote(const char *format, ...)
{
    if (this->log)
    {
        va_list args;
        va_start(args, format);
        log->vprintf(log->handle, urids.log__Note, format, args);
        va_end(args);
    }
    else
    {
        printf("Note: ");
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
    }
}
void Lv2UI::LogNote(const char *message)
{
    if (this->log)
    {
        log->printf(log->handle, urids.log__Note, "%s", message);
    }
    else
    {
        printf("Note: ");
        printf("%s", message);
        printf("\n");
    }
}

void Lv2UI::LogNote(const std::string &message)
{
    LogNote(message.c_str());
}
void Lv2UI::VLogTrace(const char *format, ...)
{
    if (this->log)
    {
        va_list args;
        va_start(args, format);
        log->vprintf(log->handle, urids.log__Trace, format, args);
        va_end(args);
    }
    else
    {
        printf("Trace: ");
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
    }
}
void Lv2UI::LogTrace(const char *message)
{
    if (this->log)
    {
        log->printf(log->handle, urids.log__Trace, "%s", message);
    }
    else
    {
        printf("Trace: ");
        printf("%s", message);
        printf("\n");
    }
}

void Lv2UI::LogTrace(const std::string &message)
{
    LogTrace(message.c_str());
}
void Lv2UI::VLogWarning(const char *format, ...)
{
    if (this->log)
    {
        va_list args;
        va_start(args, format);
        log->vprintf(log->handle, urids.log__Warning, format, args);
        va_end(args);
    }
    else
    {
        printf("Warning: ");
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        printf("\n");
    }
}
void Lv2UI::LogWarning(const char *message)
{
    if (this->log)
    {
        log->printf(log->handle, urids.log__Warning, "%s", message);
    }
    else
    {
        printf("Warning: ");
        printf("%s", message);
        printf("\n");
    }
}

void Lv2UI::LogWarning(const std::string &message)
{
    LogWarning(message.c_str());
}

LV2_URID Lv2UI::GetUrid(const char *urid)
{
    return map->map(map->handle, urid);
}

const char *Lv2UI::UridToString(LV2_URID urid)
{
    if (this->unmap)
    {
        return unmap->unmap(unmap->handle, urid);
    }
    return "#not available.";
}

Lv2cBindingProperty<double> &Lv2UI::GetControlProperty(const std::string &key)
{
    auto f = bindingSiteMap.find(key);
    if (f == bindingSiteMap.end())
    {
        throw std::invalid_argument("Key not found.");
    }
    return *(f->second);
}
const Lv2cBindingProperty<double> &Lv2UI::GetControlProperty(const std::string &key) const
{
    auto f = bindingSiteMap.find(key);
    if (f == bindingSiteMap.end())
    {
        throw std::invalid_argument("Key not found.");
    }
    return *(f->second);
}

Lv2UI &Lv2UI::SetControlValue(const std::string &key, double value)
{
    GetControlProperty(key).set(value);
    return *this;
}
double Lv2UI::GetControlValue(const std::string &key) const
{
    return GetControlProperty(key).get();
}

Lv2cContainerElement::ptr Lv2UI::Render()
{
    Lv2cScrollContainerElement::ptr scrollElement = Lv2cScrollContainerElement::Create();
    scrollElement->HorizontalScrollEnabled(false)
        .VerticalScrollEnabled(true);
    scrollElement->Style().Background(Theme()->paper).HorizontalAlignment(Lv2cAlignment::Stretch).VerticalAlignment(Lv2cAlignment::Stretch);

    scrollElement->Child(RenderControls());
    return scrollElement;
}

Lv2cContainerElement::ptr Lv2UI::RenderControls()
{
    {
        portViewFactory->Theme(this->theme);
        Lv2cContainerElement::ptr container = portViewFactory->CreatePage();

        AddRenderControls(container);
        return container;
    }
}

bool Lv2UI::IsVuMeterPair(size_t portIndex)
{
    auto &port = this->pluginInfo->ports()[portIndex];
    // 1) must be an output port.
    if (!port.is_control_port())
        return false;
    if (!port.is_output())
        return false;

    // 2) must have lv2:designation = pg:left.
    if (port.designation() != LV2_PORT_GROUPS__left)
    {
        return false;
    }
    // 3) must have a next port, which must be an output port.
    if (portIndex + 1 >= this->pluginInfo->ports().size())
        return false;
    auto &rightPort = this->pluginInfo->ports()[portIndex + 1];
    if (!port.is_control_port() || !port.is_output())
        return false;
    // 4) and must have designation lv2:designation = pg:right
    if (rightPort.designation() != LV2_PORT_GROUPS__right)
    {
        return false;
    }
    // and must be members of the same portgroup.
    if (port.port_group() != rightPort.port_group())
    {
        return false;
    }
    return true;
}

static void InsertExtendedControl(
    Lv2cContainerElement::ptr container,  
    std::vector<size_t> *containerIndex,
    size_t index, // index of the child.
    Lv2cElement::ptr child)
{
    size_t position = containerIndex->size();
    for (size_t i = 0; i < containerIndex->size(); ++i)
    {
        if (index <= containerIndex->at(i))
        {
            position = i;
            break;
        }
    }
    container->AddChild(child,position);
    containerIndex->insert(containerIndex->begin()+position,index-1);
}


void Lv2UI::AddRenderControls(Lv2cContainerElement::ptr container)
{
    std::map<std::string, Lv2cGroupElement::ptr> portGroups;

    std::map<std::string, std::vector<size_t>> portGroupControlIndices;
    std::vector<size_t> mainControlIndex;

    for (size_t i = 0; i < this->pluginInfo->ports().size(); ++i)
    {
        auto &port = this->pluginInfo->ports()[i];

        if (port.is_control_port() && !port.not_on_gui())
        {
            if (port.port_group().length() != 0)
            {

                if (IsVuMeterPair(i))
                {
                    auto &rightPort = this->pluginInfo->ports()[i + 1];
                    std::string label;

                    for (const auto &portGroupInfo : pluginInfo->port_groups())
                    {
                        if (portGroupInfo.uri() == port.port_group())
                        {
                            label = portGroupInfo.name();
                            break;
                        }
                    }
                    mainControlIndex.push_back(port.index());
                    container->AddChild(
                        RenderStereoControl(
                            label,
                            GetControlProperty(port.symbol()), port,
                            GetControlProperty(rightPort.symbol()), rightPort));

                    // skip the right control
                    ++i;
                }
                else
                {

                    Lv2cGroupElement::ptr portGroup;
                    std::vector<size_t> *groupIndex;

                    if (portGroups.contains(port.port_group()))
                    {
                        portGroup = portGroups[port.port_group()];
                        groupIndex = &(portGroupControlIndices[port.port_group()]);
                    }
                    else
                    {
                        portGroup = Lv2cGroupElement::Create();
                        portGroups[port.port_group()] = portGroup;
                        portGroupControlIndices[port.port_group()] = std::vector<size_t>();
                        groupIndex = &(portGroupControlIndices[port.port_group()]);

                        mainControlIndex.push_back(port.index());

                        container->AddChild(portGroup);
                        for (const auto &portGroupInfo : pluginInfo->port_groups())
                        {
                            if (portGroupInfo.uri() == port.port_group())
                            {
                                portGroup->Text(portGroupInfo.name());
                                break;
                            }
                        }
                    }
                    groupIndex->push_back(port.index());
                    portGroup->AddChild(
                        RenderControl(
                            GetControlProperty(port.symbol()),
                            port));
                }
            }
            else
            {
                mainControlIndex.push_back(port.index());
                container->AddChild(
                    RenderControl(
                        GetControlProperty(port.symbol()),
                        port));
            }
        }
    }

    for (auto&frequencyPlot: this->pluginInfo->piPedalUI().frequencyPlots())
    {
        Lv2cContainerElement::ptr plotContainer;
        std::vector<size_t> *controlIndex = nullptr;


        if (frequencyPlot.portGroup().length() != 0)
        {
            plotContainer = portGroups[frequencyPlot.portGroup()];
            if (!plotContainer)
            {
                plotContainer = Lv2cGroupElement::Create();
                InsertExtendedControl(container,&mainControlIndex,frequencyPlot.index(),plotContainer);
            }
            controlIndex = &(portGroupControlIndices[frequencyPlot.portGroup()]);

        } else {
            plotContainer = container;
            controlIndex = &mainControlIndex;
        }
        auto plotControl = Lv2FrequencyPlotElement::Create(this,&frequencyPlot);
        auto size = this->portViewFactory->DefaultSize();
        plotControl->Style()
            .Height(size.Height()-16)
            .Width(frequencyPlot.width())
            .MarginRight(16);
            ;
        
        InsertExtendedControl(plotContainer,controlIndex,frequencyPlot.index(),plotControl);
    }
    for (auto&fileProperty: this->pluginInfo->piPedalUI().fileProperties())
    {
        Lv2cContainerElement::ptr fileContainer;
        std::vector<size_t> *controlIndex = nullptr;


        if (fileProperty.portGroup().length() != 0)
        {
            fileContainer = portGroups[fileProperty.portGroup()];
            if (!fileContainer)
            {
                fileContainer = Lv2cGroupElement::Create();
                InsertExtendedControl(container,&mainControlIndex,fileProperty.index(),fileContainer);
            }
            controlIndex = &(portGroupControlIndices[fileProperty.portGroup()]);

        } else {
            fileContainer = container;
            controlIndex = &mainControlIndex;
        }
        auto fileElement = RenderFileControl(fileProperty);
        
        InsertExtendedControl(fileContainer,controlIndex,fileProperty.index(),fileElement);
    }

}

Lv2cElement::ptr Lv2UI::RenderStereoControl(
    const std::string &label,
    Lv2cBindingProperty<double> &leftValue, const Lv2PortInfo &leftPortInfo,
    Lv2cBindingProperty<double> &rightValue, const Lv2PortInfo &rightPortInfo)
{
    return CreateStereoPortView(label, leftValue, rightValue, leftPortInfo, this->portViewFactory);
}

Lv2cElement::ptr Lv2UI::RenderControl(Lv2cBindingProperty<double> &value, const Lv2PortInfo &portInfo)
{
    return CreatePortView(value, portInfo, this->portViewFactory);
}

void Lv2UI::OnPortValueChanged(int32_t portIndex, double value)
{
    float floatValue = (float)value;
    if (this->controller != nullptr)
    {
        if (floatValue != this->currentHostPortValues[portIndex])
        {
            this->currentHostPortValues[portIndex] = floatValue;
            this->writeFunction(
                this->controller,
                portIndex,
                sizeof(float),
                0, // 0 => ui:floatProtocol
                &floatValue);
        }
    }
}

Lv2cTheme::ptr Lv2UI::Theme()
{
    if (cairoWindow)
    {
        return cairoWindow->ThemePtr();
    }
    return theme;
}

Lv2UI &Lv2UI::Theme(Lv2cTheme::ptr theme)
{
    this->theme = theme;
    if (cairoWindow)
    {
        cairoWindow->Theme(theme);
    }
    return *this;
}

Lv2PortViewFactory &Lv2UI::PortViewFactory() const
{
    return *(portViewFactory.get());
}
Lv2UI &Lv2UI::PortViewFactory(std::shared_ptr<Lv2PortViewFactory> value)
{
    this->portViewFactory = value;
    return *this;
}

int Lv2UI::ui_resize(int width, int height)
{
    if (this->cairoWindow)
    {
        //  cairoWindow->Resize(width,height);
    }
    return 0;
}

void Lv2UI::OnPatchPropertyReceived(LV2_URID type, const uint8_t *data)
{
    const LV2_Atom*atom = (const LV2_Atom*)data;
    if (atom->type == urids.atom__Path || atom->type == urids.atom__String)
    {
        auto f = filePropertyBindingSites.find(type);
        if (f != filePropertyBindingSites.end())
        {
            const char*value = (const char*)LV2_ATOM_CONTENTS(LV2_Atom_String,atom);
            f->second->set(value);
        }
    }


    PatchPropertyEventArgs eventArgs { type,data};
    
    OnPatchProperty.Fire(eventArgs);
}

void Lv2UI::RequestPatchProperty(LV2_URID property)
{
    lv2_atom_forge_set_buffer(forge, patchRequestBuffer, sizeof(patchRequestBuffer));
    LV2_Atom_Forge_Frame frame;
    lv2_atom_forge_object(forge, &frame, 0, urids.patch__Get);
    lv2_atom_forge_key(forge, urids.patch__accept);
    lv2_atom_forge_urid(forge, property);
    lv2_atom_forge_pop(forge, &frame);

    LV2_Atom *msg = lv2_atom_forge_deref(forge, frame.ref);
    if (inputAtomPort == (uint32_t)-1)
    {
        LogError("RequestPatchProperty: plugin does not have an input atom port.");
    } else {
        this->writeFunction(controller,
            inputAtomPort,        
            lv2_atom_total_size(msg),
            urids.atom__eventTransfer,
            msg);
    }

}

void Lv2UI::WritePatchProperty(LV2_URID property,const LV2_Atom *value)
{
    size_t messageSize = 
        value->size + (sizeof(LV2_Atom) 
        + sizeof(LV2_Atom_Object) + sizeof(LV2_Atom_Property)*2 
        + sizeof(LV2_Atom_URID)+20+4);
    

    std::vector<uint8_t> buffer;
    buffer.resize(messageSize);

    lv2_atom_forge_set_buffer(forge, &buffer[0], buffer.size());


	LV2_Atom_Forge_Frame objectFrame;

	lv2_atom_forge_object(forge, &objectFrame, 0, urids.patch__Set);

	lv2_atom_forge_key(forge, urids.patch__property);
	lv2_atom_forge_urid(forge, property);

	lv2_atom_forge_key(forge, urids.patch__value);
    lv2_atom_forge_primitive(forge,value);

	lv2_atom_forge_pop(forge, &objectFrame);


    LV2_Atom *msg = (LV2_Atom*)&(buffer[0]);

    assert(msg->size + sizeof(LV2_Atom) <= buffer.size());

    if (inputAtomPort == (uint32_t)-1)
    {
        LogError("WritePatchProperty: plugin does not have an input atom port.");
    } else {
        this->writeFunction(controller,
            inputAtomPort,        
            lv2_atom_total_size(msg),
            urids.atom__eventTransfer,
            msg);
    }

}
void Lv2UI::WritePatchProperty(LV2_URID property,bool value)
{
    LV2_Atom_Bool atom;
    atom.atom.size = sizeof(atom.body);
    atom.atom.type = urids.atom__Bool;
    atom.body = value? 1: 0;
    WritePatchProperty(property,(const LV2_Atom*)&atom);
}
void Lv2UI::WritePatchProperty(LV2_URID property,float value)
{
    LV2_Atom_Float atom;
    atom.atom.size = sizeof(atom.body);
    atom.atom.type = urids.atom__Float;
    atom.body = value;
    WritePatchProperty(property,(const LV2_Atom*)&atom);

}
void Lv2UI::WritePatchProperty(LV2_URID property,const std::string& value)
{
    size_t atomSize = sizeof(LV2_Atom) + value.length() + 1;
    std::vector<uint8_t> atomBuffer;
    atomBuffer.resize(atomSize);
    uint8_t *pBuffer = &(atomBuffer[0]);
    LV2_Atom_String *atom = (LV2_Atom_String*)pBuffer;
    atom->atom.type = urids.atom__String;
    atom->atom.size = value.length()+1;
    char*pAtomString = (char*)(pBuffer + sizeof(LV2_Atom));

    // github build generates a warning here, even though it's correct.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
// xxx check this with a debugger. why does github gcc dislike this?!
    strcpy(pAtomString,value.c_str());
#pragma GCC diagnostic pop    
    WritePatchProperty(property,(const LV2_Atom*)pBuffer);
}

Lv2cElement::ptr Lv2UI::RenderFileControl(const UiFileProperty &fileProperty)
{
    auto container = this->portViewFactory->CreateContainer();
    container->AddChild(this->portViewFactory->CreateCaption(fileProperty.label(),Lv2cAlignment::Start));
    container->Style()
        .Width(portViewFactory->DefaultSize().Width()*2)
        ;

    auto midChild = Lv2cContainerElement::Create();
    midChild->Style()
        .VerticalAlignment(Lv2cAlignment::Stretch)
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        ;

    container->AddChild(midChild);
    auto fileElement = Lv2FileElement::Create();
    fileElement->Style()
        .HorizontalAlignment(Lv2cAlignment::Stretch)
        .VerticalAlignment(Lv2cAlignment::Center)
        ;
    midChild->AddChild(fileElement);

    auto spacer = Lv2cElement::Create();
    spacer->Style()
        .Width(1)
        .Height(portViewFactory->EditControlHeight());
    container->AddChild(spacer);

    std::shared_ptr<Lv2cBindingProperty<std::string>> bindingProperty 
        = std::make_shared<Lv2cBindingProperty<std::string>>();
    bindingProperty->set("");
    
    filePropertyBindingSites[GetUrid(fileProperty.patchProperty().c_str())] = bindingProperty;
    bindingProperty->Bind(fileElement->FilenameProperty);

    fileElement->PatchProperty(fileProperty.patchProperty());
    
    std::string patchProperty = fileProperty.patchProperty();
    auto handle = fileElement->Clicked.AddListener(
        [this,patchProperty](const Lv2cMouseEventArgs &args)
        {
            SelectFile(patchProperty);
            return true;
        }
    );
    fileElementClickedHandles.push_back(handle);
    this->RequestPatchProperty(GetUrid(fileProperty.patchProperty().c_str()));
    return container;
}

void Lv2UI::CloseFileDialog()
{
    okListenerHandle = EventHandle::InvalidHandle;
    cancelListenerHandle = EventHandle::InvalidHandle;
    if (fileDialog)
    {
        fileDialog->Close();
        fileDialog = nullptr;
    }

}
void Lv2UI::SelectFile(const std::string&patchProperty)
{
    CloseFileDialog();

    const UiFileProperty*pProperty = nullptr;
    for (auto&fileProperty : this->pluginInfo->piPedalUI().fileProperties())
    {
        if (fileProperty.patchProperty() == patchProperty)
        {
            pProperty = &fileProperty;
            break;
        }
    }
    if (pProperty == nullptr) 
    {
        LogError(SS("Can't find fileProperty " << patchProperty));

        return;
    }
    fileDialog = Lv2FileDialog::Create(pProperty->label(),"propertyDlg-" + patchProperty);
    fileDialog->ShowClearValue(true);
    
    std::vector<Lv2FileFilter> fileTypes;
    if (pProperty->fileTypes().size() > 1)
    {
        std::stringstream s;
        bool firstLabel = true;
        Lv2FileFilter filter;
        for (auto&fileType: pProperty->fileTypes())
        {
            if (!firstLabel)
            {
                s << ", ";
            }
            s << fileType.label();
            firstLabel = false;

            filter.extensions.push_back(fileType.fileExtension());
            filter.mimeTypes.push_back(fileType.mimeType());
        }
        filter.label = s.str();
        fileTypes.push_back(filter);
    }
    for (auto&fileType: pProperty->fileTypes())
    {
        Lv2FileFilter filter;
        filter.label = fileType.label();
        filter.extensions.push_back(fileType.fileExtension());
        filter.mimeTypes.push_back(fileType.mimeType());
        fileTypes.push_back(filter);
    }
    {
        Lv2FileFilter filter;
        filter.label = "All files";
        fileTypes.push_back(filter);
    }
    fileDialog->FileTypes(
        fileTypes
    );

    std::string defaultDirectory = pProperty->resourceDirectory();
    if (defaultDirectory.length() != 0)
    {
        fileDialog->DefaultDirectory(
            (std::filesystem::path(this->BundlePath()) / std::filesystem::path(defaultDirectory))
        );

        Lv2FileDialog::Lv2cFilePanel filePanel {
            "Plugin",
            "com.twoplay.lv2cairo.plugin.svg",
            fileDialog->DefaultDirectory()
        };
        fileDialog->AddPanel(2,filePanel);

    }
    LV2_URID propertyUrid = GetUrid(pProperty->patchProperty().c_str());

    okListenerHandle = fileDialog->OK.AddListener(
        [this,propertyUrid](const std::string&result)
        {
            this->fileDialog = nullptr;
            OnPatchPropertySelected(propertyUrid,result);
            return true;
        }
    );
    cancelListenerHandle = fileDialog->Cancelled.AddListener(
        [this](void) {
            this->fileDialog = nullptr;
            return true;
        }
    );
    fileDialog->Show(this->Window().get());
}


void Lv2UI::OnPatchPropertySelected(LV2_URID patchProperty, const std::string&filename)
{
    this->WritePatchProperty(patchProperty,filename);
    auto f = this->filePropertyBindingSites.find(patchProperty);
    if (f != filePropertyBindingSites.end())
    {
        (*f).second->set(filename);
    }
}

