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

#include "CommandLineParser.hpp"
#include <iostream>
#include <fstream>
#include <lilv/lilv.h>
#include "AutoLilvNode.hpp"
#include "ss.hpp"
#include <memory>
#include <functional>
#include "LilvPluginInfo.hpp"
#include "ClassFileWriter.hpp"

using namespace std;
using namespace lv2;
using namespace twoplay;
using namespace lv2c::ui;

class cleanup
{
public:
    cleanup(std::function<void(void)> &&fn)
    {
        this->fn = std::move(fn);
    }
    ~cleanup()
    {
        fn();
    }

private:
    std::function<void(void)> fn;
};

void Process(const std::string &pluginUri, const std::string &extraBundle, const std::string &className, std::string nameSpace,std::ostream &os)
{

    LilvWorld *world = lilv_world_new();

    cleanup t{
        [world]()
        {
            lilv_world_free(world);
        }
    };

    if (extraBundle.length() != 0)
    {
        AutoLilvNode bundleNode = lilv_new_file_uri(world, nullptr, extraBundle.c_str());
        lilv_world_load_bundle(world, bundleNode);
    } else {
        lilv_world_load_all(world);
    }

    const LilvPlugins *plugins = lilv_world_get_all_plugins(world);

    AutoLilvNode pluginUriNode = lilv_new_uri(world, pluginUri.c_str());
    const LilvPlugin *plugin = lilv_plugins_get_by_uri(plugins, pluginUriNode);
    if (!plugin)
    {
        throw std::runtime_error(SS("Plugin not found: " << pluginUri));
    }
    LilvPluginInfo x(world,plugin);
    std::shared_ptr<LilvPluginInfo> pluginInfo = std::make_shared<LilvPluginInfo>(world, plugin);

    ClassFileWriter writer(os,className,nameSpace);

    writer.Write(pluginInfo);

}

int main(int argc, char **argv)
{
    // syntax:  generate_lv2c_plugin_info [plugin_uri] [options]
    // options:
    //             --ttl [ttfile]
    //             --class [classname]
    //             --out [filename]

    std::string ttlFile;
    std::string className = "MyPluginInfo";
    std::string nameSpace;
    std::string outputFile;
    CommandLineParser parser;
    parser.AddOption("--ttl", &ttlFile);
    parser.AddOption("--out", &outputFile);
    parser.AddOption("--class", &className);
    parser.AddOption("--namespace", &nameSpace);

    try
    {
        parser.Parse(argc, argv);

        if (parser.ArgumentCount() != 1)
        {
            throw std::runtime_error("Incorrect number of arguments.");
        }
    }
    catch (const std::exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    std::string uri = parser.Argument(0);
    try
    {
        std::ostream *pOut = &cout;
        ofstream f;
        if (outputFile.length() != 0)
        {
            f.open(outputFile);
            if (!f.is_open())
            {
                throw std::runtime_error("Unable to open output file.");
            }
            pOut = &f;
        }

        Process(uri, ttlFile, className, nameSpace, *pOut);
    }
    catch (const std::exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}