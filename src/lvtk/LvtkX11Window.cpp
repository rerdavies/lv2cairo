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

#include "LvtkX11Window.hpp"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <algorithm>

#include <cairo/cairo-xlib.h>
#include <iomanip>
#include <cassert>
#include <limits>

#include "ss.hpp"

using namespace lvtk;

static constexpr int ANIMATION_RATE = 60;
static constexpr std::chrono::steady_clock::duration ANIMATION_DELAY = std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::microseconds(1000000 / ANIMATION_RATE));


#define X_INIT_ATOM(name) \
    name = XInternAtom(display,"_" #name, False);

struct LvtkX11Window::XAtoms {
    XAtoms(Display *display)
    {
        X_INIT_ATOM(NET_FRAME_EXTENTS)
        X_INIT_ATOM(NET_WM_STATE_MAXIMIZED_VERT)
        X_INIT_ATOM(NET_WM_STATE_MAXIMIZED_HORZ)
        X_INIT_ATOM(NET_WM_STATE_FOCUSED)
        X_INIT_ATOM(NET_WM_STATE)
    }
    Atom NET_FRAME_EXTENTS,
        NET_WM_STATE_MAXIMIZED_VERT,
        NET_WM_STATE_MAXIMIZED_HORZ,
        NET_WM_STATE_FOCUSED,
        NET_WM_STATE;
};

template <typename T>
bool GetIntArrayProperty(Display *display, Window window,
                         Atom atomName,
                         std::vector<T> *result,
                         size_t nItems = 0,
                         Atom reqType = AnyPropertyType,
                         Atom *out_type = nullptr)
{
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "");

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
            ? std::numeric_limits<LengthType>::max() / 4
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
    if (format != sizeof(T) * 8)
    {
        XFree(data);
        return false;
    }
    if (out_type)
    {
        *out_type = atomType;
    }
    result->resize(nItemsOut);
    const long *typedData = (const long *)data; // why long? I don't know. But it is long.
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
inline void LvtkX11Window::SetProperty(
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

LvtkX11Window::~LvtkX11Window()
{
    DestroyWindowAndSurface();
}

PangoContext *LvtkX11Window::GetPangoContext()
{
    return pangoContext;
}

bool LvtkX11Window::PostQuit()
{
    this->quitting = true;
    return true;
}

bool LvtkX11Window::PostQuit(Window x11Window)
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

bool LvtkX11Window::Quitting() const
{
    return quitting;
}
void LvtkX11Window::TraceEvents(bool value)
{
    this->traceEvents = value;
}

struct LvtkToWmWindowType
{
    LvtkWindowType windowType;
    const char *wmxWindowType;
};

static LvtkToWmWindowType gLvtkToXWindowType[]{
    {LvtkWindowType::Desktop, "_NET_WM_WINDOW_TYPE_DESKTOP"},
    {LvtkWindowType::Dock, "_NET_WM_WINDOW_TYPE_DOCK"},
    {LvtkWindowType::Toolbar, "_NET_WM_WINDOW_TYPE_TOOLBAR"},
    {LvtkWindowType::Menu, "_NET_WM_WINDOW_TYPE_MENU"},
    {LvtkWindowType::Utility, "_NET_WM_WINDOW_TYPE_UTILITY"},
    {LvtkWindowType::Splash, "_NET_WM_WINDOW_TYPE_SPLASH"},
    {LvtkWindowType::Dialog, "_NET_WM_WINDOW_TYPE_DIALOG"},
    {LvtkWindowType::ModelessDialog, "_NET_WM_WINDOW_TYPE_DIALOG"},
    {LvtkWindowType::DropdownMenu, "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU"},
    {LvtkWindowType::PopupMenu, "_NET_WM_WINDOW_TYPE_POPUP_MENU"},
    {LvtkWindowType::Tooltip, "_NET_WM_WINDOW_TYPE_TOOLTIP"},
    {LvtkWindowType::Notification, "_NET_WM_WINDOW_TYPE_NOTIFICATION"},
    {LvtkWindowType::Combo, "_NET_WM_WINDOW_TYPE_COMBO"},
    {LvtkWindowType::DragAndDrop, "_NET_WM_WINDOW_TYPE_DND"},
    {LvtkWindowType::Normal, "_NET_WM_WINDOW_TYPE_NORMAL"},
};

static const char *LvtkToXWindowType(LvtkWindowType windowType)
{
    for (size_t i = 0; i < sizeof(gLvtkToXWindowType) / sizeof(gLvtkToXWindowType[0]); ++i)
    {
        if (gLvtkToXWindowType[i].windowType == windowType)
        {
            return gLvtkToXWindowType[i].wmxWindowType;
        }
    }
    throw std::runtime_error("Invalid value.");
}
void LvtkX11Window::SetWindowType(LvtkWindowType windowType)
{
    this->windowType = windowType;
    const char *wmWindowType = LvtkToXWindowType(windowType);
    auto atomWindowType = XInternAtom(x11Display, wmWindowType, false);
    XChangeProperty(x11Display, x11Window,
                    XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE", False),
                    XA_ATOM,
                    32, PropModeReplace, (unsigned char *)&atomWindowType, 1);
}
void LvtkX11Window::WindowTitle(const std::string &title)
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

LvtkX11Window::LvtkX11Window(
    LvtkWindow::ptr window,
    LvtkX11Window *parentNativeWindow,
    LvtkCreateWindowParameters &parameters)
    : cairoWindow(window)
{

    CreateWindow(
        parentNativeWindow->x11Window,
        parentNativeWindow->x11Display,
        parameters);
    parentNativeWindow->childWindows.push_back(this);

    parent = parentNativeWindow;

    CreateSurface(size.Width(), size.Height());
}

LvtkX11Window::LvtkX11Window(
    LvtkWindow::ptr window,
    WindowHandle hWindow,
    LvtkCreateWindowParameters &parameters)
    : cairoWindow(window)
{
    Window parentWindow = (Window)hWindow.getHandle();
    CreateWindow(
        parentWindow,
        nullptr,
        parameters);
    CreateSurface(size.Width(), size.Height());
}

LvtkX11Window::LvtkX11Window(
    LvtkWindow::ptr window,
    LvtkCreateWindowParameters &parameters)
    : LvtkX11Window(window, WindowHandle(), parameters)
{
}

void LvtkX11Window::DestroyWindowAndSurface()
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

static std::string ToX11Color(const LvtkColor &color)
{
    std::stringstream s;
    return color.ToWebString();

    // s << "rgbi:" << color.R() << "/" << color.G() << '/' << color.B();
    // return s.str();
}
void LvtkX11Window::CreateWindow(
    Window parentWindow,
    Display *display,
    LvtkCreateWindowParameters &parameters)
{

    if (display)
    {
        x11Display = display;
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
    }

    this->xAtoms = std::make_unique<XAtoms>(x11Display);

    this->x11RootWindow = DefaultRootWindow(x11Display);

    this->x11LogicalParentWindow = parentWindow;

    if (parameters.positioning != LvtkWindowPositioning::ChildWindow)
    {
        parentWindow = this->x11RootWindow;
        this->x11ParentWindow = parentWindow;
    } else {
        this->x11ParentWindow = parentWindow;
    }
    
    XSizeHints *sizeHints = (XSizeHints *)GenerateNormalHints(parameters);

    XSetWindowAttributes windowAttributes;
    memset(&windowAttributes, 0, sizeof(windowAttributes));
    if (parameters.windowType == LvtkWindowType::Dialog)
    {
        windowAttributes.do_not_propagate_mask = KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonMotionMask | ButtonReleaseMask;
    }

    XColor color;
    memset(&color, 0, sizeof(color));
    Colormap colormap = 0;
    std::string backgroundColor = ToX11Color(parameters.backgroundColor);

    unsigned long backgroundPixel = BlackPixel(x11Display, DefaultScreen(x11Display));
    {
        colormap = DefaultColormap(x11Display, 0);
        XParseColor(x11Display, colormap, backgroundColor.c_str(), &color);
        XAllocColor(x11Display, colormap, &color);
        backgroundPixel = color.pixel;
    }

    windowAttributes.override_redirect = false;
    windowAttributes.background_pixel = backgroundPixel;
    windowAttributes.event_mask =
        ExposureMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask | PointerMotionMask | EnterWindowMask |
        LeaveWindowMask | KeymapStateMask |
        ButtonPressMask | ButtonMotionMask | ButtonReleaseMask |
        FocusChangeMask | StructureNotifyMask | PropertyChangeMask;
    ;

    this->x11Window = XCreateWindow(
        x11Display,
        parentWindow,
        sizeHints->x, sizeHints->y,
        sizeHints->base_width, sizeHints->base_height,
        InputOutput,
        DefaultDepth(x11Display, DefaultScreen(x11Display)),
        CopyFromParent,
        DefaultVisual(x11Display, DefaultScreen(x11Display)),
        CWBackPixel | CWEventMask | CWDontPropagate,
        &windowAttributes);
        
    // auto event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask | PointerMotionMask | EnterWindowMask |
    //                   LeaveWindowMask | KeymapStateMask |
    //                   ButtonPressMask | ButtonMotionMask | ButtonReleaseMask |
    //                   FocusChangeMask | StructureNotifyMask | PropertyChangeMask;

    // XSelectInput(x11Display, x11Window, event_mask);

    if (x11LogicalParentWindow != parentWindow)
    {
        XSetTransientForHint(x11Display, x11Window, x11LogicalParentWindow);
    }
    if (parameters.windowType == LvtkWindowType::Dialog)
    {
        // Abolute clown car of an implementation on Gnome desktop.
        // We'll implemented it elsewhere.
    }

    SetNormalHints(sizeHints);

    if (parameters.positioning != LvtkWindowPositioning::ChildWindow)
    {
        wmDeleteWindow = XInternAtom(x11Display, "WM_DELETE_WINDOW", False);
        wmProtocols = XInternAtom(x11Display, "WM_PROTOCOLS", False);
        XSetWMProtocols(x11Display, x11Window, &wmDeleteWindow, 1);

        SetStringProperty("_GTK_APPLICATION_ID",parameters.gtkApplicationId);

        XClassHint *classHint = XAllocClassHint();
        xClassHint = classHint;
        
        this->res_class = parameters.x11Windowclass;
        classHint->res_class = const_cast<char*>(res_class.c_str());
        this->res_name = parameters.x11WindowName;

        classHint->res_name = const_cast<char*>(this->res_name.c_str());
        XSetClassHint(x11Display,x11Window,classHint);
        // GetSynchronousFrameExtents();
    }
    SetWindowType(parameters.windowType);
    WindowTitle(parameters.title);

    XMapWindow(x11Display, x11Window);
    XClearWindow(x11Display, x11Window);

    this->size = parameters.size;
    this->location = LvtkPoint((double)(sizeHints->x), (double)(sizeHints->y));

    cairoWindow->OnX11SizeChanged(this->size);

    RegisterControllerMessages();
}

void LvtkX11Window::CreateSurface(int w, int h)
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
void LvtkX11Window::SurfaceResize(LvtkSize size)
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

void LvtkX11Window::OnIdle()
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

bool LvtkX11Window::AnimationLoop()
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

void LvtkX11Window::Animate()
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

void LvtkX11Window::DeleteAllChildren()
{
    auto t = childWindows;
    childWindows.resize(0);
    for (size_t i = 0; i < t.size(); ++i)
    {
        delete t[i];
    }
}
bool LvtkX11Window::DeleteDeadChildren()
{
    bool deleted = false;
    for (size_t i = 0; i < childWindows.size(); ++i)
    {
        LvtkX11Window *childWindow = childWindows[i];

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

bool LvtkX11Window::ProcessEvents()
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

void LvtkX11Window::ProcessEvent(XEvent &xEvent)
{
    switch (xEvent.type)
    {
    case ButtonPress:
    {
        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xbutton.window);

        if (window && !window->ModalDisable())
        {
            window->MouseDown(
                WindowHandle(xEvent.xbutton.window),
                xEvent.xbutton.button,
                xEvent.xbutton.x,
                xEvent.xbutton.y,
                makeModifierState(xEvent.xbutton.state));
        }
        break;
    }
    case ButtonRelease:
    {
        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xbutton.window);
        if (window)
        {
            window->MouseUp(
                WindowHandle(xEvent.xbutton.window),
                xEvent.xbutton.button,
                xEvent.xbutton.x,
                xEvent.xbutton.y,
                makeModifierState(xEvent.xbutton.state));
        }
        break;
    }
    case MotionNotify:
    {


        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xmotion.window);

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
        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xvisibility.window);
        if (window)
        {
            if (xEvent.xvisibility.type != VisibilityFullyObscured)
            {
                window->Invalidate();
            }
            logDebug(xEvent.xvisibility.window, "VisibilityNotify");
        }
        break;
    }
    case ClientMessage:
        if (xEvent.xclient.message_type == wmProtocols)
        {
            if (xEvent.xclient.data.l[0] == (long int)wmDeleteWindow)
            {
                logDebug(xEvent.xclient.window, "wmDeleteWindow");
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
        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xexpose.window);
        if (window)
        {
            window->OnExpose(
                WindowHandle(xEvent.xexpose.window),
                xEvent.xexpose.x,
                xEvent.xexpose.y,
                xEvent.xexpose.width,
                xEvent.xexpose.height);
            logDebug(xEvent.xexpose.window, "Expose");
        }
        break;
    }
    case FocusIn:
    {
        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xfocus.window);
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
        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xfocus.window);
        if (window)
        {
            window->FireFocusOut();
        }
        break;
    }
    case ReparentNotify:
    {
        logDebug(xEvent.xreparent.window, "ReparentNotify");
        break;
    }
    case ConfigureNotify:
    {
        LvtkX11Window *child = GetChild(xEvent.xconfigure.window);
        if (child)
        {

            LvtkWindow::ptr window = child->cairoWindow;
            assert(window);

            child->location = LvtkPoint((double)xEvent.xconfigure.x-this->frameExtents.left, (double)xEvent.xconfigure.y-this->frameExtents.top);

            LvtkSize size{(double)xEvent.xconfigure.width, (double)xEvent.xconfigure.height};


            logDebug(xEvent.xconfigure.window, SS("ConfigureNotify (" 
                    << xEvent.xconfigure.x << "," << xEvent.xconfigure.y
                    << "," << xEvent.xconfigure.width << "," << xEvent.xconfigure.height
                    << ") ("
                    << child->location.x << "," << child->location.y
                    << "," <<  size.Width() << "," << size.Height() << ")"
                    ));



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
        logDebug(xEvent.xcreatewindow.window, "CreateNotify");
        break;
    }
    case DestroyNotify:
    {
        logDebug(xEvent.xdestroywindow.window, "DestroyNotify");
        EraseChild(xEvent.xdestroywindow.window);
        break;
    }
    case ConfigureRequest:
    {
        logDebug(xEvent.xconfigurerequest.window, SS("ConfigureNotify " << xEvent.xconfigurerequest.width << "," << xEvent.xconfigurerequest.height));
        break;
    }
    case MapNotify:
        logDebug(xEvent.xmap.window, "MapNotify");
        break;
    case KeymapNotify:
        logDebug(xEvent.xkeymap.window, "KeymapNotify");
        break;
    case EnterNotify:
        logDebug(xEvent.xcrossing.window, "EnterNotify");
        break;
    case LeaveNotify:
        logDebug(xEvent.xcrossing.window, "LeaveNotify");
        break;
    case ResizeRequest:
    {
        logDebug(xEvent.xresizerequest.window, SS("ResizeRequest " << xEvent.xresizerequest.width << "," << xEvent.xresizerequest.height));
        // logDebug(xEvent.xresizerequest.window, SS("cairo surface size:  " << cairo_xlib_surface_get_width(cairoSurface) << "," << cairo_xlib_surface_get_height(cairoSurface)));

        // this->size = LvtkSize(xEvent.xresizerequest.width, xEvent.xresizerequest.height);
        // cairo_xlib_surface_set_size(cairoSurface, size.Width(), size.Height());
        // {
        //     XWindowAttributes attributes;
        //     XGetWindowAttributes(x11Display, x11Window, &attributes);
        //     logDebug(xEvent.xresizerequest.window, SS("X11 windows:  " << attributes.width << "," << attributes.height));
        // }
        break;
    }
    case KeyPress:
    {
        char keybuf[32];
        KeySym keySym;
        Status returnStatus;
        memset(keybuf, 0, sizeof(keybuf)); // DEL key doesn't zero-terminate in Raspberry Pi OS!

        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xkey.window);

        if (window && !window->ModalDisable())
        {
            LvtkKeyboardEventArgs eventArgs;
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

            LvtkKeyboardEventArgs eventArgs;
            eventArgs.rawKeyCode = xEvent.xkey.keycode;

            if (window && !window->ModalDisable())
            {
                switch (returnStatus)
                {
                case XBufferOverflow:
                    LogError("X11 Keyboard buffer overflow.");
                    break;
                case XLookupNone:
                    logDebug(xEvent.xkey.window, "Keyboard lookup none");
                    break;
                case XLookupChars:
                    keybuf[rc] = '\0'; // return is lenght of string.
                    strncpy(eventArgs.text, keybuf, sizeof(eventArgs.text));
                    eventArgs.textValid = true;
                    eventArgs.modifierState = makeModifierState(xEvent.xkey.state);
                    logDebug(xEvent.xkey.window, SS("Keyboard: " << keybuf << " " << toString(eventArgs.modifierState)));
                    window->OnKeyDown(eventArgs);
                    break;
                case XLookupKeySym:
                    eventArgs.textValid = false;
                    eventArgs.keysymValid = true;
                    eventArgs.modifierState = makeModifierState(xEvent.xkey.state);
                    eventArgs.keysym = keySym;
                    logDebug(xEvent.xkey.window, SS("Keyboard: sym: " << GetKeysymName(keySym) << " " << toString(eventArgs.modifierState)));
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
                    logDebug(xEvent.xkey.window, SS("Keyboard: " << kbdText << "  sym: " << GetKeysymName(keySym) << " " << toString(eventArgs.modifierState)));
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
        LvtkWindow::ptr window = GetLvtkWindow(xEvent.xkey.window);
        if (window && !window->ModalDisable())
        {
            LvtkKeyboardEventArgs eventArgs;
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
        LvtkX11Window *child = this->GetChild(e.window);
        if (child)
        {
            if (xEvent.xproperty.state == PropertyNewValue)
            {
                logDebug(e.window,SS("PropertyNotify new value: " << XGetAtomName(x11Display, e.atom)));
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
        logDebug(0,SS("Dropping unhandled XEevent.type = " << xEvent.type));
        break;
    }
}

static constexpr bool ENABLE_EVENT_TRACING = false;

void LvtkX11Window::logDebug(Window x11Window, const std::string &message)
{
    if (ENABLE_EVENT_TRACING)
    {
        std::stringstream s;
        s << "(" << x11Window << "): " << message;
        lvtk::LogDebug(s.str());
    }
}

bool LvtkX11Window::GrabPointer()
{
    int result = XGrabPointer(
        this->x11Display,
        this->x11Window,
        False,//True, // xxx False
        ButtonPressMask | ButtonMotionMask | ButtonReleaseMask ,
        GrabModeAsync,
        GrabModeAsync,
        None, //DefaultRootWindow(x11Display), // None, this->x11Window,
        None,
        CurrentTime);
    return result == GrabSuccess;
}
void LvtkX11Window::UngrabPointer()
{
    XUngrabPointer(this->x11Display, CurrentTime);
}

Atom LvtkX11Window::GetAtom(const char *atomName)
{
    return XInternAtom(x11Display, atomName, False);
}
void LvtkX11Window::SetProperty(
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
void LvtkX11Window::SetProperty(
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

void LvtkX11Window::SendAnimationFrameMessage()
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
void LvtkX11Window::SendControlChangedMessage(int32_t control, float value)
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

void LvtkX11Window::RegisterControllerMessages()
{
    controlMessage = XInternAtom(x11Display, "ControlMmsg", False);
    animateMessage = XInternAtom(x11Display, "AnimateMsg", False);
}

WindowHandle LvtkX11Window::Handle()
{
    return WindowHandle(this->x11Window);
}
void LvtkX11Window::AddFileDescriptors(int &maxFd, fd_set &fdSet)
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

LvtkWindow::ptr LvtkX11Window::GetLvtkWindow(Window x11Window)
{
    if (x11Window == this->x11Window)
    {
        return this->cairoWindow;
    }
    for (auto child : childWindows)
    {
        LvtkWindow::ptr result = child->GetLvtkWindow(x11Window);
        if (result)
        {
            return result;
        }
    }
    return nullptr;
}

LvtkX11Window *LvtkX11Window::GetChild(Window x11Window)
{
    if (this->x11Window == x11Window)
    {
        return this;
    }
    for (auto child : childWindows)
    {
        LvtkX11Window *result = child->GetChild(x11Window);
        if (result)
        {
            return result;
        }
    }
    return nullptr;
}

void LvtkX11Window::SetNormalHints(void *sizeHints)
{

    XSetWMNormalHints(x11Display, x11Window, (XSizeHints *)sizeHints);
}

void *LvtkX11Window::GenerateNormalHints(LvtkCreateWindowParameters &parameters_)
{
    LvtkCreateWindowParameters parameters{parameters_};
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
    parameters_.location = LvtkPoint(sizeHints->x, sizeHints->y);
    return sizeHints;
}

void LvtkX11Window::ConvertPositioning(void *sizeHints_, LvtkWindowPositioning positioning)
{
    XSizeHints *sizeHints = (XSizeHints *)sizeHints_;

    switch (positioning)
    {
    case LvtkWindowPositioning::ChildWindow:
        sizeHints->x = 0;
        sizeHints->y = 0;
        sizeHints->win_gravity = NorthWestGravity;
        break;
    case LvtkWindowPositioning::CenterOnParent:
    {
        this->configPositioning = LvtkWindowPositioning::RelativeToParent;
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

    case LvtkWindowPositioning::RelativeToParent:
    {
        this->configPositioning = LvtkWindowPositioning::RelativeToParent;
        int x, y;
        Window child = 0;
        XTranslateCoordinates(x11Display, this->x11LogicalParentWindow, x11RootWindow, 0, 0, &x, &y, &child);

        sizeHints->x += x;
        sizeHints->y += y;
        sizeHints->win_gravity = NorthWestGravity;
        break;
    }

    case LvtkWindowPositioning::RelativeToDesktop:
        this->configPositioning = LvtkWindowPositioning::RelativeToDesktop;
        break;

    case LvtkWindowPositioning::NotSet:
    case LvtkWindowPositioning::CenterOnDesktop:
    {
        this->configPositioning = LvtkWindowPositioning::RelativeToDesktop;
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

void LvtkX11Window::Close()
{
    if (this->x11Window)
    {
        EraseChild(x11Window);
    }
}

bool LvtkX11Window::EraseChild(Window x11Window)
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
    if (this->parent != nullptr)
    {
        return parent->EraseChild(x11Window);
    }
    return false;
}

void LvtkX11Window::FireConfigurationChanged()
{
    LvtkPoint location = this->location;
    if (this->configPositioning == LvtkWindowPositioning::RelativeToParent)
    {
        int x, y;
        Window child;
        XTranslateCoordinates(
            x11Display,
            this->x11Window, x11LogicalParentWindow,
            0, 0, &x, &y, &child);
        location = LvtkPoint(x, y) - LvtkPoint(frameExtents.left,frameExtents.top);
    }
    else
    {
        int x, y;
        Window child;
        XTranslateCoordinates(
            x11Display,
            this->x11Window, x11RootWindow,
            0, 0, &x, &y, &child);
        location = LvtkPoint(x, y) - LvtkPoint(frameExtents.left,frameExtents.top);
    }
    if (cairoWindow)
    {
        cairoWindow->OnX11ConfigurationChanged(this->configPositioning, this->windowState, location, size);
    }
}

LvtkX11Window *LvtkX11Window::GetTopmostDialog()
{
    for (auto i = childWindows.rbegin(); i != childWindows.rend(); ++i)
    {
        LvtkX11Window *result = (*i)->GetTopmostDialog();
        if (result)
        {
            return result;
        }
        if ((*i)->windowType == LvtkWindowType::Dialog)
        {
            return (*i);
        }
    }
    return nullptr;
}

void LvtkX11Window::CheckForRestoreFocus()
{
    if (delayedFocusRestore && clock_t::now() >= restoreFocusTime)
    {
        delayedFocusRestore = false;
        LvtkX11Window *dialog = GetTopmostDialog();
        if (dialog)
        {
            XSetInputFocus(dialog->x11Display, dialog->x11Window, RevertToNone, CurrentTime);
        }
    }
}
void LvtkX11Window::StartRestoreFocusDelay()
{
    using namespace std::chrono;
    delayedFocusRestore = true;
    restoreFocusTime = clock_t::now() + duration_cast<clock_t::duration>(250ms);
}

void LvtkX11Window::OnFrameExtentsUpdated()
{
    std::vector<int32_t> extents;
    if (GetIntArrayProperty(x11Display, x11Window, GetAtom("_NET_FRAME_EXTENTS"), &extents, 4, XA_CARDINAL))
    {
        if (extents.size() >= 4)
        {
            double left = extents[0];
            double right = extents[1];
            double top = extents[2];
            double bottom = extents[3];

            this->frameExtents = LvtkThickness{left, top, right, bottom};
        }
    }
}

void LvtkX11Window::OnWmStateUpdated()
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
            for (Atom atom: atoms)
            {
                s << " " << XGetAtomName(x11Display,atom);
            }
            logDebug(x11Window,s.str());
        }
        bool maximizedHorz = std::find(atoms.begin(),atoms.end(),xAtoms->NET_WM_STATE_MAXIMIZED_HORZ) != atoms.end();
        bool maximizedVert = std::find(atoms.begin(),atoms.end(),xAtoms->NET_WM_STATE_MAXIMIZED_VERT) != atoms.end();
        bool hidden = std::find(atoms.begin(),atoms.end(),xAtoms->NET_WM_STATE_MAXIMIZED_VERT) != atoms.end();

        LvtkWindowState windowState = LvtkWindowState::Normal;
        if (maximizedHorz && maximizedVert)
        {
            windowState = LvtkWindowState::Maximized;
        }
        if (hidden)
        {
            windowState = LvtkWindowState::Minimized;
        }
        this->windowState = windowState;

    }
}

bool LvtkX11Window::waitForX11Event(std::chrono::milliseconds ms)
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

void LvtkX11Window::SetStringProperty(const std::string&key, const std::string&value)
{
    XTextProperty text;
    text.value = (unsigned char*)(void*)(value.c_str());
    text.encoding = XA_STRING;
    text.format = 8;
    text.nitems = value.length();

    XSetTextProperty(x11Display,x11Window,&text,GetAtom(key.c_str()));
}
std::optional<std::string> GetStringProperty(const std::string&key)
{
    throw std::runtime_error("Not implemented.");
}

void LvtkX11Window::Resize(int width, int height)
{
    XResizeWindow(x11Display,x11Window,width,height);
}



