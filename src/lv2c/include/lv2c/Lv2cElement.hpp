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

#include <vector>
#include <memory>


#include "Lv2cTypes.hpp"
// #include "Lv2cDrawingContext.hpp"
#include "Lv2cStyle.hpp"
#include "Lv2cTheme.hpp"
#include <iostream>
#include "Lv2cUserData.hpp"
#include "Lv2cObject.hpp"
#include <chrono>


// forward declaration.
typedef struct _PangoContext PangoContext;

namespace lv2c
{

    using animation_clock_t = std::chrono::steady_clock;
    using animation_clock_time_point_t = std::chrono::steady_clock::time_point;

    class Lv2cDrawingContext;

    class Lv2cElement: public Lv2cObject
    {

    public:
        using self = Lv2cElement;
        using ptr = std::shared_ptr<self>;
        virtual const char* Tag() const { return "Element"; }
        static ptr Create() { return std::make_shared<self>(); }

        Lv2cElement(const Lv2cElement &other) = delete;
        Lv2cElement &operator==(const Lv2cElement &other) = delete;

        Lv2cElement();
        virtual ~Lv2cElement() noexcept;

        /// @brief Get the window on which this element is mounted.
        //  @returns The window on which this element is mounted, or `nullptr` if not mounted.
        const Lv2cWindow *Window() const;
        Lv2cWindow *Window();

        /// @brief Will the element draw? 
        /// Override to return true, if the overriding element wants a call to onDraw.
        // The default behaviour is to return true if the element has a border or background.
        virtual bool WillDraw() const;

        /// @brief Will the element draw over?
        /// Override to return true if the overriding element wants  call to onDrawOver.
        /// The default behaviour is to return false always.
        virtual bool WillDrawOver() const;


        /// @brief Does the element have a drawable background?
        virtual bool HasBackground() const ;

        /// @brief Does the element have a drawable border?
        virtual bool WillDrawBorder() const;

        /// @brief Request a new layout pass for this element.
        virtual void InvalidateLayout();
        /// @brief Request a new layout pass this element's parent.
        /// Does not run layout on the the entire visual tree.
        virtual void InvalidateParentLayout();
        /// @brief Request that the contents of the element be redrawn.
        virtual void Invalidate();

        /// @brief Request that the contents of the element be redrawn.
        /// @param clientRectangle The area to invalidate, in client coordinates.

        void InvalidateClientRect(const Lv2cRectangle &clientRectangle);
        /// @brief Request that the contents of the rectangle be redrawn.
        /// @param screenRectangle The area to invalidate, in screen coordinates.

        virtual void InvalidateScreenRect(const Lv2cRectangle &screenRectangle);


        void PrintStructure() const;
    public:
        bool CaptureMouse();
        const Lv2cElement*Capture() const;
        bool Hascapture() const;
        void ReleaseCapture();

        using MouseEvent = Lv2cEvent<Lv2cMouseEventArgs>;

        MouseEvent MouseDown;
        MouseEvent MouseUp;
        MouseEvent MouseMove;

        using ScrollWheelEvent = Lv2cEvent<Lv2cScrollWheelEventArgs>;

        ScrollWheelEvent ScrollWheel;

        using MouseOverEvent = Lv2cEvent<Lv2cMouseOverEventArgs>;
        MouseOverEvent MouseOver;
        MouseOverEvent MouseOut;

        using KeyDownEvent = Lv2cEvent<const Lv2cKeyboardEventArgs>;
        KeyDownEvent KeyDown;

        virtual bool isContainer() const { return false; }
        virtual bool WantsFocus() const;
        const Lv2cRectangle & ScreenBounds() const;
        const Lv2cRectangle & ScreenBorderRect() const;
        const Lv2cRectangle & ScreenClientBounds() const;

    protected:
        virtual bool ClipChildren() const;
        
        Lv2cElement &UserData(const Lv2cUserData::ptr& value);
        Lv2cElement &UserData(Lv2cUserData::ptr &&value);
        Lv2cUserData::ptr UserData();

        virtual bool OnMouseDown(Lv2cMouseEventArgs &event);
        virtual bool OnMouseUp(Lv2cMouseEventArgs &event);
        virtual bool OnMouseMove(Lv2cMouseEventArgs &event);
        virtual bool OnMouseOver(Lv2cMouseOverEventArgs &event);
        virtual bool OnMouseOut(Lv2cMouseOverEventArgs &event);

        virtual std::optional<Lv2cCursor> GetMouseCursor(Lv2cMouseEventArgs &event);

        virtual bool OnScrollWheel(Lv2cScrollWheelEventArgs &event);


        /// @brief Notification that a key was pressed.
        /// @param event Event arguments.
        /// @return True if the event was handled.
        /// events can have keycodes, text, or both. On linux, Keycodes 
        /// are x11 keycodes.
        /// The text property contains fully IME-processed UTF8 text. 
        virtual bool OnKeyDown(const Lv2cKeyboardEventArgs&event);

        /// @brief Notification that a raw X11 key was pressed.
        /// @param target The initial target of the event.
        /// @param event Event arguments.
        /// @return True if the event was handled.
        /// Not IME processing is performed on the X11 rawKeyCode. The keycode and text properties are always invalid.
        /// rawKeyCode is valid, but it is a raw x11 keycode which is, for all practical purposes, unintelligible.
        /// 
        /// You probably want OnKeyDown instead.
        virtual bool OnKeycodeDown(const Lv2cKeyboardEventArgs&event);
        /// @brief Notification that a raw X11 key was released.
        /// @param event Event arguments.
        /// @return True if the event was handled.
        /// The keycode and text property is always empty. Only rawKeyCode is valid. rawKeyCode 
        /// is the raw x11 keycode which is, for all practical purposes, unintelligible.
        /// 
        /// You probably want OnKeyDown instead.
        virtual bool OnKeycodeUp(const Lv2cKeyboardEventArgs&event);


    private:

        void PartialLayout();
        Lv2cUserData::ptr userData;
        virtual bool FireKeyDown(const Lv2cKeyboardEventArgs&event);
        virtual bool FireMouseDown(Lv2cMouseEventArgs&event);
        virtual bool FireMouseUp(Lv2cMouseEventArgs&event);
        virtual void UpdateMouseOver(Lv2cPoint mousePosition);
        virtual bool FireScrollWheel(Lv2cScrollWheelEventArgs&event);

        void SetMouseOver(bool mouseOver);

    public: // base layout
        void Style(const Lv2cStyle &style) { 
            this->style = style;
            this->style.SetElement(this);
        }
        void Style(Lv2cStyle &&style) { 
            this->style = std::move(style); 
            this->style.SetElement(this);
        }

        Lv2cStyle &Style() { return style; }
        const Lv2cStyle &Style() const { return style; }

        Lv2cElement& ClearClasses();
        Lv2cElement&AddClass(Lv2cStyle::ptr style);
        Lv2cElement&RemoveClass(Lv2cStyle::ptr style);
        Lv2cElement&Classes(Lv2cStyle::ptr style);
        Lv2cElement&Classes(std::vector<Lv2cStyle::ptr> style);

        bool Focus();
        bool ReleaseFocus();
        bool Focused() const;


        const std::vector<Lv2cStyle::ptr>&Classes() const;


        Lv2cSize ClientSize() const { return clientSize; }
        Lv2cRectangle ClientRectangle() const { return Lv2cRectangle{ClientSize()};}
        /// @brief The bounds of the border rectangle in client coordinates.
        /// @return The border rectangle.
        /// Note that Left() and Top() will be negative if the element has Lv2cStyle::Padding() or Lv2cStyle::BorderWidth().
        Lv2cRectangle ClientBorderRectangle() const;
        /// @brief The bounds of the padding rectangle (the inside bounds of the border) in client coordinates.
        /// @return The padding rectangle.
        /// Note that Left() and Top() will be negative if the element has Lv2cStyle::Padding().
        Lv2cRectangle ClientPaddingRectangle() const;
        /// @brief The bounds of the element including Lv2cStyle::Margin() in client coordinates.
        /// @return The padding rectangle.
        /// Note that Left() and Top() will be negative if the element has Lv2cStyle::Padding(), Lv2cStyle::BorderWidth() or Lv2cStyle::Margins().
        Lv2cRectangle ClientMarginRectangle() const;


        const Lv2cRectangle &Bounds() const { return bounds; }
        const Lv2cRectangle &getBorderBounds() const { return borderBounds; }
        const Lv2cRectangle &ClientBounds() const { return clientBounds; }

        Lv2cElement*Parent() { return this->parentElement; }
        const Lv2cElement*getParent() const { return this->parentElement; }
        
        bool IsMounted() const;
        const Lv2cTheme&Theme() const;
        Lv2cElement&Theme(Lv2cTheme::ptr theme);

        PangoContext *GetPangoContext();

        Lv2cEvent<Lv2cWindow*> OnMounted;
        Lv2cEvent<Lv2cWindow*> OnUnmounted;
    protected:
        Lv2cSize measure;
        Lv2cStyle style = Lv2cStyle();

    public:
        virtual void FinalizeLayout(const Lv2cRectangle&layoutClipRect,const Lv2cRectangle& screenOffest, bool clippedInLayout = false);

        Lv2cSize MeasuredSize() const;
        /// @brief Measure the desired size of the Element, given the available space.
        /// @param available The expected size of the space available for the element in which it will be arranged.
        /// The layout scheme is based on Android layout. When Measure is called,
        /// An element must decide how much space it would like  to occupy, given the available
        /// space. The result is then stored with
        /// a call to this->SetMeasure() before returning. The element must also call Measure()
        /// on all child  elements with an `available` argument that reflects any space (e.g. paddings and borderWidth)
        /// that the element itself wishes to set aside for its own use. Complex containers may call
        /// Measure() more than once in the first with trial measurements, if neccessary.
        /// During the second pass, Arrange is called for each element in the layout tree, with the
        /// actual space allocated for the control. Elements must call Arrange() for all child
        /// child elements, and then call Layout() to finalize the position of the element.
        virtual void Measure(Lv2cSize constraint, Lv2cSize maxAvailable, Lv2cDrawingContext &context);


        /// @brief Measure the desired size of the Element, given the available client constraints (with margins, borders, padding removed).
        /// @param available The desired size of the client area (excluding margins, borders, padding)
        /// Classes not derived from Lv2cContainerElement delegate to this method eventually. This is a convenience method
        /// that allows derived classes to avoid complexities of a a full Measure() implementation. 
        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context);

        /// @brief Arrange child elements into their final position.
        ///  See Measure() for a detailed description of the layout algorithm. When called,
        ///  an element must call Arrange on all child elements, with a final size, which may not
        ///  reflect the size requested in the Measure() pass of layout. The element must then call
        ///  Layout on each child with the final position of the client area (exclusive of margin)
        ///  relative to the parent's client area.

        virtual Lv2cSize Arrange(Lv2cSize available,Lv2cDrawingContext &context);

        /// @brief Update the final position of the control relative to its parent.
        /// @param x X offset of the client area of the control relative to the client area of its parent.
        /// @param y Y offset of the client area of the control relative to the client area of its parent.
        /// @param width Width of the client area.
        /// @param height Height of the client area.
        /// @param changed - true if the location has changed since the last call to layout.
        virtual void Layout(const Lv2cRectangle &bounds);


        bool LayoutValid() const;

    protected:
        // utility functions for layout calculations.
        Lv2cSize MeasuredSizeSizeFromStyle(Lv2cSize available);

        static Lv2cSize removeThickness(Lv2cSize size, const Lv2cThicknessMeasurement &thickness);
        static Lv2cSize AddThickness(Lv2cSize size, const Lv2cThicknessMeasurement &thickness);

        static Lv2cRectangle removeThickness(const Lv2cRectangle &rectangle, const Lv2cThicknessMeasurement &thickness);
        static Lv2cRectangle AddThickness(const Lv2cRectangle &rectangle, const Lv2cThicknessMeasurement &thickness);

    protected:
        void SetMeasure(Lv2cSize measuredSize);

    protected:
        /// @brief Draw the contents of the current element
        /// @param dc  A Lv2c drawing context.
        /// The default implementation draws background and borders.
        /// If the element draws in the client area, the WantsDraw() method should be 
        /// overriden to return true.
        /// On entry, the the dc is set up so that the top-left pixel of the client area 
        /// is at (0,0). In addition, a clip mask will have been set up to prevent the 
        /// element from drawing outside the area covered by the margins and borders of the element. 
        ///
        /// This method is called before all child elements have strted drawing.
        ///
        /// @see WillDraw, OnDrawOver, WillDrawOver

        virtual void OnDraw(Lv2cDrawingContext &dc);

        /// @brief Draw over contents of the current element, and all child elements.
        /// @param dc  A Lv2c drawing context.
        /// The default implementation does nothing.
        ///
        /// If the element draws in the client area, the WantsDrawOver() method should be 
        /// overriden to return true.
        ///
        /// On entry, the the dc is set up so that the top-left pixel of the client area 
        /// is at (0,0). In addition, a clip mask will have been set up to prevent the 
        /// element from drawing outside the area covered by the margins and borders of the element. 
        ///
        /// This method is called after all child elements have finished drawing.
        virtual void OnDrawOver(Lv2cDrawingContext &dc);


        virtual void OnMount();
        virtual void OnUnmount();
        // Calls OnMount() by default.
        virtual void OnMount(Lv2cWindow *window);
        // Calls OnUnmount() by default.
        virtual void OnUnmount(Lv2cWindow *window);

        // Draw all children, potentially with Opacity.
        virtual void Draw(Lv2cDrawingContext &dc, const Lv2cRectangle &parentBounds);

        // Draw all children with Opacity pre-applied. (Called by Draw).
        virtual void DrawPostOpacity(Lv2cDrawingContext &dc, const Lv2cRectangle &clipBounds);

    public:
        Lv2cEvent<Lv2cFocusEventArgs> FocusEvent;
        Lv2cEvent<Lv2cFocusEventArgs> LostFocusEvent;
    protected:
        virtual bool OnFocus(const Lv2cFocusEventArgs &eventArgs);
        virtual bool OnLostFocus(const Lv2cFocusEventArgs &eventArgs);
        virtual void Mount(Lv2cWindow *window);
        virtual void Unmount(Lv2cWindow *window);
        virtual void OnLayoutComplete();


        virtual Lv2cRectangle GetDrawBounds(
            const Lv2cRectangle&screenBounds,
            const Lv2cRectangle&borderBounds
            );

        void drawRoundBorderRect(Lv2cDrawingContext &dc);
        void drawRoundInsideBorderRect(Lv2cDrawingContext &dc);

    protected:
        virtual void OnHoverStateChanged(Lv2cHoverState hoverState);

    public:
    public:
        Lv2cHoverState HoverState() const;
        Lv2cElement&HoverState(Lv2cHoverState hoverState);


        bool Focus(const Lv2cFocusEventArgs&eventArgs);
        bool LostFocus(const Lv2cFocusEventArgs&eventArgs);
        Lv2cRoundCorners roundCorners;
        Lv2cTheme::ptr theme;
        bool mouseOver = false;
        Lv2cThicknessMeasurement margin{0, 0, 0, 0};
        Lv2cThicknessMeasurement padding{0, 0, 0, 0};
        bool layoutValid = false;
        Lv2cWindow *window = nullptr;
        Lv2cElement *parentElement = nullptr;

        Lv2cSize clientSize;

    protected:
        Lv2cRectangle bounds;
        Lv2cRectangle borderBounds;
        Lv2cRectangle paddingBounds;
        Lv2cRectangle clientBounds;
    private:
        bool clippedInLayout = false;
        std::vector<Lv2cStyle::ptr> classes;
        Lv2cRectangle savedLayoutClipRect;
        bool savedClippedInLayout = false;
        Lv2cRectangle screenDrawBounds;
        Lv2cRectangle screenBounds;
        Lv2cRectangle screenBorderBounds;
        Lv2cRectangle screenPaddingBounds;
        Lv2cRectangle screenClientBounds;

        void PrintStructure(std::ostream&s) const;
        void PrintStructure(std::ostream&s,size_t indent) const;
        Lv2cHoverState hoverState = Lv2cHoverState::Empty;
    private:
        friend class Lv2cWindow;
        friend class Lv2cContainerElement;
        friend class Lv2cRootElement;
        friend class Lv2cDialog;
        friend class Lv2cDropShadowElement;


    };


}