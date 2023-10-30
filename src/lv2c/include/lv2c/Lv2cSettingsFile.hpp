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
#include "lv2c/Lv2cTypes.hpp"



namespace lv2c {

    class Lv2cSettingsFile: public std::enable_shared_from_this<Lv2cSettingsFile>  {
    public:
        static std::shared_ptr<Lv2cSettingsFile> GetSharedFile(const std::string&identifier);

        Lv2cSettingsFile();
        ~Lv2cSettingsFile();

        void Load(const std::string &identifier);

        void Update();
        json_variant &Root();
    private: 
        std::string sharedInstanceidentifier;
        std::filesystem::path GetSettingsPath(const std::string &identifier);
        std::filesystem::path filePath;
        std::string lastValue;
        json_variant root;
        static std::map<std::string,Lv2cSettingsFile*> sharedInstances;

    };

    extern json_variant Lv2cPointToJson(Lv2cPoint value);
    extern Lv2cPoint Lv2cPointFromJson(const json_variant&value, Lv2cPoint defaultValue = Lv2cPoint{0,0});

    extern json_variant Lv2cSizeToJson(Lv2cSize value);
    extern Lv2cSize Lv2cSizeFromJson(const json_variant&value, Lv2cSize defaultValue = Lv2cSize{0,0});


}