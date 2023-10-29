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
namespace lvtk {


    /// @brief The common base class of value controls.
    /// A value control provides a Value property that takes values in the range [0..1].
    ///
    /// It provides a common base class for components that are intented to display
    /// LV2 UI port values: dial controllers, VU meters, &c.
    /// 
    /// Note that base classes must provide getter/setter methods, since the return 
    /// type of a standard getter must return a reference to the outermost class.

    class Lv2cValueElement: public Lv2cContainerElement {
    public:
        using self = Lv2cValueElement;
        using super = Lv2cContainerElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        Lv2cValueElement();
        virtual ~Lv2cValueElement() { }

        Lv2cBindingProperty<double> ValueProperty;
        self&Value(double value) { ValueProperty.set(value); return *this; }
        double Value() { return ValueProperty.get(); }

    protected:
        virtual void OnValueChanged(double value);
    };

    /// @brief A base class for elements that have mono or stereo values.
    /// Lv2cStereoValueElement provides a common base class for components that 
    /// may have stereo values.
    ///
    /// The  return value of the Stereo() method indicates whether the 
    /// component is mono or stereo. If Stereo() returns false, the RightValue
    /// will not be connected.
    ///
    ///  The value properties are ValueProperty (the left value), and RightValueProperty
    /// 
    /// Note that base classes must provide getter/setter methods, since the return 
    /// type of a standard getter must return a reference to the outermost class.

    class Lv2cStereoValueElement: public Lv2cValueElement {
    public:
        using self = Lv2cStereoValueElement;
        using super = Lv2cValueElement;
        using ptr = std::shared_ptr<self>;
        static ptr Create() { return std::make_shared<self>(); }
        Lv2cStereoValueElement();
        virtual ~Lv2cStereoValueElement() { }

        BINDING_PROPERTY(RightValue,double,0.0)
        bool Stereo() const;
    protected:
        Lv2cStereoValueElement&Stereo(bool value);

        virtual void OnRightValueChanged(double value);
    private:
        bool stereo;
    };
}
