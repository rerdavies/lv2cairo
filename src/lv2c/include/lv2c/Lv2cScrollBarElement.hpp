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

#include "Lv2cContainerElement.hpp"
#include "Lv2cBindingProperty.hpp"

namespace lvtk
{

    enum class Lv2cScrollBarOrientation {
        Horizontal,
        Vertical
    };

    class Lv2cScrollBarElement: public Lv2cContainerElement {
    public:
        using self = Lv2cScrollBarElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        Lv2cScrollBarElement();

        BINDING_PROPERTY(Orientation,Lv2cScrollBarOrientation, Lv2cScrollBarOrientation::Vertical)
        BINDING_PROPERTY(WindowSize, double,0)
        BINDING_PROPERTY(DocumentSize, double,0)
        BINDING_PROPERTY(ScrollOffset, double, 0)
        BINDING_PROPERTY_REF(ThumbHoverColors, Lv2cHoverColors, Lv2cHoverColors())
        BINDING_PROPERTY_REF(TrackHoverColors, Lv2cHoverColors,  Lv2cHoverColors())


    protected:
        // Hook for derived classes to override the default collapsed appearance without modifying the theme.
        BINDING_PROPERTY(CollapsedStyle, Lv2cStyle::ptr,  nullptr)

        Lv2cStyle* GetCollapsedStyle() const;
        virtual void OnMount() override;
        virtual void OnUnmount() override;

        bool isMouseDragging = false;
        bool mouseDown = false;
        Lv2cPoint mouseDownPoint;
        double mouseDownOffset = 0;
        Lv2cRectangle GetThumbHitRect() const;
        Lv2cRectangle GetTrackRectangle() const;

        virtual bool OnMouseDown(Lv2cMouseEventArgs &event) override;
        virtual bool OnMouseUp(Lv2cMouseEventArgs &event) override;
        virtual bool OnMouseMove(Lv2cMouseEventArgs &event) override;

        virtual Lv2cSize MeasureClient(Lv2cSize clientConstraint, Lv2cSize clientAvailable,Lv2cDrawingContext&context) override
        {
            return super::MeasureClient(clientConstraint,clientAvailable,context);
        }

        virtual bool OnMouseOver(Lv2cMouseOverEventArgs &event) override;
        virtual bool OnMouseOut(Lv2cMouseOverEventArgs &event) override;

        virtual void OnDraw(Lv2cDrawingContext &dc) override;
        virtual void OnHoverStateChanged(Lv2cHoverState hoverState) override;

        virtual  bool WillDraw() const override;
    private:
        AnimationHandle animationHandle;
        animation_clock_time_point_t lastAnimationTime;
        double animationHoldTime = 0;
        double animationOverscroll = 0;
        
        double ComputeFlubberFactor(double overscroll);

        void AnimationTick(const animation_clock_time_point_t&now);
        void StartAnimation(double targetValue);
        void StopAnimation();
        BINDING_PROPERTY(AnimationValue,double, 0.0);
        BINDING_PROPERTY(OverscrollAmount,double, 0.0);
        BINDING_PROPERTY(FlubberFactor,double, 1.0);
        double animationTarget = 0;
    };

    /// @brief Convenience wrapper for vertical Lv2cScrollBarElement.
    class Lv2cVerticalScrollBarElement: public Lv2cScrollBarElement
    {
    public:
        using self = Lv2cVerticalScrollBarElement;
        using super = Lv2cScrollBarElement;
        using ptr = std::shared_ptr<Lv2cVerticalScrollBarElement>;
        static ptr Create() { return std::make_shared<Lv2cVerticalScrollBarElement>(); }

        Lv2cVerticalScrollBarElement() {
            Orientation(Lv2cScrollBarOrientation::Vertical);
        }
    };

        /// @brief Convenience wrapper for horizontal Lv2cScrollBarElement.
    class Lv2cHorizontalScrollBarElement: public Lv2cScrollBarElement
    {
    public:
        using self = Lv2cHorizontalScrollBarElement;
        using super = Lv2cScrollBarElement;
        using ptr = std::shared_ptr<Lv2cHorizontalScrollBarElement>;
        static ptr Create() { return std::make_shared<Lv2cHorizontalScrollBarElement>(); }

        Lv2cHorizontalScrollBarElement() {
            Orientation(Lv2cScrollBarOrientation::Horizontal);
        }


    };

}
