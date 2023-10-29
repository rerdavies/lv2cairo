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

#include "lv2c/Lv2cValueElement.hpp"

using namespace lvtk;

Lv2cValueElement::Lv2cValueElement()
{
    ValueProperty.SetElement(this, &Lv2cValueElement::OnValueChanged);
}

void Lv2cValueElement::OnValueChanged(double value)
{
}

Lv2cStereoValueElement::Lv2cStereoValueElement()
{
    RightValueProperty.SetElement(this, &Lv2cStereoValueElement::OnRightValueChanged);
}

void Lv2cStereoValueElement::OnRightValueChanged(double value)
{
}

bool Lv2cStereoValueElement::Stereo() const
{
    return stereo;
}

Lv2cStereoValueElement&Lv2cStereoValueElement::Stereo(bool value)
{
    stereo = value; return *this;
}