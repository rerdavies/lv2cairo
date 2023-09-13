/*
 * MIT License
 *
 * Copyright (c) 2023 Robin E. R. Davies
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "JsonVariant.hpp"
#include <filesystem>
#include <string>
#include "lvtk/LvtkTypes.hpp"



namespace lvtk {

    class LvtkSettingsFile  {
    public:
        LvtkSettingsFile();
        ~LvtkSettingsFile();

        void Load(const std::string &identifier);

        void Update();
        json_variant &Root();
    private: 
        std::filesystem::path GetSettingsPath(const std::string &identifier);
        std::filesystem::path filePath;
        std::string lastValue;
        json_variant root;

    };

    extern json_variant LvtkPointToJson(LvtkPoint value);
    extern LvtkPoint LvtkPointFromJson(const json_variant&value, LvtkPoint defaultValue = LvtkPoint{0,0});

    extern json_variant LvtkSizeToJson(LvtkSize value);
    extern LvtkSize LvtkSizeFromJson(const json_variant&value, LvtkSize defaultValue = LvtkSize{0,0});


}