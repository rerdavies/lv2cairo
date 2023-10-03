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

#include "lvtk/LvtkLog.hpp"
#include "iostream"

using namespace std;

namespace lvtk
{

    static LvtkLogLevel logLevel = LvtkLogLevel::Info;

    void SetLogLevel(LvtkLogLevel logLevel_)
    {
        lvtk::logLevel = logLevel_;
    }

    void LogError(const std::string &message)
    {
        if (logLevel >= LvtkLogLevel::Error)
        {
            cout << "Error:   " << message << endl;
        }
    }

    void LogWarning(const std::string &message)
    {
        if (logLevel >= LvtkLogLevel::Warning)
        {
            cout << "Warning: " << message << endl;
        }
    }

    void LogInfo(const std::string &message)
    {
        if (logLevel >= LvtkLogLevel::Info)
        {
            cout << "Info:    " << message << endl;
        }
    }
    void LogDebug(const std::string &message)
    {
        if (logLevel >= LvtkLogLevel::Debug)
        {
            cout << "Debug:   " << message << endl;
        }
    }
}
