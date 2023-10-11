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
#include "lvtk_ui/Lv2Units.hpp"

using namespace std;
using namespace lvtk::ui;

template <typename T>
std::string
CConstant(const T &value)
{
    return "#NOT HANDLED";
}

static Lv2PortInfo defaultPort;
static UiFileProperty defaultUIFileProperty;
static UiFrequencyPlot defaultUiFrequencyPlot;

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

void ClassFileWriter::Write(const std::shared_ptr<Lv2PluginInfo> pluginInfo)
{

    s << "// Autogenerated by generate_lvtk_plugin_info. Do not modify." << endl;
    s << endl;

    s << "#include \"lvtk_ui/Lv2PluginInfo.hpp\"" << endl;
    s << "#include <memory>" << endl;
    s << endl;

    if (nameSpace.length() != 0)
    {
        s << "namespace {" << endl;
        Indent();
    }
    {
        s << Tab() << "class " << className << ": public lvtk::ui::Lv2PluginInfo"
          << "{" << endl;
        s << Tab() << "public:" << endl;
        Indent();
        {
            s << Tab() << "using super=lvtk::ui::Lv2PluginInfo;" << endl;
            s << Tab() << "using ptr=std::shared_ptr<" << className << ">;" << endl;
            s << Tab() << "static ptr Create() { return std::make_shared<" << className << ">(); }" << endl;
            s << endl;

            s << Tab() << className << "() {" << endl;
            Indent();
            {
                s << Tab() << "using namespace lvtk::ui;" << endl;
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

