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
#include "lvtk_ui/Lv2PluginInfo.hpp"

using namespace lvtk::ui;

class ClassFileWriter {
public:

    ClassFileWriter(std::ostream&s, const std::string &className, std::string&nameSpace): s(s),className(className),nameSpace(nameSpace) { }
    using Lv2PluginInfo = lvtk::ui::Lv2PluginInfo;
    using Lv2PortInfo = lvtk::ui::Lv2PortInfo;
    using PiPedalUI = lvtk::ui::PiPedalUI;

    void Write(const std::shared_ptr<Lv2PluginInfo> pluginInfo);
private:
    void Write(const PiPedalUI&piPedalUI);

    void WriteC(const UiFileProperty&fileProperty);
    void WriteC(const UiFrequencyPlot&fileProperty);
    void WriteC(const UiPortNotification&fileProperty);

    template<typename T>
    void WriteCArray(const std::vector<T> & array, bool addComma);

    void Write(const Lv2PortInfo& port);
    void Indent();
    void Unindent();
    std::string Tab();
    std::ostream&s;
    std::string className;
    std::string nameSpace;
    uint64_t indent =  0;
};