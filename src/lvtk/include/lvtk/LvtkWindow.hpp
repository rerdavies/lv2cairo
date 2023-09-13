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

#pragma once

#include <memory>

#include "LvtkTypes.hpp"
#include "LvtkObject.hpp"

#include "LvtkRootElement.hpp"
#include "JsonVariant.hpp"

#include "LvtkDamageList.hpp"
// #include "LvtkSvg.hpp"

#include <functional>
#include <filesystem>
#include <cstddef>
#include <map>
#include <chrono>
#include <mutex>

// forward declaration.
typedef struct _PangoContext PangoContext;

namespace lvtk
{
    class LvtkX11Window;
    class LvtkTheme;
    class LvtkSvg;
    class FocusNavigationSelector;


    using AnimationCallback = std::function<void(const animation_clock_time_point_t&  now)>;
    using DelayCallback = std::function<void()>;

    enum class LvtkWindowType
    {
        Desktop,
        Dock,
        Toolbar,
        Menu,
        Utility,
        Splash,
        Dialog,
        ModelessDialog,
        DropdownMenu,
        PopupMenu,
        Tooltip,
        Notification,
        Combo,
        DragAndDrop,
        Normal
    };

    enum class LvtkWindowPositioning
    {
        NotSet,
        RelativeToDesktop,
        CenterOnDesktop,
        RelativeToParent,
        CenterOnParent,
        ChildWindow
    };

    enum class LvtkWindowGravity
    {
        NoGravity,
        NorthWest,
        NorthEast,
        SouthWest,
        SouthEast
    };
    /// @brief Specifies parameters used to create windows.
    struct LvtkCreateWindowParameters
    {
        /// @brief Controls the default window position.
        LvtkWindowPositioning positioning = LvtkWindowPositioning::NotSet;
        /// @brief The default location of the window to use when first loaded.
        LvtkPoint location;
        /// @brief The default size of the window.
        /// If either Width() or Height() is set, then window's element will be measured to determine the size of the window. If
        /// the window position has been saved, then that size will be used instead.
        LvtkSize size;
        // @brief The intiial state of the window.
        // Either Normal or Maximized. The effect of setting state to Minimized is not defined.
        LvtkWindowState state = LvtkWindowState::Normal;
        /// @brief The title of the window.
        std::string title;
        /// @brief The X11 window class of the window.
        /// Used by X11 to locate resource files (of which we have none). Leave it at the default value unless you have a 
        /// compelling need to change it.
        std::string x11Windowclass = "com.twoplay.lvtk-plugin";

        /// @brief The GTK Application ID
        /// Used to locate icons and/or desktop files on KDE Plasma and Gnome Destkop. Use the default 
        /// value to get the default Lv2 Plugin icon (recommmended).  Since we aren't an application, the 
        /// primary purpose of this property is to allow KDE Plasma and Gnome Desktop to location the 
        /// dummy com.twoplay.lvtk-plugin.desktop file, which contains a reference to the LV2 Plugin
        /// icon.
        std::string gtkApplicationId = "com.twoplay.lvtk-plugin";


        /// @brief The X11 name of the  window. Used by Windowm Managers for labelling task buttons &c.
        std::string x11WindowName = "";


        LvtkWindowType windowType = LvtkWindowType::Normal;
        /// @brief The anchor position for popup windows (menus, dropdowns, context menus, &c).
        /// Specified in coordinates relative to the parent window.
        LvtkRectangle anchorPosition;
        /// @brief The corner which the window location is relative to.
        LvtkWindowGravity gravity = LvtkWindowGravity::NorthWest;
        /// @brief The default location of the window.
        /// Location is ignored if the positioning is CenterOnDesktop or CenterOnParent, and ignored if
        /// the window has a saved position.

        /// @brief If set, then the size and position of the window will be saved and restored.
        /// See the LvtkWindow::Settings
        std::string settingsKey;

        /// @brief The dictionary used to save and restore settings.
        /// Must be a json_variant::object(). The host is responsible for saving and restoring the object. Applies only to
        /// the top-most window. Child windows use the settings object of their parent window. See LvtkWindow::Settings().
        json_variant settingsObject;
        /// @brief Minimum size of the window.
        /// If not set, then the size of the window will be used as the minimum size.
        LvtkSize minSize;
        /// @brief Maximum size of the window.
        /// If not set, then the size of the window will be used as the minimum size.
        LvtkSize maxSize;

        /// @brief The background color of the native (X11) window.
        /// Usually, this should be the same as LvtkTheme::paper. If the property has default value,
        /// the theme color will be copied in automatically.
        LvtkColor backgroundColor = LvtkColor(0,0,0,0);
    public:
        /// @brief Load saved values from the settingsObject.
        void Load();
        /// @brief Save current values to the settingsObject.
        void Save();
    };

    class LvtkWindow : public LvtkObject,
                        public std::enable_shared_from_this<LvtkWindow>
    {
    public:
        using ptr = std::shared_ptr<LvtkWindow>;
        using super = LvtkObject;
        static ptr Create() { return std::make_shared<LvtkWindow>(); }

        LvtkWindow();
        virtual ~LvtkWindow();

        LvtkWindow(const LvtkWindow &other) = delete;
        LvtkWindow &operator==(const LvtkWindow &other) = delete;

        void Close();

        void CloseRootWindow();

        WindowHandle Handle() const;
        
        
        const LvtkTheme &Theme() const;
        LvtkWindow &Theme(LvtkTheme::ptr theme);
        LvtkTheme::ptr ThemePtr();

        LvtkWindow &WindowScale(double scale);
        double WindowScale() const;

        /// @brief Enable or disable event tracing.
        /// @param trace true= enable, false=disable
        void TraceEvents(bool trace);
        /// @brief Get the root element for this window.
        /// @return
        std::shared_ptr<LvtkRootElement> GetRootElement();

        /// @brief Invalidate the contents of the window
        /// Request an update of window contents. The window contents
        /// will be redrawn on the next OnIdle cycle.
        void Invalidate();

        /// @brief Selectively invalidate the contents of the window
        /// @param rectangle The area of the screen to invalidate, in screen coordinates.
        /// Request an update of window contents. The window contents
        /// will be redrawn on the next OnIdle cycle.
        void Invalidate(const LvtkRectangle &bounds);

        /// @brief Invalidate the layout of elements in the window
        /// Request an update of layout. The window contents
        /// will be layed out on the next OnIdle cycle.
        void InvalidateLayout();

        /// @brief Create child window.
        /// @param hParent The window handle of the parent window.
        /// @param parameters Parameters that control how the window is created.
        void CreateWindow(
            WindowHandle hParent,
            const LvtkCreateWindowParameters &parameters);

        /// @brief Create a top-level window.
        /// @param parameters Parameters that control how a window is created.
        void CreateWindow(const LvtkCreateWindowParameters &parameters);

        /// @brief Create a child of a LvtkWindow.
        /// @param parent The parent window.
        /// @param parameters
        void CreateChildWindow(
            LvtkWindow *parent,
            const LvtkCreateWindowParameters &parameters,
            LvtkElement::ptr element);

        void WindowTitle(const std::string &title);
        const std::string &WindowTitle() const;

        const LvtkRectangle &ClientRectangle() const;
        LvtkSize Size() const;

        void Resize(int width, int height);


        /// @brief Show a message-box dialog.
        /// @param dialogType dialog type: Info, Warning or Error.
        /// @param title Title of the dialog.
        /// @param text Message text.
        /// The LvtkMessageDialog class provides a customizable message box, with options such as a second button,
        /// changing the name of buttons, and a wide variety customizations.

        void MessageBox(LvtkMessageDialogType dialogType, const std::string &title, const std::string &text);


        /// @brief Set a string property on the native window.
        /// @param key Name of the property.
        /// @param value Value of the property.
        /// Window properties are OS-specific. On Linux, properties are X11 window properties. 
        /// There is no current plan for supporting properties on other platforms.
        void SetStringProperty(const std::string&key, const std::string&value);
        
        /// @brief Get a string property from the native window.
        /// @param key Name of the property.
        /// @return An option string value for the property. If the property is not set, the has_value() will be false.
        /// Window properties are OS-specific. On Linux, properties are X11 window properties. 
        /// There is no current plan for supporting properties on other platforms.
        std::optional<std::string> GetStringProperty(const std::string&key);
        
        void PostQuit();
        bool Quitting() const;
        bool PumpMessages(bool block);

        bool Capture(LvtkElement *element);
        LvtkElement *Capture();
        void releaseCapture(LvtkElement *element);
        LvtkPoint MousePosition() const;

        bool Focus(LvtkElement *element);
        LvtkElement *FocusedElement();
        const LvtkElement *FocusedElement() const;
        void releaseFocus(LvtkElement *element);

        PangoContext *GetPangoContext();

        AnimationHandle RequestAnimationCallback(const AnimationCallback &callback);
        AnimationHandle RequestAnimationCallback(AnimationCallback &&callback);
        bool CancelAnimationCallback(AnimationHandle handle);

        AnimationHandle PostDelayed(std::chrono::milliseconds delay, const DelayCallback &callback);
        AnimationHandle PostDelayed(std::chrono::milliseconds delay, DelayCallback &&callback);
        AnimationHandle PostDelayed(uint32_t milliseconds, const DelayCallback &callback);
        AnimationHandle PostDelayed(uint32_t milliseconds, DelayCallback &&callback);

        bool CancelPostDelayed(AnimationHandle handle);

        std::shared_ptr<LvtkSvg> GetSvgImage(const std::string &filename);
        LvtkSurface GetPngImage(const std::string &filename);
        static void SetResourceDirectories(const std::vector<std::filesystem::path> &paths);
        static std::filesystem::path findResourceFile(const std::filesystem::path &path);

        void SendAnimationFrameMessage();
        void SendControlChangedMessage(int32_t control, float value);

        const json_variant &Settings() const;
        LvtkWindow &Settings(const json_variant &settings);

        bool ModalDisable() const;

    protected:
        virtual void OnClosing();
        virtual void OnDraw(LvtkDrawingContext &dc);
        virtual void OnDrawOver(LvtkDrawingContext &dc);
        virtual bool OnMouseDown(LvtkMouseEventArgs &event);
        virtual bool OnMouseMove(LvtkMouseEventArgs &event);
        virtual bool OnMouseUp(LvtkMouseEventArgs &event);
        virtual void OnIdle();
        virtual void OnSizeChanged(const LvtkSize &size);

        void AddModalDisable();
        void RemoveModalDisable();

        int modalDisableCount = 0;
        static LvtkCreateWindowParameters Scale(const LvtkCreateWindowParameters &v, double windowScale);

        void OnX11ConfigurationChanged(LvtkWindowPositioning positioning, LvtkWindowState windowState, LvtkPoint location, LvtkSize size);
        void OnX11SizeChanged(LvtkSize size);

        json_variant settings;
        LvtkWindow::ptr SelfPointer();
        void OnX11WindowClosed();

        /// @brief Set the root element for this window.
        /// @param element

        void FireFocusIn();
        void FireFocusOut();
        void Draw();
        void Layout();

        void Animate();

    protected:
        virtual bool OnKeyDown(LvtkKeyboardEventArgs &eventArgs);
        virtual void OnLayoutComplete();

    private:
        LvtkDrawingContext CreateDrawingContext();
        void Idle();
        void Size(const LvtkSize &size);

        // Native Window callbask.
        void OnExpose(WindowHandle h, int64_t x, int64_t y, int64_t width, int64_t height);
        virtual void MouseDown(WindowHandle h, uint64_t button, int64_t x, int64_t y, ModifierState state);
        virtual void MouseMove(WindowHandle h, int64_t x, int64_t y, ModifierState state);
        virtual void MouseUp(WindowHandle h, uint64_t button, int64_t x, int64_t y, ModifierState state);

        /// @brief Notification of a raw X11 keysim down event.
        /// @param eventArgs Keyboard event arguments.
        /// @return True if the event was handled.
        /// Use KeyDown instead to received translated text input. You probably want KeyDown.
        ///
        /// Only the element that has keyboard capture receives this event.
        bool OnX11KeycodeDown(LvtkKeyboardEventArgs &eventArgs);
        /// @brief Notification of a raw X11 keysim up event.
        /// @param eventArgs Keyboard event arguments.
        /// @return True if the event was handled.
        /// Use KeyDown instead to received translated text input. You probably want KeyDown.
        ///
        /// Only the element that has keyboard capture receives this event.
        bool OnX11KeycodeUp(LvtkKeyboardEventArgs &eventArgs);
        bool HandleKeyboardNavigation(LvtkKeyboardEventArgs &eventArgs);

        void NavigateFocus(FocusNavigationSelector &selector);

    private:
        double windowScale = 1.0;
        LvtkRectangle lastFocusRectangle;

        LvtkElement *savedFocusElement = nullptr;
        LvtkMouseEventArgs lastMouseEventArgs;

        LvtkPoint mousePosition{-100000, -100000};

        LvtkElement *captureElement = nullptr;
        LvtkElement *focusElement = nullptr;
        LvtkSize size;
        std::string windowTitle;
        LvtkX11Window *nativeWindow = nullptr;
        LvtkCreateWindowParameters windowParameters;
        LvtkRectangle bounds;

        LvtkDamageList damageList;

        bool valid = false;
        bool layoutValid = false;

        std::shared_ptr<LvtkRootElement> rootElement;

        std::shared_ptr<LvtkTheme> theme;

        std::map<AnimationHandle, AnimationCallback> animationCallbacks;

        struct DelayRecord
        {
            animation_clock_t::time_point time;
            DelayCallback callback;
        };

        std::recursive_mutex delayCallbacksMutex;
        std::map<AnimationHandle, DelayRecord> delayCallbacks;

        static std::vector<std::filesystem::path> resourceDirectories;

        std::map<std::string, std::shared_ptr<LvtkSvg>> svgCache;
        std::map<std::string, LvtkSurface> pngCache;


    private:
        friend class LvtkX11Window;
        friend class LvtkDialog;
        friend class LvtkElement;
    };

    ///////////////////////////////////////////////////////////////
    inline const LvtkRectangle &LvtkWindow::ClientRectangle() const
    {
        return this->bounds;
    }
    inline LvtkSize LvtkWindow::Size() const
    {
        return this->bounds.Size();
    }
    inline AnimationHandle LvtkWindow::PostDelayed(uint32_t milliseconds, const DelayCallback &callback)
    {
        return PostDelayed(std::chrono::milliseconds(milliseconds), callback);
    }
    inline AnimationHandle LvtkWindow::PostDelayed(uint32_t milliseconds, DelayCallback &&callback)
    {
        return PostDelayed(std::chrono::milliseconds(milliseconds), callback);
    }

}