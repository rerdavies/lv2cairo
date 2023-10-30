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

#include "lv2c/Lv2cSettingsFile.hpp"
#include "lv2c/JsonIo.hpp"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include "lv2c/Lv2cLog.hpp"
#include "ss.hpp"


using namespace lv2c;


std::filesystem::path Lv2cSettingsFile::GetSettingsPath(const std::string &identifier)
{
#ifdef __linux__ 
    std::filesystem::path home = std::getenv("HOME");
std::filesystem::path path = home / ".config" / "io.github.rerdavies.lv2cairo" / identifier;
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

Lv2cSettingsFile::Lv2cSettingsFile()
{
    root = json_variant::object();
}

json_variant&Lv2cSettingsFile::Root() {
    return root;
}


void Lv2cSettingsFile::Load(const std::string &identifier)
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

void Lv2cSettingsFile::Update()
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
        std::filesystem::path directory = filePath.parent_path();

        std::filesystem::create_directories(directory);
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
Lv2cSettingsFile::~Lv2cSettingsFile()
{
    Update();
    if (sharedInstanceidentifier.length() != 0)
    {
        sharedInstances[this->sharedInstanceidentifier] = nullptr;
    }
}



json_variant lv2c::Lv2cPointToJson(Lv2cPoint value)
{
    json_variant result = json_variant::array();
    result.resize(2);
    result[0] = value.x;
    result[1] = value.y;
    return result;
}
Lv2cPoint lv2c::Lv2cPointFromJson(const json_variant&value, Lv2cPoint defaultValue)
{
    if (value.is_null())
    {
        return defaultValue;
    }
    Lv2cPoint result;
    result.x = (value[0].as<double>());
    result.y = (value[1].as<double>());
    return result;
}

json_variant lv2c::Lv2cSizeToJson(Lv2cSize value)
{
    json_variant result = json_variant::array();
    result.resize(2);
    result[0] = value.Width();
    result[1] = value.Height();
    return result;
}
Lv2cSize lv2c::Lv2cSizeFromJson(const json_variant&value, Lv2cSize defaultValue)
{
    if (value.is_null())
    {
        return defaultValue;
    }
    Lv2cSize result {
        value[0].as<double>(),
        value[1].as<double>()
    };
    return result;
}


std::shared_ptr<Lv2cSettingsFile> Lv2cSettingsFile::GetSharedFile(const std::string&identifier)
{
    {
        Lv2cSettingsFile*pFile = sharedInstances[identifier];
        if (pFile)
        {
            return pFile->shared_from_this();
        }
    }
    std::shared_ptr<Lv2cSettingsFile> pFile = std::make_shared<Lv2cSettingsFile>();
    pFile->Load(identifier);
    pFile->sharedInstanceidentifier = identifier;
    sharedInstances[identifier] = pFile.get();
    return pFile;
}
std::map<std::string,Lv2cSettingsFile*> Lv2cSettingsFile::sharedInstances;
