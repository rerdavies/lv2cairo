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

#include "lv2c/Lv2cBindingProperty.hpp"
#include "lv2c/Lv2cObject.hpp"
#include <atomic>

using namespace lv2c;

std::atomic<uint64_t> gNextHandle;

// explicit specializations of bindings..
namespace lv2c
{
    template class Observable<double>;
    template class Lv2cBindingProperty<double>;
    template class Observable<float>;
    template class Lv2cBindingProperty<float>;
    template class Observable<bool>;
    template class Lv2cBindingProperty<bool>;

    template class Observable<std::string>;
    template class Lv2cBindingProperty<std::string>;

}

/*static*/
MapRangeBindingTransform::ptr MapRangeBindingTransform::Create(double sourceMinVal, double sourceMaxVal, double targetMinVal, double targetMaxVal)
{
    return std::make_shared<self>(sourceMinVal, sourceMaxVal, targetMinVal, targetMaxVal);
}

double MapRangeBindingTransform::Forward(double value)
{
    return (value-sourceMinVal)*(targetMaxVal-targetMinVal)/(sourceMaxVal-sourceMinVal)+targetMinVal;
}
double MapRangeBindingTransform::Reverse(double value)
{
    return (value-targetMinVal)*(sourceMaxVal-sourceMinVal)/(targetMaxVal-targetMinVal)+sourceMinVal;
}

MapRangeBindingTransform::MapRangeBindingTransform(double sourceMinVal, double sourceMaxVal, double targetMinVal, double targetMaxVal)
: sourceMinVal(sourceMinVal)
, sourceMaxVal(sourceMaxVal)
, targetMinVal(targetMinVal)
, targetMaxVal(targetMaxVal)
{
}

uint64_t lv2c::implementation::nextHandle()
{
    return ++gNextHandle;
}

uint64_t lv2c::implementation::handleCount = 0;
uint64_t lv2c::implementation::bindingRecordCount = 0;
uint64_t lv2c::implementation::observerLinkCount = 0;

namespace lv2c::implementation
{
    ObserverLink::ObserverLink()
    {
        ++observerLinkCount; // test use only
    }
    ObserverLink::~ObserverLink()
    {
        --observerLinkCount; // test use only
    }

    void ObserverLink::ObserverDeleted()
    {
        observerValid = false;
        if (!observableValid)
        {
            delete this;
        }
        else
        {
            OnObserverDeleted();
        }
    }
    void ObserverLink::ObservableDeleted()
    {
        observableValid = false;
        if (!observerValid)
        {
            delete this;
        }
    }

    // movable, not copyable.
    ObserverHandle::ObserverHandle() : link(nullptr) {}

    ObserverHandle::ObserverHandle(ObserverLink *link) { this->link = link; }
    ObserverHandle::ObserverHandle(ObserverHandle &&other)
    {
        this->link = other.link;
        other.link = nullptr;
    }
    ObserverHandle &ObserverHandle::operator=(ObserverHandle &&other)
    {
        std::swap(link, other.link);
        return *this;
    }
    ObserverHandle::~ObserverHandle()
    {
        Release();
    }
    void ObserverHandle::Release()
    {
        if (link)
        {
            link->ObserverDeleted();
            link = nullptr;
        }
    }

} // namespace lv2c::implementation
