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

#include "LilvPiPedalUI.hpp"
#include <lilv/lilv.h>
#include "AutoLilvNode.hpp"
#include <iostream>
#include "ss.hpp"
#include "lv2/ui/ui.h"
#include "lv2/urid/urid.h"
//#include "lv2.h"
#include "lv2/atom/atom.h"
#include "lv2/time/time.h"
#include "lv2/state/state.h"
#include "lv2/lv2plug.in/ns/ext/buf-size/buf-size.h"
#include "lv2/lv2plug.in/ns/ext/presets/presets.h"
#include "lv2/lv2plug.in/ns/ext/port-props/port-props.h"
#include "lv2/lv2plug.in/ns/extensions/units/units.h"
#include "lv2/lv2plug.in/ns/ext/port-groups/port-groups.h"
#include "lvtk_ui/MimeTypes.hpp"


using namespace lvtk::ui;
using namespace lv2;
using namespace std;


static void LogWarning(const std::string&message)
{
    cout << "Warning: " << message << endl;
}
LilvPiPedalUI::LilvPiPedalUI(LilvWorld*pWorld, const LilvNode *uiNode, const std::filesystem::path &resourcePath)
{
    AutoLilvNode pipedalUI__fileProperties = lilv_new_uri(pWorld, PIPEDAL_UI__fileProperties);
    AutoLilvNodes  fileNodes = lilv_world_find_nodes(pWorld, uiNode, /*lvuri*/pipedalUI__fileProperties, nullptr);
    LILV_FOREACH(nodes, i, fileNodes)
    {
        const LilvNode *fileNode = lilv_nodes_get(fileNodes, i);
        try
        {
            LilvUiFileProperty fileUI {pWorld, fileNode, resourcePath};
            this->fileProperties_.push_back(fileUI);
        }
        catch (const std::exception &e)
        {
            LogWarning(SS("Failed to read pipedalui::fileProperties. " << e.what()));
        }
    }
    AutoLilvNode pipedalUI__frequencyPlot = lilv_new_uri(pWorld, PIPEDAL_UI__frequencyPlot);
    AutoLilvNodes  frequencyPlotNodes = lilv_world_find_nodes(pWorld, uiNode, /*lvuri*/pipedalUI__frequencyPlot, nullptr);
    LILV_FOREACH(nodes, i, frequencyPlotNodes)
    {
        const LilvNode *frequencyPlotNode = lilv_nodes_get(frequencyPlotNodes, i);
        try
        {
            LilvUiFrequencyPlot frequencyPlotUI (pWorld, frequencyPlotNode, resourcePath);
            this->frequencyPlots_.push_back(frequencyPlotUI);
        }
        catch (const std::exception &e)
        {
            LogWarning(SS("Failed to read pipedalui::frequencyPlots. " << e.what()));
        }
    }

    AutoLilvNode ui__portNotification = lilv_new_uri(pWorld, LV2_UI__portNotification);
    AutoLilvNodes portNotifications = lilv_world_find_nodes(pWorld, uiNode, /*lvuri*/ui__portNotification, nullptr);

    LILV_FOREACH(nodes, i, portNotifications)
    {
        const LilvNode *portNotificationNode = lilv_nodes_get(portNotifications, i);
        try
        {
            LilvUiPortNotification portNotification = LilvUiPortNotification(pWorld, portNotificationNode);
            this->portNotifications_.push_back(portNotification);
        }
        catch (const std::exception &e)
        {
            LogWarning(SS("Failed to read ui:portNotifications. " << e.what()));
        }
    }
}


LilvUiFileType::LilvUiFileType(LilvWorld *pWorld, const LilvNode *node)
{

    AutoLilvNode rdfs__label = lilv_new_uri(pWorld, LILV_NS_RDFS "label");
    AutoLilvNode label = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/rdfs__label,
        nullptr);
    if (label)
    {
        this->label_ = label.AsString();
    }
    else
    {
        throw std::runtime_error("pipedal_ui:fileType is missing label property.");
    }
    AutoLilvNode pipedalUI__fileExtension = lilv_new_uri(pWorld, PIPEDAL_UI__fileExtension);
    AutoLilvNode fileExtension = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/pipedalUI__fileExtension,
        nullptr);
    if (fileExtension)
    {
        this->fileExtension_ = fileExtension.AsString();
    }
    else
    {
        this->fileExtension_ = "";
    }
    AutoLilvNode pipedalUI__mimeType = lilv_new_uri(pWorld, PIPEDAL_UI__mimeType);
    AutoLilvNode mimeType = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/pipedalUI__mimeType,
        nullptr);
    if (mimeType)
    {
        this->mimeType_ = mimeType.AsString();
    }
    if (fileExtension_ == "")
    {
        fileExtension_ = MimeTypes::ExtensionFromMimeType(mimeType_);
    }
    if (mimeType_ == "")
    {
        mimeType_ = MimeTypes::MimeTypeFromExtension(fileExtension_);
        if (mimeType_ == "")
        {
            mimeType_ = "application/octet-stream";
        }
    }
}
LilvUiFileProperty::LilvUiFileProperty(LilvWorld *pWorld, const LilvNode *node, const std::filesystem::path &resourcePath)
{

    AutoLilvNode rdfs__label = lilv_new_uri(pWorld, LILV_NS_RDFS "label");
    AutoLilvNode label = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/rdfs__label,
        nullptr);
    if (label)
    {
        this->label_ = label.AsString();
    }
    else
    {
        this->label_ = "File";
    }
    AutoLilvNode lv2core__index = lilv_new_uri(pWorld, LV2_CORE__index);
    AutoLilvNode index = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/lv2core__index,
        nullptr);
    if (index)
    {
        this->index_ = index.AsInt(-1);
    }
    else
    {
        this->index_ = -1;
    }

    AutoLilvNode pipedalUI__directory = lilv_new_uri(pWorld, PIPEDAL_UI__directory);
    AutoLilvNode directory = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/pipedalUI__directory,
        nullptr);
    if (directory)
    {
        this->directory_ = directory.AsString();
    }
    if (directory_.length() == 0)
    {
        throw std::runtime_error("PipedalUI::fileProperty: must specify at least a directory.");
    }

    AutoLilvNode pipedalUI__patchProperty = lilv_new_uri(pWorld, PIPEDAL_UI__patchProperty);
    AutoLilvNode patchProperty = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/pipedalUI__patchProperty,
        nullptr);
    if (patchProperty)
    {
        this->patchProperty_ = patchProperty.AsUri();
    }
    else
    {
        throw std::runtime_error("PiPedal FileProperty is missing pipedalui:patchProperty value.");
    }
    AutoLilvNode     portgroups__group = lilv_new_uri(pWorld, LV2_PORT_GROUPS__group);
;
    AutoLilvNode portGroup = lilv_world_get(pWorld,node,/*lvuri*/portgroups__group,nullptr);
    if (portGroup)
    {
        this->portGroup_ = portGroup.AsUri();
    }

    AutoLilvNode pipedalUI__fileTypes = lilv_new_uri(pWorld, PIPEDAL_UI__fileTypes);

    this->fileTypes_ = LilvUiFileType::GetArray(pWorld, node, /*lvuri*/pipedalUI__fileTypes);
}


std::vector<UiFileType> LilvUiFileType::GetArray(LilvWorld *pWorld, const LilvNode *node, const LilvNode *uri)
{
    std::vector<UiFileType> result;

    AutoLilvNode pipedalUI__fileTypes = lilv_new_uri(pWorld, PIPEDAL_UI__fileTypes);
    LilvNodes *fileTypeNodes = lilv_world_find_nodes(pWorld, node, /*lvuri*/pipedalUI__fileTypes, nullptr);
    LILV_FOREACH(nodes, i, fileTypeNodes)
    {
        const LilvNode *fileTypeNode = lilv_nodes_get(fileTypeNodes, i);
        try
        {
            UiFileType fileType = LilvUiFileType(pWorld, fileTypeNode);
            result.push_back(std::move(fileType));
        }
        catch (const std::exception &e)
        {
            LogWarning(e.what());
        }
    }
    lilv_nodes_free(fileTypeNodes);
    return result;
}




LilvUiPortNotification::LilvUiPortNotification(LilvWorld *pWorld, const LilvNode *node)
{
    // ui:portNotification
    // [
    //         ui:portIndex 3;
    //         ui:plugin <http://two-play.com/plugins/toob-convolution-reverb>;
    //         ui:protocol ui:floatProtocol;
    //         // pipedal_ui:style pipedal_ui:text ;
    //         // pipedal_ui:redLevel 0;
    //         // pipedal_ui:yellowLevel -12;
    // ]

    AutoLilvNode ui__portIndex = lilv_new_uri(pWorld, LV2_UI__portIndex);
    AutoLilvNode portIndex = lilv_world_get(pWorld, node, /*lvuri*/ui__portIndex, nullptr);
    if (!portIndex)
    {
        this->portIndex_ = -1;
    }
    else
    {
        this->portIndex_ = (uint32_t)lilv_node_as_int(portIndex);
    }
    AutoLilvNode lv2__symbol = lilv_new_uri(pWorld, LV2_CORE__symbol);
    AutoLilvNode symbol = lilv_world_get(pWorld, node, /*lvuri*/lv2__symbol, nullptr);
    if (!symbol)
    {
        this->symbol_ = "";
    }
    else
    {
        this->symbol_ = symbol.AsString();
    }
    AutoLilvNode ui__plugin = lilv_new_uri(pWorld, LV2_UI__plugin);
    AutoLilvNode plugin = lilv_world_get(pWorld, node, /*lvuri*/ui__plugin, nullptr);
    if (!plugin)
    {
        this->plugin_ = "";
    }
    else
    {
        this->plugin_ = plugin.AsUri();
    }
    AutoLilvNode ui__protocol = lilv_new_uri(pWorld, LV2_UI__protocol);
    AutoLilvNode protocol = lilv_world_get(pWorld, node, /*lvuri*/ui__protocol, nullptr);
    if (!protocol)
    {
        this->protocol_ = "";
    }
    else
    {
        this->protocol_ = protocol.AsUri();
    }
    if (this->portIndex_ == -1 && this->symbol_ == "")
    {
        LogWarning("ui:portNotification specifies neither a ui:portIndex nor an lv2:symbol.");
    }
}

static float GetFloat(LilvWorld *pWorld,const LilvNode*node,const LilvNode*property,float defaultValue)
{
    AutoLilvNode value = lilv_world_get(
        pWorld,
        node,
        property,
        nullptr);
    return value.AsFloat(defaultValue);
}


LilvUiFrequencyPlot::LilvUiFrequencyPlot(LilvWorld*pWorld, const LilvNode*node,
    const std::filesystem::path&resourcePath)
{


    AutoLilvNode pipedalUI__patchProperty = lilv_new_uri(pWorld, PIPEDAL_UI__patchProperty);
    AutoLilvNode patchProperty = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/pipedalUI__patchProperty,
        nullptr);
    if (patchProperty)
    {
        this->patchProperty_ = patchProperty.AsUri();
    }
    else
    {
        throw std::runtime_error("PiPedal FileProperty is missing pipedalui:patchProperty value.");
    }

    AutoLilvNode lv2core__index = lilv_new_uri(pWorld, LV2_CORE__index);
    AutoLilvNode index = lilv_world_get(
        pWorld,
        node,
        /*lvuri*/lv2core__index,
        nullptr);
    if (index)
    {
        this->index_ = index.AsInt(-1);
    }
    else
    {
        this->index_ = -1;
    }
    AutoLilvNode portgroups__group = lilv_new_uri(pWorld, LV2_PORT_GROUPS__group);
    AutoLilvNode portGroup = lilv_world_get(pWorld,node,/*lvuri*/portgroups__group,nullptr);
    if (portGroup)
    {
        this->portGroup_ = portGroup.AsUri();
    }
    AutoLilvNode pipedalUI__xLeft = lilv_new_uri(pWorld, PIPEDAL_UI__xLeft);
    AutoLilvNode pipedalUI__xRight = lilv_new_uri(pWorld, PIPEDAL_UI__xRight);
    AutoLilvNode pipedalUI__yTop = lilv_new_uri(pWorld, PIPEDAL_UI__yTop);
    AutoLilvNode pipedalUI__yBottom = lilv_new_uri(pWorld, PIPEDAL_UI__yBottom);
    AutoLilvNode pipedalUI__xLog = lilv_new_uri(pWorld, PIPEDAL_UI__xLog);
    AutoLilvNode pipedalUI__width = lilv_new_uri(pWorld, PIPEDAL_UI__width);
    AutoLilvNode pipedalUI__yDb = lilv_new_uri(pWorld, PIPEDAL_UI__yDb);

    this->xLeft_ = GetFloat(pWorld,node,/*lvuri*/pipedalUI__xLeft,30);
    this->xRight_ = GetFloat(pWorld,node,/*lvuri*/pipedalUI__xRight,22000);
    this->yTop_ = GetFloat(pWorld,node,/*lvuri*/pipedalUI__yTop,5);
    this->yBottom_ = GetFloat(pWorld,node,/*lvuri*/pipedalUI__yBottom,-35);
    this->xLog_ = GetFloat(pWorld,node,/*lvuri*/pipedalUI__xLog,1) != 0;
    this->yDb_ = GetFloat(pWorld,node,/*lvuri*/pipedalUI__yDb,1) != 0;
    this->width_ = GetFloat(pWorld,node,/*lvuri*/pipedalUI__width,60);
}






