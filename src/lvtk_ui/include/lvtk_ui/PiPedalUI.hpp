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

/***************************
 * Extensions for PiPedal UI
 * 
***************************/

#include <string>
#include <memory>
#include <vector>

#ifdef LV2_JSON
// see json.hpp in PipPedal project. Not working in this version, but schema info has been preserved for future use.
#include "json.hpp"
#else 

#ifndef DECLARE_JSON_MAP
#define DECLARE_JSON_MAP(x)
#define JSON_MAP_BEGIN(x)
#define JSON_MAP_REFERENCE(x,y)
#define JSON_MAP_END()

#endif

#endif
#include <filesystem>



namespace lvtk::ui {



    class UiFileType {
        friend class LilvUiFileType;
    private:
        std::string label_;
        std::string mimeType_;
        std::string fileExtension_;
    public:
        UiFileType() { }
        UiFileType(const std::string&label, const std::string &fileType);
        UiFileType(const std::string&label, const std::string &mimeType, std::string &fileExtension);


        const std::string& label() const { return label_;}
        const std::string &fileExtension() const { return fileExtension_; }
        const std::string &mimeType() const { return mimeType_; }

        bool operator==(const UiFileType&other) const {
            return label_ == other.label_ 
                && mimeType_ == other.mimeType_
                && fileExtension_ == other.fileExtension_;
        }

    public:
        DECLARE_JSON_MAP(UiFileType)

    };



    class UiPortNotification {
        friend class LilvUiPortNotification;
    private:
        int32_t portIndex_;
        std::string symbol_;
        std::string plugin_;
        std::string protocol_;
    public:

        UiPortNotification() { }
    
    public:
        DECLARE_JSON_MAP(UiPortNotification)

    };
    class UiFileProperty_Init {
    public:
        std::string label_;
        std::int32_t index_ = -1;
        std::string directory_;
        std::vector<UiFileType> fileTypes_;
        std::string patchProperty_;
        std::string portGroup_;

    };
    class UiFileProperty: private UiFileProperty_Init {
        friend class LilvUiFileProperty;
    public:
        UiFileProperty() { }
        UiFileProperty(UiFileProperty_Init && values)
        {
            *(UiFileProperty_Init*)(this) = std::move(values);
        }
        UiFileProperty(const std::string&name, const std::string&patchProperty,const std::string &directory);


        const std::string &label() const { return label_; }
        int32_t index() const { return index_; }
        const std::string &directory() const { return directory_; }
        const std::string&portGroup() const { return portGroup_; }

        const std::vector<UiFileType> &fileTypes() const { return fileTypes_; }
        std::vector<UiFileType> &fileTypes() { return fileTypes_; }

        const std::string &patchProperty() const { return patchProperty_; }
        bool IsValidExtension(const std::string&extension) const;

    public:
        DECLARE_JSON_MAP(UiFileProperty)
    };

    class UiFrequencyPlot_Init {
    public:
        std::string patchProperty_;
        std::int32_t index_ = -1;
        std::string portGroup_;
        float xLeft_ = 100;
        float xRight_ = 22000;
        float yTop_ = 5;
        float yBottom_ = -30;
        bool xLog_ = true;
        bool yDb_ = true;
        float width_ = 60;
    };
    class UiFrequencyPlot: private UiFrequencyPlot_Init  {
        friend class LilvUiFrequencyPlot;
    public:
        UiFrequencyPlot() { }
        UiFrequencyPlot(const UiFrequencyPlot_Init&init) { 
            *((UiFrequencyPlot_Init*)(this)) = init;
        }

        const std::string &patchProperty() const { return patchProperty_; }
        int32_t index() const { return index_; }
        const std::string&portGroup() const { return portGroup_; }
        float xLeft() const { return xLeft_; }
        float xRight() const { return xRight_; }
        bool xLog() const { return xLog_; }
        float yTop() const { return yTop_; }
        float yBottom() const { return yBottom_; }
        bool yDb() const { return yDb_; }
        float width() const { return width_; }

    public:
        DECLARE_JSON_MAP(UiFrequencyPlot)
    };

    class PiPedalUI  {
        friend class LilvPiPedalUI;
    public:
        PiPedalUI() { }       
        // PiPedalUI(
        //     std::vector<UiFileProperty> &&fileProperties,
        //     std::vector<UiFrequencyPlot> &&frequencyPlots);
        PiPedalUI(
            std::vector<UiFileProperty> &&fileProperties,
            std::vector<UiFrequencyPlot> &&frequencyPlots,
            std::vector<UiPortNotification> &&portNotifications            
            );
        PiPedalUI(
            std::vector<UiFileProperty> &&fileProperties);

        const std::vector<UiFileProperty>& fileProperties() const
        {
            return fileProperties_;
        }
        const std::vector<UiFrequencyPlot>& frequencyPlots() const
        {
            return frequencyPlots_;
        }

        const std::vector<UiPortNotification> &portNotifications() const { return portNotifications_; }

        const UiFileProperty *GetFileProperty(const std::string &propertyUri) const
        {
            for (const auto&fileProperty : fileProperties())
            {
                if (fileProperty.patchProperty() == propertyUri)
                {
                    return &fileProperty;
                }
            }
            return nullptr;
        }
    private:
        std::vector<UiFileProperty> fileProperties_;
        std::vector<UiFrequencyPlot> frequencyPlots_;
        std::vector<UiPortNotification> portNotifications_;
    };
}


