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

#include "LvtkWindow.hpp"
#include "LvtkBindingProperty.hpp"

namespace lvtk
{
    class LvtkDialog: public LvtkWindow
    {
    public:
        using self = LvtkDialog;
        using super = LvtkWindow;
        using ptr = std::shared_ptr<self>;

        virtual ~LvtkDialog();


        /// @brief Set the default size of the dialog.
        /// @param size The size.
        /// If either size.Width() or size.Height() is zero, the rendered element 
        /// will be measured to determine the size of the dialog. The element 
        /// must not have stretchable width or height if Width(), or Height(), respectively,
        /// are zero.
        LvtkDialog&DefaultSize(LvtkSize size);
        LvtkSize DefaultSize() const;

        /// @brief The title of the dialog.
        /// @param value The title.
        /// @return self&.
        LvtkDialog&Title(const std::string &value);
        const std::string& Title() const;


        /// @brief The key to save and load position from settings.
        /// @param value The key.
        /// @return self&.
        /// If the value is not empty, the position of the dialog will 
        /// be saved and loaded using the LvtkWindow::Settings property
        /// of the parent LvtkWindow.
        LvtkDialog&SettingsKey(const std::string &value);
        const std::string& SettingsKey() const;


        /// @brief The minimum size of the dialog.
        /// @param value size.
        /// @return self&
        /// If either Width() or Height() is zero, then the 
        /// value set on the window will be the size of the window when 
        /// created. If MinSize and MaxSize Width() or Height() are both 
        /// zero, then the window will not be resizable in that direction.

        LvtkDialog&MinSize(LvtkSize value);
        LvtkSize MinSize() const;

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

        LvtkDialog&MaxSize(LvtkSize value);
        LvtkSize MaxSize() const;

        LvtkWindowPositioning Positioning() const;
        LvtkDialog&Positioning(LvtkWindowPositioning positioning);

       /// @brief Gravity 
        /// @param value gravity.
        /// @return self&
        /// The gravity of the window position when setting the default 
        /// position of the window.

        LvtkWindowGravity Gravity() const;
        LvtkDialog&Gravity(LvtkWindowGravity value);


        /// @brief Window type.
        /// @return The type of window to create.
        /// Defaults to LvtkWindowType::Dialog.
        LvtkWindowType WindowType() const;
        LvtkDialog&WindowType(LvtkWindowType windowType);


        virtual void Show(LvtkWindow *parentWindow);

        struct ClosingEventArgs {
        };
        LvtkEvent<ClosingEventArgs> Closing;
    protected:
        BINDING_PROPERTY_REF(X11WindowName,std::string,"");
        // Default name used by Window Managers.
        virtual void OnMount();
        virtual void OnClosing() override;
    private:
        LvtkWindow *modalDisableWindow = nullptr;
        LvtkWindowType windowType = LvtkWindowType::Dialog;
        LvtkWindowPositioning positioning = LvtkWindowPositioning::CenterOnParent;
        

        LvtkWindowGravity gravity = LvtkWindowGravity::NorthWest;
        std::string title;
        std::string settingsKey;
        LvtkSize minSize, maxSize;
        LvtkSize defaultSize { 640,480};
        virtual LvtkElement::ptr Render() = 0;
    };
}