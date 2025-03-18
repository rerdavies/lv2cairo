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

#include "Lv2cX11Window.hpp"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <X11/extensions/Xrandr.h>
#include <algorithm>

#include <cairo/cairo-xlib.h>
#include <iomanip>
#include <cassert>
#include <limits>

#include "ss.hpp"

using namespace lv2c;

#define DEBUG_ENABLE_EVENT_TRACING false

static constexpr bool DEBUG_INTERCEPT_X_ERROR_HANDLER = false;

static constexpr int ANIMATION_RATE = 60;
static constexpr std::chrono::steady_clock::duration ANIMATION_DELAY = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::microseconds(1000000 / ANIMATION_RATE));

void Lv2cX11Window::logDebug(Window x11Window, const std::string &message)
{
    std::stringstream s;
    s << "(" << x11Window << "): " << message;
    lv2c::LogDebug(s.str());
}
#if (DEBUG_ENABLE_EVENT_TRACING)
#define LOG_TRACE(window, message) logDebug(window, message)
#else
#define LOG_TRACE(window, message) \
    {                              \
    }
#endif

#define X_INIT_ATOM(name) \
    name = XInternAtom(display, "_" #name, False)

struct Lv2cX11Window::XAtoms
{
    XAtoms(Display *display)
    {
        X_INIT_ATOM(NET_FRAME_EXTENTS);
        X_INIT_ATOM(NET_WM_STATE_MAXIMIZED_VERT);
        X_INIT_ATOM(NET_WM_STATE_MAXIMIZED_HORZ);
        X_INIT_ATOM(NET_WM_STATE_FOCUSED);
        X_INIT_ATOM(NET_WM_STATE);
        X_INIT_ATOM(NET_ACTIVE_WINDOW);
        X_INIT_ATOM(NET_RESTACK_WINDOW);
        X_INIT_ATOM(NET_CLIENT_LIST);
    }
    Atom NET_FRAME_EXTENTS,
        NET_WM_STATE_MAXIMIZED_VERT,
        NET_WM_STATE_MAXIMIZED_HORZ,
        NET_WM_STATE_FOCUSED,
        NET_WM_STATE,
        NET_ACTIVE_WINDOW,
        NET_RESTACK_WINDOW,
        NET_CLIENT_LIST;
};

static int (*old_handler)(Display *, XErrorEvent *) = nullptr;

static int Lv2c_ErrorHandler(Display *display, XErrorEvent *event)
{

    char buffer[1024];
    ::XGetErrorText(display, event->error_code, buffer, sizeof(buffer));

    std::cout << "X11Error*(" << std::hex << event->resourceid << std::dec << "): " << buffer << std::endl;
    return 0;
}

void Lv2cX11Window::SetErrorHandler()
{
    if (DEBUG_INTERCEPT_X_ERROR_HANDLER)
    {
        if (old_handler == nullptr)
        {
            old_handler = XSetErrorHandler(Lv2c_ErrorHandler);
        }
    }
}

void Lv2cX11Window::ReleaseErrorHandler()
{
    if (DEBUG_INTERCEPT_X_ERROR_HANDLER)
    {
        if (old_handler != nullptr)
        {
            XSetErrorHandler(old_handler);
            old_handler = nullptr;
        }
    }
}

inline int Lv2cX11Window::CheckX11Error(int retCode)
{
    if (retCode != Success)
    {
        std::cout << "X11 error: " << GetX11ErrorText(retCode) << std::endl;
    }
    return retCode;
}

inline void Lv2cX11Window::Sync()
{
    XSync(x11Display, false);
}

template <typename T>
bool GetX11ArrayProperty(Display *display, Window window,
                         Atom atomName,
                         std::vector<T> *result,
                         size_t nItems = 0,
                         Atom reqType = AnyPropertyType,
                         Atom *out_type = nullptr)
{
    // static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "");

    if (atomName == None)
    {
        return false;
    }

    Atom atomType;
    int format;

    using LengthType = unsigned long;
    LengthType nItemsOut;
    LengthType bytesAfter;
    unsigned char *data = nullptr;
    unsigned long requestedLength =
        nItems == 0
            ? std::numeric_limits<LengthType>::max() / sizeof(LengthType)
            : (LengthType)((sizeof(T) * nItems + 3) / 4);

    int rc = XGetWindowProperty(
        display,
        window,
        atomName,
        0,
        requestedLength,
        False,
        reqType,
        &atomType, &format,
        &nItemsOut,
        &bytesAfter,
        &data);
    if (rc != Success)
    {
        return false;
    }
    // if (format != sizeof(T) * 8)
    // {
    //     XFree(data);
    //     return false;
    // }
    if (out_type)
    {
        *out_type = atomType;
    }
    result->resize(nItemsOut);
    const long *typedData = (const long *)data; // All values are return as long[]. The size argument only indicates the size on the wire.
    for (size_t i = 0; i < nItemsOut; ++i)
    {
        (*result)[i] = (T)(typedData[i]);
    }
    XFree(data);
    return true;
}

static bool GetAtomArrayProperty(
    Display *display, Window window,
    Atom atomName,
    std::vector<Atom> *value)
{
    if (atomName == None)
    {
        return false;
    }
    Atom type = None;
    int format = 0; // size in bits of each item in 'property'
    unsigned long numItems = 0, extraBytes = 0;
    unsigned char *properties = NULL;
    int result = XGetWindowProperty(
        display, window,
        atomName,
        0, (1000), // (all of them)
        False,
        AnyPropertyType,
        &type, &format, &numItems, &extraBytes, &properties);
    if (result != Success)
        return false;
    if (type != XA_ATOM)
    {
        XFree(properties);
        return false;
    }
    Atom *atom_properties = reinterpret_cast<Atom *>(properties);
    value->clear();
    value->reserve(numItems);
    for (size_t i = 0; i < numItems; ++i)
    {
        value->push_back(atom_properties[i]);
    }
    XFree(properties);
    return true;
}
#pragma GCC diagnostic ignored "-Wunused-function"

static std::string toString(ModifierState modifierState)
{
    std::stringstream s;
    bool first = true;
    if (modifierState && ModifierState::Shift)
    {
        s << "Shift";
        first = false;
    }
    if (modifierState && ModifierState::Control)
    {
        if (!first)
            s << "+";
        s << "Ctl";
    }
    if (modifierState && ModifierState::Alt)
    {
        if (!first)
            s << "+";
        s << "Alt";
    }
    if (modifierState && ModifierState::Super)
    {
        if (!first)
            s << "+";
        s << "Super";
    }
    return s.str();
}

template <typename T>
inline void Lv2cX11Window::SetProperty(
    const std::string &property,
    const std::string &type,
    const std::vector<T> &data)
{
    static_assert(sizeof(T) <= 4);
    Atom propertyAtom = XInternAtom(x11Display, property.c_str(), False);
    Atom typeAtom = XInternAtom(x11Display, type.c_str(), False);
    XChangeProperty(
        x11Display,
        x11Window,
        propertyAtom,
        typeAtom,
        sizeof(T) * 8,
        PropModeReplace,
        (unsigned char *)(data.data()),
        (int)data.size());
}

Lv2cX11Window::~Lv2cX11Window()
{
    DestroyWindowAndSurface();
}

PangoContext *Lv2cX11Window::GetPangoContext()
{
    return pangoContext;
}

bool Lv2cX11Window::PostQuit()
{
    this->quitting = true;
    return true;
}

bool Lv2cX11Window::PostQuit(Window x11Window)
{
    if (this->x11Window == x11Window)
    {
        this->quitting = true;

        // destroy children.
        while (childWindows.size() != 0)
        {
            auto child = childWindows[childWindows.size() - 1];
            childWindows.pop_back();
            delete child;
        }

        return true;
    }
    for (auto child : childWindows)
    {
        if (child->PostQuit(x11Window))
        {
            return true;
        }
    }
    return false;
}

bool Lv2cX11Window::Quitting() const
{
    return quitting;
}
void Lv2cX11Window::TraceEvents(bool value)
{
    this->traceEvents = value;
}

struct Lv2cToWmWindowType
{
    Lv2cWindowType windowType;
    const char *wmxWindowType;
};

static Lv2cToWmWindowType gLv2cToXWindowType[]{
    {Lv2cWindowType::Desktop, "_NET_WM_WINDOW_TYPE_DESKTOP"},
    {Lv2cWindowType::Dock, "_NET_WM_WINDOW_TYPE_DOCK"},
    {Lv2cWindowType::Toolbar, "_NET_WM_WINDOW_TYPE_TOOLBAR"},
    {Lv2cWindowType::Menu, "_NET_WM_WINDOW_TYPE_MENU"},
    {Lv2cWindowType::Utility, "_NET_WM_WINDOW_TYPE_UTILITY"},
    {Lv2cWindowType::Splash, "_NET_WM_WINDOW_TYPE_SPLASH"},
    {Lv2cWindowType::Dialog, "_NET_WM_WINDOW_TYPE_DIALOG"},
    {Lv2cWindowType::ModelessDialog, "_NET_WM_WINDOW_TYPE_DIALOG"},
    {Lv2cWindowType::DropdownMenu, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU"},
    {Lv2cWindowType::PopupMenu, "_NET_WM_WINDOW_TYPE_POPUP_MENU"},
    {Lv2cWindowType::Tooltip, "_NET_WM_WINDOW_TYPE_TOOLTIP"},
    {Lv2cWindowType::Notification, "_NET_WM_WINDOW_TYPE_NOTIFICATION"},
    {Lv2cWindowType::Combo, "_NET_WM_WINDOW_TYPE_COMBO"},
    {Lv2cWindowType::DragAndDrop, "_NET_WM_WINDOW_TYPE_DND"},
    {Lv2cWindowType::Normal, "_NET_WM_WINDOW_TYPE_NORMAL"},
};

static const char *Lv2cToXWindowType(Lv2cWindowType windowType)
{
    for (size_t i = 0; i < sizeof(gLv2cToXWindowType) / sizeof(gLv2cToXWindowType[0]); ++i)
    {
        if (gLv2cToXWindowType[i].windowType == windowType)
        {
            return gLv2cToXWindowType[i].wmxWindowType;
        }
    }
    throw std::runtime_error("Invalid value.");
}
void Lv2cX11Window::SetWindowType(Lv2cWindowType windowType)
{
    this->windowType = windowType;
    const char *wmWindowType = Lv2cToXWindowType(windowType);
    auto atomWindowType = XInternAtom(x11Display, wmWindowType, false);
    XChangeProperty(x11Display, x11Window,
                    XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE", False),
                    XA_ATOM,
                    32, PropModeReplace, (unsigned char *)&atomWindowType, 1);
}
void Lv2cX11Window::WindowTitle(const std::string &title)
{
    windowTitle = title;
    if (x11Window != 0)
    {
        if (windowTitle.size() != 0)
        {
            XChangeProperty(x11Display, x11Window,
                            XInternAtom(x11Display, "_NET_WM_NAME", False),
                            XInternAtom(x11Display, "UTF8_STRING", False),
                            8, PropModeReplace, (unsigned char *)windowTitle.c_str(), windowTitle.size());
        }
        else
        {
            XChangeProperty(x11Display, x11Window,
                            XInternAtom(x11Display, "_NET_WM_NAME", False),
                            XInternAtom(x11Display, "UTF8_STRING", False),
                            8, PropModeReplace, (unsigned char *)res_name.c_str(), res_name.size());
        }
    }
}

Lv2cX11Window::Lv2cX11Window(
    Lv2cWindow::ptr window,
    Lv2cX11Window *parentNativeWindow,
    Lv2cCreateWindowParameters &parameters)
    : cairoWindow(window)
{

    SetErrorHandler();
    CreateWindow(
        parentNativeWindow->x11Window,
        parentNativeWindow->x11Display,
        parameters);
    parentNativeWindow->childWindows.push_back(this);

    parent = parentNativeWindow;

    CreateSurface(size.Width(), size.Height());
    ReleaseErrorHandler();
}

Lv2cX11Window::Lv2cX11Window(
    Lv2cWindow::ptr window,
    WindowHandle hWindow,
    Lv2cCreateWindowParameters &parameters)
    : cairoWindow(window)
{
    SetErrorHandler();
    Window parentWindow = (Window)hWindow.getHandle();
    CreateWindow(
        parentWindow,
        nullptr,
        parameters);

    if (parameters.owner)
    {
        parameters.owner->nativeWindow->childWindows.push_back(this);
        this->parent = parameters.owner->nativeWindow;
    }
    CreateSurface(size.Width(), size.Height());
    Sync();
    ReleaseErrorHandler();
}

Lv2cX11Window::Lv2cX11Window(
    Lv2cWindow::ptr window,
    Lv2cCreateWindowParameters &parameters)
    : Lv2cX11Window(window, WindowHandle(), parameters)
{
}

std::string Lv2cX11Window::GetX11ErrorText(int code)
{
    char buffer[1024];
    XGetErrorText(x11Display, code, buffer, sizeof(buffer));
    return buffer;
}

void Lv2cX11Window::DestroyWindowAndSurface()
{
    if (cairoSurface)
    {
        cairo_surface_destroy(cairoSurface);
        cairoSurface = nullptr;
    }

    if (x11Window)
    {
        XDestroyWindow(x11Display, x11Window);

        x11Window = 0;
        x11ParentWindow = 0;
        x11RootWindow = 0;
    }
    if (this->parent == nullptr)
    {
        // if (xInputController)
        // {
        //     XDestroyIC(xInputController);
        //     xInputController = nullptr;
        // }

        // if (xim)
        // {
        //     XCloseIM(xim);
        //     xim = 0;
        // }

        if (x11Display)
        {
            XCloseDisplay(x11Display);
            x11Display = nullptr;
            x11Window = 0;
        }
        if (this->xClassHint)
        {
            XFree((XClassHint *)this->xClassHint);
            this->xClassHint = nullptr;
        }
    }
    if (sizeHints != nullptr)
    {
        XFree(sizeHints);
        sizeHints = nullptr;
    }
    if (this->cairoWindow)
    {
        auto t = this->cairoWindow;
        cairoWindow = nullptr;
        t->OnX11WindowClosed();
    }
}
static ModifierState makeModifierState(unsigned int state)
{
    ModifierState result = ModifierState::Empty;
    if (state & ShiftMask)
    {
        result += ModifierState::Shift;
    }
    if (state & ControlMask)
    {
        result += ModifierState::Control;
    }
    if (state & Mod1Mask)
    {
        result += ModifierState::Alt;
    }
    if (state & Mod4Mask)
    {
        result += ModifierState::Super;
    }

    return result;
}

static std::string ToX11Color(const Lv2cColor &color)
{
    std::stringstream s;
    return color.ToWebString();

    // s << "rgbi:" << color.R() << "/" << color.G() << '/' << color.B();
    // return s.str();
}

Window Lv2cX11Window::GetTransientTarget(Window win)
{
    Window transientFor = 0;

    while (true)
    {
        auto status = XGetTransientForHint(x11Display, win, &transientFor);
        if (status != 0)
        {

            if (transientFor != 0)
            {
                return transientFor;
            }
        }
        Window root, parent, *children = nullptr;
        unsigned int num_children;

        XQueryTree(x11Display, win, &root, &parent, &children, &num_children);
        XFree(children);
        if (parent != 0)
        {
            win = parent;
        }
        else
        {
            break;
        }
    }
    return 0;
}
Window Lv2cX11Window::GetOwnerFrameWindow(Display *x11Display, Window win)
{
    Window transientFor = 0;

    std::vector<Window> topLevelWindows;

    bool hasTopLevelWindows = GetTopLevelWindows(topLevelWindows);
    while (true)
    {
        if (hasTopLevelWindows)
        {
            for (auto topLevelWindow : topLevelWindows)
            {
                if (topLevelWindow == win)
                {
                    return win;
                }
            }
        }
        // backstop in case the WM doesn't support GetTopLevelWindows.
        // Not entirely correct, since the host app may not use TransientFor.

        auto status = XGetTransientForHint(x11Display, win, &transientFor);
        if (status != 0)
        {

            if (transientFor != 0)
            {
                return win;
            }
        }
        Window root, parent, *children = nullptr;
        unsigned int num_children;

        XQueryTree(x11Display, win, &root, &parent, &children, &num_children);
        XFree(children);
        if (parent != 0)
        {
            win = parent;
        }
        else
        {
            break;
        }
    }
    return x11RootWindow;
}

void Lv2cX11Window::CreateWindow(
    Window parentWindow,
    Display *display,
    Lv2cCreateWindowParameters &parameters)
{
    if (display)
    {
        x11Display = display;
    }
    else if (parameters.owner != nullptr)
    {
        x11Display = parameters.owner->nativeWindow->x11Display;
    }
    else
    {
        if ((x11Display = XOpenDisplay(NULL)) == NULL)
        {
            std::runtime_error("Can't open X11 display");
        }
        xim = XOpenIM(x11Display, 0, 0, 0);
        xInputController = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, NULL);
        if (xInputController == nullptr)
        {
            std::runtime_error("Can't create X11 input context.");
        }

        LOG_TRACE(0, "Created x11Display");
    }

    this->xAtoms = std::make_unique<XAtoms>(x11Display);

    this->x11RootWindow = DefaultRootWindow(x11Display);

    this->x11LogicalParentWindow = parentWindow;

    if (parameters.positioning != Lv2cWindowPositioning::ChildWindow)
    {
        parentWindow = this->x11RootWindow;
        this->x11ParentWindow = parentWindow;

        if (parameters.owner)
        {
            this->x11LogicalParentWindow = GetOwnerFrameWindow(
                x11Display,
                (Window)(parameters.owner->nativeWindow->Handle().getHandle()));

            // this->x11LogicalParentWindow = GetTransientTarget(
            //     (Window)(parameters.owner->nativeWindow->Handle().getHandle()));
            if (!this->x11LogicalParentWindow)
            {
                this->x11LogicalParentWindow = this->x11RootWindow;
            }

            // this->x11LogicalParentWindow = GetTransientFor(parameters.owner->nativeWindow->Handle());
            //  this->x11LogicalParentWindow = (Window)(parameters.owner->Handle().getHandle());

            parentWindow = this->x11ParentWindow = this->x11RootWindow; // this->x11LogicalParentWindow;
            // parentWindow = this->x11ParentWindow = this->x11LogicalParentWindow;
        }
        // normal window path.
    }
    else
    {
        // plugin UI window.
        this->x11ParentWindow = parentWindow;
        if (parentWindow == 0)
        {
            // plugin UI window, test case.
            this->x11ParentWindow = x11RootWindow;
            parentWindow = x11RootWindow;
        }
    }

    XSizeHints *sizeHints = (XSizeHints *)GenerateNormalHints(parameters);

    XColor color;
    memset(&color, 0, sizeof(color));
    Colormap colormap = 0;
    (void)colormap;

    std::string backgroundColor = ToX11Color(parameters.backgroundColor);

    unsigned long backgroundPixel = BlackPixel(x11Display, DefaultScreen(x11Display));
    unsigned long borderPixel = BlackPixel(x11Display, DefaultScreen(x11Display));
    {
        colormap = DefaultColormap(x11Display, 0);
        XParseColor(x11Display, colormap, backgroundColor.c_str(), &color);
        XAllocColor(x11Display, colormap, &color);
        backgroundPixel = color.pixel;
    }

    this->x11Window = XCreateSimpleWindow(
        x11Display, parentWindow,
        sizeHints->x, sizeHints->y,
        sizeHints->base_width, sizeHints->base_height,
        0, backgroundPixel, borderPixel);
    auto event_mask =
        ExposureMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask | PointerMotionMask | EnterWindowMask |
        LeaveWindowMask | KeymapStateMask |
        ButtonPressMask | ButtonMotionMask | ButtonReleaseMask |
        FocusChangeMask | StructureNotifyMask | PropertyChangeMask;
    ;

    XSelectInput(x11Display, x11Window, event_mask);

    if (x11LogicalParentWindow != parentWindow || windowType == Lv2cWindowType::Dialog)
    {
        XSetTransientForHint(x11Display, x11Window, x11LogicalParentWindow);
    }
    SetNormalHints(sizeHints);

    if (parameters.positioning != Lv2cWindowPositioning::ChildWindow)
    {

        wmDeleteWindow = XInternAtom(x11Display, "WM_DELETE_WINDOW", False);
        wmProtocols = XInternAtom(x11Display, "WM_PROTOCOLS", False);
        XSetWMProtocols(x11Display, x11Window, &wmDeleteWindow, 1);

        SetStringProperty("_GTK_APPLICATION_ID", parameters.gtkApplicationId);

        XClassHint *classHint = XAllocClassHint();
        xClassHint = classHint;

        this->res_class = parameters.x11Windowclass;
        classHint->res_class = const_cast<char *>(res_class.c_str());
        this->res_name = parameters.x11WindowName;

        classHint->res_name = const_cast<char *>(this->res_name.c_str());

        XSetClassHint(x11Display, x11Window, classHint);

        // GetSynchronousFrameExtents();
    }

    SetWindowType(parameters.windowType);
    WindowTitle(parameters.title);

    XMapWindow(x11Display, x11Window);

    // XRaiseWindow(x11Display, x11Window);
    XClearWindow(x11Display, x11Window);

    this->size = parameters.size;
    this->location = Lv2cPoint((double)(sizeHints->x), (double)(sizeHints->y));

    cairoWindow->OnX11SizeChanged(this->size);

    RegisterControllerMessages();
    Sync();
}

void Lv2cX11Window::CreateSurface(int w, int h)
{
    int screen = DefaultScreen(x11Display);
    cairoSurface = cairo_xlib_surface_create(x11Display, x11Window, DefaultVisual(x11Display, screen), 0, 0);
    if (cairoSurface == nullptr)
    {
        throw std::runtime_error("Failed to create cairo surface.");
    }
    cairo_xlib_surface_set_size(cairoSurface, w, h);

    // create  a PangoContext.
    cairo_t *cr = cairo_create(cairoSurface);
    {
        this->pangoContext = pango_cairo_create_context(cr);
        // pango_context_set_round_glyph_positions(this->pangoContext,false);
    }
    cairo_destroy(cr);
}
void Lv2cX11Window::SurfaceResize(Lv2cSize size)
{
    if (!cairoSurface)
        return;

    cairo_surface_destroy(cairoSurface);
    cairoSurface = nullptr;

    int screen = DefaultScreen(x11Display);

    cairoSurface = cairo_xlib_surface_create(x11Display, x11Window, DefaultVisual(x11Display, screen), 0, 0);
    if (cairoSurface == nullptr)
    {
        throw std::runtime_error("Failed to create cairo surface.");
    }
    cairo_xlib_surface_set_size(cairoSurface, size.Width(), size.Height());
}

void Lv2cX11Window::OnIdle()
{
    if (this->cairoWindow)
    {
        this->cairoWindow->Idle();
    }
    for (auto child : childWindows)
    {
        child->OnIdle();
    }
}

bool Lv2cX11Window::AnimationLoop()
{
    using namespace std::chrono;
    fd_set in_fds;
    struct timeval tv;

    // Main loop
    while (1)
    {
        clock_t::time_point now = clock_t::now();

        clock_t::duration timeToNextAnimation = (lastAnimationFrameTime + ANIMATION_DELAY) - now;
        auto microseconds = duration_cast<std::chrono::microseconds>(timeToNextAnimation).count();
        if (microseconds > 0)
        {

            // Create a File Description Set containing x11_fd
            FD_ZERO(&in_fds);
            int maxFd = 0;
            AddFileDescriptors(maxFd, in_fds);

            tv.tv_usec = microseconds % 1000000;
            tv.tv_sec = microseconds / 1000000;

            // Wait for X Event or a Timer
            int num_ready_fds = select(maxFd, &in_fds, NULL, NULL, &tv);
            if (num_ready_fds < 0)
            {
                throw std::runtime_error("Animation loop select failed.");
            }
        }

        ProcessEvents();
        if (quitting)
        {
            return true;
        }
    }
}

void Lv2cX11Window::Animate()
{

    using namespace std::chrono;
    auto now = clock_t::now();

    auto t = this->childWindows;
    for (auto child : t)
    {
        child->Animate();
    }
    if (cairoWindow)
    {
        cairoWindow->Animate();
    }
    lastAnimationFrameTime = now;
}

void Lv2cX11Window::DeleteAllChildren()
{
    auto t = childWindows;
    childWindows.resize(0);
    for (size_t i = 0; i < t.size(); ++i)
    {
        delete t[i];
    }
}
bool Lv2cX11Window::DeleteDeadChildren()
{
    bool deleted = false;
    for (size_t i = 0; i < childWindows.size(); ++i)
    {
        Lv2cX11Window *childWindow = childWindows[i];

        deleted |= childWindow->DeleteDeadChildren();

        if (childWindow->Quitting())
        {
            childWindow->DeleteAllChildren();
            childWindows.erase(childWindows.begin() + i);
            --i;
            delete childWindow;
            deleted = true;
        }
    }
    return deleted;
}

bool Lv2cX11Window::ProcessEvents()
{

    XEvent xEvent;

    bool processedAnyMessage = false;
    for (;;)
    {

        if (DeleteDeadChildren())
        {
            processedAnyMessage = true;
        }

        Display *display = this->x11Display;

        bool pendingEvent = XPending(display);
        if (!pendingEvent)
        {
            CheckForRestoreFocus();
            Animate();
            OnIdle();
            XFlush(x11Display);
            return processedAnyMessage;
        }
        else
        {
            XNextEvent(display, &xEvent);

            ProcessEvent(xEvent);
            processedAnyMessage = true;
        }
    }
}

void Lv2cX11Window::ProcessEvent(XEvent &xEvent)
{
    switch (xEvent.type)
    {
    case ButtonPress:
    {
        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xbutton.window);

        if (window && !window->ModalDisable())
        {
            if (xEvent.xbutton.button >= 4 && xEvent.xbutton.button <= 7)
            {
                window->MouseScrollWheel(
                    WindowHandle(xEvent.xbutton.window),
                    (Lv2cScrollDirection)(xEvent.xbutton.button - 4),
                    xEvent.xbutton.x,
                    xEvent.xbutton.y,
                    makeModifierState(xEvent.xbutton.state));
            }
            else
            {
                window->MouseDown(
                    WindowHandle(xEvent.xbutton.window),
                    xEvent.xbutton.button,
                    xEvent.xbutton.x,
                    xEvent.xbutton.y,
                    makeModifierState(xEvent.xbutton.state));
            }
        }
        break;
    }
    case ButtonRelease:
    {
        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xbutton.window);
        if (window)
        {
            if (!(xEvent.xbutton.button >= 4 && xEvent.xbutton.button <= 7)) // ignore scrollwheel up events
            {
                window->MouseUp(
                    WindowHandle(xEvent.xbutton.window),
                    xEvent.xbutton.button,
                    xEvent.xbutton.x,
                    xEvent.xbutton.y,
                    makeModifierState(xEvent.xbutton.state));
            }
        }
        break;
    }
    case MotionNotify:
    {

        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xmotion.window);

        if (window && !window->ModalDisable())
        {
            window->MouseMove(
                WindowHandle(xEvent.xmotion.window),
                xEvent.xmotion.x,
                xEvent.xmotion.y,
                makeModifierState(xEvent.xbutton.state));
        }
        break;
    }
    case VisibilityNotify:
    {
        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xvisibility.window);
        if (window)
        {
            if (xEvent.xvisibility.type != VisibilityFullyObscured)
            {
                window->Invalidate();
            }
            LOG_TRACE(xEvent.xvisibility.window, "VisibilityNotify");
        }
        break;
    }
    case ClientMessage:
        if (xEvent.xclient.message_type == wmProtocols)
        {
            if (xEvent.xclient.data.l[0] == (long int)wmDeleteWindow)
            {
                LOG_TRACE(xEvent.xclient.window, "wmDeleteWindow");
                EraseChild(xEvent.xclient.window);
            }
        }
        else if (xEvent.xclient.message_type == animateMessage)
        {
        }
        else if (xEvent.xclient.message_type == controlMessage)
        {
        }
        break;

    case Expose:
    {
        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xexpose.window);
        if (window)
        {
            window->OnExpose(
                WindowHandle(xEvent.xexpose.window),
                xEvent.xexpose.x,
                xEvent.xexpose.y,
                xEvent.xexpose.width,
                xEvent.xexpose.height);
            LOG_TRACE(xEvent.xexpose.window, "Expose");
        }
        break;
    }
    case FocusIn:
    {
        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xfocus.window);
        if (window)
        {
            window->FireFocusIn();
            if (window->ModalDisable())
            {
                StartRestoreFocusDelay();
            }
        }
        break;
    }
    case FocusOut:
    {
        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xfocus.window);
        if (window)
        {
            window->FireFocusOut();
        }
        break;
    }
    case ReparentNotify:
    {
        LOG_TRACE(xEvent.xreparent.window, "ReparentNotify");
        break;
    }
    case ConfigureNotify:
    {
        Lv2cX11Window *child = GetChild(xEvent.xconfigure.window);
        if (child)
        {

            Lv2cWindow::ptr window = child->cairoWindow;
            assert(window);

            child->location = Lv2cPoint((double)xEvent.xconfigure.x - this->frameExtents.left, (double)xEvent.xconfigure.y - this->frameExtents.top);

            Lv2cSize size{(double)xEvent.xconfigure.width, (double)xEvent.xconfigure.height};

            LOG_TRACE(xEvent.xconfigure.window, SS("ConfigureNotify ("
                                                   << xEvent.xconfigure.x << "," << xEvent.xconfigure.y
                                                   << "," << xEvent.xconfigure.width << "," << xEvent.xconfigure.height
                                                   << ") ("
                                                   << child->location.x << "," << child->location.y
                                                   << "," << size.Width() << "," << size.Height() << ")"));

            if (child->size != size)
            {
                child->size = size;
                cairo_xlib_surface_set_size(child->cairoSurface, size.Width(), size.Height());
            }
            child->FireConfigurationChanged();
        }
    }
    break;
    case CreateNotify:
    {
        LOG_TRACE(xEvent.xcreatewindow.window, "CreateNotify");
        break;
    }
    case DestroyNotify:
    {
        LOG_TRACE(xEvent.xdestroywindow.window, "DestroyNotify");
        EraseChild(xEvent.xdestroywindow.window);
        break;
    }
    case ConfigureRequest:
    {
        LOG_TRACE(xEvent.xconfigurerequest.window, SS("ConfigureNotify " << xEvent.xconfigurerequest.width << "," << xEvent.xconfigurerequest.height));
        break;
    }
    case MapNotify:
        LOG_TRACE(xEvent.xmap.window, "MapNotify");
        break;
    case KeymapNotify:
        LOG_TRACE(xEvent.xkeymap.window, "KeymapNotify");
        break;
    case EnterNotify:
        LOG_TRACE(xEvent.xcrossing.window, "EnterNotify");
        break;
    case LeaveNotify:
    {
        LOG_TRACE(xEvent.xcrossing.window, "LeaveNotify");
        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xcrossing.window);

        if (window)
        {
            window->MouseLeave(WindowHandle(xEvent.xcrossing.window));
        }
    }
    break;
    case ResizeRequest:
    {
        LOG_TRACE(xEvent.xresizerequest.window, SS("ResizeRequest " << xEvent.xresizerequest.width << "," << xEvent.xresizerequest.height));
        // LOG_TRACE(xEvent.xresizerequest.window, SS("cairo surface size:  " << cairo_xlib_surface_get_width(cairoSurface) << "," << cairo_xlib_surface_get_height(cairoSurface)));

        // this->size = Lv2cSize(xEvent.xresizerequest.width, xEvent.xresizerequest.height);
        // cairo_xlib_surface_set_size(cairoSurface, size.Width(), size.Height());
        // {
        //     XWindowAttributes attributes;
        //     XGetWindowAttributes(x11Display, x11Window, &attributes);
        //     LOG_TRACE(xEvent.xresizerequest.window, SS("X11 windows:  " << attributes.width << "," << attributes.height));
        // }
        break;
    }
    case KeyPress:
    {
        char keybuf[32];
        KeySym keySym;
        Status returnStatus;
        memset(keybuf, 0, sizeof(keybuf)); // DEL key doesn't zero-terminate in Raspberry Pi OS!

        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xkey.window);

        if (window && !window->ModalDisable())
        {
            Lv2cKeyboardEventArgs eventArgs;
            eventArgs.keysymValid = true;
            eventArgs.rawKeyCode = xEvent.xkey.keycode;
            eventArgs.modifierState = makeModifierState(xEvent.xkey.state);
            window->OnX11KeycodeDown(eventArgs);
        }

        int rc = Xutf8LookupString(xInputController, &xEvent.xkey, keybuf, sizeof(keybuf), &keySym, &returnStatus);
        if (rc < 0)
        {
            LogError(SS("Xutf8LookupString failed. (" << rc << ")"));
        }
        else
        {

            Lv2cKeyboardEventArgs eventArgs;
            eventArgs.rawKeyCode = xEvent.xkey.keycode;

            if (window && !window->ModalDisable())
            {
                switch (returnStatus)
                {
                case XBufferOverflow:
                    LogError("X11 Keyboard buffer overflow.");
                    break;
                case XLookupNone:
                    LOG_TRACE(xEvent.xkey.window, "Keyboard lookup none");
                    break;
                case XLookupChars:
                    keybuf[rc] = '\0'; // return is lenght of string.
                    strncpy(eventArgs.text, keybuf, sizeof(eventArgs.text));
                    eventArgs.textValid = true;
                    eventArgs.modifierState = makeModifierState(xEvent.xkey.state);
                    LOG_TRACE(xEvent.xkey.window, SS("Keyboard: " << keybuf << " " << toString(eventArgs.modifierState)));
                    window->OnKeyDown(eventArgs);
                    break;
                case XLookupKeySym:
                    eventArgs.textValid = false;
                    eventArgs.keysymValid = true;
                    eventArgs.modifierState = makeModifierState(xEvent.xkey.state);
                    eventArgs.keysym = keySym;
                    LOG_TRACE(xEvent.xkey.window, SS("Keyboard: sym: " << GetKeysymName(keySym) << " " << toString(eventArgs.modifierState)));
                    window->OnKeyDown(eventArgs);

                    break;
                case XLookupBoth:
                    keybuf[rc] = '\0'; // return is lenght of string.
                    strncpy(eventArgs.text, keybuf, sizeof(eventArgs.text));
                    eventArgs.textValid = true;
                    eventArgs.keysymValid = true;
                    eventArgs.modifierState = makeModifierState(xEvent.xkey.state);
                    eventArgs.keysym = keySym;
                    const char *kbdText = keybuf;
                    if (kbdText[0] < 0x20) // control codes confuse GCC streambuf.
                    {
                        kbdText = (const char *)u8"\uFFFD";
                    }
                    LOG_TRACE(xEvent.xkey.window, SS("Keyboard: " << kbdText << "  sym: " << GetKeysymName(keySym) << " " << toString(eventArgs.modifierState)));
                    window->OnKeyDown(eventArgs);
                    break;
                }
            }
        }
        // return key;
        break;
    }
    case KeyRelease:
    {
        Lv2cWindow::ptr window = GetLv2cWindow(xEvent.xkey.window);
        if (window && !window->ModalDisable())
        {
            Lv2cKeyboardEventArgs eventArgs;
            eventArgs.keysymValid = true;
            eventArgs.keysym = xEvent.xkey.keycode;
            eventArgs.modifierState = makeModifierState(xEvent.xkey.state);
            window->OnX11KeycodeUp(eventArgs);
            break;
        }
        break;
    }
    case PropertyNotify:
    {
        auto &e = xEvent.xproperty;
        Lv2cX11Window *child = this->GetChild(e.window);
        if (child)
        {
            if (xEvent.xproperty.state == PropertyNewValue)
            {
                LOG_TRACE(e.window, SS("PropertyNotify new value: " << XGetAtomName(x11Display, e.atom)));
                if (e.atom == xAtoms->NET_FRAME_EXTENTS)
                {
                    child->OnFrameExtentsUpdated();
                }
                else if (e.atom == GetAtom("_NET_WM_DESKTOP"))
                {
                }
                else if (e.atom == xAtoms->NET_WM_STATE)
                {
                    child->OnWmStateUpdated();
                }
            }
        }
        break;
    }
    default:
        // LOG_TRACE(0, SS("Dropping unhandled XEevent.type = " << xEvent.type));
        break;
    }
}

bool Lv2cX11Window::GrabPointer()
{
    int result = XGrabPointer(
        this->x11Display,
        this->x11Window,
        False,
        ButtonPressMask | ButtonMotionMask | ButtonReleaseMask,
        GrabModeAsync,
        GrabModeAsync,
        None, // DefaultRootWindow(x11Display), // None, this->x11Window,
        None,
        CurrentTime);
    return result == GrabSuccess;
}
void Lv2cX11Window::UngrabPointer()
{
    XUngrabPointer(this->x11Display, CurrentTime);
}

Atom Lv2cX11Window::GetAtom(const char *atomName)
{
    return XInternAtom(x11Display, atomName, False);
}
void Lv2cX11Window::SetProperty(
    const std::string &property,
    const std::string &type,
    const std::vector<uint8_t> &data)
{
    Atom propertyAtom = XInternAtom(x11Display, property.c_str(), False);
    Atom typeAtom = XInternAtom(x11Display, type.c_str(), False);
    XChangeProperty(
        x11Display,
        x11Window,
        propertyAtom,
        typeAtom,
        8,
        PropModeReplace,
        (unsigned char *)(data.data()),
        (int)data.size());
}
void Lv2cX11Window::SetProperty(
    const std::string &property,
    const std::string &type,
    const std::vector<uint32_t> &data)
{
    Atom propertyAtom = XInternAtom(x11Display, property.c_str(), False);
    Atom typeAtom = XInternAtom(x11Display, type.c_str(), False);
    XChangeProperty(
        x11Display,
        x11Window,
        propertyAtom,
        typeAtom,
        16,
        PropModeReplace,
        (unsigned char *)(data.data()),
        (int)data.size());
}

void Lv2cX11Window::SendAnimationFrameMessage()
{
    XClientMessageEvent xevent;
    memset(&xevent, 0, sizeof(xevent));
    xevent.type = ClientMessage;
    xevent.message_type = this->animateMessage;
    xevent.display = this->x11Display;
    xevent.window = this->x11Window;
    xevent.format = 32;
    XSendEvent(this->x11Display, this->x11Window, 0, 0, (XEvent *)&xevent);
}
void Lv2cX11Window::SendControlChangedMessage(int32_t control, float value)
{
    XClientMessageEvent xevent;
    memset(&xevent, 0, sizeof(xevent));

    xevent.type = ClientMessage;
    xevent.message_type = this->controlMessage;
    xevent.display = this->x11Display;
    xevent.window = this->x11Window;
    xevent.format = 32;
    xevent.data.l[0] = control;
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
    xevent.data.l[1] = *(int32_t *)&value;

    XSendEvent(this->x11Display, this->x11Window, 0, 0, (XEvent *)&xevent);
}

void Lv2cX11Window::RegisterControllerMessages()
{
    controlMessage = XInternAtom(x11Display, "ControlMmsg", True);
    animateMessage = XInternAtom(x11Display, "AnimateMsg", True);
}

WindowHandle Lv2cX11Window::Handle()
{
    return WindowHandle(this->x11Window);
}
void Lv2cX11Window::AddFileDescriptors(int &maxFd, fd_set &fdSet)
{
    auto x11_fd = ConnectionNumber(x11Display);
    FD_SET(x11_fd, &fdSet);
    if (x11_fd + 1 > maxFd)
    {
        maxFd = x11_fd + 1;
    }
    for (auto child : childWindows)
    {
        child->AddFileDescriptors(maxFd, fdSet);
    }
}

Lv2cWindow::ptr Lv2cX11Window::GetLv2cWindow(Window x11Window)
{
    if (x11Window == this->x11Window)
    {
        return this->cairoWindow;
    }
    for (auto child : childWindows)
    {
        Lv2cWindow::ptr result = child->GetLv2cWindow(x11Window);
        if (result)
        {
            return result;
        }
    }
    return nullptr;
}

Lv2cX11Window *Lv2cX11Window::GetChild(Window x11Window)
{
    if (this->x11Window == x11Window)
    {
        return this;
    }
    for (auto child : childWindows)
    {
        Lv2cX11Window *result = child->GetChild(x11Window);
        if (result)
        {
            return result;
        }
    }
    return nullptr;
}

void Lv2cX11Window::SetNormalHints(void *sizeHints)
{

    XSetWMNormalHints(x11Display, x11Window, (XSizeHints *)sizeHints);
}

void *Lv2cX11Window::GenerateNormalHints(Lv2cCreateWindowParameters &parameters_)
{
    Lv2cCreateWindowParameters parameters{parameters_};
    if (sizeHints != nullptr)
    {
        XFree(sizeHints);
    }
    XSizeHints *sizeHints = XAllocSizeHints();
    this->sizeHints = (void *)sizeHints;
    if (parameters.minSize.Width() == 0)
    {
        parameters.minSize.Width((double)parameters.size.Width());
    }
    if (parameters.minSize.Height() == 0)
    {
        parameters.minSize.Height((double)parameters.size.Height());
    }
    if (parameters.maxSize.Height() == 0)
    {
        parameters.maxSize.Height((double)parameters.size.Height());
    }
    if (parameters.maxSize.Width() == 0)
    {
        parameters.maxSize.Width((double)parameters.size.Width());
    }

    sizeHints->flags = (PPosition | PMinSize | PMaxSize | PBaseSize);

    sizeHints->x = (int)parameters.location.x;
    sizeHints->y = (int)parameters.location.y;
    sizeHints->width = (int)parameters.size.Width();
    sizeHints->height = (int)parameters.size.Height();
    sizeHints->min_width = (int)parameters.minSize.Width();
    sizeHints->min_height = (int)parameters.minSize.Height();
    sizeHints->max_width = (int)parameters.maxSize.Width();
    sizeHints->max_height = (int)parameters.maxSize.Height();
    sizeHints->base_width = (int)parameters.size.Width();
    sizeHints->base_height = (int)parameters.size.Height();

    ConvertPositioning(sizeHints, parameters.positioning);
    parameters_.positioning = this->configPositioning;
    parameters_.location = Lv2cPoint(sizeHints->x, sizeHints->y);
    return sizeHints;
}

void Lv2cX11Window::ConvertPositioning(void *sizeHints_, Lv2cWindowPositioning positioning)
{
    XSizeHints *sizeHints = (XSizeHints *)sizeHints_;

    switch (positioning)
    {
    case Lv2cWindowPositioning::ChildWindow:
        sizeHints->x = 0;
        sizeHints->y = 0;
        sizeHints->win_gravity = NorthWestGravity;
        break;
    case Lv2cWindowPositioning::CenterOnParent:
    {
        this->configPositioning = Lv2cWindowPositioning::RelativeToParent;
        XWindowAttributes parentAttributes;
        int x, y;
        Window child = 0;
        XTranslateCoordinates(x11Display, this->x11LogicalParentWindow, x11RootWindow, 0, 0, &x, &y, &child);
        XGetWindowAttributes(x11Display, this->x11LogicalParentWindow, &parentAttributes);
        sizeHints->x = x + (parentAttributes.width - sizeHints->base_width) / 2;
        sizeHints->y = y + (parentAttributes.width - sizeHints->base_width) / 2;
        sizeHints->win_gravity = NorthWestGravity;

        break;
    }

    case Lv2cWindowPositioning::RelativeToParent:
    {
        this->configPositioning = Lv2cWindowPositioning::RelativeToParent;
        int x, y;
        Window child = 0;
        XTranslateCoordinates(x11Display, this->x11LogicalParentWindow, x11RootWindow, 0, 0, &x, &y, &child);

        sizeHints->x += x;
        sizeHints->y += y;
        sizeHints->win_gravity = NorthWestGravity;
        break;
    }

    case Lv2cWindowPositioning::RelativeToDesktop:
        this->configPositioning = Lv2cWindowPositioning::RelativeToDesktop;
        break;

    case Lv2cWindowPositioning::NotSet:
    case Lv2cWindowPositioning::CenterOnDesktop:
    {
        this->configPositioning = Lv2cWindowPositioning::RelativeToDesktop;
        int event_base_return, error_base_return;
        if (XRRQueryExtension(x11Display, &event_base_return, &error_base_return))
        {
            XRRScreenConfiguration *xrrConfig;
            XRRScreenSize *xrrSizes;
            xrrConfig = XRRGetScreenInfo(x11Display, x11RootWindow);
            if (xrrConfig)
            {
                int numSizes;
                xrrSizes = XRRConfigSizes(xrrConfig, &numSizes);
                if (numSizes >= 1)
                {
                    sizeHints->x = (xrrSizes[0].width - sizeHints->base_width) / 2;
                    sizeHints->y = (xrrSizes[0].height - sizeHints->base_height) / 2;
                    break;
                }
            }
        }

        XWindowAttributes rootAttributes;
        XGetWindowAttributes(x11Display, this->x11RootWindow, &rootAttributes);
        sizeHints->x = (rootAttributes.width - sizeHints->base_width) / 2;
        sizeHints->y = (rootAttributes.height - sizeHints->base_height) / 2;
        sizeHints->win_gravity = NorthWestGravity;
        break;
    }
    }

    // // clamp the position to the desktop.
    // {
    //     XWindowAttributes rootAttributes;
    //     XGetWindowAttributes(x11Display, this->x11RootWindow, &rootAttributes);
    //     if (sizeHints->x + sizeHints->base_width > rootAttributes.width - 64)
    //     {
    //         sizeHints->x = rootAttributes.width - 64 - sizeHints->base_width;
    //     }
    //     if (sizeHints->x < 64)
    //     {
    //         sizeHints->x = 64;
    //     }
    //     if (sizeHints->y + sizeHints->base_height > rootAttributes.height - 64)
    //     {
    //         sizeHints->y = rootAttributes.height - 64 - sizeHints->base_width;
    //     }
    //     if (sizeHints->y < 64)
    //     {
    //         sizeHints->y = 64;
    //     }
    // }
}

void Lv2cX11Window::Close()
{
    if (this->x11Window)
    {
        auto parent = this;
        while (parent->parent)
        {
            parent = parent->parent;
        }
        parent->EraseChild(x11Window);
    }
}

bool Lv2cX11Window::EraseChild(Window x11Window)
{
    if (this->x11Window == x11Window && this->parent == nullptr)
    {
        this->quitting = true;
        this->x11Window = 0;
        return true;
    }
    for (size_t i = 0; i < childWindows.size(); ++i)
    {
        auto child = childWindows[i];
        if (child->x11Window == x11Window)
        {
            childWindows.erase(childWindows.begin() + i);
            delete child;
            return true;
        }
        else
        {
            if (child->EraseChild(x11Window))
            {
                return true;
            }
        }
    }
    return false;
}

void Lv2cX11Window::FireConfigurationChanged()
{
    Lv2cPoint location = this->location;
    if (this->configPositioning == Lv2cWindowPositioning::RelativeToParent)
    {
        int x, y;
        Window child;
        XTranslateCoordinates(
            x11Display,
            this->x11Window, x11LogicalParentWindow,
            0, 0, &x, &y, &child);
        location = Lv2cPoint(x, y) - Lv2cPoint(frameExtents.left, frameExtents.top);
    }
    else
    {
        int x, y;
        Window child;
        XTranslateCoordinates(
            x11Display,
            this->x11Window, x11RootWindow,
            0, 0, &x, &y, &child);
        location = Lv2cPoint(x, y) - Lv2cPoint(frameExtents.left, frameExtents.top);
    }
    if (cairoWindow)
    {
        cairoWindow->OnX11ConfigurationChanged(this->configPositioning, this->windowState, location, size);
    }
}

Lv2cX11Window *Lv2cX11Window::GetTopmostDialog()
{
    for (auto i = childWindows.rbegin(); i != childWindows.rend(); ++i)
    {
        Lv2cX11Window *result = (*i)->GetTopmostDialog();
        if (result)
        {
            return result;
        }
        if ((*i)->windowType == Lv2cWindowType::Dialog)
        {
            return (*i);
        }
    }
    return nullptr;
}

void Lv2cX11Window::CheckForRestoreFocus()
{
    if (delayedFocusRestore && clock_t::now() >= restoreFocusTime)
    {
        delayedFocusRestore = false;
        Lv2cX11Window *dialog = GetTopmostDialog();
        if (dialog)
        {
            XSetInputFocus(dialog->x11Display, dialog->x11Window, RevertToNone, CurrentTime);
        }
    }
}
void Lv2cX11Window::StartRestoreFocusDelay()
{
    using namespace std::chrono;
    delayedFocusRestore = true;
    restoreFocusTime = clock_t::now() + duration_cast<clock_t::duration>(250ms);
}

void Lv2cX11Window::OnFrameExtentsUpdated()
{
    std::vector<int32_t> extents;
    if (GetX11ArrayProperty(x11Display, x11Window, GetAtom("_NET_FRAME_EXTENTS"), &extents, 4, XA_CARDINAL))
    {
        if (extents.size() >= 4)
        {
            double left = extents[0];
            double right = extents[1];
            double top = extents[2];
            double bottom = extents[3];

            this->frameExtents = Lv2cThickness{left, top, right, bottom};
        }
    }
}

void Lv2cX11Window::OnWmStateUpdated()
{
    std::vector<Atom> atoms;
    if (GetAtomArrayProperty(
            x11Display, x11Window,
            xAtoms->NET_WM_STATE,
            &atoms))
    {
        {
            std::stringstream s;
            s << "_NET_WM_STATE:";
            for (Atom atom : atoms)
            {
                s << " " << XGetAtomName(x11Display, atom);
            }
            LOG_TRACE(x11Window, s.str());
        }
        bool maximizedHorz = std::find(atoms.begin(), atoms.end(), xAtoms->NET_WM_STATE_MAXIMIZED_HORZ) != atoms.end();
        bool maximizedVert = std::find(atoms.begin(), atoms.end(), xAtoms->NET_WM_STATE_MAXIMIZED_VERT) != atoms.end();
        bool hidden = std::find(atoms.begin(), atoms.end(), xAtoms->NET_WM_STATE_MAXIMIZED_VERT) != atoms.end();

        Lv2cWindowState windowState = Lv2cWindowState::Normal;
        if (maximizedHorz && maximizedVert)
        {
            windowState = Lv2cWindowState::Maximized;
        }
        if (hidden)
        {
            windowState = Lv2cWindowState::Minimized;
        }
        this->windowState = windowState;
    }
}

bool Lv2cX11Window::waitForX11Event(std::chrono::milliseconds ms)
{
    if (this->parent != nullptr)
    {
        return this->parent->waitForX11Event(ms);
    }
    using namespace std::chrono;

    auto microseconds = duration_cast<std::chrono::microseconds>(ms).count();
    if (microseconds < 0)
    {
        microseconds = 1;
    }

    fd_set in_fds;
    struct timeval tv;

    // Create a File Description Set containing x11_fd
    FD_ZERO(&in_fds);
    int maxFd = 0;
    AddFileDescriptors(maxFd, in_fds);

    tv.tv_usec = microseconds % 1000000;
    tv.tv_sec = microseconds / 1000000;

    // Wait for X Event or a Timer
    int num_ready_fds = select(maxFd, &in_fds, NULL, NULL, &tv);
    if (num_ready_fds <= 0)
    {
        return false;
    }
    return true;
}

void Lv2cX11Window::SetStringProperty(const std::string &key, const std::string &value)
{
    XTextProperty text;
    text.value = (unsigned char *)(void *)(value.c_str());
    text.encoding = XA_STRING;
    text.format = 8;
    text.nitems = value.length();

    XSetTextProperty(x11Display, x11Window, &text, GetAtom(key.c_str()));
}
std::optional<std::string> GetStringProperty(const std::string &key)
{
    throw std::runtime_error("Not implemented.");
}

void Lv2cX11Window::Resize(int width, int height)
{
    XResizeWindow(x11Display, x11Window, width, height);
}

bool Lv2cX11Window::GetTopLevelWindows(std::vector<Window> &result)
{
    if (!xAtoms->NET_CLIENT_LIST)
    {
        return false;
    }
    bool ret = GetX11ArrayProperty<Window>(
        x11Display,
        x11RootWindow,
        xAtoms->NET_CLIENT_LIST,
        &result);
    return ret;
}


void Lv2cX11Window::SetMouseCursor(Lv2cCursor cursor)
{
    // if (cursor == Lv2cCursor::Arrow)
    // {
    //     XUndefineCursor(x11Display, x11Window);
    // }
    // else
    {
        XID x11Cursor = XC_left_ptr;
        switch (cursor)
        {
        case Lv2cCursor::Arrow:
            x11Cursor = XC_left_ptr;
            break;
        case Lv2cCursor::Pointer:
            x11Cursor = XC_arrow;
            break;
        case Lv2cCursor::Hand:
            x11Cursor = XC_hand2;
            break;
        case Lv2cCursor::IBeam:
            x11Cursor = XC_xterm;
            break;
        case Lv2cCursor::Wait:
            x11Cursor = XC_watch;
            break;
        // case Lv2cCursor::SizeWE:
        //     x11Cursor = XC_sb_h_double_arrow;
        //     break;
        // case Lv2cCursor::SizeNS:
        //     x11Cursor = XC_sb_v_double_arrow;
        //     break;
        // case Lv2cCursor::SizeNWSE:
        //     x11Cursor = XC_bottom_right_corner;
        //     break;
        // case Lv2cCursor::SizeNESW:
        //     x11Cursor = XC_bottom_left_corner;
        //     break;
        }
        Cursor xidCursor;

        if (x11CursorMap.contains(x11Cursor)) {
            xidCursor = x11CursorMap[x11Cursor];
        } else {
            xidCursor = XCreateFontCursor(x11Display, x11Cursor);
            x11CursorMap[x11Cursor] = xidCursor;
        }
        if (xidCursor != lastCursor)
        {
            XDefineCursor(x11Display, x11Window, xidCursor);
            lastCursor = xidCursor;
        }
    }
}
