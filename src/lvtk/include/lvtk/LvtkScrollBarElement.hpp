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

#include "LvtkContainerElement.hpp"
#include "LvtkBindingProperty.hpp"

namespace lvtk
{

    enum class LvtkScrollBarOrientation {
        Horizontal,
        Vertical
    };

    class LvtkScrollBarElement: public LvtkContainerElement {
    public:
        using self = LvtkScrollBarElement;
        using super = LvtkContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }

        LvtkScrollBarElement();

        BINDING_PROPERTY(Orientation,LvtkScrollBarOrientation, LvtkScrollBarOrientation::Vertical)
        BINDING_PROPERTY(WindowSize, double,0)
        BINDING_PROPERTY(DocumentSize, double,0)
        BINDING_PROPERTY(ScrollOffset, double, 0)
        BINDING_PROPERTY_REF(ThumbHoverColors, LvtkHoverColors, LvtkHoverColors())
        BINDING_PROPERTY_REF(TrackHoverColors, LvtkHoverColors,  LvtkHoverColors())


    protected:
        // Hook for derived classes to override the default collapsed appearance without modifying the theme.
        BINDING_PROPERTY(CollapsedStyle, LvtkStyle::ptr,  nullptr)

        LvtkStyle* GetCollapsedStyle() const;
        virtual void OnMount() override;
        virtual void OnUnmount() override;

        bool isMouseDragging = false;
        bool mouseDown = false;
        LvtkPoint mouseDownPoint;
        double mouseDownOffset = 0;
        LvtkRectangle GetThumbHitRect() const;
        LvtkRectangle GetTrackRectangle() const;

        virtual bool OnMouseDown(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseUp(LvtkMouseEventArgs &event) override;
        virtual bool OnMouseMove(LvtkMouseEventArgs &event) override;

        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context) override
        {
            return super::MeasureClient(clientConstraint,clientAvailable,context);
        }

        virtual bool OnMouseOver(LvtkMouseOverEventArgs &event) override;
        virtual bool OnMouseOut(LvtkMouseOverEventArgs &event) override;

        virtual void OnDraw(LvtkDrawingContext &dc) override;
        virtual void OnHoverStateChanged(LvtkHoverState hoverState) override;

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

    /// @brief Convenience wrapper for vertical LvtkScrollBarElement.
    class LvtkVerticalScrollBarElement: public LvtkScrollBarElement
    {
    public:
        using self = LvtkVerticalScrollBarElement;
        using super = LvtkScrollBarElement;
        using ptr = std::shared_ptr<LvtkVerticalScrollBarElement>;
        static ptr Create() { return std::make_shared<LvtkVerticalScrollBarElement>(); }

        LvtkVerticalScrollBarElement() {
            Orientation(LvtkScrollBarOrientation::Vertical);
        }
    };

        /// @brief Convenience wrapper for horizontal LvtkScrollBarElement.
    class LvtkHorizontalScrollBarElement: public LvtkScrollBarElement
    {
    public:
        using self = LvtkHorizontalScrollBarElement;
        using super = LvtkScrollBarElement;
        using ptr = std::shared_ptr<LvtkHorizontalScrollBarElement>;
        static ptr Create() { return std::make_shared<LvtkHorizontalScrollBarElement>(); }

        LvtkHorizontalScrollBarElement() {
            Orientation(LvtkScrollBarOrientation::Horizontal);
        }


    };

}
