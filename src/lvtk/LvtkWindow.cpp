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

#include "lvtk/LvtkWindow.hpp"
#include "lvtk/LvtkDrawingContext.hpp"
#include "lvtk/LvtkContainerElement.hpp"
#include "lvtk/LvtkSvg.hpp"
#include "lvtk/LvtkSettingsFile.hpp"
#include "lvtk/LvtkMessageDialog.hpp"

#include <stdexcept>
#include <iostream>
#include <memory>
#include <filesystem>

#define XK_MISCELLANY
#define XK_LATIN1
#define XK_XKB_KEYS
#include <X11/keysymdef.h>

using namespace std;
using namespace lvtk;

// native window for x11/cairo
#include "lvtk/LvtkLog.hpp"
#include "ss.hpp"

#include "LvtkX11Window.hpp"
#include "lvtk/LvtkRootElement.hpp"

std::vector<std::filesystem::path> LvtkWindow::resourceDirectories;

namespace lvtk
{
    class FocusNavigationSelector
    {
    public:
        virtual ~FocusNavigationSelector() {}
        virtual void SetLastFocusRect(const LvtkRectangle &focusRect) = 0;
        virtual void Evaluate(std::shared_ptr<LvtkElement> element) = 0;
        virtual std::shared_ptr<LvtkElement> GetBestElement() = 0;
    };

}

LvtkWindow::LvtkWindow()
{
    this->theme = std::make_shared<LvtkTheme>(true);
    auto rootWindow = LvtkRootElement::Create();
    rootWindow->Style().Theme(this->theme);
    this->rootElement = rootWindow;
}

LvtkWindow::~LvtkWindow()
{
    if (this->rootElement)
    {
        this->rootElement->Unmount(this);
        this->rootElement = nullptr;
    }
    delete nativeWindow;
}

std::shared_ptr<LvtkRootElement> LvtkWindow::GetRootElement()
{
    return rootElement;
}

void LvtkWindow::Invalidate()
{
    LvtkSize size = Size();
    Invalidate(LvtkRectangle(0, 0, size.Width(), size.Height()));
}
void LvtkWindow::Invalidate(const LvtkRectangle &bounds)
{
    LvtkRectangle rc = LvtkRectangle(
        bounds.Left () * windowScale,
        bounds.Top() * windowScale,
        bounds.Width() * windowScale,
        bounds.Height() * windowScale);
        damageList.Invalidate(rc);
}

void LvtkWindow::OnExpose(WindowHandle h, int64_t x, int64_t y, int64_t width, int64_t height)
{
    damageList.ExposeRect(x, y, width, height);
}

LvtkDrawingContext LvtkWindow::CreateDrawingContext()
{
    return LvtkDrawingContext(nativeWindow->GetSurface());
}

void LvtkWindow::Draw()
{
    cairo_surface_t *surface = nativeWindow->GetSurface();

    LvtkDrawingContext context{surface};

    auto damageRects = this->damageList.GetDamageList();
    if (damageRects.size() == 0)
        return;
    for (auto &damageRect : damageRects)
    {

        // std::cout << damageRect.getLeft() << "," << damageRect.getTop() << "," << damageRect.getRight() << "," << damageRect.getBottom() << std::endl;
        LvtkRectangle displayRect{
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

LvtkCreateWindowParameters LvtkWindow::Scale(const LvtkCreateWindowParameters &v, double windowScale)
{

    LvtkCreateWindowParameters result = v;
    result.settingsObject = json_variant();
    result.location = LvtkPoint(
        (int)std::floor(v.location.x * windowScale),
        (int)std::floor(v.location.y * windowScale));
    result.size = LvtkSize(
        (int)std::ceil(v.size.Width() * windowScale),
        (int)std::ceil(v.size.Height() * windowScale));

    result.minSize = LvtkSize(
        (int)std::ceil(v.minSize.Width() * windowScale),
        (int)std::ceil(v.minSize.Height() * windowScale));

    result.maxSize = LvtkSize(
        (int)std::ceil(v.maxSize.Width() * windowScale),
        (int)std::ceil(v.maxSize.Height() * windowScale));
    return result;
}

void LvtkWindow::Close()
{
    if (this->nativeWindow)
    {
        this->nativeWindow->Close();
    }
}

void LvtkWindow::CloseRootWindow()
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
void LvtkWindow::OnClosing()
{
}

void LvtkWindow::CreateChildWindow(
    LvtkWindow *parent,
    const LvtkCreateWindowParameters &parameters,
    LvtkElement::ptr element)
{
    this->windowScale = parent->windowScale;
    this->windowParameters = parameters;
    this->windowParameters.settingsObject = parent->Settings();

    this->windowParameters.Load();
    LvtkCreateWindowParameters scaledParameters = Scale(windowParameters, windowScale);
    this->nativeWindow = new LvtkX11Window(SelfPointer(), scaledParameters);
    this->windowParameters.positioning = scaledParameters.positioning;
    this->windowParameters.location = scaledParameters.location / windowScale;

    this->nativeWindow->WindowTitle(this->windowTitle);

    if (this->rootElement)
    {
        rootElement->Mount(this);
    }
}

void LvtkWindow::CreateWindow(
    WindowHandle hParent,
    const LvtkCreateWindowParameters &parameters)
{
    this->windowParameters = parameters;
    if (this->windowParameters.positioning != LvtkWindowPositioning::ChildWindow)
    {
        this->windowParameters.Load();
    }
    if (settings.is_null())
    {
        settings = this->windowParameters.settingsObject;
    }
    LvtkCreateWindowParameters scaledParameters = Scale(windowParameters, windowScale);
    this->nativeWindow = new LvtkX11Window(SelfPointer(), hParent, scaledParameters);
    this->windowParameters.positioning = scaledParameters.positioning;
    this->windowParameters.location = scaledParameters.location / windowScale;

    if (this->rootElement)
    {
        rootElement->Mount(this);
    }
}

void LvtkWindow::CreateWindow(
    const LvtkCreateWindowParameters &parameters)
{
    CreateWindow(WindowHandle(), parameters);
}

bool LvtkWindow::PumpMessages(bool block)
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

bool LvtkWindow::OnMouseDown(LvtkMouseEventArgs &event)
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
bool LvtkWindow::OnMouseUp(LvtkMouseEventArgs &event)
{

    if (this->Capture() != nullptr)
    {
        auto element = this->Capture();
        event.point = event.screenPoint - LvtkPoint(element->screenClientBounds.Left(), element->screenClientBounds.Top());
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
bool LvtkWindow::OnMouseMove(LvtkMouseEventArgs &event)
{
    this->mousePosition = event.point;
    if (this->GetRootElement() != nullptr)
    {
        GetRootElement()->UpdateMouseOver(event.screenPoint);
    }

    // only send mouse move if captured.
    if (this->Capture() != nullptr)
    {
        LvtkElement *element = this->Capture();
        event.point = event.screenPoint - LvtkPoint(
                                              element->screenClientBounds.Left(),
                                              element->screenClientBounds.Top());
        if (this->Capture()->OnMouseMove(event))
        {
            return true;
        }
    }
    return false;
}

void LvtkWindow::MouseDown(WindowHandle h, uint64_t button, int64_t x, int64_t y, ModifierState state)
{
    LvtkMouseEventArgs event{h, button, x / windowScale, y / windowScale, state};
    this->lastMouseEventArgs = event;
    OnMouseDown(event);
}
void LvtkWindow::MouseUp(WindowHandle h, uint64_t button, int64_t x, int64_t y, ModifierState state)
{
    LvtkMouseEventArgs event{h, button, x / windowScale, y / windowScale, state};
    this->lastMouseEventArgs = event;
    OnMouseUp(event);
}
void LvtkWindow::MouseMove(WindowHandle h, int64_t x, int64_t y, ModifierState state)
{
    LvtkMouseEventArgs event{h, (uint64_t)-1, x / windowScale, y / windowScale, state};
    this->lastMouseEventArgs = event;
    OnMouseMove(event);
}
void LvtkWindow::MouseLeave(WindowHandle h)
{
    if (this->GetRootElement() != nullptr)
    {
        GetRootElement()->UpdateMouseOver(LvtkPoint(-1000,-1000));
    }
}

void LvtkWindow::PostQuit()
{
    if (this->nativeWindow)
    {
        this->nativeWindow->PostQuit();
    }
}
bool LvtkWindow::Quitting() const
{
    if (this->nativeWindow)
    {
        return this->nativeWindow->Quitting();
    }
    return true;
}

void LvtkWindow::TraceEvents(bool trace)
{
    nativeWindow->TraceEvents(trace);
}

void LvtkWindow::OnDraw(LvtkDrawingContext &dc)
{
}
void LvtkWindow::OnDrawOver(LvtkDrawingContext &dc)
{
}

void LvtkWindow::OnIdle()
{
    Draw();
}
void LvtkWindow::Layout()
{
    LvtkSize t = nativeWindow->Size();

    LvtkSize size{
        t.Width() / windowScale,
        t.Height() / windowScale};
    if (this->rootElement)
    {
        cairo_surface_t *surface = nativeWindow->GetSurface();
        LvtkDrawingContext context(surface);
        rootElement->Measure(size, size, context);
        rootElement->Arrange(size, context);

        LvtkRectangle clientRect = LvtkRectangle(0, 0, size.Width(), size.Height());
        rootElement->Layout(clientRect);
        rootElement->FinalizeLayout(clientRect, clientRect);
        rootElement->OnLayoutComplete();
    }
    OnLayoutComplete();
}
void LvtkWindow::Idle()
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

void LvtkWindow::InvalidateLayout()
{
    if (this->layoutValid)
        Invalidate();
    this->layoutValid = false;
}

void LvtkWindow::WindowTitle(const std::string &title)
{
    this->windowTitle = title;
    if (this->nativeWindow)
    {
        this->nativeWindow->WindowTitle(this->windowTitle);
    }
}
const std::string &LvtkWindow::WindowTitle() const
{
    return this->windowTitle;
}

void LvtkWindow::Size(const LvtkSize &size)
{
    if (this->size != size)
    {
        this->size = size;
        this->bounds = LvtkRectangle(0, 0, size.Width(), size.Height());

        Invalidate();
        InvalidateLayout();
        OnSizeChanged(this->size);
    }
}
void LvtkWindow::OnSizeChanged(const LvtkSize &size)
{
}

void LvtkWindow::OnX11SizeChanged(LvtkSize size)
{
    if (damageList.Width() != size.Width() || damageList.Height() != size.Height())
    {
        damageList.SetSize(
            (int64_t)std::ceil(size.Width()),
            (int64_t)std::ceil(size.Height()));
    }
    Size(size / windowScale);
}
bool LvtkWindow::Focus(LvtkElement *element)
{
    if (this->focusElement != element)
    {
        LvtkElement *oldElement = this->focusElement;
        this->focusElement = nullptr;
        if (oldElement != nullptr)
        {
            lastFocusRectangle = oldElement->ScreenBounds();
            oldElement->LostFocus(LvtkFocusEventArgs(oldElement, element));
            oldElement = nullptr;
        }
        this->focusElement = element;
        if (element != nullptr)
        {
            lastFocusRectangle = element->ScreenBounds();
            element->Focus(LvtkFocusEventArgs(oldElement, element));
        }
    }
    return true;
}
LvtkElement *LvtkWindow::FocusedElement()
{
    return this->focusElement;
}
const LvtkElement *LvtkWindow::FocusedElement() const
{
    return this->focusElement;
}
void LvtkWindow::releaseFocus(LvtkElement *element)
{
    if (this->focusElement == element)
    {
        lastFocusRectangle = element->ScreenBounds();
        this->focusElement = nullptr;
        element->LostFocus(LvtkFocusEventArgs(element, nullptr));
    }
}

bool LvtkWindow::Capture(LvtkElement *element)
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
LvtkElement *LvtkWindow::Capture()
{
    return this->captureElement;
}
void LvtkWindow::releaseCapture(LvtkElement *element)
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

LvtkPoint LvtkWindow::MousePosition() const
{
    return mousePosition;
}

LvtkTheme::ptr LvtkWindow::ThemePtr()
{
    return this->theme;
}

LvtkWindow &LvtkWindow::Theme(std::shared_ptr<LvtkTheme> theme)
{
    this->theme = theme;
    this->Invalidate();
    return *this;
}

const LvtkTheme &LvtkWindow::Theme() const
{
    return *(theme.get());
}

PangoContext *LvtkWindow::GetPangoContext()
{
    if (this->nativeWindow)
    {
        return this->nativeWindow->GetPangoContext();
    }
    return nullptr;
}

void LvtkWindow::Animate()
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

AnimationHandle LvtkWindow::PostDelayed(std::chrono::milliseconds delay, const DelayCallback &callback)
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
AnimationHandle LvtkWindow::PostDelayed(std::chrono::milliseconds delay, DelayCallback &&callback)
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
bool LvtkWindow::CancelPostDelayed(AnimationHandle handle)
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

AnimationHandle LvtkWindow::RequestAnimationCallback(const AnimationCallback &callback)
{
    AnimationHandle h = AnimationHandle::Next();
    animationCallbacks[h] = callback;
    return h;
}

AnimationHandle LvtkWindow::RequestAnimationCallback(AnimationCallback &&callback)
{
    AnimationHandle h = AnimationHandle::Next();
    animationCallbacks[h] = std::move(callback);
    return h;
}
bool LvtkWindow::CancelAnimationCallback(AnimationHandle handle)
{
    auto f = animationCallbacks.find(handle);
    if (f != animationCallbacks.end())
    {
        animationCallbacks.erase(f);
        return true;
    }
    return false;
}

std::filesystem::path LvtkWindow::findResourceFile(const std::filesystem::path &path)
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

LvtkSurface LvtkWindow::GetPngImage(const std::string &filename)
{
    if (pngCache.contains(filename))
    {
        return pngCache[filename];
    }
    std::filesystem::path path = findResourceFile(filename);
    if (!std::filesystem::exists(path))
    {
        LogError(SS("Can't find resourcefile " << path << ". Call static void LvtkWindow::SetResourceDirectories()."));
        return LvtkSurface();
    }
    LvtkSurface result = LvtkSurface::create_from_png(path.string());
    if (result.get())
    {
        if (result.status() != cairo_status_t::CAIRO_STATUS_SUCCESS)
        {
            LogError(SS("Failed to load PNG file " << path << ". (" << LvtkStatusMessage(result.status()) << ")"));
            return LvtkSurface();
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

LvtkSvg::ptr LvtkWindow::GetSvgImage(const std::string &filename)
{
    if (svgCache.contains(filename))
    {
        return svgCache[filename];
    }

    std::filesystem::path path = findResourceFile(filename);
    if (!std::filesystem::exists(path))
    {
        LogError(SS("Can't find resourcefile " << path << ". Call static void LvtkWindow::SetResourceDirectories()."));
        return nullptr;
    }
    LvtkSvg::ptr result = LvtkSvg::Create();
    result->load(path.string());
    svgCache[filename] = result;
    return result;
}

void LvtkWindow::SetResourceDirectories(const std::vector<std::filesystem::path> &paths)
{
    resourceDirectories = paths;
}

bool LvtkWindow::OnX11KeycodeDown(LvtkKeyboardEventArgs &eventArgs)
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
bool LvtkWindow::OnX11KeycodeUp(LvtkKeyboardEventArgs &eventArgs)
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

bool LvtkWindow::OnKeyDown(LvtkKeyboardEventArgs &eventArgs)
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

static LvtkElement *FindElement(LvtkElement *root, LvtkElement *target)
{
    if (root == target)
        return target;
    if (root->isContainer())
    {
        LvtkContainerElement *container = (LvtkContainerElement *)root;
        for (auto &child : container->LayoutChildren())
        {
            LvtkElement *childResult = FindElement(child.get(), target);
            if (childResult)
            {
                return childResult;
            }
        }
    }
    return nullptr;
}
void LvtkWindow::FireFocusIn()
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
void LvtkWindow::FireFocusOut()
{
    savedFocusElement = FocusedElement();
    Focus(nullptr);
}

class TabNavigationSelector : public FocusNavigationSelector
{
private:
    LvtkPoint startPoint;

    LvtkPoint bestPoint;
    LvtkElement::ptr bestElement;

public:
    virtual void SetLastFocusRect(const LvtkRectangle &focusRect)
    {
        startPoint = LvtkPoint(focusRect.Left(), focusRect.Top());
    }
    virtual void Evaluate(std::shared_ptr<LvtkElement> element)
    {
        auto &bounds = element->ScreenBounds();
        LvtkPoint location{bounds.Left(), bounds.Top()};
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
    virtual std::shared_ptr<LvtkElement> GetBestElement()
    {
        return bestElement;
    }
};
class ReverseTabNavigationSelector : public FocusNavigationSelector
{
private:
    LvtkPoint startPoint;

    LvtkPoint bestPoint;
    LvtkElement::ptr bestElement;

public:
    virtual void SetLastFocusRect(const LvtkRectangle &focusRect)
    {
        startPoint = LvtkPoint(focusRect.Left(), focusRect.Top());
    }
    virtual void Evaluate(std::shared_ptr<LvtkElement> element)
    {
        auto &bounds = element->ScreenBounds();
        LvtkPoint location{bounds.Left(), bounds.Top()};
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
    virtual std::shared_ptr<LvtkElement> GetBestElement()
    {
        return bestElement;
    }
};

class DownNavigationSelector : public FocusNavigationSelector
{
private:
    LvtkPoint startPoint;

    LvtkPoint bestPoint;
    LvtkElement::ptr bestElement;

public:
    virtual void SetLastFocusRect(const LvtkRectangle &focusRect)
    {
        startPoint = LvtkPoint(focusRect.Left(), focusRect.Top());
    }
    virtual void Evaluate(std::shared_ptr<LvtkElement> element)
    {
        auto &bounds = element->ScreenBounds();
        LvtkPoint location{bounds.Left(), bounds.Top()};
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
    virtual std::shared_ptr<LvtkElement> GetBestElement()
    {
        return bestElement;
    }
};

class UpNavigationSelector : public FocusNavigationSelector
{
private:
    LvtkPoint startPoint;

    LvtkPoint bestPoint;
    LvtkElement::ptr bestElement;

public:
    virtual void SetLastFocusRect(const LvtkRectangle &focusRect)
    {
        startPoint = LvtkPoint(focusRect.Left(), focusRect.Top());
    }
    virtual void Evaluate(std::shared_ptr<LvtkElement> element)
    {
        auto &bounds = element->ScreenBounds();
        LvtkPoint location{bounds.Left(), bounds.Top()};
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
    virtual std::shared_ptr<LvtkElement> GetBestElement()
    {
        return bestElement;
    }
};

static void Visit(LvtkElement::ptr element, FocusNavigationSelector &selector)
{
    if (element->Style().Visibility() == LvtkVisibility::Collapsed)
    {
        return;
    }
    if (element->isContainer())
    {
        LvtkContainerElement *pContainer = (LvtkContainerElement *)element.get();
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
void LvtkWindow::NavigateFocus(FocusNavigationSelector &selector)
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

bool LvtkWindow::HandleKeyboardNavigation(LvtkKeyboardEventArgs &eventArgs)
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

void LvtkWindow::SendAnimationFrameMessage()
{
    if (nativeWindow)
    {
        nativeWindow->SendAnimationFrameMessage();
    }
}
void LvtkWindow::SendControlChangedMessage(int32_t control, float value)
{
    if (nativeWindow)
    {
        nativeWindow->SendControlChangedMessage(control, value);
    }
}

LvtkWindow &LvtkWindow::WindowScale(double scale)
{
    this->windowScale = scale;
    return *this;
}

double LvtkWindow::WindowScale() const
{
    return windowScale;
}
void LvtkWindow::OnX11WindowClosed()
{
    if (this->nativeWindow)
    {
        OnClosing();
        this->nativeWindow = nullptr;
    }
}

LvtkWindow::ptr LvtkWindow::SelfPointer()
{
    try
    {
        return shared_from_this();
    }
    catch (const std::exception &)
    {
        throw std::runtime_error("LvtkWindow must be created as a shared_ptr<>. Use LvtkWindow::Create() to create an instance of LvtkWindow.");
    }
}

const json_variant &LvtkWindow::Settings() const
{
    return settings;
}
LvtkWindow &LvtkWindow::Settings(const json_variant &settings)
{
    this->settings = settings;
    return *this;
}

void LvtkCreateWindowParameters::Load()
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
            this->positioning = myPosition["positioning"].as<LvtkWindowPositioning>(this->positioning);
            this->location = LvtkPointFromJson(myPosition["location"], this->location);
            this->size = LvtkSizeFromJson(myPosition["size"], this->size);
            this->state = myPosition["state"].as<LvtkWindowState>(LvtkWindowState::Normal);
        }
    }
}
/// @brief Save current valeus to the settingsObject.
void LvtkCreateWindowParameters::Save()
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
        json["location"] = LvtkPointToJson(this->location);
        json["size"] = LvtkSizeToJson(this->size);
        json["state"] = this->state;
        windowPositions[this->settingsKey] = json;
    }
}

void LvtkWindow::OnX11ConfigurationChanged(
    LvtkWindowPositioning positioning,
    LvtkWindowState windowState,
    LvtkPoint location,
    LvtkSize size)
{
    if ((!this->windowParameters.settingsObject.is_null()) && this->windowParameters.settingsKey.length() != 0)
    {
        if (windowState == LvtkWindowState::Maximized)
        {
            windowParameters.state = LvtkWindowState::Maximized;
            windowParameters.Save();
        }
        else if (windowState == LvtkWindowState::Normal)
        {
            windowParameters.state = LvtkWindowState::Normal;
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

bool LvtkWindow::ModalDisable() const
{
    return modalDisableCount != 0;
}
void LvtkWindow::AddModalDisable()
{
    ++modalDisableCount;
}
void LvtkWindow::RemoveModalDisable()
{
    --modalDisableCount;
}

void LvtkWindow::MessageBox(LvtkMessageDialogType dialogType, const std::string &title, const std::string &text)
{
    LvtkMessageDialog::ptr dlg = LvtkMessageDialog::Create(dialogType, title, text);
    dlg->Show(this);
}

void LvtkWindow::OnLayoutComplete()
{
}
void LvtkWindow::SetStringProperty(const std::string &key, const std::string &value)
{
    // if (!this->nativeWindow)
    // {
    //     throw std::runtime_error("Window has not yet been created.");
    // }
    // nativeWindow->SetStringProperty(key,value);
}

std::optional<std::string> LvtkWindow::GetStringProperty(const std::string &key)
{
    return std::optional<std::string>();
}

void LvtkWindow::Resize(int width, int height)
{
    if (this->nativeWindow)
    {
        this->nativeWindow->Resize(width, height);
    }
}

WindowHandle LvtkWindow::Handle() const
{
    if (this->nativeWindow)
    {
        return nativeWindow->Handle();
    }
    return WindowHandle();
}