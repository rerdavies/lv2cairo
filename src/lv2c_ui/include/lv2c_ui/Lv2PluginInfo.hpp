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
#include <string>
#include <vector>
#include <memory>
#include <limits>
#include "Lv2Units.hpp"
#include "Lv2PluginType.hpp"
#include <cmath>
#include "Lv2Exception.hpp"

#ifndef LV2_PIPEDAL_UI
#define LV2_PIPEDAL_UI 1
#endif

#if LV2_PIPEDAL_UI
#include "PiPedalUI.hpp"
#endif

namespace lv2c::ui {

#ifndef LV2_PROPERTY_GETSET
#define LV2_PROPERTY_GETSET(name)             \
    const auto& name() const     \
    {                                         \
        return name##_;                       \
    }                                        \
    void name(decltype(name##_) value) \
    {                                         \
        name##_ = value;                      \
    }
#endif

#ifndef LV2_PROPERTY_GETSET_SCALAR
#define LV2_PROPERTY_GETSET_SCALAR(name) \
    decltype(name##_) name() const       \
    {                                    \
        return name##_;                  \
    }                                   \
    void name(decltype(name##_) value)   \
    {                                    \
        name##_ = value;                 \
    }
#endif

    class Lv2PluginClass
    {
    public:
        friend class PluginHost;

    private:
        Lv2PluginClass *parent_ = nullptr; // NOT SERIALIZED!
        std::string parent_uri_;
        std::string display_name_;
        std::string uri_;
        PluginType plugin_type_;
        std::vector<std::shared_ptr<Lv2PluginClass>> children_;

        // hide copy constructor.
        Lv2PluginClass(const Lv2PluginClass &other)
        {
        }
        void set_parent(std::shared_ptr<Lv2PluginClass> &parent)
        {
            this->parent_ = parent.get();
        }
        void add_child(std::shared_ptr<Lv2PluginClass> &child)
        {
            for (size_t i = 0; i < children_.size(); ++i)
            {
                if (children_[i]->uri_ == child->uri_)
                {
                    return;
                }
            }
            children_.push_back(child);
        }

    public:
        Lv2PluginClass(
            const char *display_name, const char *uri, const char *parent_uri)
            : parent_uri_(parent_uri), display_name_(display_name), uri_(uri), plugin_type_(uri_to_plugin_type(uri))
        {
        }
        Lv2PluginClass() {}
        LV2_PROPERTY_GETSET(uri)
        LV2_PROPERTY_GETSET(display_name)
        LV2_PROPERTY_GETSET(parent_uri)

        const Lv2PluginClass *parent() { return parent_; }

        bool operator==(const Lv2PluginClass &other)
        {
            return uri_ == other.uri_;
        }
        bool is_a(const std::string &classUri) const;

        //static json_map::storage_type<Lv2PluginClass> jmap;
    };
    class Lv2PluginClasses
    {
    private:
        std::vector<std::string> classes_;

    public:
        Lv2PluginClasses()
        {
        }
        Lv2PluginClasses(std::vector<std::string> classes)
            : classes_(classes)
        {
        }
        const std::vector<std::string> &classes() const
        {
            return classes_;
        }
        bool is_a(const char*classUri) const;

        bool operator==(const Lv2PluginClasses&other) const {
            return this->classes_ == other.classes_;
        }

        #ifdef JUNK
        bool is_a(PluginHost *lv2Plugins, const char *classUri) const;
        #endif

        //static json_map::storage_type<Lv2PluginClasses> jmap;
    };

    class Lv2ScalePoint
    {
    private:
        float value_;
        std::string label_;

    public:
        Lv2ScalePoint() {}
        Lv2ScalePoint(float value, std::string label)
            : value_(value), label_(label)
        {
        }


        LV2_PROPERTY_GETSET_SCALAR(value);
        LV2_PROPERTY_GETSET(label);

        bool operator==(const Lv2ScalePoint&other) const {
            return this->value_ == other.value_ && this->label_ == other.label_; 
        }

        #ifdef LV2_JSON
        static json_map::storage_type<Lv2ScalePoint> jmap;
        #endif
    };

    enum class Lv2BufferType
    {
        None,
        Event,
        Sequence,
        Unknown
    };


    constexpr float INVALID_FLOAT = -std::numeric_limits<float>::max();

    class Lv2PortInfo_Init {
    public:
        uint32_t index_ = (uint32_t)-1;
        std::string symbol_;
        std::string name_;
        float min_value_ = 0, 
        max_value_ = 1, 
        default_value_ = 0;
        Lv2PluginClasses classes_;
        std::vector<Lv2ScalePoint> scale_points_;
        bool is_input_ = false;
        bool is_output_ = false;

        bool is_control_port_ = false;
        bool is_audio_port_ = false;
        bool is_atom_port_ = false;
        bool is_cv_port_ = false;
        bool connection_optional_ = false;

        bool supports_midi_ = false;
        bool supports_time_position_ = false;
        bool is_logarithmic_ = false;
        int display_priority_ = -1;
        int range_steps_ = 0;
        bool trigger_ = false;
        bool integer_property_ = false;
        bool mod_momentaryOffByDefault_ = false;
        bool mod_momentaryOnByDefault_ = false;
        bool pipedal_graphicEq_ = false;

        bool enumeration_property_ = false;
        bool toggled_property_ = false;
        bool not_on_gui_ = false;
        Lv2BufferType buffer_type_ = Lv2BufferType::None;
        std::string port_group_;

        std::string designation_;
        Lv2Units units_ = Lv2Units::none;
        std::string comment_;

        bool is_valid_ = true;

        std::string pipedal_ledColor_;

    };
    class Lv2PortInfo: private Lv2PortInfo_Init
    {
        friend class LilvPortInfo;
        friend class Lv2PluginInfo;
    public:
        Lv2PortInfo() { }
        Lv2PortInfo(Lv2PortInfo_Init&&init) {
            (Lv2PortInfo_Init&)(*this) = std::move(init);
        }

    public:


    public:
        const std::string pipedal_ledColor() const { return pipedal_ledColor_; }
        bool IsSwitch() const
        {
            return min_value_ == 0 && max_value_ == 1 && (integer_property_ || toggled_property_ || enumeration_property_);
        }

        float rangeToValue(float range) const
        {
            float value;
            if (is_logarithmic_)
            {
                value = std::pow(max_value_ / min_value_, range) * min_value_;
            }
            else
            {
                value = (max_value_ - min_value_) * range + min_value_;
            }
            if (integer_property_ || enumeration_property_)
            {
                value = std::round(value);
            }
            else if (range_steps_ >= 2)
            {
                value = std::round(value * (range_steps_ - 1)) / (range_steps_ - 1);
            }
            if (toggled_property_)
            {
                value = value == 0 ? 0 : max_value_;
            }
            if (value > max_value_)
                value = max_value_;
            if (value < min_value_)
                value = min_value_;
            return value;
        }
        LV2_PROPERTY_GETSET(symbol)
        LV2_PROPERTY_GETSET_SCALAR(index)
        LV2_PROPERTY_GETSET(name)
        LV2_PROPERTY_GETSET(classes)
        LV2_PROPERTY_GETSET(scale_points)
        LV2_PROPERTY_GETSET_SCALAR(min_value)
        LV2_PROPERTY_GETSET_SCALAR(max_value)
        LV2_PROPERTY_GETSET_SCALAR(default_value)

        LV2_PROPERTY_GETSET_SCALAR(is_input)
        LV2_PROPERTY_GETSET_SCALAR(is_output)
        LV2_PROPERTY_GETSET_SCALAR(is_control_port)
        LV2_PROPERTY_GETSET_SCALAR(is_audio_port)
        LV2_PROPERTY_GETSET_SCALAR(is_atom_port)
        LV2_PROPERTY_GETSET_SCALAR(connection_optional)
        LV2_PROPERTY_GETSET_SCALAR(is_cv_port)
        LV2_PROPERTY_GETSET_SCALAR(is_valid)
        LV2_PROPERTY_GETSET_SCALAR(supports_midi)
        LV2_PROPERTY_GETSET_SCALAR(supports_time_position)
        LV2_PROPERTY_GETSET_SCALAR(is_logarithmic)
        LV2_PROPERTY_GETSET_SCALAR(display_priority)
        LV2_PROPERTY_GETSET_SCALAR(range_steps)
        LV2_PROPERTY_GETSET_SCALAR(trigger)
        LV2_PROPERTY_GETSET_SCALAR(integer_property)
        LV2_PROPERTY_GETSET_SCALAR(mod_momentaryOffByDefault)
        LV2_PROPERTY_GETSET_SCALAR(mod_momentaryOnByDefault)
        LV2_PROPERTY_GETSET_SCALAR(pipedal_graphicEq)
        LV2_PROPERTY_GETSET_SCALAR(enumeration_property)
        LV2_PROPERTY_GETSET_SCALAR(toggled_property)
        LV2_PROPERTY_GETSET_SCALAR(not_on_gui)
        LV2_PROPERTY_GETSET(designation)
        LV2_PROPERTY_GETSET(port_group)
        LV2_PROPERTY_GETSET(comment)
        LV2_PROPERTY_GETSET_SCALAR(units)

        LV2_PROPERTY_GETSET(buffer_type)


    public:
        virtual ~Lv2PortInfo() = default;
        bool is_a(const char *classUri) { return classes_.is_a(classUri); }

        #ifdef LV2_JSON
        static json_map::storage_type<Lv2PortInfo> jmap;
        #endif
    };

    class Lv2PortGroup
    {
    private:
        std::string uri_;
        std::string symbol_;
        std::string name_;

    public:
        LV2_PROPERTY_GETSET(uri);
        LV2_PROPERTY_GETSET(symbol);
        LV2_PROPERTY_GETSET(name);

        Lv2PortGroup() {}
        Lv2PortGroup(const std::string& uri, const std::string &symbol, const std::string&name);

    #ifdef LV2_JSON
        static json_map::storage_type<Lv2PortGroup> jmap;
    #endif
    };



    class Lv2PluginInfo
    {
    private:
        friend class LilvPluginInfo;

    public:
        Lv2PluginInfo() {}

    private:

        std::string bundle_path_;
        std::string uri_;
        std::string ui_;
        std::string name_;
        std::string plugin_class_;
        std::string brand_;
        std::string label_;        
        std::vector<std::string> supported_features_;
        std::vector<std::string> required_features_;
        std::vector<std::string> optional_features_;
        std::vector<std::string> extensions_;
        bool has_factory_presets_ = false;

        std::string author_name_;
        std::string author_homepage_;

        std::string comment_;
        std::vector<Lv2PortInfo> ports_;
        std::vector<Lv2PortGroup> port_groups_;
        bool is_valid_ = false;
        #if LV2_PIPEDAL_UI
        PiPedalUI piPedalUI_;
        #endif

        bool IsSupportedFeature(const std::string &feature) const;

    public:
        LV2_PROPERTY_GETSET(bundle_path)
        LV2_PROPERTY_GETSET(uri)
        LV2_PROPERTY_GETSET(ui)
        LV2_PROPERTY_GETSET(name)
        LV2_PROPERTY_GETSET(brand)
        LV2_PROPERTY_GETSET(label)
        LV2_PROPERTY_GETSET(plugin_class)
        LV2_PROPERTY_GETSET(supported_features)
        LV2_PROPERTY_GETSET(required_features)
        LV2_PROPERTY_GETSET(optional_features)
        LV2_PROPERTY_GETSET(author_name)
        LV2_PROPERTY_GETSET(author_homepage)
        LV2_PROPERTY_GETSET(comment)
        LV2_PROPERTY_GETSET(extensions)
        LV2_PROPERTY_GETSET(ports)
        LV2_PROPERTY_GETSET(is_valid)
        LV2_PROPERTY_GETSET(port_groups)
        LV2_PROPERTY_GETSET(has_factory_presets)
        #if LV2_PIPEDAL_UI
        LV2_PROPERTY_GETSET(piPedalUI)
        #endif

        const Lv2PortInfo &getPort(const std::string &symbol)
        {
            for (size_t i = 0; i < ports_.size(); ++i)
            {
                if (ports_[i].symbol() == symbol)
                {
                    return ports_[i];
                }
            }
            throw Lv2Exception("Port not found.");
        }
        bool hasExtension(const std::string &uri)
        {
            for (size_t i = 0; i < extensions_.size(); ++i)
            {
                if (extensions_[i] == uri)
                    return true;
            }
            return false;
        }
        bool hasCvPorts() const
        {
            for (size_t i = 0; i < ports_.size(); ++i)
            {
                if (ports_[i].is_cv_port())
                {
                    return true;
                }
            }
            return false;
        }
        bool hasMidiInput() const
        {
            for (size_t i = 0; i < ports_.size(); ++i)
            {
                if (ports_[i].is_atom_port() && ports_[i].supports_midi() && ports_[i].is_input())
                {
                    return true;
                }
            }
            return false;
        }
        bool hasMidiOutput() const
        {
            for (size_t i = 0; i < ports_.size(); ++i)
            {
                if (ports_[i].is_atom_port() && ports_[i].supports_midi() && ports_[i].is_output())
                {
                    return true;
                }
            }
            return false;
        }

    public:
        virtual ~Lv2PluginInfo();

        //static json_map::storage_type<Lv2PluginInfo> jmap;
    };
}

#undef LV2_PROPERTY_GETSET
#undef LV2_PROPERTY_GETSET_SCALAR

