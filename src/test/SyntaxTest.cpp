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

// A scratch file to test ideas for style syntax.

#include <vector>
#include <string>
#include "lvtk/LvtkTypes.hpp"
#include "lvtk/LvtkElement.hpp"
#include <iostream>
#include <type_traits>
#include <concepts>
using namespace std;

using namespace lvtk;

namespace test
{

    void MeasurementConversion(const LvtkThicknessMeasurement &v)
    {
    }

    void TestEventSyntax()
    {
        using MouseEvent = LvtkEvent<LvtkMouseEventArgs>;

        MouseEvent e;

        MouseEvent::EventListener handler =
            [](const LvtkMouseEventArgs &e) -> bool
        {
            cout << "Event received." << endl;
            return true;
        };

        EventHandle h = e.AddListener(handler);
        LvtkMouseEventArgs args;
        e.Fire(args);
        if (!e.RemoveListener(h))
        {
            throw std::runtime_error("Failed to remove");
        }
    }
    void TestSyntax()
    {

#pragma GCC diagnostic ignored "-Wunused-variable"
        struct T {
            int a;
            int b;
        };

        T x { .a=0,.b=0};

        x = T{.a=0,.b=0};

        LvtkThicknessMeasurement test{1};
        MeasurementConversion(4);

        LvtkThicknessMeasurement test2{4};
        (void)test2;

        auto style = LvtkStyle().Margin(4.0).Margin(4).BorderWidth(1);
    }

}


template <typename T> struct ArgumentTraits {
    using arg_t = const T &;
    using value_t = T;
};

template <typename T> requires 
    std::integral<T> 
    || std::floating_point<T>
    || std::is_enum_v<T>
struct ArgumentTraits<T> 
{
    using arg_t = T;
    using value_t = T;
};

template <typename T> requires std::is_pointer_v<T> 
struct ArgumentTraits<T> 
{
    using arg_t = T;
    using value_t = T;
};

static_assert( std::same_as< int,int>);

static_assert( std::same_as< ArgumentTraits<std::string>::arg_t,const std::string&>);
static_assert( std::same_as< ArgumentTraits<int>::arg_t,int>);
static_assert( std::same_as< ArgumentTraits<uint64_t>::arg_t,uint64_t>);
static_assert( std::same_as< ArgumentTraits<bool>::arg_t,bool>);
static_assert( std::same_as< ArgumentTraits<float>::arg_t,float>);
static_assert( std::same_as< ArgumentTraits<double>::arg_t,double>);


using X = ArgumentTraits<int*>::arg_t;

enum class EnumType {
    A,
    B,
    C

};
static_assert( std::same_as< ArgumentTraits<int*>::arg_t, int*>);
static_assert( std::same_as< ArgumentTraits<int*>::arg_t, int*>);


enum class AnEnumType {
    A,
    B,
    C
};

static_assert( std::same_as< ArgumentTraits<AnEnumType>::arg_t, AnEnumType>);
// static_assert( std::same_as< ArgumentTraits<int>::arg_t,int>::value);
// static_assert( std::same_as< ArgumentTraits<int*>::arg_t,int>::value);
