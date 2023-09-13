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

#include <cairo/cairo.h>
#include <memory.h>
#include <string>
#include "pango/pango.h"
#include "pango/pangocairo.h"
#include "lvtk/LvtkLog.hpp"
#include "lvtk/LvtkWindow.hpp"
#include "keysym_names.hpp"

#include <stdlib.h>
#include <stdexcept>
#include <chrono>

// keep main x11 includes (and their toxic #define's) out of global namespace.
typedef struct _XIM *XIM;
typedef struct _XIC *XIC;
typedef struct _XDisplay Display;
typedef unsigned long XID;
typedef XID Window;
typedef XID Atom;
typedef union _XEvent XEvent;

// typedef struct fd_set;

namespace lvtk
{

    class LvtkX11Window
    {
        LvtkX11Window(const LvtkX11Window &) = delete;
        LvtkX11Window(LvtkX11Window &&) = delete;

    public:
        /// @brief Create a child of a Host window.
        /// @param window The Window associated with this native window.
        /// @param hParentWindow Host-supplied parent hWindow.
        /// @param parameters Window creation parameters.

        LvtkX11Window(
            LvtkWindow::ptr window,
            WindowHandle hParentWindow,
            LvtkCreateWindowParameters&parameters);

        /// @brief Create a top-level window.
        /// @param window The Window associated with this native window.
        /// @param parameters Window creation parameters.

        LvtkX11Window(
            LvtkWindow::ptr window,
            LvtkCreateWindowParameters&parameters);

        /// @brief Create a child of an existing Lvtk window.
        /// @param window The LvtkWindow to associate with this x11 window.
        /// @param parentNativeWindow The parent window.
        /// @param position Default screen position
        /// @param title Title
        /// @param className x11 class name.
        /// @param minSize Minimum size. If Width() or Height() are zero, then the actual value is set to the position width or height.
        /// @param maxSize Maximum size. If Width() or Height() are zero, then the actual value is set to the position width or height.
        LvtkX11Window(
            LvtkWindow::ptr window,
            LvtkX11Window *parentNativeWindow,
            LvtkCreateWindowParameters&parameters);

        ~LvtkX11Window();

        WindowHandle Handle();

        void Close();

        void WindowTitle(const std::string &title);
        void SetWindowType(LvtkWindowType windowType);

        template <typename T>
        void SetProperty(
            const std::string &property,
            const std::string &type,
            const std::vector<T> &data);

        void SetProperty(
            const std::string &property,
            const std::string &type,
            const std::vector<uint8_t> &data);

        void SetProperty(
            const std::string &property,
            const std::string &type,
            const std::vector<uint32_t> &data);

        void SetProperty(const std::string &property, const std::vector<uint8_t> &data);
        void SetProperty(const std::string &property, const std::vector<int16_t> &data);
        void SetProperty(const std::string &property, const std::vector<int32_t> &data);

        bool ProcessEvents();

        void ProcessEvent(XEvent &xEvent);

        bool PostQuit();
        bool PostQuit(Window x11Window);
        bool Quitting() const;

        // returns true if done.
        bool AnimationLoop();

        void TraceEvents(bool value);
        cairo_surface_t *GetSurface() { return cairoSurface; }

        PangoContext *GetPangoContext();

        LvtkSize Size() const { return size; }

        bool GrabPointer();
        void UngrabPointer();

        void SendAnimationFrameMessage();
        void SendControlChangedMessage(int32_t control, float value);

        void SetStringProperty(const std::string&key, const std::string&value);
        std::optional<std::string> GetStringProperty(const std::string&key);

        void Resize(int width, int height);

    private:
        struct XAtoms;

        std::unique_ptr<XAtoms> xAtoms;

        bool waitForX11Event(std::chrono::milliseconds ms);

        void OnFrameExtentsUpdated();
        void OnWmStateUpdated();

        Atom GetAtom(const char*name);
        
        bool EraseChild(Window x11Window);
        void logDebug(Window x11Window, const std::string &s);
        void ConvertPositioning(void*sizeHint,LvtkWindowPositioning positioning);
        LvtkWindow::ptr GetLvtkWindow(Window x11Window);
        LvtkX11Window *GetChild(Window x11Window);

        void CreateWindow(
            Window parentWindow, 
            Display*display,
            LvtkCreateWindowParameters&parameters);

        void CreateSurface(int w, int h);
        void SurfaceResize(LvtkSize size);
        void*GenerateNormalHints(LvtkCreateWindowParameters &parameters);
        void SetNormalHints(void*);

        void DestroyWindowAndSurface();
        void OnIdle();

        void RegisterControllerMessages();

    private:
        using clock_t = std::chrono::steady_clock;


        LvtkThickness frameExtents { 0,0,0,0};

        LvtkX11Window* GetTopmostDialog();
        void CheckForRestoreFocus();
        void StartRestoreFocusDelay();
        bool delayedFocusRestore = false;
        clock_t::time_point restoreFocusTime;

        bool DeleteDeadChildren();
        void DeleteAllChildren();

        void AddFileDescriptors(int &maxFd, fd_set &fd);
        void FireConfigurationChanged();

        void Animate();
        clock_t::time_point lastAnimationFrameTime;

        LvtkWindowType windowType = LvtkWindowType::Normal;
        Atom controlMessage = 0;
        Atom animateMessage = 0;

        PangoContext *pangoContext = nullptr;

        LvtkPoint location;
        LvtkSize size;
        void *sizeHints = nullptr;
        Atom wmProtocols = 0;
        Atom wmDeleteWindow;
        std::string res_name;
        std::string res_class;
        void *xClassHint;
        bool traceEvents = false;
        bool quitting = false;
        cairo_surface_t *cairoSurface = nullptr;
        Display *x11Display = nullptr;
        Window x11Window = 0;
        Window x11ParentWindow = 0;
        Window x11RootWindow = 0;
        Window x11LogicalParentWindow = 0;
        LvtkWindowPositioning configPositioning = LvtkWindowPositioning::RelativeToDesktop;

        XIM xim = 0;
        XIC xInputController = nullptr;

        LvtkWindow::ptr cairoWindow;
        std::string windowTitle;
        LvtkX11Window *parent = nullptr;
        std::vector<LvtkX11Window *> childWindows;

        LvtkWindowState windowState = LvtkWindowState::Withdrawn;
    };

    /////

} // namespace
