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

namespace lvtk
{

    /// @brief A base-class for storing custom user data.
    /// Extend this class with a custom class that
    /// provides data required by custom controls.
    ///
    /// The principle use of this object is storing
    /// theming data for custom controls other than
    /// colors and styles in the LvtkTheme::customUserData
    /// dictionary.
    ///
    /// CustomUserData objects can also be asssigned to
    /// the UserData property of LvtkElement objects.
    /// But, generally, a better approach is to extend a
    /// LvtkContainer element, and add the control
    /// you're trying to customize as a child element,
    ///  as this provides better life-time management.
    ///
    /// LvtkElements provide a UserData() property
    /// that allows custom data to be stored in a
    /// LvtkElement control. The pointer is released when
    /// the LvtkElement is unmounted.
    ///
    /// You must not store a std::shared_ptr<> to
    /// the LvtkElement to which you assign the
    /// custom user-data object, or to any parent of
    /// the LvtkElement, as this will cause a
    /// circular reference, and prevent memory from
    /// being deallocated.
    ///
    /// Assigning a CairUserData object to the UserData
    /// object while holding a shared_ptr elsewhere
    /// may cause challenging lifetime management issues.
    ///
    /// This is probably not the solution you're looking for.
    /// It's just easier to extend LvtkContainerElement.
    ///
    class LvtkUserData
    {
    protected:
        LvtkUserData() {}

    public:
        using self = LvtkUserData;
        virtual ~LvtkUserData() noexcept {}

        using ptr = std::shared_ptr<self>;
    };

}