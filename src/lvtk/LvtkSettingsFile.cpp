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

#include "lvtk/LvtkSettingsFile.hpp"
#include "lvtk/JsonIo.hpp"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <lvtk/LvtkLog.hpp>
#include "ss.hpp"


using namespace lvtk;


std::filesystem::path LvtkSettingsFile::GetSettingsPath(const std::string &identifier)
{
#ifdef __linux__ 
    std::filesystem::path home = std::getenv("HOME");
    std::filesystem::path path = home / ".config" / "lvtk" / identifier;
    std::filesystem::create_directories(path);
    path = path / "settings.json";
    return path;
#elif _WIN32
    // windows code goes here
    static_assert("Fix me.");
    // somewhere in AppData/Roaming.
    xxx 
#else
    static_assert("Fix me.");
    // somewhere in AppData/Roaming.
    xxx a path to wherever per-app user settings get saved.

#endif
}

LvtkSettingsFile::LvtkSettingsFile()
{
    root = json_variant::object();
}

json_variant&LvtkSettingsFile::Root() {
    return root;
}


void LvtkSettingsFile::Load(const std::string &identifier)
{

    root = json_variant::object();
    std::filesystem::path path = GetSettingsPath(identifier);
    this->filePath = path;
    if (std::filesystem::exists(path))
    {
        try {
        std::ifstream f;
        f.open(path);
        f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        if (f.is_open())
        {
            json_reader reader(f);

            root.read(reader);

            {
                std::stringstream s;
                s << root;
                this->lastValue = s.str();
            }

        }
        } catch(const std::exception &e)
        {
            LogError(SS("Invalid settings file." << e.what()));
        }
    }
}

void LvtkSettingsFile::Update()
{
    if (filePath.string().length() == 0) return;
    std::stringstream s;
    s << root;

    std::string newValue = s.str();
    bool written = false;
    if (newValue != this->lastValue)
    {
        std::filesystem::path tmpPath = 
            std::filesystem::path(
                filePath.string() + ".$$$"
            );
        {
            std::ofstream f(tmpPath);
            f << newValue << std::endl;
            if (f)
            {
                written = true;
            }
        }
        if (written)
        {
            std::filesystem::remove(this->filePath);
            std::filesystem::rename(tmpPath,filePath);
        }
    }
}
LvtkSettingsFile::~LvtkSettingsFile()
{
    Update();
}



json_variant lvtk::LvtkPointToJson(LvtkPoint value)
{
    json_variant result = json_variant::array();
    result.resize(2);
    result[0] = value.x;
    result[1] = value.y;
    return result;
}
LvtkPoint lvtk::LvtkPointFromJson(const json_variant&value, LvtkPoint defaultValue)
{
    if (value.is_null())
    {
        return defaultValue;
    }
    LvtkPoint result;
    result.x = (value[0].as<double>());
    result.y = (value[1].as<double>());
    return result;
}

json_variant lvtk::LvtkSizeToJson(LvtkSize value)
{
    json_variant result = json_variant::array();
    result.resize(2);
    result[0] = value.Width();
    result[1] = value.Height();
    return result;
}
LvtkSize lvtk::LvtkSizeFromJson(const json_variant&value, LvtkSize defaultValue)
{
    if (value.is_null())
    {
        return defaultValue;
    }
    LvtkSize result {
        value[0].as<double>(),
        value[1].as<double>()
    };
    return result;
}
