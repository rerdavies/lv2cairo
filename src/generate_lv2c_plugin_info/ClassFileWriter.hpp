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
#include <iostream>
#include <string>
#include <memory>
#include "lv2c_ui/Lv2PluginInfo.hpp"


class ClassFileWriter {
public:


    ClassFileWriter(
        std::ostream&s, 
        const std::string &uiBaseClassName, 
        const std::string &pluginBaseClassName,
        std::string&nameSpace
    ): 
        s(s),
        uiBaseClassName(uiBaseClassName),
        nameSpace(nameSpace),
        pluginBaseClassName(pluginBaseClassName) { }


    void Write(const std::shared_ptr<lv2c::ui::Lv2PluginInfo> pluginInfo);
private:
    void Write(const lv2c::ui::PiPedalUI&piPedalUI);

    void WriteC(const lv2c::ui::UiFileProperty&fileProperty);
    void WriteC(const lv2c::ui::UiFrequencyPlot&fileProperty);
    void WriteC(const lv2c::ui::UiPortNotification&fileProperty);
    void WritePluginBase(const std::string &nameSpace,const std::string&pluginBaseClassName,const std::shared_ptr<lv2c::ui::Lv2PluginInfo> pluginInfo);

    template<typename T>
    void WriteCArray(const std::vector<T> & array, bool addComma);

    void Write(const lv2c::ui::Lv2PortInfo& port);
    void Indent();
    void Unindent();
    std::string Tab();
    std::ostream&s;
    std::string uiBaseClassName;
    std::string pluginBaseClassName;
    std::string nameSpace;
    uint64_t indent =  0;
};