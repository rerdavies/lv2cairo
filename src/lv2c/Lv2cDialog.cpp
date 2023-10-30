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

#include "lv2c/Lv2cDialog.hpp"
#include "Lv2cX11Window.hpp"

using namespace lv2c;

Lv2cDialog::~Lv2cDialog()
{
}

Lv2cDialog &Lv2cDialog::Title(const std::string &value)
{
    this->title = value;
    return *this;
}
const std::string &Lv2cDialog::Title() const
{
    return title;
}
Lv2cDialog &Lv2cDialog::SettingsKey(const std::string &value)
{
    this->settingsKey = value;
    return *this;
}
const std::string &Lv2cDialog::SettingsKey() const
{
    return settingsKey;
}

Lv2cSize Lv2cDialog::DefaultSize() const
{
    return defaultSize;
}
Lv2cDialog &Lv2cDialog::DefaultSize(Lv2cSize size)
{
    defaultSize = size;
    return *this;
}

Lv2cDialog &Lv2cDialog::MinSize(Lv2cSize value)
{
    minSize = value;
    return *this;
}
Lv2cSize Lv2cDialog::MinSize() const
{
    return minSize;
}
Lv2cDialog &Lv2cDialog::MaxSize(Lv2cSize value)
{
    maxSize = value;
    return *this;
}
Lv2cSize Lv2cDialog::MaxSize() const
{
    return maxSize;
}

void Lv2cDialog::Show(Lv2cWindow *parentWindow)
{
    Theme(parentWindow->ThemePtr());

    Lv2cCreateWindowParameters parameters;
    parameters.location = Lv2cPoint(0, 0);
    parameters.positioning = Lv2cWindowPositioning::CenterOnParent;
    parameters.size = this->DefaultSize();
    parameters.minSize = MinSize();
    parameters.maxSize = MaxSize();
    parameters.settingsKey = this->SettingsKey();
    parameters.settingsObject = parentWindow->Settings();
    parameters.title = Title();
    parameters.x11Windowclass = parentWindow->windowParameters.x11Windowclass;
    parameters.x11WindowName = X11WindowName();
    parameters.gravity = this->Gravity();
    parameters.positioning = this->positioning;
    parameters.windowType = Lv2cWindowType::Utility;
    parameters.backgroundColor = Theme().dialogBackgroundColor;
    parameters.owner = parentWindow;

    parameters.Load();
    this->windowScale = parentWindow->windowScale;

    Lv2cElement::ptr element = Render();
    // perform prelayout to determine the size of the window.
    if (parameters.size.Width() == 0 || parameters.size.Height() == 0)
    {
        // borrow the current cairo surface in order to perform layout on
        // the new element.
        this->nativeWindow = parentWindow->nativeWindow;

        // i.e. "unlimited" space.
        constexpr double LARGE_BOUNDS = 32767;

        Lv2cRectangle bounds{
            0, 0,
            parameters.size.Width() == 0 ? LARGE_BOUNDS : parameters.size.Width(),
            parameters.size.Height() == 0 ? LARGE_BOUNDS : parameters.size.Height()};
        element->Mount(this);
        Lv2cSize constraint{(double)parameters.size.Width(), (double)parameters.size.Height()};
        Lv2cSize available{bounds.Width(), bounds.Height()};
        {
            Lv2cDrawingContext context(this->nativeWindow->GetSurface());
            element->Measure(constraint, available, context);
            Lv2cSize arrangeSize = element->Arrange(element->MeasuredSize(), context);
            if (parameters.size.Height() == 0)
            {
                parameters.size.Height((int)std::ceil(arrangeSize.Height()));
                if (parameters.size.Height() >= LARGE_BOUNDS - 100)
                {
                    throw std::runtime_error("WindowPosition has zero height, element layout has unconstrained height.");
                }
            }
            if (parameters.size.Width() == 0)
            {
                parameters.size.Width((int)std::ceil(arrangeSize.Width()));
                if (parameters.size.Width() >= LARGE_BOUNDS - 100)
                {
                    throw std::runtime_error("WindowPosition has zero width, element layout has unconstrained width.");
                }
            }
        }
        element->Unmount(this);
        this->nativeWindow = nullptr;
    }

    this->GetRootElement()->AddChild(element);
    this->windowParameters = parameters;
    this->Settings(parameters.settingsObject);
    Lv2cCreateWindowParameters scaledParameters = Lv2cWindow::Scale(this->windowParameters,windowScale);
    this->nativeWindow = new Lv2cX11Window(
        this->shared_from_this(),
        parentWindow->nativeWindow,
        scaledParameters);
    this->windowParameters.positioning = scaledParameters.positioning;
    this->windowParameters.location = scaledParameters.location / windowScale;
    if (GetRootElement())
    {
        ((Lv2cElement *)GetRootElement().get())->Mount(this);
    }
    if (parameters.windowType == Lv2cWindowType::Dialog)
    {
        this->modalDisableWindow = parentWindow;
        parentWindow->AddModalDisable();
    }
    OnMount();
}

Lv2cDialog &Lv2cDialog::Gravity(Lv2cWindowGravity value)
{
    this->gravity = value;
    return *this;
}
Lv2cWindowGravity Lv2cDialog::Gravity() const
{
    return gravity;
}

Lv2cWindowType Lv2cDialog::WindowType() const
{
    return this->windowType;
}
Lv2cDialog &Lv2cDialog::WindowType(Lv2cWindowType windowType)
{
    this->windowType = windowType;
    return *this;
}

Lv2cWindowPositioning Lv2cDialog::Positioning() const
{
    return this->positioning;
}
Lv2cDialog &Lv2cDialog::Positioning(Lv2cWindowPositioning positioning)
{
    this->positioning = positioning;
    return *this;
}

void Lv2cDialog::OnMount()
{

}


void Lv2cDialog::OnClosing()
{

    if (modalDisableWindow)
    {
        modalDisableWindow->RemoveModalDisable();
        modalDisableWindow = nullptr;
    }
    ClosingEventArgs args;
    Closing.Fire(args);
}
