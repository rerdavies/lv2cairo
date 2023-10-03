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
#include "LvtkValueElement.hpp"
#include <memory>
#include <string>


namespace lvtk
{
    /// @brief Renders a PNG strip file as an animation.
    /// Takes an png file consisting of a NxM tiles horizontally arranged. 
    /// The Value property controls which tile will be displayed. Value 
    /// ranges from [0..1].

    class LvtkPngStripElement: public LvtkValueElement
    {
    public:
        using ptr = std::shared_ptr<LvtkPngStripElement>;
        using super=LvtkValueElement;
        using self=LvtkPngStripElement;
        static ptr Create() { return std::make_shared<LvtkPngStripElement>(); }

        LvtkPngStripElement();

        LvtkPngStripElement&Value(double value) { ValueProperty.set(value);return *this; }
        double Value() { return ValueProperty.get(); }

        /// @brief The source path for the PNG strip file.
        BINDING_PROPERTY_REF(Source,std::string,"")

        /// @brief The size of tiles within the PNG file.
        /// If set to default, LvtkPngStripElement will assume that the strip 
        /// consists of (height x height) tiles.
        BINDING_PROPERTY_REF(TileSize,LvtkRectangle,LvtkRectangle(0,0,0,0))

    protected:
        virtual void OnValueChanged(double value) override;
        virtual void OnMount() override;
        virtual void OnDraw(LvtkDrawingContext &dc) override;
        virtual LvtkSize MeasureClient(LvtkSize clientConstraint, LvtkSize clientAvailable,LvtkDrawingContext&context) override;

    private:
        bool WillDraw() const override { return true; }
        bool sourceChanged = false;
        void OnSourceChanged(const std::string&source);
        void Load();

        int tileCount = 0;
        LvtkRectangle tileSize;
        LvtkSurface surface;

    };
} // namespace