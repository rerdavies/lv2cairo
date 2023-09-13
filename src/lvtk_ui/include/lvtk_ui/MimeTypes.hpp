// Copyright (c) 2023 Robin Davies
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
#include <map>
#include <set>
#include <filesystem>

namespace lvtk::ui {
    class MimeTypes {
    public:
        static const std::string& MimeTypeFromPath(const std::filesystem::path &path);
        static const std::string& MimeTypeFromExtension(const std::string &extension);
        static const std::string& ExtensionFromMimeType(const std::string &mimeType);
    private:
        static void AddMimeType(const std::string&extension, const std::string&mimeType);
        static std::map<std::string,std::string> mimeTypeToExtension;
        static std::map<std::string,std::string> extensionToMimeType;
        static std::set<std::string> audioExtensions;
        static std::set<std::string> videoExtensions;
        static void MaybeInitialize();
        static bool initialized;
    };
}
 