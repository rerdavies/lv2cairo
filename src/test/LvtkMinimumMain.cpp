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

#include <stdexcept>
#include <iostream>
#include <memory.h>
#include <vector>
#include <filesystem>
#include "lv2c/Lv2cLog.hpp"
#include "ss.hpp"

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

using namespace std;
using namespace lv2c;

Window x11Window = 0;
Display *x11Display = nullptr;
Window x11RootWindow;
Atom wmDeleteWindow;
Atom wmProtocols;

XIM xim;
XIC xInputController;


template <typename T>
bool GetIntArrayProperty(Display *display, Window window,
                      Atom atomName,
                      std::vector<T> *result,
                      size_t nItems = 0,
                      Atom reqType = AnyPropertyType,
                      Atom *out_type = nullptr
                      )
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
    uint32_t tmp[4] {};
    unsigned char *data = (unsigned char*)tmp;
    unsigned long requestedLength =
        nItems == 0
            ? std::numeric_limits<LengthType>::max()/4
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
        return false;
    }
    if (out_type)
    {
        *out_type = atomType;
    }
    result->resize(nItemsOut);
    const T *typedData = (const T *)data;
    for (size_t i = 0; i < nItemsOut; ++i)
    {
        (*result)[i] = typedData[i];
    }
    return true;
}


void CreateWindow()
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

    x11RootWindow = DefaultRootWindow(x11Display);

    // XSizeHints *sizeHints = (XSizeHints *)GenerateNormalHints(parameters);

    XSetWindowAttributes windowAttributes;
    memset(&windowAttributes, 0, sizeof(windowAttributes));

    windowAttributes.override_redirect = false;
    windowAttributes.background_pixel = BlackPixel(x11Display, DefaultScreen(x11Display));
    windowAttributes.event_mask =
        ExposureMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask | PointerMotionMask | EnterWindowMask |
        LeaveWindowMask | KeymapStateMask |
        ButtonPressMask | ButtonMotionMask | ButtonReleaseMask |
        FocusChangeMask | StructureNotifyMask | PropertyChangeMask;
    ;


    auto fg = BlackPixel(x11Display, DefaultScreen(x11Display));
    auto bg = WhitePixel(x11Display, DefaultScreen(x11Display));
    x11Window = XCreateSimpleWindow(x11Display,x11RootWindow, 0, 0, 200, 200, 0, fg, bg);

    // x11Window = XCreateWindow(
    //     x11Display,
    //     x11RootWindow,
    //     300, 200,
    //     640, 480,
    //     InputOutput,
    //     DefaultDepth(x11Display, DefaultScreen(x11Display)),
    //     CopyFromParent,
    //     DefaultVisual(x11Display, DefaultScreen(x11Display)),
    //     CWBackPixel | CWEventMask | CWDontPropagate,
    //     &windowAttributes);
    auto event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask | PointerMotionMask | EnterWindowMask |
                      LeaveWindowMask | KeymapStateMask |
                      ButtonPressMask | ButtonMotionMask | ButtonReleaseMask |
                      FocusChangeMask | StructureNotifyMask | PropertyChangeMask;
    event_mask = ButtonPressMask | PropertyChangeMask;

    XSelectInput(x11Display, x11Window, event_mask);

    // SetNormalHints(sizeHints);

    wmDeleteWindow = XInternAtom(x11Display, "WM_DELETE_WINDOW", False);
    wmProtocols = XInternAtom(x11Display, "WM_PROTOCOLS", False);
    XSetWMProtocols(x11Display, x11Window, &wmDeleteWindow, 1);

    auto atomWindowType = XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE_NORMAL", false);
    XChangeProperty(x11Display, x11Window,
                    XInternAtom(x11Display, "_NET_WM_WINDOW_TYPE", False),
                    XA_ATOM,
                    32, PropModeReplace, (unsigned char *)&atomWindowType, 1);

    // GetSynchronousFrameExtents();

    XMapWindow(x11Display, x11Window);
    XClearWindow(x11Display, x11Window);
}

Atom GetAtom(const char *atomName)
{
    return XInternAtom(x11Display, atomName, False);
}

void OnIconGeometryUpdated()
{
    std::vector<int32_t> extents;

    if (GetIntArrayProperty(x11Display, x11Window, GetAtom("_NET_WM_ICON_GEOMETRY"), &extents,4,XA_CARDINAL))
    {
        if (extents.size() >= 4)
        {
            std::cout << "Icon geometry: " << extents[0] << "," << extents[1] << "," << extents[2] << "," << extents[3] << std::endl;
        }
    }
}


void OnFrameExtentsUpdated()
{
    std::vector<int32_t> extents;

    if (GetIntArrayProperty(x11Display, x11Window, GetAtom("_NET_FRAME_EXTENTS"), &extents,4,XA_CARDINAL))
    {
        if (extents.size() >= 4)
        {
            std::cout << "Frame extents: " << extents[0] << "," << extents[1] << "," << extents[2] << "," << extents[3] << std::endl;
        }
    }
}

bool ProcessEvent(XEvent &xEvent)
{
    switch (xEvent.type)
    {
    case ButtonPress:
    {
        OnFrameExtentsUpdated();
        break;
    }
    case ButtonRelease:
    {
        break;
    }
    case MotionNotify:
    {
        break;
    }
    case VisibilityNotify:
    {
        break;
    }
    case ClientMessage:
        if (xEvent.xclient.message_type == wmProtocols)
        {
            if (xEvent.xclient.data.l[0] == (long int)wmDeleteWindow)
            {
                return true;
            }
        }
        break;

    case Expose:
    {
        break;
    }
    case FocusIn:
    {
        break;
    }
    case FocusOut:
    {
        break;
    }
    case ReparentNotify:
    {
        break;
    }
    case ConfigureNotify:
    {
        cout << "ConfigureNotify: ("
             << xEvent.xconfigure.x
             << "," << xEvent.xconfigure.y
             << "," << xEvent.xconfigure.width
             << "," << xEvent.xconfigure.height
             << ")" << endl;
        break;
    }
    case CreateNotify:
    {
        break;
    }
    case DestroyNotify:
    {
        break;
    }
    case ConfigureRequest:
    {
        break;
    }
    case MapNotify:
        break;
    case KeymapNotify:
        break;
    case EnterNotify:
        break;
    case LeaveNotify:
        break;
    case ResizeRequest:
    {
        break;
    }
    case KeyPress:
    {
        char keybuf[32];
        KeySym keySym;
        Status returnStatus;
        memset(keybuf, 0, sizeof(keybuf)); // DEL key doesn't zero-terminate in Raspberry Pi OS!

        int rc = Xutf8LookupString(xInputController, &xEvent.xkey, keybuf, sizeof(keybuf), &keySym, &returnStatus);
        if (rc < 0)
        {
            LogError(SS("Xutf8LookupString failed. (" << rc << ")"));
        }
        else
        {
            switch (returnStatus)
            {
            case XBufferOverflow:
                LogError("X11 Keyboard buffer overflow.");
                break;
            case XLookupNone:
                break;
            case XLookupChars:
                keybuf[rc] = '\0'; // return is lenght of string.

                cout << "Key: " << keybuf << endl;
                break;
            case XLookupKeySym:
                break;
            case XLookupBoth:
            {
                keybuf[rc] = '\0'; // return is lenght of string.

                const char *txt = keybuf;

                if (keybuf[0] < 0x20) // control codes confuse GCC streambuf.
                {
                    txt = (const char *)u8"\uFFFD";
                }
                cout << "Key: " << txt << endl;
                break;
            }
            }
        }
    }
    case KeyRelease:
    {
        break;
    }
    case PropertyNotify:
    {
        auto &e = xEvent.xproperty;
        if (xEvent.xproperty.state == PropertyNewValue)
        {
            std::cout << "Property updated: " << XGetAtomName(x11Display, e.atom) << std::endl;

            if (e.atom == GetAtom("_NET_WM_ICON_GEOMETRY"))
            {
                OnIconGeometryUpdated();
            }
            else if (e.atom == GetAtom("_NET_FRAME_EXTENTS"))
            {
                OnFrameExtentsUpdated();
            }
            else if (e.atom == GetAtom("_NET_WM_DESKTOP"))
            {
            }
            else if (e.atom == GetAtom("_NET_WM_STATE"))
            {
                //child->OnWmStateUpdated();
            }
            else
            {
                //std::cout << "Property updated: " << XGetAtomName(x11Display, e.atom) << std::endl;
            }
        }
        break;
    }
    default:
        // logDebug(SS("Dropping unhandled XEevent.type = " << xEvent.type));
        break;
    }
    return false;
}

bool ProcessEvents(bool wait = true)
{

    XEvent xEvent;

    bool processedAnyMessage = false;
    for (;;)
    {

        Display *display = x11Display;

        bool pendingEvent = XPending(display);
        if (!pendingEvent && !wait)
        {
            XFlush(x11Display);
            return processedAnyMessage;
        }
        else
        {
            XNextEvent(display, &xEvent);

            bool quitting = ProcessEvent(xEvent);
            if (quitting)
                return true;
            processedAnyMessage = true;
        }
    }
}

int main(int argc, char **argv)
{
    CreateWindow();
    ProcessEvents();
}