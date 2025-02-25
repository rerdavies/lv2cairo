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

#include "ClassFileWriter.hpp"
#include <sstream>
#include "lv2c_ui/Lv2Units.hpp"
#include "lv2c_ui/Lv2PluginInfo.hpp"
#include "lv2/state/state.h"
#include "ss.hpp"

using namespace std;
using namespace lv2c::ui;

template <typename T>
std::string
CConstant(const T &value)
{
    return "#NOT HANDLED";
};

static Lv2PortInfo defaultPort;
static UiFileProperty defaultUIFileProperty;
static UiFrequencyPlot defaultUiFrequencyPlot;

static std::string encodeCString(const std::string &value)
{
    std::ostringstream ss;

    ss << '"';
    for (char c : value) 
    {
        switch(c) {
        case '\r':
            ss << "\\r";
            break;
        case '\n':
            ss << "\\n";
            break;
        case '\t':
            ss << "\\t";
            break;
        case '\\':
            ss << "\\\\";
            break;
        case '"':
            ss << "\\\"";
            break;
        default:
            ss << c;
        }
    }
    ss << '"';
    return ss.str();
}



std::string CConstant(const std::string &value)
{
    std::stringstream ss;
    ss << '\"';
    for (char c : value)
    {
        if (c == '\r')
        {
            // do nothing.
        }
        else if (c == '\n')
        {
            ss << "\\n";
        }
        else if (c == '\"' || c == '\\')
        {
            ss << "\\";
            ss << c;
        }
        else
        {
            ss << c;
        }
    }
    ss << '\"';
    return ss.str();
}
template <>
std::string CConstant<double>(const double &value)
{
    std::stringstream ss;
    ss.precision(15);
    ss << value;
    return ss.str();
}

std::string CConstant(const int32_t &value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}
std::string CConstant(const uint32_t &value)
{
    std::stringstream ss;
    ss << value;
    return ss.str();
}

std::string CConstant(const float &value)
{
    std::stringstream ss;
    ss.precision(15);
    ss << value;
    return ss.str();
}

std::string CConstant(const bool &value)
{
    std::stringstream ss;
    ss << (value ? "true" : "false");
    return ss.str();
}

std::string CConstant(const std::vector<std::string> &values)
{
    std::stringstream ss;
    ss << '{';
    bool first = true;
    for (auto &value : values)
    {
        if (!first)
        {
            ss << ',';
        }
        first = false;
        ss << CConstant(value);
    }
    ss << '}';
    return ss.str();
}

std::string CConstant(const Lv2Units &units)
{
    std::stringstream ss;
    ss << "Lv2Units::" << UnitsToString(units);
    return ss.str();
}

std::string CConstant(const Lv2BufferType &bufferType)
{
    std::stringstream ss;
    ss << "Lv2BufferType::";
    switch (bufferType)
    {
    case Lv2BufferType::Event:
        ss << "Event";
        break;
    case Lv2BufferType::Sequence:
        ss << "Sequence";
        break;
    case Lv2BufferType::None:
        ss << "None";
        break;
    default:
    case Lv2BufferType::Unknown:
        ss << "Unknown";
        break;
    }

    return ss.str();
}


std::string CConstant(const Lv2ScalePoint &scalePoint)
{
    std::stringstream ss;
    ss << "Lv2ScalePoint{";
    ss << CConstant(scalePoint.value()) << "," << CConstant(scalePoint.label());
    ss << '}';
    return ss.str();
}
std::string CConstant(const UiFileType &uiFileType)
{
    std::stringstream ss;
    ss << "UiFileType{";
    ss << CConstant(uiFileType.label()) 
        << "," << CConstant(uiFileType.mimeType())
        << "," << CConstant(uiFileType.fileExtension())
        ;
    ss << '}';
    return ss.str();
}
std::string CConstant(const Lv2PortGroup &portGroup)
{
    std::stringstream ss;
    ss << "Lv2PortGroup{"
       << CConstant(portGroup.uri())
       << "," << CConstant(portGroup.symbol())
       << "," << CConstant(portGroup.name())
       << '}';
    return ss.str();
}
template <typename U>
std::string CConstant(const std::vector<U> &values)
{
    std::stringstream ss;
    ss << '{';
    bool first = true;
    for (auto &value : values)
    {
        if (!first)
        {
            ss << ',';
        }
        first = false;
        ss << CConstant(value);
    }
    ss << '}';
    return ss.str();
}

std::string CConstant(const Lv2PluginClasses &value)
{
    std::stringstream ss;
    ss << "Lv2PluginClasses{" << CConstant(value.classes()) << "}";
    return ss.str();
}

#define WRITE_PROPERTY(obj, name) \
    s << Tab() << #name << "(" << CConstant(obj->name()) << ");" << endl;

#define WRITE_PORT_PROPERTY(obj, name)                                               \
    if (obj.name() != defaultPort.name())                                    \
    {                                                                                \
        s << Tab() << "." << (#name "_") << " = " << CConstant(obj.name()) << "," << endl; \
    }
#define WRITE_UI_FILE_PROPERTY(obj, name)                                               \
    if ( obj.name() != defaultUIFileProperty.name())                                    \
    {                                                                                \
        s << Tab() << "." << (#name "_") << " = " << CConstant(obj.name()) << "," << endl; \
    }
#define WRITE_UI_FREQUENCYPLOT_PROPERTY(obj, name)                                               \
    if ( obj.name() != defaultUiFrequencyPlot.name())                                    \
    {                                                                                \
        s << Tab() << "." << (#name "_") << " = " << CConstant(obj.name()) << "," << endl; \
    }

;
void ClassFileWriter::Write(const std::shared_ptr<lv2c::ui::Lv2PluginInfo> pluginInfo)
{

    s << "// Autogenerated by generate_lv2c_plugin_info. Do not modify." << endl;
    s << endl;

    s << "#ifndef DEFINE_LV2_PLUGIN_BASE" << endl;
    s << "#include \"lv2c_ui/Lv2PluginInfo.hpp\"" << endl;
    s << "#include <memory>" << endl;
    s << endl;

    if (nameSpace.length() != 0)
    {
        s << "namespace " << nameSpace << "{" << endl;
        Indent();
    }
    {
        s << Tab() << "class " << uiBaseClassName << ": public lv2c::ui::Lv2PluginInfo"
          << "{" << endl;
        s << Tab() << "public:" << endl;
        Indent();
        {
            s << Tab() << "static constexpr const char* UI_URI = " << encodeCString(pluginInfo->uri()+"-ui") << ";" << endl;

            s << Tab() << "using super=lv2c::ui::Lv2PluginInfo;" << endl;
            s << Tab() << "using ptr=std::shared_ptr<" << uiBaseClassName << ">;" << endl;
            s << Tab() << "static ptr Create() { return std::make_shared<" << uiBaseClassName << ">(); }" << endl;
            s << endl;


            s << Tab() << uiBaseClassName << "() {" << endl;
            Indent();
            {
                s << Tab() << "using namespace lv2c::ui;" << endl;
                s << endl;

                WRITE_PROPERTY(pluginInfo, uri)

                WRITE_PROPERTY(pluginInfo, name)
                WRITE_PROPERTY(pluginInfo, brand)
                WRITE_PROPERTY(pluginInfo, label)
                WRITE_PROPERTY(pluginInfo, plugin_class)
                WRITE_PROPERTY(pluginInfo, supported_features)
                WRITE_PROPERTY(pluginInfo, required_features)
                WRITE_PROPERTY(pluginInfo, optional_features)
                WRITE_PROPERTY(pluginInfo, author_name)
                WRITE_PROPERTY(pluginInfo, author_homepage)
                WRITE_PROPERTY(pluginInfo, comment)
                WRITE_PROPERTY(pluginInfo, extensions)

                s << Tab() << "ports({" << endl;
                Indent();
                for (auto &port : pluginInfo->ports())
                {
                    Write(port);
                }
                Unindent();
                s << Tab() << "});" << endl;

                WRITE_PROPERTY(pluginInfo, port_groups)
                WRITE_PROPERTY(pluginInfo, has_factory_presets)
                if (!pluginInfo->is_valid())
                {
                    WRITE_PROPERTY(pluginInfo, is_valid)
                }
                Write(pluginInfo->piPedalUI());
            }
            Unindent();
            s << Tab() << "}" << endl;
        }
        Unindent();
        s << Tab() << "};" << endl;
    }

    if (nameSpace.length() != 0)
    {

        Unindent();
        s << Tab() << "} // namespace" << endl;
    }
    s << "#endif" << endl;


    if (pluginBaseClassName.length() != 0) {
        s << "#ifdef DEFINE_LV2_PLUGIN_BASE" << endl;
        WritePluginBase(nameSpace,pluginBaseClassName,pluginInfo);
        s << "#endif" << endl;
    }

}

void ClassFileWriter::Indent()
{
    indent += 4;
}
void ClassFileWriter::Unindent()
{
    if (indent < 4)
        throw std::runtime_error("Unbalanced indents!");
    indent -= 4;
}
std::string ClassFileWriter::Tab()
{
    return std::string(indent, ' ');
}

void ClassFileWriter::Write(const Lv2PortInfo &port)
{
    s << Tab() << "Lv2PortInfo_Init {" << endl;
    Indent();
    WRITE_PORT_PROPERTY(port, index)
    WRITE_PORT_PROPERTY(port, symbol)
    WRITE_PORT_PROPERTY(port, name)
    WRITE_PORT_PROPERTY(port, min_value)
    WRITE_PORT_PROPERTY(port, max_value)
    WRITE_PORT_PROPERTY(port, default_value)
    WRITE_PORT_PROPERTY(port, classes)
    WRITE_PORT_PROPERTY(port, scale_points)
    WRITE_PORT_PROPERTY(port, is_input)
    WRITE_PORT_PROPERTY(port, is_output)

    WRITE_PORT_PROPERTY(port, is_control_port)
    WRITE_PORT_PROPERTY(port, is_audio_port)
    WRITE_PORT_PROPERTY(port, is_atom_port)
    WRITE_PORT_PROPERTY(port, is_cv_port)
    WRITE_PORT_PROPERTY(port, connection_optional)

    WRITE_PORT_PROPERTY(port, supports_midi)
    WRITE_PORT_PROPERTY(port, supports_time_position)
    WRITE_PORT_PROPERTY(port, is_logarithmic)
    WRITE_PORT_PROPERTY(port, display_priority)
    WRITE_PORT_PROPERTY(port, range_steps)
    WRITE_PORT_PROPERTY(port, trigger)
    WRITE_PORT_PROPERTY(port, integer_property)
    WRITE_PORT_PROPERTY(port, enumeration_property)
    WRITE_PORT_PROPERTY(port, toggled_property)
    WRITE_PORT_PROPERTY(port, not_on_gui)
    WRITE_PORT_PROPERTY(port, buffer_type)
    WRITE_PORT_PROPERTY(port, port_group)
    
    WRITE_PORT_PROPERTY(port, designation)
    WRITE_PORT_PROPERTY(port, units)
    WRITE_PORT_PROPERTY(port, comment)
    WRITE_PORT_PROPERTY(port, pipedal_ledColor)
    
    WRITE_PORT_PROPERTY(port, is_valid)

    Unindent();
    s << Tab() << "}," << endl;
}

template<typename T>
void ClassFileWriter::WriteCArray(const std::vector<T> & array, bool addComma)
{
    s << Tab() << "{" << endl;
    Indent();
    {
        for (const auto&item: array)
        {
            WriteC(item);
        }
    }
    Unindent();
    s << Tab() << "}" << (addComma ? "," : "") << endl;
}

void ClassFileWriter::Write(const PiPedalUI&piPedalUI)
{
    s << Tab() << "piPedalUI(PiPedalUI{" << endl;
    Indent();
    {
        WriteCArray(piPedalUI.fileProperties(),true);
        WriteCArray(piPedalUI.frequencyPlots(),true);
        // not currently used.
        WriteCArray(piPedalUI.portNotifications(),false);


    }
    Unindent();
    s << Tab() << "});" << endl;
}

void ClassFileWriter::WriteC(const UiFileProperty&fileProperty)
{
    s << Tab() << "UiFileProperty_Init{" << endl;
    Indent();
    {
        WRITE_UI_FILE_PROPERTY(fileProperty,label)
        WRITE_UI_FILE_PROPERTY(fileProperty,index)
        WRITE_UI_FILE_PROPERTY(fileProperty,directory)
        WRITE_UI_FILE_PROPERTY(fileProperty,resourceDirectory)
        WRITE_UI_FILE_PROPERTY(fileProperty,fileTypes)
        WRITE_UI_FILE_PROPERTY(fileProperty,patchProperty)
        WRITE_UI_FILE_PROPERTY(fileProperty,portGroup)

    }
    Unindent();
    s << Tab() << "}," << endl;

}

void ClassFileWriter::WriteC(const UiFrequencyPlot&frequencyPlot)
{
    s << Tab() << "UiFrequencyPlot_Init {" << endl;
    Indent();
    {
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,patchProperty)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,index)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,portGroup)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,xLeft)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,xRight)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,yTop)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,yBottom)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,xLog)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,yDb)
        WRITE_UI_FREQUENCYPLOT_PROPERTY(frequencyPlot,width)
    }
    Unindent();
    s << Tab() << "}," << endl;

}


void ClassFileWriter::WriteC(const UiPortNotification&portNotification){
    s << Tab() << "\"#Not implemented.\"" << endl;

}

static void CheckForValidIndices(const std::shared_ptr<Lv2PluginInfo> pluginInfo)
{
    std::vector<bool> indexPresent;
    for (const auto& port : pluginInfo->ports()) {
        auto index = port.index();
        if (index > 100000 || index < 0)
        {
            std::stringstream msg;
            msg << "Invalid port index: " << port.symbol() << " = " << index;
            throw std::runtime_error(msg.str());
        }
        if (index >= indexPresent.size())
        {
            indexPresent.resize(index+1);
        }
        if (indexPresent[index])
        {
            std::stringstream msg;
            msg << "Duplicate port index: "<< port.symbol() << " = " << index;
            throw std::runtime_error(msg.str());
        }
        indexPresent[index] = true;
    }
    for (bool present: indexPresent)
    {
        if (!present) 
        {
            throw std::runtime_error("Port indexes are not consecutive.");
        }
    }

}

static std::string patchPropertyVariableName(const std::string &uri)
{
    auto pos = uri.find_last_of('#');
    if (pos == std::string::npos)
    {
        throw std::runtime_error(SS("Can't figure out a variable name for " << uri << ". Expecting the uri to be in the form \"urlpart#variablename\"."));
    }
    return uri.substr(pos+1) + "_urid";
}
void ClassFileWriter::WritePluginBase(const std::string&nameSpace,const std::string&pluginBaseClassName, const std::shared_ptr<Lv2PluginInfo> pluginInfo)
{
    if (pluginBaseClassName.length() == 0)
    {
        return;
    }
    s << Tab() << "#include <lv2_plugin/Lv2Plugin.hpp>" << endl;
    s << Tab() << "#include <lv2_plugin/Lv2Ports.hpp>" << endl;

    s << Tab() << "using namespace lv2c::lv2_plugin;" << endl;
    s << endl;
    if (nameSpace.length() != 0)
    {
        s << "namespace " << nameSpace << "{" << endl;
        Indent();
    }

    bool hasState = pluginInfo->hasExtension(LV2_STATE__interface);

    std::string baseClass = "Lv2Plugin";
    if (hasState)
    {
        baseClass = "Lv2PluginWithState";

    }

    std::vector<std::string> initStatements;            

    s << Tab() << "class " << pluginBaseClassName << ": public " << baseClass <<" {" << endl;
    {
        s << Tab() << "public:" << endl;
        Indent();
        {

            s << Tab() << "static constexpr const char* URI = " << encodeCString(pluginInfo->uri()) << ";" << endl;
            s << Tab() << pluginBaseClassName <<  "(double rate,const char*bundlePath,const LV2_Feature *const *features)" << endl;
            s << Tab() << ":   " << baseClass << "(rate,bundlePath,features)" << endl;
            s << Tab() << "{" << endl;
            Indent();
            {
                for (const auto &fileProperty: pluginInfo->piPedalUI().fileProperties())
                {
                    s << Tab() << patchPropertyVariableName(fileProperty.patchProperty()) << " = MapURI(" 
                        <<  encodeCString(fileProperty.patchProperty()) << ");" << endl;
                }
                for (const auto &fileProperty: pluginInfo->piPedalUI().fileProperties())
                {
                    s << Tab() << "Lv2Plugin::AddPatchProperty(" << patchPropertyVariableName(fileProperty.patchProperty()) << ");" << endl;
                }
            }
            Unindent();
            s << Tab() << "}" << endl;
        }
        Unindent();

        s << Tab() << "protected:" << endl;
        Indent();
        {
            for (const auto &fileProperty: pluginInfo->piPedalUI().fileProperties())
            {
                s << Tab() << "LV2_URID " << patchPropertyVariableName(fileProperty.patchProperty()) << " = 0; // urid for patch property "  << fileProperty.patchProperty() << endl;
            }
            s << endl;

            if (pluginInfo->piPedalUI().fileProperties().size() != 0)
            {
                // indicate that the following methods need to be implemented.

			    s << Tab() << "virtual bool OnPatchPathSet(LV2_URID propertyUrid,const char*value) = 0;" << endl;
                s << Tab() << "virtual const char* OnGetPatchPropertyValue(LV2_URID propertyUrid) = 0;" << endl;
                s << endl;
            }

            CheckForValidIndices(pluginInfo);
            ////// enum class PortId
            s << Tab() << "enum class PortId {" << endl;
            Indent();
            for (const auto& port : pluginInfo->ports()) {
                auto index = port.index();
                s << Tab() << port.symbol() << " = " << index << "," << endl;
            }
            Unindent();
            s << Tab() << "};" << endl;
            s << endl;

            ////// port declarations. 
            for (const auto& port : pluginInfo->ports())
            {
                const std::string portName = port.symbol();
                s << Tab();
                if (port.is_control_port())
                {
                    if (port.is_input())
                    {
                        // lv2:toggled, pprop:trigger
                        if (port.toggled_property())
                        {
                            if (port.trigger())
                            {
                                s << "TriggerInputPort " << portName << "{"  "};" << endl;

                            } else {
                                s << "ToggledInputPort " << portName << "{"  "};" << endl;
                            }

                        } else if (port.units() == Lv2Units::db)
                        {
                            s << "RangedDbInputPort " << portName << "{" << port.min_value() << "," << port.max_value() << "};" << endl;
                        } else {
                            s << "RangedInputPort " << portName << "{" << port.min_value() << "," << port.max_value() << "};" << endl;
                        }
                    } else {
                        if (port.units() == Lv2Units::db)
                        {
                            s << "VuOutputPort " << portName << "{" << port.min_value() << "," << port.max_value() << "};" << endl;
                            std::ostringstream initStatement;
                            initStatement << portName << ".SetSampleRate((float)getRate());";
                            initStatements.push_back(initStatement.str());
                        } else {
                            s << "RateLimitedOutputPort " << portName << "{" << port.default_value() << "};" << endl;
                            std::ostringstream initStatement;
                            initStatement << portName << ".SetSampleRate((float)getRate());";
                            initStatements.push_back(initStatement.str());
                        }

                    }
                } else if (port.is_audio_port())
                {
                    if (port.is_input())
                    {
                        s << "AudioInputPort " << portName << "{};" << endl;

                    } else {
                        s << "AudioOutputPort " << portName << "{};" << endl;
                    }


                } else if (port.is_atom_port())
                {
                    if (port.is_input())
                    {
                        s << "AtomInputPort " << portName << "{};" << endl;

                    } else {
                        s << "AtomOutputPort " << portName << "{};" << endl;
                    }
                } else {
                    throw std::runtime_error("Invalid port type. Port type not supported.");
                }
            }
        }
        s << endl;


        ////// void SamplePlugin::ConnectPort
        s << Tab() << "virtual void ConnectPort(uint32_t port, void *data) override" << endl;
        s << Tab() << "{" << endl;
        Indent();
        {
            s << Tab() << "switch ((PortId)port)" << endl;
            s << Tab() << "{" << endl;
            for (const auto& port : pluginInfo->ports()) {
                s << Tab() << "case PortId::" << port.symbol() << ":"  << endl;
                s << Tab() << "    " << port.symbol() << ".SetData(data);" << endl;
                s << Tab() << "    break;" << endl;
            }
            s << Tab() << "default:" << endl;
            s << Tab() << "    LogError(\"Invalid port id\");" << endl;
            s << Tab() << "    break;" << endl;
            s << Tab() << "}" << endl;

        }
        Unindent();
        s << Tab() << "}" << endl;


        ////// void SamplePlugin::Activate
        s << Tab() << "virtual void Activate() override" << endl;
        s << Tab() << "{" << endl;
        Indent();
        {
            std::string inputAtomPortName, outputAtomPortName;
            for (const auto& port : pluginInfo->ports()) {
                if (port.is_atom_port())
                {
                    if (port.is_atom_port())
                    {
                        if (port.is_input())
                        {
                            inputAtomPortName = port.symbol() + ".Get()";
                        } else {
                            outputAtomPortName = port.symbol() + ".Get()";
                        }
                    }
                }
            }
            if (inputAtomPortName.empty())
            {
                inputAtomPortName = "nullptr";
            }
            if (outputAtomPortName.empty())
            {
                outputAtomPortName = "nullptr";
            }
            s << Tab() << "SetAtomPortBuffers((LV2_Atom_Sequence*)(" << inputAtomPortName << 
                 "), (LV2_Atom_Sequence*)(" << outputAtomPortName << "));" << endl;
        
            
            for (const auto&initStatement: initStatements)
            {
                s << Tab() << initStatement << endl;
            }

        }
        Unindent();
        s << Tab() << "}" << endl;




    }
    Unindent();
    s << Tab() << "};" << endl;

    if (nameSpace.length() != 0)
    {

        Unindent();
        s << Tab() << "} // namespace" << endl;
    }

    
}

