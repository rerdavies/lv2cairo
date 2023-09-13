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

#include <vector>
#include <memory>


#include "LvtkTypes.hpp"
// #include "LvtkDrawingContext.hpp"
#include "LvtkStyle.hpp"
#include "LvtkTheme.hpp"
#include <iostream>
#include "LvtkUserData.hpp"
#include "LvtkObject.hpp"
#include <chrono>


// forward declaration.
typedef struct _PangoContext PangoContext;

namespace lvtk
{

    using animation_clock_t = std::chrono::steady_clock;
    using animation_clock_time_point_t = std::chrono::steady_clock::time_point;

    class LvtkDrawingContext;

    class LvtkElement: public LvtkObject
    {

    public:
        using self = LvtkElement;
        using ptr = std::shared_ptr<self>;
        virtual const char* Tag() const { return "Element"; }
        static ptr Create() { return std::make_shared<self>(); }

        LvtkElement(const LvtkElement &other) = delete;
        LvtkElement &operator==(const LvtkElement &other) = delete;

        LvtkElement();
        virtual ~LvtkElement() noexcept;

        /// @brief Get the window on which this element is mounted.
        //  @returns The window on which this element is mounted, or `nullptr` if not mounted.
        const LvtkWindow *Window() const;
        LvtkWindow *Window();

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

        void InvalidateClientRect(const LvtkRectangle &clientRectangle);
        /// @brief Request that the contents of the rectangle be redrawn.
        /// @param screenRectangle The area to invalidate, in screen coordinates.

        virtual void InvalidateScreenRect(const LvtkRectangle &screenRectangle);


        void PrintStructure() const;
    public:
        bool CaptureMouse();
        const LvtkElement*Capture() const;
        bool Hascapture() const;
        void ReleaseCapture();

        using MouseEvent = LvtkEvent<LvtkMouseEventArgs>;

        MouseEvent MouseDown;
        MouseEvent MouseUp;
        MouseEvent MouseMove;

        using MouseOverEvent = LvtkEvent<LvtkMouseOverEventArgs>;
        MouseOverEvent MouseOver;
        MouseOverEvent MouseOut;

        using KeyDownEvent = LvtkEvent<const LvtkKeyboardEventArgs>;
        KeyDownEvent KeyDown;

        virtual bool isContainer() const { return false; }
        virtual bool WantsFocus() const;
        const LvtkRectangle & ScreenBounds() const;
        const LvtkRectangle & ScreenBorderRect() const;
        const LvtkRectangle & ScreenClientBounds() const;

    protected:
        virtual bool ClipChildren() const;
        
        LvtkElement &UserData(const LvtkUserData::ptr& value);
        LvtkElement &UserData(LvtkUserData::ptr &&value);
        LvtkUserData::ptr UserData();

        virtual bool OnMouseDown(LvtkMouseEventArgs &event);
        virtual bool OnMouseUp(LvtkMouseEventArgs &event);
        virtual bool OnMouseMove(LvtkMouseEventArgs &event);
        virtual bool OnMouseOver(LvtkMouseOverEventArgs &event);
        virtual bool OnMouseOut(LvtkMouseOverEventArgs &event);


        /// @brief Notification that a key was pressed.
        /// @param event Event arguments.
        /// @return True if the event was handled.
        /// events can have keycodes, text, or both. On linux, Keycodes 
        /// are x11 keycodes.
        /// The text property contains fully IME-processed UTF8 text. 
        virtual bool OnKeyDown(const LvtkKeyboardEventArgs&event);

        /// @brief Notification that a raw X11 key was pressed.
        /// @param target The initial target of the event.
        /// @param event Event arguments.
        /// @return True if the event was handled.
        /// Not IME processing is performed on the X11 rawKeyCode. The keycode and text properties are always invalid.
        /// rawKeyCode is valid, but it is a raw x11 keycode which is, for all practical purposes, unintelligible.
        /// 
        /// You probably want OnKeyDown instead.
        virtual bool OnKeycodeDown(const LvtkKeyboardEventArgs&event);
        /// @brief Notification that a raw X11 key was released.
        /// @param event Event arguments.
        /// @return True if the event was handled.
        /// The keycode and text property is always empty. Only rawKeyCode is valid. rawKeyCode 
        /// is the raw x11 keycode which is, for all practical purposes, unintelligible.
        /// 
        /// You probably want OnKeyDown instead.
        virtual bool OnKeycodeUp(const LvtkKeyboardEventArgs&event);


    private:

        void PartialLayout();
        LvtkUserData::ptr userData;
        virtual bool FireKeyDown(const LvtkKeyboardEventArgs&event);
        virtual bool FireMouseDown(LvtkMouseEventArgs&event);
        virtual bool FireMouseUp(LvtkMouseEventArgs&event);
        virtual void UpdateMouseOver(LvtkPoint mousePosition);

        void SetMouseOver(bool mouseOver);

    public: // base layout
        void Style(const LvtkStyle &style) { 
            this->style = style;
            this->style.SetElement(this);
        }
        void Style(LvtkStyle &&style) { 
            this->style = std::move(style); 
            this->style.SetElement(this);
        }

        LvtkStyle &Style() { return style; }
        const LvtkStyle &Style() const { return style; }

        LvtkElement& ClearClasses();
        LvtkElement&AddClass(LvtkStyle::ptr style);
        LvtkElement&RemoveClass(LvtkStyle::ptr style);
        LvtkElement&Classes(LvtkStyle::ptr style);
        LvtkElement&Classes(std::vector<LvtkStyle::ptr> style);

        bool Focus();
        bool ReleaseFocus();
        bool Focused() const;


        const std::vector<LvtkStyle::ptr>&Classes() const;


        LvtkSize ClientSize() const { return clientSize; }
        LvtkRectangle ClientRectangle() const { return LvtkRectangle{ClientSize()};}
        /// @brief The bounds of the border rectangle in client coordinates.
        /// @return The border rectangle.
        /// Note that Left() and Top() will be negative if the element has LvtkStyle::Padding() or LvtkStyle::BorderWidth().
        LvtkRectangle ClientBorderRectangle() const;
        /// @brief The bounds of the padding rectangle (the inside bounds of the border) in client coordinates.
        /// @return The padding rectangle.
        /// Note that Left() and Top() will be negative if the element has LvtkStyle::Padding().
        LvtkRectangle ClientPaddingRectangle() const;
        /// @brief The bounds of the element including LvtkStyle::Margin() in client coordinates.
        /// @return The padding rectangle.
        /// Note that Left() and Top() will be negative if the element has LvtkStyle::Padding(), LvtkStyle::BorderWidth() or LvtkStyle::Margins().
        LvtkRectangle ClientMarginRectangle() const;


        const LvtkRectangle &Bounds() const { return bounds; }
        const LvtkRectangle &getBorderBounds() const { return borderBounds; }
        const LvtkRectangle &ClientBounds() const { return clientBounds; }

        LvtkElement*Parent() { return this->parentElement; }
        const LvtkElement*getParent() const { return this->parentElement; }
        
        bool IsMounted() const;
        const LvtkTheme&Theme() const;
        LvtkElement&Theme(LvtkTheme::ptr theme);

        PangoContext *GetPangoContext();

        LvtkEvent<LvtkWindow*> OnMounted;
        LvtkEvent<LvtkWindow*> OnUnmounted;
    protected:
        LvtkSize measure;
        LvtkStyle style = LvtkStyle();

    public:
        virtual void FinalizeLayout(const LvtkRectangle&layoutClipRect,const LvtkRectangle& screenOffest, bool clippedInLayout = false);

        LvtkSize MeasuredSize() const;
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
        virtual void Measure(LvtkSize constraint, LvtkSize maxAvailable, LvtkDrawingContext &context);


        /// @brief Measure the desired size of the Element, given the available client constraints (with margins, borders, padding removed).
        /// @param available The desired size of the client area (excluding margins, borders, padding)
        /// Classes not derived from LvtkContainerElement delegate to this method eventually. This is a convenience method
        /// that allows derived classes to avoid complexities of a a full Measure() implementation. 
        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context);

        /// @brief Arrange child elements into their final position.
        ///  See Measure() for a detailed description of the layout algorithm. When called,
        ///  an element must call Arrange on all child elements, with a final size, which may not
        ///  reflect the size requested in the Measure() pass of layout. The element must then call
        ///  Layout on each child with the final position of the client area (exclusive of margin)
        ///  relative to the parent's client area.

        virtual LvtkSize Arrange(LvtkSize available,LvtkDrawingContext &context);

        /// @brief Update the final position of the control relative to its parent.
        /// @param x X offset of the client area of the control relative to the client area of its parent.
        /// @param y Y offset of the client area of the control relative to the client area of its parent.
        /// @param width Width of the client area.
        /// @param height Height of the client area.
        /// @param changed - true if the location has changed since the last call to layout.
        virtual void Layout(const LvtkRectangle &bounds);


        bool LayoutValid() const;

    protected:
        // utility functions for layout calculations.
        LvtkSize MeasuredSizeSizeFromStyle(LvtkSize available);

        static LvtkSize removeThickness(LvtkSize size, const LvtkThicknessMeasurement &thickness);
        static LvtkSize AddThickness(LvtkSize size, const LvtkThicknessMeasurement &thickness);

        static LvtkRectangle removeThickness(const LvtkRectangle &rectangle, const LvtkThicknessMeasurement &thickness);
        static LvtkRectangle AddThickness(const LvtkRectangle &rectangle, const LvtkThicknessMeasurement &thickness);

    protected:
        void SetMeasure(LvtkSize measuredSize);

    protected:
        /// @brief Draw the contents of the current element
        /// @param dc  A Lvtk drawing context.
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

        virtual void OnDraw(LvtkDrawingContext &dc);

        /// @brief Draw over contents of the current element, and all child elements.
        /// @param dc  A Lvtk drawing context.
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
        virtual void OnDrawOver(LvtkDrawingContext &dc);


        virtual void OnMount();
        virtual void OnUnmount();
        // Calls OnMount() by default.
        virtual void OnMount(LvtkWindow *window);
        // Calls OnUnmount() by default.
        virtual void OnUnmount(LvtkWindow *window);

        // Draw all children, potentially with Opacity.
        virtual void Draw(LvtkDrawingContext &dc, const LvtkRectangle &parentBounds);

        // Draw all children with Opacity pre-applied. (Called by Draw).
        virtual void DrawPostOpacity(LvtkDrawingContext &dc, const LvtkRectangle &clipBounds);

    public:
        LvtkEvent<LvtkFocusEventArgs> FocusEvent;
        LvtkEvent<LvtkFocusEventArgs> LostFocusEvent;
    protected:
        virtual bool OnFocus(const LvtkFocusEventArgs &eventArgs);
        virtual bool OnLostFocus(const LvtkFocusEventArgs &eventArgs);
        virtual void Mount(LvtkWindow *window);
        virtual void Unmount(LvtkWindow *window);
        virtual void OnLayoutComplete();


        virtual LvtkRectangle GetDrawBounds(
            const LvtkRectangle&screenBounds,
            const LvtkRectangle&borderBounds
            );

        void drawRoundBorderRect(LvtkDrawingContext &dc);
        void drawRoundInsideBorderRect(LvtkDrawingContext &dc);

    protected:
        virtual void OnHoverStateChanged(LvtkHoverState hoverState);

    public:
    public:
        LvtkHoverState HoverState() const;
        LvtkElement&HoverState(LvtkHoverState hoverState);


        bool Focus(const LvtkFocusEventArgs&eventArgs);
        bool LostFocus(const LvtkFocusEventArgs&eventArgs);
        LvtkRoundCorners roundCorners;
        LvtkTheme::ptr theme;
        bool mouseOver = false;
        LvtkThicknessMeasurement margin{0, 0, 0, 0};
        LvtkThicknessMeasurement padding{0, 0, 0, 0};
        bool layoutValid = false;
        LvtkWindow *window = nullptr;
        LvtkElement *parentElement = nullptr;

        LvtkSize clientSize;

    protected:
        LvtkRectangle bounds;
        LvtkRectangle borderBounds;
        LvtkRectangle paddingBounds;
        LvtkRectangle clientBounds;
    private:
        bool clippedInLayout = false;
        std::vector<LvtkStyle::ptr> classes;
        LvtkRectangle savedLayoutClipRect;
        bool savedClippedInLayout = false;
        LvtkRectangle screenDrawBounds;
        LvtkRectangle screenBounds;
        LvtkRectangle screenBorderBounds;
        LvtkRectangle screenPaddingBounds;
        LvtkRectangle screenClientBounds;

        void PrintStructure(std::ostream&s) const;
        void PrintStructure(std::ostream&s,size_t indent) const;
        LvtkHoverState hoverState = LvtkHoverState::Empty;
    private:
        friend class LvtkWindow;
        friend class LvtkContainerElement;
        friend class LvtkRootElement;
        friend class LvtkDialog;
        friend class LvtkDropShadowElement;


    };


}