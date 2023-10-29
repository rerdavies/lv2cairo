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

#include "Lv2cWindow.hpp"
#include "Lv2cBindingProperty.hpp"

namespace lvtk
{
    class Lv2cDialog: public Lv2cWindow
    {
    public:
        using self = Lv2cDialog;
        using super = Lv2cWindow;
        using ptr = std::shared_ptr<self>;

        virtual ~Lv2cDialog();


        /// @brief Set the default size of the dialog.
        /// @param size The size.
        /// If either size.Width() or size.Height() is zero, the rendered element 
        /// will be measured to determine the size of the dialog. The element 
        /// must not have stretchable width or height if Width(), or Height(), respectively,
        /// are zero.
        Lv2cDialog&DefaultSize(Lv2cSize size);
        Lv2cSize DefaultSize() const;

        /// @brief The title of the dialog.
        /// @param value The title.
        /// @return self&.
        Lv2cDialog&Title(const std::string &value);
        const std::string& Title() const;


        /// @brief The key to save and load position from settings.
        /// @param value The key.
        /// @return self&.
        /// If the value is not empty, the position of the dialog will 
        /// be saved and loaded using the Lv2cWindow::Settings property
        /// of the parent Lv2cWindow.
        Lv2cDialog&SettingsKey(const std::string &value);
        const std::string& SettingsKey() const;


        /// @brief The minimum size of the dialog.
        /// @param value size.
        /// @return self&
        /// If either Width() or Height() is zero, then the 
        /// value set on the window will be the size of the window when 
        /// created. If MinSize and MaxSize Width() or Height() are both 
        /// zero, then the window will not be resizable in that direction.

        Lv2cDialog&MinSize(Lv2cSize value);
        Lv2cSize MinSize() const;

        /// @brief The maximum size of the dialog.
        /// @param value size.
        /// @return self&
        /// If either Width() or Height() is zero, then the 
        /// value set on the window will be the size of the window when 
        /// created. If MinSize and MaxSize Width() or Height() are both 
        /// zero, then the window will not be resizable in that direction.
        /// 
        /// It is probably prudent to not seith either Width() or Height() 
        /// to a value larger than 32767.

        Lv2cDialog&MaxSize(Lv2cSize value);
        Lv2cSize MaxSize() const;

        Lv2cWindowPositioning Positioning() const;
        Lv2cDialog&Positioning(Lv2cWindowPositioning positioning);

       /// @brief Gravity 
        /// @param value gravity.
        /// @return self&
        /// The gravity of the window position when setting the default 
        /// position of the window.

        Lv2cWindowGravity Gravity() const;
        Lv2cDialog&Gravity(Lv2cWindowGravity value);


        /// @brief Window type.
        /// @return The type of window to create.
        /// Defaults to Lv2cWindowType::Dialog.
        Lv2cWindowType WindowType() const;
        Lv2cDialog&WindowType(Lv2cWindowType windowType);


        virtual void Show(Lv2cWindow *parentWindow);

        struct ClosingEventArgs {
        };
        Lv2cEvent<ClosingEventArgs> Closing;
    protected:
        BINDING_PROPERTY_REF(X11WindowName,std::string,"");
        // Default name used by Window Managers.
        virtual void OnMount();
        virtual void OnClosing() override;
    private:
        Lv2cWindow *modalDisableWindow = nullptr;
        Lv2cWindowType windowType = Lv2cWindowType::Dialog;
        Lv2cWindowPositioning positioning = Lv2cWindowPositioning::CenterOnParent;
        

        Lv2cWindowGravity gravity = Lv2cWindowGravity::NorthWest;
        std::string title;
        std::string settingsKey;
        Lv2cSize minSize, maxSize;
        Lv2cSize defaultSize { 640,480};
        virtual Lv2cElement::ptr Render() = 0;
    };
}