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

#pragma once

#include <memory>

#include "Lv2cTypes.hpp"
#include "Lv2cObject.hpp"

#include "Lv2cRootElement.hpp"
#include "JsonVariant.hpp"

#include "Lv2cDamageList.hpp"
// #include "Lv2cSvg.hpp"

#include <map>
#include <functional>
#include <filesystem>
#include <cstddef>
#include <map>
#include <chrono>
#include <mutex>

// forward declaration.
typedef struct _PangoContext PangoContext;

namespace lv2c
{
    class Lv2cX11Window;
    class Lv2cTheme;
    class Lv2cSvg;
    class FocusNavigationSelector;


    using AnimationCallback = std::function<void(const animation_clock_time_point_t&  now)>;
    using DelayCallback = std::function<void()>;

    enum class Lv2cWindowType
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

    enum class Lv2cWindowPositioning
    {
        NotSet,
        RelativeToDesktop,
        CenterOnDesktop,
        RelativeToParent,
        CenterOnParent,
        ChildWindow
    };

    enum class Lv2cWindowGravity
    {
        NoGravity,
        NorthWest,
        NorthEast,
        SouthWest,
        SouthEast
    };
    /// @brief Specifies parameters used to create windows.
    struct Lv2cCreateWindowParameters
    {
        /// @brief Controls the default window position.
        Lv2cWindowPositioning positioning = Lv2cWindowPositioning::NotSet;
        /// @brief The default location of the window to use when first loaded.
        Lv2cPoint location;
        /// @brief The default size of the window.
        /// If either Width() or Height() is set, then window's element will be measured to determine the size of the window. If
        /// the window position has been saved, then that size will be used instead.
        Lv2cSize size;
        // @brief The intiial state of the window.
        // Either Normal or Maximized. The effect of setting state to Minimized is not defined.
        Lv2cWindowState state = Lv2cWindowState::Normal;
        /// @brief The title of the window.
        std::string title;
        /// @brief The X11 window class of the window.
        /// Used by X11 to locate resource files (of which we have none). Leave it at the default value unless you have a 
        /// compelling need to change it.
        std::string x11Windowclass = "lv2c.github.io-plugin";

        /// @brief The GTK Application ID
        /// Used to locate icons and/or desktop files on KDE Plasma and Gnome Destkop. Use the default 
        /// value to get the default Lv2 Plugin icon (recommmended).  Since we aren't an application, the 
        /// primary purpose of this property is to allow KDE Plasma and Gnome Desktop to location the 
        /// dummy lv2c.github.io-plugin.desktop file, which contains a reference to the LV2 Plugin
        /// icon.
        std::string gtkApplicationId = "lv2c.github.io-plugin";


        /// @brief The X11 name of the  window. Used by Windowm Managers for labelling task buttons &c.
        std::string x11WindowName = "";


        Lv2cWindowType windowType = Lv2cWindowType::Normal;
        /// @brief The anchor position for popup windows (menus, dropdowns, context menus, &c).
        /// Specified in coordinates relative to the parent window.
        Lv2cRectangle anchorPosition;
        /// @brief The corner which the window location is relative to.
        Lv2cWindowGravity gravity = Lv2cWindowGravity::NorthWest;
        /// @brief The default location of the window.
        /// Location is ignored if the positioning is CenterOnDesktop or CenterOnParent, and ignored if
        /// the window has a saved position.

        /// @brief If set, then the size and position of the window will be saved and restored.
        /// See the Lv2cWindow::Settings
        std::string settingsKey;

        /// @brief The dictionary used to save and restore settings.
        /// Must be a json_variant::object(). The host is responsible for saving and restoring the object. Applies only to
        /// the top-most window. Child windows use the settings object of their parent window. See Lv2cWindow::Settings().
        json_variant settingsObject;
        /// @brief Minimum size of the window.
        /// If not set, then the size of the window will be used as the minimum size.
        Lv2cSize minSize;
        /// @brief Maximum size of the window.
        /// If not set, then the size of the window will be used as the minimum size.
        Lv2cSize maxSize;

        /// @brief The background color of the native (X11) window.
        /// Usually, this should be the same as Lv2cTheme::paper. If the property has default value,
        /// the theme color will be copied in automatically.
        Lv2cColor backgroundColor = Lv2cColor(0,0,0,0);


        Lv2cWindow* owner = nullptr;
    public:
        /// @brief Load saved values from the settingsObject.
        void Load();
        /// @brief Save current values to the settingsObject.
        void Save();
    };

    class Lv2cWindow : public Lv2cObject,
                        public std::enable_shared_from_this<Lv2cWindow>
    {
    public:
        using ptr = std::shared_ptr<Lv2cWindow>;
        using super = Lv2cObject;
        static ptr Create() { return std::make_shared<Lv2cWindow>(); }

        Lv2cWindow();
        virtual ~Lv2cWindow();

        Lv2cWindow(const Lv2cWindow &other) = delete;
        Lv2cWindow &operator==(const Lv2cWindow &other) = delete;

        void Close();


        void CloseRootWindow();

        WindowHandle Handle() const;
        
        
        const Lv2cTheme &Theme() const;
        Lv2cWindow &Theme(Lv2cTheme::ptr theme);
        Lv2cTheme::ptr ThemePtr();

        Lv2cWindow &WindowScale(double scale);
        double WindowScale() const;

        /// @brief Enable or disable event tracing.
        /// @param trace true= enable, false=disable
        void TraceEvents(bool trace);
        /// @brief Get the root element for this window.
        /// @return
        std::shared_ptr<Lv2cRootElement> GetRootElement();

        /// @brief Invalidate the contents of the window
        /// Request an update of window contents. The window contents
        /// will be redrawn on the next OnIdle cycle.
        void Invalidate();

        /// @brief Selectively invalidate the contents of the window
        /// @param rectangle The area of the screen to invalidate, in screen coordinates.
        /// Request an update of window contents. The window contents
        /// will be redrawn on the next OnIdle cycle.
        void Invalidate(const Lv2cRectangle &bounds);

        /// @brief Invalidate the layout of elements in the window
        /// Request an update of layout. The window contents
        /// will be layed out on the next OnIdle cycle.
        void InvalidateLayout();

        /// @brief Create child window.
        /// @param hParent The window handle of the parent window.
        /// @param parameters Parameters that control how the window is created.
        void CreateWindow(
            WindowHandle hParent,
            const Lv2cCreateWindowParameters &parameters);

        /// @brief Create a top-level window.
        /// @param parameters Parameters that control how a window is created.
        void CreateWindow(const Lv2cCreateWindowParameters &parameters);

        /// @brief Create a child of a Lv2cWindow.
        /// @param parent The parent window.
        /// @param parameters
        void CreateChildWindow(
            Lv2cWindow *parent,
            const Lv2cCreateWindowParameters &parameters,
            Lv2cElement::ptr element);

        void WindowTitle(const std::string &title);
        const std::string &WindowTitle() const;

        const Lv2cRectangle &ClientRectangle() const;
        Lv2cSize Size() const;

        void Resize(int width, int height);


        /// @brief Show a message-box dialog.
        /// @param dialogType dialog type: Info, Warning or Error.
        /// @param title Title of the dialog.
        /// @param text Message text.
        /// The Lv2cMessageDialog class provides a customizable message box, with options such as a second button,
        /// changing the name of buttons, and a wide variety customizations.

        void MessageBox(Lv2cMessageDialogType dialogType, const std::string &title, const std::string &text);


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

        bool Capture(Lv2cElement *element);
        Lv2cElement *Capture();
        void releaseCapture(Lv2cElement *element);
        Lv2cPoint MousePosition() const;

        bool Focus(Lv2cElement *element);
        Lv2cElement *FocusedElement();
        const Lv2cElement *FocusedElement() const;
        void releaseFocus(Lv2cElement *element);

        PangoContext *GetPangoContext();

        AnimationHandle RequestAnimationCallback(const AnimationCallback &callback);
        AnimationHandle RequestAnimationCallback(AnimationCallback &&callback);
        bool CancelAnimationCallback(AnimationHandle handle);

        AnimationHandle PostDelayed(std::chrono::milliseconds delay, const DelayCallback &callback);
        AnimationHandle PostDelayed(std::chrono::milliseconds delay, DelayCallback &&callback);
        AnimationHandle PostDelayed(uint32_t milliseconds, const DelayCallback &callback);
        AnimationHandle PostDelayed(uint32_t milliseconds, DelayCallback &&callback);

        bool CancelPostDelayed(AnimationHandle handle);

        std::shared_ptr<Lv2cSvg> GetSvgImage(const std::string &filename);
        Lv2cSurface GetPngImage(const std::string &filename);
        static void SetResourceDirectories(const std::vector<std::filesystem::path> &paths);
        static std::filesystem::path findResourceFile(const std::filesystem::path &path);

        std::shared_ptr<Lv2cObject> GetMemoObject(const std::string&name);
        void SetMemoObject(const std::string &name, std::shared_ptr<Lv2cObject> obj);

        void SendAnimationFrameMessage();
        void SendControlChangedMessage(int32_t control, float value);

        const json_variant &Settings() const;
        Lv2cWindow &Settings(const json_variant &settings);

        bool ModalDisable() const;


        Lv2cCreateWindowParameters& WindowParameters() { return windowParameters; }


    protected:
        virtual void OnClosing();
        virtual void OnDraw(Lv2cDrawingContext &dc);
        virtual void OnDrawOver(Lv2cDrawingContext &dc);
        virtual bool OnMouseDown(Lv2cMouseEventArgs &event);
        virtual bool OnMouseMove(Lv2cMouseEventArgs &event);
        virtual bool OnMouseUp(Lv2cMouseEventArgs &event);
        virtual bool OnScrollWheel(Lv2cScrollWheelEventArgs &event);
        virtual void OnIdle();
        virtual void OnSizeChanged(const Lv2cSize &size);

        void AddModalDisable();
        void RemoveModalDisable();

        int modalDisableCount = 0;
        static Lv2cCreateWindowParameters Scale(const Lv2cCreateWindowParameters &v, double windowScale);

        void OnX11ConfigurationChanged(Lv2cWindowPositioning positioning, Lv2cWindowState windowState, Lv2cPoint location, Lv2cSize size);
        void OnX11SizeChanged(Lv2cSize size);

        json_variant settings;
        Lv2cWindow::ptr SelfPointer();
        void OnX11WindowClosed();

        /// @brief Set the root element for this window.
        /// @param element

        void FireFocusIn();
        void FireFocusOut();
        void Draw();
        void Layout();

        void Animate();

    protected:
        virtual bool OnKeyDown(Lv2cKeyboardEventArgs &eventArgs);
        virtual void OnLayoutComplete();

    private:


        std::map<std::string,std::weak_ptr<Lv2cObject>> memoObjects;


        Lv2cDrawingContext CreateDrawingContext();
        void Idle();
        void Size(const Lv2cSize &size);

        // Native Window callback.
        void OnExpose(WindowHandle h, int64_t x, int64_t y, int64_t width, int64_t height);
        virtual void MouseDown(WindowHandle h, uint64_t button, int64_t x, int64_t y, ModifierState state);
        virtual void MouseScrollWheel(WindowHandle h, Lv2cScrollDirection direction, int64_t x, int64_t y, ModifierState state);
        virtual void MouseMove(WindowHandle h, int64_t x, int64_t y, ModifierState state);
        virtual void MouseUp(WindowHandle h, uint64_t button, int64_t x, int64_t y, ModifierState state);
        virtual void MouseLeave(WindowHandle h);
        virtual void UpdateMouseCursor(WindowHandle h, int64_t x, int64_t y, ModifierState state);

        /// @brief Notification of a raw X11 keysim down event.
        /// @param eventArgs Keyboard event arguments.
        /// @return True if the event was handled.
        /// Use KeyDown instead to received translated text input. You probably want KeyDown.
        ///
        /// Only the element that has keyboard capture receives this event.
        bool OnX11KeycodeDown(Lv2cKeyboardEventArgs &eventArgs);
        /// @brief Notification of a raw X11 keysim up event.
        /// @param eventArgs Keyboard event arguments.
        /// @return True if the event was handled.
        /// Use KeyDown instead to received translated text input. You probably want KeyDown.
        ///
        /// Only the element that has keyboard capture receives this event.
        bool OnX11KeycodeUp(Lv2cKeyboardEventArgs &eventArgs);
        bool HandleKeyboardNavigation(Lv2cKeyboardEventArgs &eventArgs);

        void NavigateFocus(FocusNavigationSelector &selector);

    private:
        double windowScale = 1.0;
        Lv2cRectangle lastFocusRectangle;

        Lv2cElement *savedFocusElement = nullptr;
        Lv2cMouseEventArgs lastMouseEventArgs;

        Lv2cPoint mousePosition{-100000, -100000};

        Lv2cElement *captureElement = nullptr;
        Lv2cElement *focusElement = nullptr;
        Lv2cSize size;
        std::string windowTitle;
        Lv2cX11Window *nativeWindow = nullptr;
        Lv2cCreateWindowParameters windowParameters;
        Lv2cRectangle bounds;

        Lv2cDamageList damageList;

        bool valid = false;
        bool layoutValid = false;

        std::shared_ptr<Lv2cRootElement> rootElement;

        std::shared_ptr<Lv2cTheme> theme;

        std::map<AnimationHandle, AnimationCallback> animationCallbacks;

        struct DelayRecord
        {
            animation_clock_t::time_point time;
            DelayCallback callback;
        };

        std::recursive_mutex delayCallbacksMutex;
        std::map<AnimationHandle, DelayRecord> delayCallbacks;

        static std::vector<std::filesystem::path> resourceDirectories;

        std::map<std::string, std::shared_ptr<Lv2cSvg>> svgCache;
        std::map<std::string, Lv2cSurface> pngCache;


    private:
        friend class Lv2cX11Window;
        friend class Lv2cDialog;
        friend class Lv2cElement;
    };

    ///////////////////////////////////////////////////////////////
    inline const Lv2cRectangle &Lv2cWindow::ClientRectangle() const
    {
        return this->bounds;
    }
    inline Lv2cSize Lv2cWindow::Size() const
    {
        return this->bounds.Size();
    }
    inline AnimationHandle Lv2cWindow::PostDelayed(uint32_t milliseconds, const DelayCallback &callback)
    {
        return PostDelayed(std::chrono::milliseconds(milliseconds), callback);
    }
    inline AnimationHandle Lv2cWindow::PostDelayed(uint32_t milliseconds, DelayCallback &&callback)
    {
        return PostDelayed(std::chrono::milliseconds(milliseconds), callback);
    }

}