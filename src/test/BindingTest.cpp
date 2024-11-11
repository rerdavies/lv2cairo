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

#include "CatchTest.hpp"
#include "lv2c/Lv2cBindingProperty.hpp"
#include "lv2c/Lv2cElement.hpp"
#include <iostream>
#include <stdexcept>
#include <concepts>

using namespace std;
using namespace lv2c;


static_assert(std::same_as<Lv2cArgumentTraits<double>::arg_t, double>);

class TestElement : public Lv2cElement
{
public:
    TestElement()
    {
        ValueProperty.SetElement(this, &TestElement::OnValueChanged);
        AlignmentProperty.SetElement(this, &TestElement::OnAlignmentChanged);
        observerHandle = ValueProperty.addObserver(
            [this](const double &value)
            {
                observerFired = true;
            });
    }

    bool observerFired = false;
    observer_handle_t observerHandle;

    Lv2cBindingProperty<double> ValueProperty{0.0};

    bool valueChanged = false;
    void OnValueChanged(double value)
    {
        valueChanged = true;
    }
    Lv2cBindingProperty<Lv2cAlignment> AlignmentProperty;
    bool alignmentChanged = false;
    void OnAlignmentChanged(Lv2cAlignment value)
    {
        REQUIRE(value == AlignmentProperty.get());
        alignmentChanged = true;
    }
};

void ElementBindingTest()
{
    TestElement testElement;
    REQUIRE(testElement.valueChanged == false);
    testElement.ValueProperty.set(9.0);
    REQUIRE(testElement.valueChanged == true);
    REQUIRE(testElement.observerFired == true);

    testElement.valueChanged = false;
    testElement.ValueProperty.set(9.0);
    REQUIRE(testElement.valueChanged == false);

    testElement.ValueProperty.set(1.0);
    REQUIRE(testElement.valueChanged == true);

    testElement.AlignmentProperty.set(Lv2cAlignment::Center);
    REQUIRE(testElement.alignmentChanged == true);

    // test for movabilty.
    {
        TestElement movedElement;
    }
}

void CheckForLeaks()
{
    REQUIRE(lv2c::implementation::bindingRecordCount == 0);
    REQUIRE(lv2c::implementation::bindingRecordCount == 0);
    REQUIRE(lv2c::implementation::observerLinkCount == 0);
    REQUIRE(lv2c::implementation::handleCount == 0);
}

void BindingTest()
{
    // deleting an observer_handle_t removes the observer.
    {
        Observable<double> observable;
        {
            int nCallbacks = 0;
            ObserverCallback<double> callback =
                [&nCallbacks](double value) mutable
            {
                ++nCallbacks;
            };
            ObserverCallback<double> t = callback;

            observer_handle_t h = observable.addObserver(
                callback);

            observable.set(3.1);
            REQUIRE(nCallbacks == 1);
            REQUIRE(observable.observerCount() == 1);

            // observable.set(3.0);
        }
        REQUIRE(observable.observerCount() == 0);
    }
    CheckForLeaks();

    // deleting an Binding removes the bindings and observers.
    {
        Lv2cBindingProperty<double> source;
        {
            Lv2cBindingProperty<double> target;
            source.Bind(target);
            source.set(1);
            REQUIRE(target.get() == 1);

            target.set(2);
            REQUIRE(source.get() == 2);

            REQUIRE(source.observerCount() == 1);
            REQUIRE(source.bindingCount() == 1);
            REQUIRE(target.observerCount() == 1);
            REQUIRE(target.bindingCount() == 1);
            REQUIRE(implementation::bindingRecordCount == 1);
            REQUIRE(implementation::observerLinkCount == 2);
        }
        REQUIRE(source.observerCount() == 0);
        REQUIRE(source.bindingCount() == 0);
    }
    CheckForLeaks();

    // Binding target destroyed before Binding source.
    {
        Lv2cBindingProperty<double> target;
        {
            Lv2cBindingProperty<double> source;
            source.Bind(target);
            source.set(1);
            REQUIRE(target.get() == 1);

            target.set(2);
            REQUIRE(source.get() == 2);

            REQUIRE(source.observerCount() == 1);
            REQUIRE(source.bindingCount() == 1);
            REQUIRE(target.observerCount() == 1);
            REQUIRE(target.bindingCount() == 1);
        }
        CheckForLeaks();
    }

    // Observer destroyed before observable.
    {
        Observable<double> observable{1.0};

        double outputValue = -1;
        {
            observer_handle_t handle = observable.addObserver(
                [&outputValue](double value) mutable
                {
                    outputValue = value;
                });
            observable.set(2.0);
            REQUIRE(outputValue == 2.0);
            REQUIRE(observable.observerCount() == 1);
        }
        REQUIRE(observable.observerCount() == 0);
        observable.set(3.0);
        REQUIRE(outputValue == 2.0);
    }
    CheckForLeaks();

    // Observable destroyed before Observer
    {

        observer_handle_t handle;
        double outputValue = -1;
        {
            Observable<double> observable{1.0};
            handle = observable.addObserver(
                [&outputValue](double value) mutable
                {
                    outputValue = value;
                });
            observable.set(2.0);
            REQUIRE(outputValue == 2.0);
            REQUIRE(observable.observerCount() == 1);
        }
        REQUIRE(implementation::observerLinkCount == 1);
    }
    REQUIRE(implementation::observerLinkCount == 0);

    CheckForLeaks();

    ElementBindingTest();
}
TEST_CASE("Lv2cBindingProperty test", "[binding_properties]")
{

    BindingTest();
}
