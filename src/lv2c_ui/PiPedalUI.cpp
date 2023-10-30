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


#include "lv2c_ui/PiPedalUI.hpp"
#include "ss.hpp"
#include "lv2c_ui/MimeTypes.hpp"

using namespace lv2c::ui;

UiFileProperty::UiFileProperty(
    const std::string &name, 
    const std::string &patchProperty, 
    const std::string &directory, 
    const std::string &resourceDirectory)
{
    label_ = (name);
    patchProperty_ = (patchProperty);
    directory_ = (directory);
    resourceDirectory_ = resourceDirectory;
}

// PiPedalUI::PiPedalUI(
//     std::vector<UiFileProperty> &&fileProperties,
//     std::vector<UiFrequencyPlot> &&frequencyPlots)
// {
//     this->fileProperties_ = std::move(fileProperties);
//     this->frequencyPlots_ = std::move(frequencyPlots);
// }

PiPedalUI::PiPedalUI(
    std::vector<lv2c::ui::UiFileProperty> &&fileProperties,
    std::vector<lv2c::ui::UiFrequencyPlot> &&frequencyPlots,
    std::vector<lv2c::ui::UiPortNotification> &&portNotifications)
{

    this->fileProperties_ = std::move(fileProperties);
    this->frequencyPlots_ = std::move(frequencyPlots);
    this->portNotifications_ = std::move(portNotifications);
}

PiPedalUI::PiPedalUI(
    std::vector<UiFileProperty> &&fileProperties)
{
    this->fileProperties_ = std::move(fileProperties);
}

UiFileType::UiFileType(const std::string &label, const std::string &mimeType, const std::string &fileExtension)
    : label_(label), mimeType_(mimeType), fileExtension_(fileExtension)
{
}

UiFileType::UiFileType(const std::string &label, const std::string &fileType)
    : label_(label), fileExtension_(fileType)
{
    if (fileType.starts_with('.'))
    {
        fileExtension_ = fileType;
        mimeType_ = MimeTypes::MimeTypeFromExtension(fileType);
        if (mimeType_ == "")
        {
            mimeType_ = "application/octet-stream";
        }
    }
    else
    {
        fileExtension_ = MimeTypes::ExtensionFromMimeType(fileType); // (may be blank, esp. for audio/* and video/*.
        mimeType_ = fileType;
    }
    if (mimeType_ == "*")
    {
        mimeType_ = "application/octet-stream";
    }
}

JSON_MAP_BEGIN(UiPortNotification)
JSON_MAP_REFERENCE(UiPortNotification, portIndex)
JSON_MAP_REFERENCE(UiPortNotification, symbol)
JSON_MAP_REFERENCE(UiPortNotification, plugin)
JSON_MAP_REFERENCE(UiPortNotification, protocol)
JSON_MAP_END()

JSON_MAP_BEGIN(UiFileType)
JSON_MAP_REFERENCE(UiFileType, label)
JSON_MAP_REFERENCE(UiFileType, mimeType)
JSON_MAP_REFERENCE(UiFileType, fileExtension)
JSON_MAP_END()

JSON_MAP_BEGIN(UiFileProperty)
JSON_MAP_REFERENCE(UiFileProperty, label)
JSON_MAP_REFERENCE(UiFileProperty, index)
JSON_MAP_REFERENCE(UiFileProperty, directory)
JSON_MAP_REFERENCE(UiFileProperty, patchProperty)
JSON_MAP_REFERENCE(UiFileProperty, fileTypes)
JSON_MAP_REFERENCE(UiFileProperty, portGroup)
JSON_MAP_END()

JSON_MAP_BEGIN(UiFrequencyPlot)
JSON_MAP_REFERENCE(UiFrequencyPlot, patchProperty)
JSON_MAP_REFERENCE(UiFrequencyPlot, index)
JSON_MAP_REFERENCE(UiFrequencyPlot, portGroup)
JSON_MAP_REFERENCE(UiFrequencyPlot, xLeft)
JSON_MAP_REFERENCE(UiFrequencyPlot, xRight)
JSON_MAP_REFERENCE(UiFrequencyPlot, xLog)
JSON_MAP_REFERENCE(UiFrequencyPlot, yTop)
JSON_MAP_REFERENCE(UiFrequencyPlot, yBottom)
JSON_MAP_REFERENCE(UiFrequencyPlot, width)

JSON_MAP_END()