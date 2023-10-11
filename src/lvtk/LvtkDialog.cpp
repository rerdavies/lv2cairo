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

#include "lvtk/LvtkDialog.hpp"
#include "LvtkX11Window.hpp"

using namespace lvtk;

LvtkDialog::~LvtkDialog()
{
}

LvtkDialog &LvtkDialog::Title(const std::string &value)
{
    this->title = value;
    return *this;
}
const std::string &LvtkDialog::Title() const
{
    return title;
}
LvtkDialog &LvtkDialog::SettingsKey(const std::string &value)
{
    this->settingsKey = value;
    return *this;
}
const std::string &LvtkDialog::SettingsKey() const
{
    return settingsKey;
}

LvtkSize LvtkDialog::DefaultSize() const
{
    return defaultSize;
}
LvtkDialog &LvtkDialog::DefaultSize(LvtkSize size)
{
    defaultSize = size;
    return *this;
}

LvtkDialog &LvtkDialog::MinSize(LvtkSize value)
{
    minSize = value;
    return *this;
}
LvtkSize LvtkDialog::MinSize() const
{
    return minSize;
}
LvtkDialog &LvtkDialog::MaxSize(LvtkSize value)
{
    maxSize = value;
    return *this;
}
LvtkSize LvtkDialog::MaxSize() const
{
    return maxSize;
}

void LvtkDialog::Show(LvtkWindow *parentWindow)
{
    Theme(parentWindow->ThemePtr());

    LvtkCreateWindowParameters parameters;
    parameters.location = LvtkPoint(0, 0);
    parameters.positioning = LvtkWindowPositioning::CenterOnParent;
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
    parameters.windowType = LvtkWindowType::Utility;
    parameters.backgroundColor = Theme().dialogBackgroundColor;
    parameters.owner = parentWindow;

    parameters.Load();
    this->windowScale = parentWindow->windowScale;

    LvtkElement::ptr element = Render();
    // perform prelayout to determine the size of the window.
    if (parameters.size.Width() == 0 || parameters.size.Height() == 0)
    {
        // borrow the current cairo surface in order to perform layout on
        // the new element.
        this->nativeWindow = parentWindow->nativeWindow;

        // i.e. "unlimited" space.
        constexpr double LARGE_BOUNDS = 32767;

        LvtkRectangle bounds{
            0, 0,
            parameters.size.Width() == 0 ? LARGE_BOUNDS : parameters.size.Width(),
            parameters.size.Height() == 0 ? LARGE_BOUNDS : parameters.size.Height()};
        element->Mount(this);
        LvtkSize constraint{(double)parameters.size.Width(), (double)parameters.size.Height()};
        LvtkSize available{bounds.Width(), bounds.Height()};
        {
            LvtkDrawingContext context(this->nativeWindow->GetSurface());
            element->Measure(constraint, available, context);
            LvtkSize arrangeSize = element->Arrange(element->MeasuredSize(), context);
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
    LvtkCreateWindowParameters scaledParameters = LvtkWindow::Scale(this->windowParameters,windowScale);
    this->nativeWindow = new LvtkX11Window(
        this->shared_from_this(),
        parentWindow->nativeWindow,
        scaledParameters);
    this->windowParameters.positioning = scaledParameters.positioning;
    this->windowParameters.location = scaledParameters.location / windowScale;
    if (GetRootElement())
    {
        ((LvtkElement *)GetRootElement().get())->Mount(this);
    }
    if (parameters.windowType == LvtkWindowType::Dialog)
    {
        this->modalDisableWindow = parentWindow;
        parentWindow->AddModalDisable();
    }
    OnMount();
}

LvtkDialog &LvtkDialog::Gravity(LvtkWindowGravity value)
{
    this->gravity = value;
    return *this;
}
LvtkWindowGravity LvtkDialog::Gravity() const
{
    return gravity;
}

LvtkWindowType LvtkDialog::WindowType() const
{
    return this->windowType;
}
LvtkDialog &LvtkDialog::WindowType(LvtkWindowType windowType)
{
    this->windowType = windowType;
    return *this;
}

LvtkWindowPositioning LvtkDialog::Positioning() const
{
    return this->positioning;
}
LvtkDialog &LvtkDialog::Positioning(LvtkWindowPositioning positioning)
{
    this->positioning = positioning;
    return *this;
}

void LvtkDialog::OnMount()
{

}


void LvtkDialog::OnClosing()
{

    if (modalDisableWindow)
    {
        modalDisableWindow->RemoveModalDisable();
        modalDisableWindow = nullptr;
    }
    ClosingEventArgs args;
    Closing.Fire(args);
}
