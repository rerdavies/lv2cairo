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

#include "lv2c/Lv2cWindow.hpp"
#include "lv2c/Lv2cDrawingContext.hpp"
#include "lv2c/Lv2cContainerElement.hpp"
#include "lv2c/Lv2cSvg.hpp"
#include "lv2c/Lv2cSettingsFile.hpp"
#include "lv2c/Lv2cMessageDialog.hpp"

#include <stdexcept>
#include <iostream>
#include <memory>
#include <filesystem>

#define XK_MISCELLANY
#define XK_LATIN1
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

using namespace std;
using namespace lv2c;

// native window for x11/cairo
#include "lv2c/Lv2cLog.hpp"
#include "ss.hpp"

#include "Lv2cX11Window.hpp"
#include "lv2c/Lv2cRootElement.hpp"

std::vector<std::filesystem::path> Lv2cWindow::resourceDirectories;

namespace lv2c
{
    class FocusNavigationSelector
    {
    public:
        virtual ~FocusNavigationSelector() {}
        virtual void SetLastFocusRect(const Lv2cRectangle &focusRect) = 0;
        virtual void Evaluate(std::shared_ptr<Lv2cElement> element) = 0;
        virtual std::shared_ptr<Lv2cElement> GetBestElement() = 0;
    };

}

Lv2cWindow::Lv2cWindow()
{
    this->theme = std::make_shared<Lv2cTheme>(true);
    auto rootWindow = Lv2cRootElement::Create();
    rootWindow->Style().Theme(this->theme);
    this->rootElement = rootWindow;
    this->rootElement->Style().Cursor(Lv2cCursor::Arrow);
}

Lv2cWindow::~Lv2cWindow()
{
    if (this->rootElement)
    {
        this->rootElement->Unmount(this);
        this->rootElement = nullptr;
    }
    delete nativeWindow;
}

std::shared_ptr<Lv2cRootElement> Lv2cWindow::GetRootElement()
{
    return rootElement;
}

void Lv2cWindow::Invalidate()
{
    Lv2cSize size = Size();
    Invalidate(Lv2cRectangle(0, 0, size.Width(), size.Height()));
}
void Lv2cWindow::Invalidate(const Lv2cRectangle &bounds)
{
    Lv2cRectangle rc = Lv2cRectangle(
        bounds.Left () * windowScale,
        bounds.Top() * windowScale,
        bounds.Width() * windowScale,
        bounds.Height() * windowScale);
        damageList.Invalidate(rc);
}

void Lv2cWindow::OnExpose(WindowHandle h, int64_t x, int64_t y, int64_t width, int64_t height)
{
    damageList.ExposeRect(x, y, width, height);
}

Lv2cDrawingContext Lv2cWindow::CreateDrawingContext()
{
    return Lv2cDrawingContext(nativeWindow->GetSurface());
}

void Lv2cWindow::Draw()
{
    cairo_surface_t *surface = nativeWindow->GetSurface();

    Lv2cDrawingContext context{surface};

    auto damageRects = this->damageList.GetDamageList();
    if (damageRects.size() == 0)
        return;
    for (auto &damageRect : damageRects)
    {

        // std::cout << damageRect.getLeft() << "," << damageRect.getTop() << "," << damageRect.getRight() << "," << damageRect.getBottom() << std::endl;
        Lv2cRectangle displayRect{
            damageRect.Left() / windowScale,
            damageRect.Top() / windowScale,
            damageRect.Width() / windowScale,
            damageRect.Height() / windowScale};
        context.save();
        context.scale(windowScale, windowScale);
        displayRect = context.round_to_device(displayRect);
        context.rectangle(displayRect);
        context.clip();
        //
        try
        {

            context.check_status();

            context.push_group_with_content(cairo_content_t::CAIRO_CONTENT_COLOR);
            OnDraw(context);
            if (rootElement)
            {
                rootElement->Draw(context, displayRect);
            }
            OnDrawOver(context);
            context.check_status();
            context.pop_group_to_source();

            context.check_status();
            auto t = context.get_operator();
            context.set_operator(cairo_operator_t::CAIRO_OPERATOR_SOURCE);
            context.rectangle(displayRect);
            context.fill();
            context.set_operator(t);
        }
        catch (const std::exception &e)
        {
            LogError(e.what());
        }
        context.restore();
        context.log_status();
    }
    // std::cout << "---" << std::endl;
}

Lv2cCreateWindowParameters Lv2cWindow::Scale(const Lv2cCreateWindowParameters &v, double windowScale)
{

    Lv2cCreateWindowParameters result = v;
    result.settingsObject = json_variant();
    result.location = Lv2cPoint(
        (int)std::floor(v.location.x * windowScale),
        (int)std::floor(v.location.y * windowScale));
    result.size = Lv2cSize(
        (int)std::ceil(v.size.Width() * windowScale),
        (int)std::ceil(v.size.Height() * windowScale));

    result.minSize = Lv2cSize(
        (int)std::ceil(v.minSize.Width() * windowScale),
        (int)std::ceil(v.minSize.Height() * windowScale));

    result.maxSize = Lv2cSize(
        (int)std::ceil(v.maxSize.Width() * windowScale),
        (int)std::ceil(v.maxSize.Height() * windowScale));
    return result;
}

void Lv2cWindow::Close()
{
    if (this->nativeWindow)
    {
        this->nativeWindow->Close();
    }
}

void Lv2cWindow::CloseRootWindow()
{
    Close();
    if (this->nativeWindow)
    {
        auto t = this->nativeWindow;
        this->nativeWindow = nullptr;
        delete t; // also deletes this!
        
        return;   // this may no longer be valid.
    }
}
void Lv2cWindow::OnClosing()
{
}

void Lv2cWindow::CreateChildWindow(
    Lv2cWindow *parent,
    const Lv2cCreateWindowParameters &parameters,
    Lv2cElement::ptr element)
{
    this->rootElement->AddChild(element);
    
    this->windowScale = parent->windowScale;
    this->windowParameters = parameters;
    this->windowParameters.settingsObject = parent->Settings();

    this->windowParameters.Load();
    Lv2cCreateWindowParameters scaledParameters = Scale(windowParameters, windowScale);
    this->nativeWindow = new Lv2cX11Window(SelfPointer(), scaledParameters);
    this->windowParameters.positioning = scaledParameters.positioning;
    this->windowParameters.location = scaledParameters.location / windowScale;

    this->nativeWindow->WindowTitle(this->windowTitle);

    if (this->rootElement)
    {
        rootElement->Mount(this);
    }
}

void Lv2cWindow::CreateWindow(
    WindowHandle hParent,
    const Lv2cCreateWindowParameters &parameters)
{
    this->windowParameters = parameters;
    if (this->windowParameters.positioning != Lv2cWindowPositioning::ChildWindow)
    {
        this->windowParameters.Load();
    }
    if (settings.is_null())
    {
        settings = this->windowParameters.settingsObject;
    }
    Lv2cCreateWindowParameters scaledParameters = Scale(windowParameters, windowScale);
    this->nativeWindow = new Lv2cX11Window(SelfPointer(), hParent, scaledParameters);
    this->windowParameters.positioning = scaledParameters.positioning;
    this->windowParameters.location = scaledParameters.location / windowScale;

    if (this->rootElement)
    {
        rootElement->Mount(this);
    }
    // pump messages once.
    this->nativeWindow->ProcessEvents();
}

void Lv2cWindow::CreateWindow(
    const Lv2cCreateWindowParameters &parameters)
{
    CreateWindow(WindowHandle(), parameters);
}

bool Lv2cWindow::PumpMessages(bool block)
{
    if (block)
    {
        if (nativeWindow == nullptr)
        {
            return true;
        }
        bool result = nativeWindow->AnimationLoop();
        if (Quitting())
        {
            auto t = this->nativeWindow;
            this->nativeWindow = nullptr;
            delete t;
        }
        return result;
    }
    else
    {
        if (nativeWindow == nullptr)
        {
            return true;
        }
        bool result = nativeWindow->ProcessEvents();
        if (Quitting())
        {
            auto t = this->nativeWindow;
            this->nativeWindow = nullptr;
            delete t;
        }
        return result;
    }
}

bool Lv2cWindow::OnScrollWheel(Lv2cScrollWheelEventArgs &event)
{
    if (this->GetRootElement() != nullptr)
    {
        if (GetRootElement()->FireScrollWheel(event))
        {
            return true;
        }
    }
    return false;

}

bool Lv2cWindow::OnMouseDown(Lv2cMouseEventArgs &event)
{
    if (this->Capture() != nullptr)
    {
        return this->Capture()->OnMouseDown(event);
    }
    if (this->GetRootElement() != nullptr)
    {
        if (GetRootElement()->FireMouseDown(event))
        {
            return true;
        }
    }
    this->Focus(nullptr);
    return false;
}
bool Lv2cWindow::OnMouseUp(Lv2cMouseEventArgs &event)
{

    if (this->Capture() != nullptr)
    {
        auto element = this->Capture();
        event.point = event.screenPoint - Lv2cPoint(element->screenClientBounds.Left(), element->screenClientBounds.Top());
        return this->Capture()->OnMouseUp(event);
    }
    if (this->GetRootElement() != nullptr)
    {
        if (GetRootElement()->FireMouseUp(event))
        {
            return true;
        }
    }
    return false;
}


bool Lv2cWindow::OnMouseMove(Lv2cMouseEventArgs &event)
{
    this->mousePosition = event.point;
    if (this->GetRootElement() != nullptr)
    {
        GetRootElement()->UpdateMouseOver(event.screenPoint);
    }


    // only send mouse move if captured.
    if (this->Capture() != nullptr)
    {
        Lv2cElement *element = this->Capture();
        event.point = event.screenPoint - Lv2cPoint(
                                              element->screenClientBounds.Left(),
                                              element->screenClientBounds.Top());
        if (this->Capture()->OnMouseMove(event))
        {
            return true;
        }
    }
    return false;
}


void Lv2cWindow::MouseScrollWheel(WindowHandle h, Lv2cScrollDirection direction, int64_t x, int64_t y, ModifierState state)
{
    Lv2cScrollWheelEventArgs event{h, direction, x / windowScale, y / windowScale, state};
    OnScrollWheel(event);

}


void Lv2cWindow::MouseDown(WindowHandle h, uint64_t button, int64_t x, int64_t y, ModifierState state)
{
    Lv2cMouseEventArgs event{h, button, x / windowScale, y / windowScale, state};
    this->lastMouseEventArgs = event;
    OnMouseDown(event);
}
void Lv2cWindow::MouseUp(WindowHandle h, uint64_t button, int64_t x, int64_t y, ModifierState state)
{
    Lv2cMouseEventArgs event{h, button, x / windowScale, y / windowScale, state};
    this->lastMouseEventArgs = event;
    OnMouseUp(event);
}
void Lv2cWindow::MouseMove(WindowHandle h, int64_t x, int64_t y, ModifierState state)
{
    Lv2cMouseEventArgs event{h, (uint64_t)-1, x / windowScale, y / windowScale, state};
    this->lastMouseEventArgs = event;
    OnMouseMove(event);
    UpdateMouseCursor(h,x,y,state);
}

void Lv2cWindow::UpdateMouseCursor(WindowHandle h, int64_t x, int64_t y, ModifierState state)
{
    Lv2cMouseEventArgs event{h, (uint64_t)-1, x / windowScale, y / windowScale, state};
    std::optional<Lv2cCursor> cursor = this->rootElement->GetMouseCursor(event);
    if (cursor) {
        nativeWindow->SetMouseCursor(*cursor);
    } else {
        nativeWindow->SetMouseCursor(Lv2cCursor::Arrow);
    }
}


void Lv2cWindow::MouseLeave(WindowHandle h)
{
    if (this->GetRootElement() != nullptr)
    {
        GetRootElement()->UpdateMouseOver(Lv2cPoint(-1000,-1000));
    }
}

void Lv2cWindow::PostQuit()
{
    if (this->nativeWindow)
    {
        this->nativeWindow->PostQuit();
    }
}
bool Lv2cWindow::Quitting() const
{
    if (this->nativeWindow)
    {
        return this->nativeWindow->Quitting();
    }
    return true;
}

void Lv2cWindow::TraceEvents(bool trace)
{
    nativeWindow->TraceEvents(trace);
}

void Lv2cWindow::OnDraw(Lv2cDrawingContext &dc)
{
}
void Lv2cWindow::OnDrawOver(Lv2cDrawingContext &dc)
{
}

void Lv2cWindow::OnIdle()
{
    Draw();
}
void Lv2cWindow::Layout()
{
    Lv2cSize t = nativeWindow->Size();

    Lv2cSize size{
        t.Width() / windowScale,
        t.Height() / windowScale};
    if (this->rootElement)
    {
        cairo_surface_t *surface = nativeWindow->GetSurface();
        Lv2cDrawingContext context(surface);
        rootElement->Measure(size, size, context);
        rootElement->Arrange(size, context);

        Lv2cRectangle clientRect = Lv2cRectangle(0, 0, size.Width(), size.Height());
        rootElement->Layout(clientRect);
        rootElement->FinalizeLayout(clientRect, clientRect);
        rootElement->OnLayoutComplete();
    }
    OnLayoutComplete();
}
void Lv2cWindow::Idle()
{
    while (!this->layoutValid)
    {
        this->layoutValid = true;
        Layout();
    }
    if (!this->valid)
    {
        this->valid = true;
        Draw();
    }
    OnIdle();
}

void Lv2cWindow::InvalidateLayout()
{
    if (this->layoutValid)
        Invalidate();
    this->layoutValid = false;
}

void Lv2cWindow::WindowTitle(const std::string &title)
{
    this->windowTitle = title;
    if (this->nativeWindow)
    {
        this->nativeWindow->WindowTitle(this->windowTitle);
    }
}
const std::string &Lv2cWindow::WindowTitle() const
{
    return this->windowTitle;
}

void Lv2cWindow::Size(const Lv2cSize &size)
{
    if (this->size != size)
    {
        this->size = size;
        this->bounds = Lv2cRectangle(0, 0, size.Width(), size.Height());

        Invalidate();
        InvalidateLayout();
        OnSizeChanged(this->size);
    }
}
void Lv2cWindow::OnSizeChanged(const Lv2cSize &size)
{
}

void Lv2cWindow::OnX11SizeChanged(Lv2cSize size)
{
    if (damageList.Width() != size.Width() || damageList.Height() != size.Height())
    {
        damageList.SetSize(
            (int64_t)std::ceil(size.Width()),
            (int64_t)std::ceil(size.Height()));
    }
    Size(size / windowScale);
}
bool Lv2cWindow::Focus(Lv2cElement *element)
{
    if (this->focusElement != element)
    {
        Lv2cElement *oldElement = this->focusElement;
        this->focusElement = nullptr;
        if (oldElement != nullptr)
        {
            lastFocusRectangle = oldElement->ScreenBounds();
            oldElement->LostFocus(Lv2cFocusEventArgs(oldElement, element));
            oldElement = nullptr;
        }
        this->focusElement = element;
        if (element != nullptr)
        {
            lastFocusRectangle = element->ScreenBounds();
            element->Focus(Lv2cFocusEventArgs(oldElement, element));
        }
    }
    return true;
}
Lv2cElement *Lv2cWindow::FocusedElement()
{
    return this->focusElement;
}
const Lv2cElement *Lv2cWindow::FocusedElement() const
{
    return this->focusElement;
}
void Lv2cWindow::releaseFocus(Lv2cElement *element)
{
    if (this->focusElement == element)
    {
        lastFocusRectangle = element->ScreenBounds();
        this->focusElement = nullptr;
        element->LostFocus(Lv2cFocusEventArgs(element, nullptr));
    }
}

bool Lv2cWindow::Capture(Lv2cElement *element)
{
    if (nativeWindow == nullptr)
        return false;

    if (!nativeWindow->GrabPointer())
    {
        LogWarning("Failed to grab pointer");
        return false;
    }

    this->captureElement = element;
    if (this->GetRootElement() != nullptr)
    {
        GetRootElement()->UpdateMouseOver(lastMouseEventArgs.screenPoint);
    }
    return true;
}
Lv2cElement *Lv2cWindow::Capture()
{
    return this->captureElement;
}
void Lv2cWindow::releaseCapture(Lv2cElement *element)
{
    if (this->captureElement && this->captureElement == element)
    {
        this->captureElement = nullptr;
        nativeWindow->UngrabPointer();
        if (this->GetRootElement() != nullptr)
        {
            GetRootElement()->UpdateMouseOver(lastMouseEventArgs.screenPoint);
        }
    }
}

Lv2cPoint Lv2cWindow::MousePosition() const
{
    return mousePosition;
}

Lv2cTheme::ptr Lv2cWindow::ThemePtr()
{
    return this->theme;
}

Lv2cWindow &Lv2cWindow::Theme(std::shared_ptr<Lv2cTheme> theme)
{
    this->theme = theme;
    this->Invalidate();
    return *this;
}

const Lv2cTheme &Lv2cWindow::Theme() const
{
    return *(theme.get());
}

PangoContext *Lv2cWindow::GetPangoContext()
{
    if (this->nativeWindow)
    {
        return this->nativeWindow->GetPangoContext();
    }
    return nullptr;
}

void Lv2cWindow::Animate()
{

    // keep *this alive for the duration of the call.
    auto safetyPtr = this->shared_from_this();
    auto now = animation_clock_t::now();

    std::vector<AnimationCallback> callbacks;

    if (animationCallbacks.size() != 0)
    {
        callbacks.reserve(this->animationCallbacks.size());

        for (auto &callback : this->animationCallbacks)
        {
            callbacks.push_back(std::move(callback.second));
        }
        this->animationCallbacks.clear();

        for (auto &callback : callbacks)
        {
            callback(now);
        }
    }

    if (delayCallbacks.size() != 0)
    {

        while (true)
        {
            // TODO: horrendously inefficient. Rewrite this for correct O.
            // The challenge is that the iterator gets inalidated if any of
            // the callbacks post a new Delayed item.
            // Also a challenge with the mutex.
            bool hasCallback = false;
            DelayCallback callback;
            {
                std::lock_guard guard{delayCallbacksMutex};
                for (auto &delayEntry : delayCallbacks)
                {
                    if (delayEntry.second.time <= now)
                    {
                        hasCallback = true;
                        callback = delayEntry.second.callback;
                        hasCallback = true;
                        auto h = delayEntry.first;
                        delayCallbacksMutex.unlock();

                        callback();

                        delayCallbacksMutex.lock();
                        auto f = delayCallbacks.find(h);
                        if (f != delayCallbacks.end())
                        {
                            delayCallbacks.erase(f);
                        }

                        break;
                    }
                }
            }
            if (!hasCallback)
            {
                break;
            }
        }
    }
}

AnimationHandle Lv2cWindow::PostDelayed(std::chrono::milliseconds delay, const DelayCallback &callback)
{
    AnimationHandle h = AnimationHandle::Next();
    auto delayRecord = DelayRecord{
        animation_clock_t::now() + std::chrono::duration_cast<animation_clock_t::duration>(delay),
        callback};
    {
        std::lock_guard guard{delayCallbacksMutex};
        delayCallbacks[h] = std::move(delayRecord);
    }
    return h;
}
AnimationHandle Lv2cWindow::PostDelayed(std::chrono::milliseconds delay, DelayCallback &&callback)
{
    AnimationHandle h = AnimationHandle::Next();
    auto delayRecord = DelayRecord{
        animation_clock_t::now() + duration_cast<animation_clock_t::duration>(delay),
        std::move(callback)};
    {
        std::lock_guard guard{delayCallbacksMutex};
        delayCallbacks[h] = std::move(delayRecord);
    }
    return h;
}
bool Lv2cWindow::CancelPostDelayed(AnimationHandle handle)
{
    std::lock_guard guard{delayCallbacksMutex};

    auto f = delayCallbacks.find(handle);
    if (f != delayCallbacks.end())
    {
        delayCallbacks.erase(f);
        return true;
    }
    return false;
}

AnimationHandle Lv2cWindow::RequestAnimationCallback(const AnimationCallback &callback)
{
    AnimationHandle h = AnimationHandle::Next();
    animationCallbacks[h] = callback;
    return h;
}

AnimationHandle Lv2cWindow::RequestAnimationCallback(AnimationCallback &&callback)
{
    AnimationHandle h = AnimationHandle::Next();
    animationCallbacks[h] = std::move(callback);
    return h;
}
bool Lv2cWindow::CancelAnimationCallback(AnimationHandle handle)
{
    auto f = animationCallbacks.find(handle);
    if (f != animationCallbacks.end())
    {
        animationCallbacks.erase(f);
        return true;
    }
    return false;
}

std::filesystem::path Lv2cWindow::findResourceFile(const std::filesystem::path &path)
{
    std::filesystem::path result = path;
    if (std::filesystem::exists(path))
        return path;

    for (const std::filesystem::path &directory : resourceDirectories)
    {
        std::filesystem::path result = directory / path;
        if (std::filesystem::exists(result))
            return result;
    }
    return path;
}

Lv2cSurface Lv2cWindow::GetPngImage(const std::string &filename)
{
    if (pngCache.contains(filename))
    {
        return pngCache[filename];
    }
    std::filesystem::path path = findResourceFile(filename);
    if (!std::filesystem::exists(path))
    {
        LogError(SS("Can't find resourcefile " << path << ". Call static void Lv2cWindow::SetResourceDirectories()."));
        return Lv2cSurface();
    }
    Lv2cSurface result = Lv2cSurface::create_from_png(path.string());
    if (result.get())
    {
        if (result.status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
        {
            LogError(SS("Failed to load PNG file " << path << ". (" << Lv2cStatusMessage(result.status()) << ")"));
            return Lv2cSurface();
        }
        else
        {
            pngCache[filename] = result;
        }
    }
    else
    {
        LogError(SS("Failed to load PNG file " << path << '.'));
    }
    return result;
}

Lv2cSvg::ptr Lv2cWindow::GetSvgImage(const std::string &filename)
{
    if (svgCache.contains(filename))
    {
        return svgCache[filename];
    }

    std::filesystem::path path = findResourceFile(filename);
    if (!std::filesystem::exists(path))
    {
        LogError(SS("Can't find resourcefile " << path << ". Call static void Lv2cWindow::SetResourceDirectories()."));
        return nullptr;
    }
    Lv2cSvg::ptr result = Lv2cSvg::Create();
    result->load(path.string());
    svgCache[filename] = result;
    return result;
}

void Lv2cWindow::SetResourceDirectories(const std::vector<std::filesystem::path> &paths)
{
    resourceDirectories = paths;
}

bool Lv2cWindow::OnX11KeycodeDown(Lv2cKeyboardEventArgs &eventArgs)
{
    if (this->focusElement)
    {
        eventArgs.target = this->focusElement;
        if (this->focusElement->OnKeycodeDown(eventArgs))
        {
            return true;
        }
    }
    return false;
}
bool Lv2cWindow::OnX11KeycodeUp(Lv2cKeyboardEventArgs &eventArgs)
{
    if (this->focusElement)
    {
        eventArgs.target = this->focusElement;
        if (this->focusElement->OnKeycodeUp(eventArgs))
        {
            return true;
        }
    }
    return false;
}

bool Lv2cWindow::OnKeyDown(Lv2cKeyboardEventArgs &eventArgs)
{
    if (this->focusElement)
    {
        eventArgs.target = this->focusElement;
        if (this->focusElement->FireKeyDown(eventArgs))
        {
            return true;
        }
    }
    return HandleKeyboardNavigation(eventArgs);
}

static Lv2cElement *FindElement(Lv2cElement *root, Lv2cElement *target)
{
    if (root == target)
        return target;
    if (root->isContainer())
    {
        Lv2cContainerElement *container = (Lv2cContainerElement *)root;
        for (auto &child : container->LayoutChildren())
        {
            Lv2cElement *childResult = FindElement(child.get(), target);
            if (childResult)
            {
                return childResult;
            }
        }
    }
    return nullptr;
}
void Lv2cWindow::FireFocusIn()
{
    if (savedFocusElement && rootElement)
    {
        // it's not a given that the element is still in the render tree.
        // so search for it before we hand focus back.

        auto focusElement = FindElement(rootElement.get(), savedFocusElement);
        if (focusElement)
        {
            focusElement->Focus();
        }
    }
    savedFocusElement = nullptr;
}
void Lv2cWindow::FireFocusOut()
{
    savedFocusElement = FocusedElement();
    Focus(nullptr);
}

class TabNavigationSelector : public FocusNavigationSelector
{
private:
    Lv2cPoint startPoint;

    Lv2cPoint bestPoint;
    Lv2cElement::ptr bestElement;

public:
    virtual void SetLastFocusRect(const Lv2cRectangle &focusRect)
    {
        startPoint = Lv2cPoint(focusRect.Left(), focusRect.Top());
    }
    virtual void Evaluate(std::shared_ptr<Lv2cElement> element)
    {
        auto &bounds = element->ScreenBounds();
        Lv2cPoint location{bounds.Left(), bounds.Top()};
        location.y -= startPoint.y;
        if (location.y < 0)
        {
            location.y += 200000;
        }
        else if (location.y == 0 && location.x <= startPoint.x)
        {
            location.y += 200000;
        }

        if (!bestElement)
        {
            bestElement = element;
            bestPoint = location;
        }
        else
        {
            if (location.y > bestPoint.y)
            {
                return;
            }
            if (location.y == bestPoint.y)
            {
                if (location.x >= bestPoint.x)
                {
                    return;
                }
            }
        }
        bestPoint = location;
        bestElement = element;
    }
    virtual std::shared_ptr<Lv2cElement> GetBestElement()
    {
        return bestElement;
    }
};
class ReverseTabNavigationSelector : public FocusNavigationSelector
{
private:
    Lv2cPoint startPoint;

    Lv2cPoint bestPoint;
    Lv2cElement::ptr bestElement;

public:
    virtual void SetLastFocusRect(const Lv2cRectangle &focusRect)
    {
        startPoint = Lv2cPoint(focusRect.Left(), focusRect.Top());
    }
    virtual void Evaluate(std::shared_ptr<Lv2cElement> element)
    {
        auto &bounds = element->ScreenBounds();
        Lv2cPoint location{bounds.Left(), bounds.Top()};
        location.y -= startPoint.y;
        if (location.y > 0)
        {
            location.y -= 200000;
        }
        else if (location.y == 0 && location.x >= startPoint.x)
        {
            location.y -= 200000;
        }

        if (!bestElement)
        {
            bestElement = element;
            bestPoint = location;
        }
        else
        {
            if (location.y < bestPoint.y)
            {
                return;
            }
            if (location.y == bestPoint.y)
            {
                if (location.x <= bestPoint.x)
                {
                    return;
                }
            }
        }
        bestPoint = location;
        bestElement = element;
    }
    virtual std::shared_ptr<Lv2cElement> GetBestElement()
    {
        return bestElement;
    }
};

class DownNavigationSelector : public FocusNavigationSelector
{
private:
    Lv2cPoint startPoint;

    Lv2cPoint bestPoint;
    Lv2cElement::ptr bestElement;

public:
    virtual void SetLastFocusRect(const Lv2cRectangle &focusRect)
    {
        startPoint = Lv2cPoint(focusRect.Left(), focusRect.Top());
    }
    virtual void Evaluate(std::shared_ptr<Lv2cElement> element)
    {
        auto &bounds = element->ScreenBounds();
        Lv2cPoint location{bounds.Left(), bounds.Top()};
        location.x -= startPoint.x;
        if (location.x < 0)
        {
            location.x += 200000;
        }
        else if (location.x == 0 && location.y <= startPoint.y)
        {
            location.x += 200000;
        }

        if (!bestElement)
        {
            bestElement = element;
            bestPoint = location;
        }
        else
        {
            if (location.x > bestPoint.x)
            {
                return;
            }
            if (location.x == bestPoint.x)
            {
                if (location.y >= bestPoint.y)
                {
                    return;
                }
            }
        }
        bestPoint = location;
        bestElement = element;
    }
    virtual std::shared_ptr<Lv2cElement> GetBestElement()
    {
        return bestElement;
    }
};

class UpNavigationSelector : public FocusNavigationSelector
{
private:
    Lv2cPoint startPoint;

    Lv2cPoint bestPoint;
    Lv2cElement::ptr bestElement;

public:
    virtual void SetLastFocusRect(const Lv2cRectangle &focusRect)
    {
        startPoint = Lv2cPoint(focusRect.Left(), focusRect.Top());
    }
    virtual void Evaluate(std::shared_ptr<Lv2cElement> element)
    {
        auto &bounds = element->ScreenBounds();
        Lv2cPoint location{bounds.Left(), bounds.Top()};
        location.x -= startPoint.x;
        if (location.x > 0)
        {
            location.x -= 200000;
        }
        else if (location.x == 0 && location.y >= startPoint.y)
        {
            location.x -= 200000;
        }

        if (!bestElement)
        {
            bestElement = element;
            bestPoint = location;
        }
        else
        {
            if (location.x < bestPoint.x)
            {
                return;
            }
            if (location.x == bestPoint.x)
            {
                if (location.y <= bestPoint.y)
                {
                    return;
                }
            }
        }
        bestPoint = location;
        bestElement = element;
    }
    virtual std::shared_ptr<Lv2cElement> GetBestElement()
    {
        return bestElement;
    }
};

static void Visit(Lv2cElement::ptr element, FocusNavigationSelector &selector)
{
    if (element->Style().Visibility() == Lv2cVisibility::Collapsed)
    {
        return;
    }
    if (element->isContainer())
    {
        Lv2cContainerElement *pContainer = (Lv2cContainerElement *)element.get();
        for (auto &child : pContainer->LayoutChildren())
        {
            Visit(child, selector);
        }
    }
    if (element->WantsFocus())
    {
        selector.Evaluate(element);
    }
}
void Lv2cWindow::NavigateFocus(FocusNavigationSelector &selector)
{
    selector.SetLastFocusRect(lastFocusRectangle);
    if (this->rootElement)
    {
        Visit(this->rootElement, selector);
    }
    auto result = selector.GetBestElement();
    if (result)
    {
        this->Focus(result.get());
    }
}

bool Lv2cWindow::HandleKeyboardNavigation(Lv2cKeyboardEventArgs &eventArgs)
{
    if (FocusedElement() != nullptr)
    {
        lastFocusRectangle = FocusedElement()->ScreenBounds();
    }
    if (eventArgs.keysymValid)
    {
        if (eventArgs.modifierState == ModifierState::Alt)
        {
            switch (eventArgs.keysym)
            {
            case XK_Left:
            case XK_KP_Left:
            {
                ReverseTabNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }
            case XK_Right:
            case XK_KP_Right:
            {
                TabNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }
            case XK_Down:
            case XK_KP_Down:
            {
                DownNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }
            case XK_Up:
            case XK_KP_Up:
            {
                UpNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }
            default:
                break;
            }
        }
        else if (eventArgs.modifierState == ModifierState::Empty)
        {
            switch (eventArgs.keysym)
            {
            case XK_KP_Left:
            {
                ReverseTabNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }
            case XK_Tab:
            case XK_KP_Right:
            {
                TabNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }
            case XK_KP_Down:
            {
                DownNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }
            case XK_KP_Up:
            {
                UpNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }

            default:
                break;
            }
        }
        else if (eventArgs.modifierState == ModifierState::Shift)
        {
            switch (eventArgs.keysym)
            {
            case XK_Tab:
            case XK_ISO_Left_Tab:
            {
                ReverseTabNavigationSelector selector;
                NavigateFocus(selector);
                return true;
            }
            default:
                break;
            }
        }
    }
    return false;
}

void Lv2cWindow::SendAnimationFrameMessage()
{
    if (nativeWindow)
    {
        nativeWindow->SendAnimationFrameMessage();
    }
}
void Lv2cWindow::SendControlChangedMessage(int32_t control, float value)
{
    if (nativeWindow)
    {
        nativeWindow->SendControlChangedMessage(control, value);
    }
}

Lv2cWindow &Lv2cWindow::WindowScale(double scale)
{
    this->windowScale = scale;
    return *this;
}

double Lv2cWindow::WindowScale() const
{
    return windowScale;
}
void Lv2cWindow::OnX11WindowClosed()
{
    if (this->nativeWindow)
    {
        OnClosing();
        this->nativeWindow = nullptr;
    }
}

Lv2cWindow::ptr Lv2cWindow::SelfPointer()
{
    try
    {
        return shared_from_this();
    }
    catch (const std::exception &)
    {
        throw std::runtime_error("Lv2cWindow must be created as a shared_ptr<>. Use Lv2cWindow::Create() to create an instance of Lv2cWindow.");
    }
}

const json_variant &Lv2cWindow::Settings() const
{
    return settings;
}
Lv2cWindow &Lv2cWindow::Settings(const json_variant &settings)
{
    this->settings = settings;
    return *this;
}

void Lv2cCreateWindowParameters::Load()
{
    if (!settingsObject.is_null() && settingsKey.length() != 0)
    {
        json_variant windowPositions = settingsObject["window_positions"];
        if (windowPositions.is_null())
        {
            settingsObject["window_positions"] = windowPositions = json_variant::object();
        }
        json_variant myPosition = windowPositions[this->settingsKey];
        if (!myPosition.is_null())
        {
            this->positioning = myPosition["positioning"].as<Lv2cWindowPositioning>(this->positioning);
            this->location = Lv2cPointFromJson(myPosition["location"], this->location);
            this->size = Lv2cSizeFromJson(myPosition["size"], this->size);
            this->state = myPosition["state"].as<Lv2cWindowState>(Lv2cWindowState::Normal);
        }
    }
}
/// @brief Save current valeus to the settingsObject.
void Lv2cCreateWindowParameters::Save()
{
    if (!settingsObject.is_null() && settingsKey.length() != 0)
    {
        json_variant windowPositions = settingsObject["window_positions"];
        if (windowPositions.is_null())
        {
            settingsObject["window_positions"] = windowPositions = json_variant::object();
        }
        auto json = json_variant::object();
        json["positioning"] = this->positioning;
        json["location"] = Lv2cPointToJson(this->location);
        json["size"] = Lv2cSizeToJson(this->size);
        json["state"] = this->state;
        windowPositions[this->settingsKey] = json;
    }
}

void Lv2cWindow::OnX11ConfigurationChanged(
    Lv2cWindowPositioning positioning,
    Lv2cWindowState windowState,
    Lv2cPoint location,
    Lv2cSize size)
{
    if ((!this->windowParameters.settingsObject.is_null()) && this->windowParameters.settingsKey.length() != 0)
    {
        if (windowState == Lv2cWindowState::Maximized)
        {
            windowParameters.state = Lv2cWindowState::Maximized;
            windowParameters.Save();
        }
        else if (windowState == Lv2cWindowState::Normal)
        {
            windowParameters.state = Lv2cWindowState::Normal;
            windowParameters.positioning = positioning;
            windowParameters.location = location / windowScale;
            windowParameters.size = size / windowScale;
            windowParameters.Save();
        }
        else
        {
            // minimized.
            // don't record anyrhing.
        }
    }
    OnX11SizeChanged(size);
}

bool Lv2cWindow::ModalDisable() const
{
    return modalDisableCount != 0;
}
void Lv2cWindow::AddModalDisable()
{
    ++modalDisableCount;
}
void Lv2cWindow::RemoveModalDisable()
{
    --modalDisableCount;
}

void Lv2cWindow::MessageBox(Lv2cMessageDialogType dialogType, const std::string &title, const std::string &text)
{
    Lv2cMessageDialog::ptr dlg = Lv2cMessageDialog::Create(dialogType, title, text);
    dlg->Show(this);
}

void Lv2cWindow::OnLayoutComplete()
{
}
void Lv2cWindow::SetStringProperty(const std::string &key, const std::string &value)
{
    // if (!this->nativeWindow)
    // {
    //     throw std::runtime_error("Window has not yet been created.");
    // }
    // nativeWindow->SetStringProperty(key,value);
}

std::optional<std::string> Lv2cWindow::GetStringProperty(const std::string &key)
{
    return std::optional<std::string>();
}

void Lv2cWindow::Resize(int width, int height)
{
    if (this->nativeWindow)
    {
        this->nativeWindow->Resize(width, height);
    }
}

WindowHandle Lv2cWindow::Handle() const
{
    if (this->nativeWindow)
    {
        return nativeWindow->Handle();
    }
    return WindowHandle();
}


std::shared_ptr<Lv2cObject> Lv2cWindow::GetMemoObject(const std::string&name)
{
    if (this->memoObjects.contains(name))
    {
        return this->memoObjects[name].lock();
    }
    return nullptr;
}
void Lv2cWindow::SetMemoObject(const std::string &name, std::shared_ptr<Lv2cObject> obj)
{
    this->memoObjects[name] = std::weak_ptr<Lv2cObject>(obj);
}



