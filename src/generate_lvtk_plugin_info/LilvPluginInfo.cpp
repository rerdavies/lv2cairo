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

#include "LilvPluginInfo.hpp"
#include "LilvPiPedalUI.hpp"
#include "AutoLilvNode.hpp"
#include "lv2/atom/atom.h"
#include "lv2/port-props/port-props.h"
#include "lv2/port-groups/port-groups.h"
#include "lv2/core/lv2.h"
#include "lv2/midi/midi.h"
#include "lv2/time/time.h"
#include "lv2/units/units.h"
#include "lv2/presets/presets.h"
#include "lv2/patch/patch.h"

#include <iostream>

#include <string.h>


using namespace lvtk::ui;
using namespace lv2;

static constexpr const char *RDFS__comment = "http://www.w3.org/2000/01/rdf-schema#"
                                             "comment";

static constexpr const char *RDFS__range = "http://www.w3.org/2000/01/rdf-schema#"
                                           "range";

static bool scale_points_sort_compare(const Lv2ScalePoint &v1, const Lv2ScalePoint &v2)
{
    return v1.value() < v2.value();
}


static std::string trimComment(const std::string &value)
{
    auto begin = value.begin();
    auto end = value.end();
    while (begin != end && *begin == '\n')
    {
        ++begin;
    }
    while (begin != end && *(end-1) == '\n')
    {
        --end;
    }
    return std::string(begin,end);
}

static std::vector<std::string> nodeAsStringArray(const LilvNodes *nodes)
{
    std::vector<std::string> result;
    LILV_FOREACH(nodes, iNode, nodes)
    {
        AutoLilvNode t{lilv_nodes_get(nodes, iNode)};
        result.push_back(t.AsString());
    }
    return result;
}

static Lv2PortGroup GetLv2PortGroup(LilvWorld*pWorld, const std::string &groupUri)
{
    Lv2PortGroup result;

    result.uri(groupUri);
    AutoLilvNode uri = lilv_new_uri(pWorld, groupUri.c_str());

    AutoLilvNode lv2core__symbol = lilv_new_uri(pWorld, LV2_CORE__symbol);
    AutoLilvNode lv2core__name = lilv_new_uri(pWorld, LV2_CORE__name);

    AutoLilvNode symbolNode = lilv_world_get(pWorld, uri, lv2core__symbol, nullptr);
    result.symbol(symbolNode.AsString());
    AutoLilvNode nameNode = lilv_world_get(pWorld, uri, lv2core__name, nullptr);
    result.name(nameNode.AsString());

    return result;
}




static bool ports_sort_compare(const Lv2PortInfo &p1, const Lv2PortInfo &p2)
{
    return p1.index() < p2.index();
}


static Lv2PluginClasses GetPluginPortClass(const LilvPlugin *lilvPlugin, const LilvPort *lilvPort)
{
    std::vector<std::string> result;
    const LilvNodes *nodes = lilv_port_get_classes(lilvPlugin, lilvPort);
    if (nodes != nullptr)
    {
        LILV_FOREACH(nodes, iNode, nodes)
        {
            AutoLilvNode node = lilv_nodes_get(nodes, iNode);
            std::string classUri = node.AsString();
            result.push_back(classUri);
        }
    }
    return Lv2PluginClasses(result);
}


LilvPortInfo::LilvPortInfo(LilvWorld *pWorld, const LilvPlugin *plugin, const LilvPort *pPort)
{
    index_ = lilv_port_get_index(plugin, pPort);
    symbol_ = AutoLilvNode(lilv_port_get_symbol(plugin, pPort)).AsString();

    AutoLilvNode name = lilv_port_get_name(plugin, pPort);
    name_ = name.AsString();

    classes_ = GetPluginPortClass(plugin, pPort);

    AutoLilvNode minNode, maxNode, defaultNode;
    min_value_ = 0;
    max_value_ = 1;
    default_value_ = 0;
    lilv_port_get_range(plugin, pPort, defaultNode, minNode, maxNode);
    if (defaultNode)
    {
        default_value_ = defaultNode.AsFloat();
    }
    if (minNode)
    {
        min_value_ = minNode.AsFloat();
    }
    if (maxNode)
    {
        max_value_ = (maxNode).AsFloat();
    }
    if (default_value_ > max_value_)
        default_value_ = max_value_;
    if (default_value_ < min_value_)
        default_value_ = min_value_;

    AutoLilvNode port_logarithmic = lilv_new_uri(pWorld, LV2_PORT_PROPS__logarithmic);
    this->is_logarithmic_ = lilv_port_has_property(plugin, pPort, port_logarithmic);

    // typo in invada plugins.

    AutoLilvNode invada_portprops__logarithmic = lilv_new_uri(pWorld, "http://lv2plug.in/ns/dev/extportinfo#logarithmic"); // a typo in invada plugin ttl files.

    this->is_logarithmic_ |= lilv_port_has_property(plugin, pPort, invada_portprops__logarithmic);

    AutoLilvNode port__display_priority = lilv_new_uri(pWorld, LV2_PORT_PROPS__displayPriority);
    AutoLilvNodes priority_nodes = lilv_port_get_value(plugin, pPort, port__display_priority);

    this->display_priority_ = -1;
    if (priority_nodes)
    {
        auto priority_node = lilv_nodes_get_first(priority_nodes);
        if (priority_node)
        {
            this->display_priority_ = lilv_node_as_int(priority_node);
        }
    }

    AutoLilvNode port_range_steps = lilv_new_uri(pWorld, LV2_PORT_PROPS__rangeSteps);;
    AutoLilvNodes range_steps_nodes = lilv_port_get_value(plugin, pPort, port_range_steps);
    this->range_steps_ = 0;
    if (range_steps_nodes)
    {
        auto range_steps_node = lilv_nodes_get_first(range_steps_nodes);
        if (range_steps_node)
        {
            this->range_steps_ = lilv_node_as_int(range_steps_node);
        }
    }
    AutoLilvNode integer_property_uri = lilv_new_uri(pWorld, LV2_CORE__integer);
    this->integer_property_ = lilv_port_has_property(plugin, pPort, integer_property_uri);

    AutoLilvNode enumeration_property_uri = lilv_new_uri(pWorld, LV2_CORE__enumeration);
    this->enumeration_property_ = lilv_port_has_property(plugin, pPort, enumeration_property_uri);

    AutoLilvNode core__toggled = lilv_new_uri(pWorld, LV2_CORE__toggled);
    this->toggled_property_ = lilv_port_has_property(plugin, pPort, core__toggled);

    AutoLilvNode portprops__not_on_gui_property_uri = lilv_new_uri(pWorld, LV2_PORT_PROPS__notOnGUI);
    this->not_on_gui_ = lilv_port_has_property(plugin, pPort, portprops__not_on_gui_property_uri);

    AutoLilvNode core__connectionOptional = lilv_new_uri(pWorld, LV2_CORE__connectionOptional);
    this->connection_optional_ = lilv_port_has_property(plugin, pPort, core__connectionOptional);

    LilvScalePoints *pScalePoints = lilv_port_get_scale_points(plugin, pPort);
    LILV_FOREACH(scale_points, iSP, pScalePoints)
    {
        const LilvScalePoint *pSP = lilv_scale_points_get(pScalePoints, iSP);
        float value = AutoLilvNode(lilv_scale_point_get_value(pSP)).AsFloat();
        auto label = AutoLilvNode(lilv_scale_point_get_label(pSP)).AsString();
        scale_points_.push_back(Lv2ScalePoint(value, label));
    }
    lilv_scale_points_free(pScalePoints);

    // sort is what we want, but the implementation is broken in g++ 8.3

    std::sort(scale_points_.begin(), scale_points_.end(), scale_points_sort_compare);

    is_input_ = is_a(LV2_CORE__InputPort);
    is_output_ = is_a(LV2_CORE__OutputPort);

    is_control_port_ = is_a(LV2_CORE__ControlPort);
    is_audio_port_ = is_a(LV2_CORE__AudioPort);
    is_atom_port_ = is_a(LV2_ATOM__AtomPort);
    is_cv_port_ = is_a(LV2_CORE__CVPort);


    AutoLilvNode midi__event = lilv_new_uri(pWorld, LV2_MIDI__MidiEvent);
    supports_midi_ = lilv_port_supports_event(plugin, pPort, midi__event);

    AutoLilvNode time_Position = lilv_new_uri(pWorld, LV2_TIME__Position);
    supports_time_position_ = lilv_port_supports_event(plugin, pPort, time_Position);

    AutoLilvNode core__designation = lilv_new_uri(pWorld, LV2_CORE__designation);
    AutoLilvNode designationValue = lilv_port_get(plugin, pPort, core__designation);
    designation_ = designationValue.AsUri();

    AutoLilvNode portgroups__group = lilv_new_uri(pWorld, LV2_PORT_GROUPS__group);;
    AutoLilvNode portGroup_value = lilv_port_get(plugin, pPort,portgroups__group);
    port_group_ = portGroup_value.AsUri();

    AutoLilvNode units__unit = lilv_new_uri(pWorld, LV2_UNITS__unit);
    AutoLilvNode unitsValueUri = lilv_port_get(plugin, pPort, units__unit);
    if (unitsValueUri)
    {
        this->units_ = UriToUnits(unitsValueUri.AsUri());
    }
    else
    {
        // invada plugins use the wrong URI.
        AutoLilvNode invada_units__unit = lilv_new_uri(
            pWorld,
            "http://lv2plug.in/ns/extension/units#unit"); // a typo in invada plugin ttl files, or a legacy implementation?

        AutoLilvNode invadaUnitsValueUri = lilv_port_get(plugin, pPort, invada_units__unit);
        if (invadaUnitsValueUri)
        {
            std::string uri = invadaUnitsValueUri.AsUri();
            static const char *INCORRECT_URI = "http://lv2plug.in/ns/extension/units#";
            static const char *CORRECT_URI = "http://lv2plug.in/ns/extensions/units#";
            if (uri.starts_with(INCORRECT_URI))
            {
                uri = uri.replace(0, strlen(INCORRECT_URI), CORRECT_URI);
            }
            this->units_ = UriToUnits(uri);
        }
        else
        {
            this->units(Lv2Units::none);
        }
    }

    AutoLilvNode rdfs__Comment = lilv_new_uri(pWorld, RDFS__comment);

    AutoLilvNode commentNode = lilv_port_get(plugin, pPort, rdfs__Comment);
    this->comment_ = trimComment(commentNode.AsString());

    AutoLilvNode atom__bufferType = lilv_new_uri(pWorld, LV2_ATOM__bufferType);
    AutoLilvNode bufferType = lilv_port_get(plugin, pPort, atom__bufferType);

    this->buffer_type_ = Lv2BufferType::None;
    if (bufferType)
    {
        this->buffer_type_ =  GetBufferType(bufferType.AsUri());
    }

    is_valid_ = is_control_port_ ||
                ((is_input_ || is_output_) && (is_audio_port_ || is_atom_port_ || is_cv_port_));
}


LilvPluginInfo::LilvPluginInfo(LilvWorld *pWorld, const LilvPlugin *pPlugin)
{
    AutoLilvNode bundleUriNode = lilv_plugin_get_bundle_uri(pPlugin);
    if (!bundleUriNode)
        throw std::runtime_error("Invalid bundle uri.");

    std::string bundleUri = bundleUriNode.AsUri();

    std::string bundlePath = lilv_file_uri_parse(bundleUri.c_str(), nullptr);
    if (bundlePath.length() == 0)
        throw std::runtime_error("Bundle uri is not a file uri.");

    this->bundle_path_ = bundlePath;

    this->has_factory_presets_ = HasFactoryPresets(pWorld, pPlugin);

    AutoLilvNode plugUri = lilv_plugin_get_uri(pPlugin);
    this->uri_ = plugUri.AsUri();

    AutoLilvNode name = (lilv_plugin_get_name(pPlugin));
    this->name_ = name.AsString();

#define MOD_PREFIX "http://moddevices.com/ns/mod#"
    AutoLilvNode mod__label = lilv_new_uri(pWorld, MOD_PREFIX "label");
    AutoLilvNode mod__brand = lilv_new_uri(pWorld, MOD_PREFIX "brand");

    AutoLilvNode brand = lilv_world_get(pWorld, plugUri, mod__brand, nullptr);
    this->brand_ = brand.AsString();

    AutoLilvNode label = lilv_world_get(pWorld, plugUri, mod__label, nullptr);
    this->label_ = label.AsString();
    if (label_.length() == 0)
    {
        this->label_ = this->name_;
    }

    AutoLilvNode author_name = (lilv_plugin_get_author_name(pPlugin));
    this->author_name_ = author_name.AsString();

    AutoLilvNode author_homepage = (lilv_plugin_get_author_homepage(pPlugin));
    this->author_homepage_ = author_homepage.AsString();

    const LilvPluginClass *pClass = lilv_plugin_get_class(pPlugin);
    AutoLilvNode classUri = lilv_plugin_class_get_uri(pClass);
    this->plugin_class_ = classUri.AsUri();

    AutoLilvNodes required_features = lilv_plugin_get_required_features(pPlugin);
    this->required_features_ = nodeAsStringArray(required_features);

    AutoLilvNodes supported_features = lilv_plugin_get_supported_features(pPlugin);
    this->supported_features_ = nodeAsStringArray(supported_features);

    AutoLilvNodes optional_features = lilv_plugin_get_optional_features(pPlugin);
    this->optional_features_ = nodeAsStringArray(optional_features);

    AutoLilvNodes extensions = lilv_plugin_get_extension_data(pPlugin);
    this->extensions_ = nodeAsStringArray(extensions);

    AutoLilvNode rdfs__Comment = lilv_new_uri(pWorld, RDFS__comment);
    AutoLilvNode comment = lilv_world_get(pWorld, plugUri, rdfs__Comment, nullptr);
    this->comment_ = trimComment(comment.AsString());

    uint32_t ports = lilv_plugin_get_num_ports(pPlugin);

    bool isValid = true;
    std::vector<std::string> portGroups;

    for (uint32_t i = 0; i < ports; ++i)
    {
        const LilvPort *pPort = lilv_plugin_get_port_by_index(pPlugin, i);

        LilvPortInfo portInfo { pWorld, pPlugin, pPort };
        if (!portInfo.is_valid())
        {
            isValid = false;
        }
        const auto &portGroup = portInfo.port_group();
        if (portGroup.size() != 0)
        {
            if (std::find(portGroups.begin(), portGroups.end(), portGroup) == portGroups.end())
            {
                portGroups.push_back(portGroup);
            }
        }
        ports_.push_back(portInfo);
    }

    for (auto &portGroup : portGroups)
    {
        Lv2PortGroup pg = GetLv2PortGroup(pWorld,portGroup);

        port_groups_.push_back(pg);
    }

    std::sort(ports_.begin(), ports_.end(), ports_sort_compare);

    // Fetch pipedal plugin UI

#if LV2_PIPEDAL_UI


    AutoLilvNode pipedalUI__ui = lilv_new_uri(pWorld, PIPEDAL_UI__ui);
    AutoLilvNode pipedalUINode = lilv_world_get(
        pWorld,
        lilv_plugin_get_uri(pPlugin),
        pipedalUI__ui,
        nullptr);
    if (pipedalUINode)
    {
        this->piPedalUI_ = LilvPiPedalUI(pWorld, pipedalUINode, std::filesystem::path(bundlePath));
    }
    else
    {
        // look for
        this->piPedalUI_ = FindWritablePathProperties(pWorld, pPlugin);
    }
#endif
    int nInputs = 0;
    for (size_t i = 0; i < ports_.size(); ++i)
    {
        auto& port = ports_[i];
        if (port.is_audio_port() && port.is_input())
        {
            if (nInputs >= 2 && !port.connection_optional())
            {
                isValid = false;
                break;
            }
            ++nInputs;
        }
    }
    int nOutputs = 0;
    for (size_t i = 0; i < ports_.size(); ++i)
    {
        auto port = ports_[i];
        if (port.is_audio_port() && port.is_output())
        {
            if (nOutputs >= 2 && !port.connection_optional())
            {
                isValid = false;
                break;
            }
            ++nOutputs;
        }
    }

    this->is_valid_ = isValid;
}


bool LilvPluginInfo::HasFactoryPresets(LilvWorld*world,const LilvPlugin *plugin)
{
    AutoLilvNode presets__preset = lilv_new_uri(world, LV2_PRESETS__Preset);
    AutoLilvNodes nodes = lilv_plugin_get_related(plugin, presets__preset);
    bool result = false;
    LILV_FOREACH(nodes, iNode, nodes)
    {
        result = true;
        break;
    }
    return result;

}

Lv2BufferType LilvPortInfo::GetBufferType(const std::string& buffer_type)
{
    if (buffer_type == "")
        return Lv2BufferType::None;
    if (buffer_type == LV2_ATOM__Sequence)
        return Lv2BufferType::Sequence;
    return Lv2BufferType::Unknown;
}

PiPedalUI LilvPluginInfo::FindWritablePathProperties(LilvWorld*pWorld, const LilvPlugin *pPlugin)
{
    // example:

    // <http://github.com/mikeoliphant/neural-amp-modeler-lv2#model>
    //     a lv2:Parameter;
    //     rdfs:label "Model";
    //     rdfs:range atom:Path.
    // ...
    //          patch:writable <http://github.com/mikeoliphant/neural-amp-modeler-lv2#model>;

    AutoLilvNode pluginUri = lilv_plugin_get_uri(pPlugin);
    AutoLilvNode patch__writable = lilv_new_uri(pWorld, LV2_PATCH__writable);
    AutoLilvNodes patchWritables = lilv_world_find_nodes(pWorld, pluginUri, patch__writable, nullptr);

    std::vector<UiFileProperty> fileProperties;

    LILV_FOREACH(nodes, iNode, patchWritables)
    {
        AutoLilvNode propertyUri = lilv_nodes_get(patchWritables, iNode);
        if (propertyUri)
        {
            // isA lv2:Parameter?

            //AutoLilvNode isA;
            AutoLilvNode lv2core__Parameter = lilv_new_uri(pWorld, LV2_CORE_PREFIX "Parameter");
            AutoLilvNode isA = lilv_new_uri(pWorld,"http://www.w3.org/1999/02/22-rdf-syntax-ns#type");

            // a lv2:parameter?
            if (lilv_world_ask(pWorld, propertyUri, isA, lv2core__Parameter))
            {


                //  rfs:range atom:Path?
                AutoLilvNode rdfs__range = lilv_new_uri(pWorld, RDFS__range);
                AutoLilvNode atom__Path = lilv_new_uri(pWorld, LV2_ATOM__Path);
                if (lilv_world_ask(pWorld, propertyUri, rdfs__range, atom__Path))
                {
                    AutoLilvNode rdfs__label = lilv_new_uri(pWorld, LILV_NS_RDFS "label");
                    AutoLilvNode label = lilv_world_get(pWorld, propertyUri, rdfs__label, nullptr);
                    std::string strLabel = label.AsString();
                    if (strLabel.length() != 0)
                    {
                        std::filesystem::path path = this->bundle_path();
                        path = path.parent_path();
                        std::string lv2DirectoryName = path.filename().string();
                        // we have a valid path property!

                        auto fileProperty =
                            UiFileProperty(
                                strLabel, propertyUri.AsUri(), lv2DirectoryName);

                        AutoLilvNode dc__format = lilv_new_uri(pWorld, "http://purl.org/dc/terms/format");
                        AutoLilvNodes dc_types = lilv_world_find_nodes(pWorld, propertyUri, dc__format, nullptr);
                        LILV_FOREACH(nodes, i, dc_types)
                        {
                            AutoLilvNode dc_type = lilv_nodes_get(dc_types, i);
                            std::string fileType = dc_type.AsString();
                            std::string label = "";
                            fileProperty.fileTypes().push_back(UiFileType(label, fileType));
                        }

                        fileProperties.push_back(
                            fileProperty);
                    }
                }
            }
        }
    }
    if (fileProperties.size() != 0)
    {
        return PiPedalUI(std::move(fileProperties));
    }

    return PiPedalUI();
}




