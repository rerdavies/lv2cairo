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

#include "Lv2ControlTestPage.hpp"

#include "lvtk_ui/Lv2PortView.hpp"
#include "lvtk/LvtkFlexGridElement.hpp"
#include "lvtk/LvtkDialElement.hpp"
#include "lv2/port-groups/port-groups.h"
#include "lvtk/LvtkTypographyElement.hpp"
#include "lvtk/LvtkGroupElement.hpp"
#include "lvtk/LvtkScrollContainerElement.hpp"

using namespace lvtk;
using namespace lvtk::ui;

LvtkElement::ptr Lv2ControlTestPage::CreatePageView(LvtkTheme::ptr theme)
{

    auto scroll = LvtkScrollContainerElement::Create();
    scroll->HorizontalScrollEnabled(false)
        .VerticalScrollEnabled(true);
        
    scroll->Style()
        .HorizontalAlignment(LvtkAlignment::Stretch)
        .VerticalAlignment(LvtkAlignment::Stretch)
//        .Background(theme->paper)
    ;
    {

        auto main = LvtkFlexGridElement::Create();
        main->Style()
            .Theme(theme)
            .Padding({16, 8, 16, 8})
            .Width(LvtkMeasurement::Percent(100))
            .Height(LvtkMeasurement::Percent(100))
            .FlexWrap(LvtkFlexWrap::Wrap)
            .FlexAlignItems(LvtkAlignment::Center)
            .FlexRowGap(16);

        {
            auto element = LvtkTypographyElement::Create();
            main->AddChild(element);
            element->Text("Input Port Controls").Variant(LvtkTypographyVariant::Heading);
            element->Style()
                .Width(LvtkMeasurement::Percent(100))
                .Margin({0, 16, 0, 16});
        }
        if (true)
        {
            {
                Lv2PortInfo portInfo;
                portInfo.name("VOLUME");
                portInfo.units(Lv2Units::db);
                portInfo.min_value(-96);
                portInfo.max_value(20);
                portInfo.default_value(0);
                std::vector<Lv2ScalePoint> scalePoints;
                scalePoints.push_back(Lv2ScalePoint(-96, "-INF"));
                portInfo.scale_points(scalePoints);
                portInfo.is_input(true);

                auto portView = CreatePortView(portInfo);
                main->AddChild(portView);
            }
            {
                Lv2PortInfo portInfo;
                portInfo.name("DELAY");
                portInfo.units(Lv2Units::ms);
                portInfo.min_value(0.1);
                portInfo.max_value(100);
                portInfo.default_value(10);
                portInfo.is_logarithmic(true);
                portInfo.is_input(true);

                auto portView = CreatePortView(portInfo);
                main->AddChild(portView);
            }
            {
                Lv2PortInfo portInfo;
                portInfo.name("INTEGER");
                portInfo.min_value(0);
                portInfo.max_value(127);
                portInfo.default_value(0);
                portInfo.integer_property(true);
                portInfo.is_input(true);

                auto portView = CreatePortView(portInfo);
                main->AddChild(portView);

                portInfo.integer_property(false);
            }
            {
                Lv2PortInfo portInfo;
                portInfo.name("TONE STACK");
                portInfo.min_value(0.0);
                portInfo.max_value(3);
                portInfo.default_value(0);
                portInfo.is_input(true);
                portInfo.enumeration_property(true);
                std::vector<Lv2ScalePoint> scalePoints;
                scalePoints.push_back(Lv2ScalePoint(0.0, "Fender"));
                scalePoints.push_back(Lv2ScalePoint(1.0, "JCM800"));
                scalePoints.push_back(Lv2ScalePoint(2.0, "Baxandall"));
                portInfo.scale_points(scalePoints);

                auto portView = CreatePortView(portInfo);
                main->AddChild(portView);
            }
            {

                Lv2PortInfo portInfo;
                portInfo.name("BYPASS");

                portInfo.name("BYPASS");
                portInfo.min_value(0.0);
                portInfo.max_value(1);
                portInfo.default_value(0);
                portInfo.is_input(true);
                portInfo.is_control_port(true);
                portInfo.toggled_property(true);

                auto portView = CreatePortView(portInfo);
                main->AddChild(portView);
            }
            {
                Lv2PortInfo portInfo;
                portInfo.name("CAB");
                portInfo.min_value(0.0);
                portInfo.max_value(1);
                portInfo.default_value(0);
                portInfo.is_input(true);
                portInfo.toggled_property(true);
                std::vector<Lv2ScalePoint> scalePoints;
                scalePoints.push_back(Lv2ScalePoint(0.0, "Bassman"));
                scalePoints.push_back(Lv2ScalePoint(1.0, "Marshall 4x4"));
                portInfo.scale_points(scalePoints);

                LvtkBindingProperty<double> t;

                auto portView = CreatePortView(t, portInfo);
                main->AddChild(portView);
            }

            auto outputPortLabel = LvtkTypographyElement::Create();
            outputPortLabel->Text("Output Port Controls").Variant(LvtkTypographyVariant::Heading);
            outputPortLabel->Style().Width(LvtkMeasurement::Percent(100)).Margin({0, 16, 0, 16});
            main->AddChild(outputPortLabel);

            Lv2PortInfo dialPortInfo;
            dialPortInfo.name("(VAL L)");
            dialPortInfo.min_value(0);
            dialPortInfo.max_value(1);
            dialPortInfo.default_value(0.6);
            dialPortInfo.is_input(true);

            auto dialL = CreatePortView(dialPortInfo);

            dialPortInfo.name("(VAL R)");
            auto dialR = CreatePortView(dialPortInfo);

            LvtkValueElement::ptr lampToggle;
            {
                Lv2PortInfo portInfo;
                portInfo.name("(Lamp)");
                portInfo.min_value(0);
                portInfo.max_value(1);
                portInfo.is_input(true);
                portInfo.is_control_port(true);
                portInfo.toggled_property(true);
                lampToggle = CreatePortView(portInfo);
            }

            {

                Lv2PortInfo portInfo;
                portInfo.name("IN L");
                portInfo.min_value(0);
                portInfo.max_value(1.00);
                portInfo.default_value(0.3);
                portInfo.is_input(false);
                portInfo.is_output(true);

                auto portView = CreatePortView(dialL->ValueProperty, portInfo);
                main->AddChild(portView);
                portInfo.name("IN R");
                portView = CreatePortView(dialR->ValueProperty, portInfo);
                main->AddChild(portView);

                // stereo VU.
                portInfo.name("Out");
                portInfo.designation(LV2_PORT_GROUPS__left);
                portView = CreateStereoPortView("Out",dialL->ValueProperty, dialR->ValueProperty,portInfo);
                main->AddChild(portView);

                portInfo.name("Lamp");
                portInfo.max_value(1.0);
                portInfo.integer_property(true);
                portInfo.designation("");
                portView = CreatePortView(lampToggle->ValueProperty, portInfo);
                main->AddChild(portView);
            }

            Lv2PortInfo dbDialPortInfo;
            dbDialPortInfo.name("(DB VAL L)");
            dbDialPortInfo.units(Lv2Units::db);
            dbDialPortInfo.min_value(-35);
            dbDialPortInfo.max_value(5);
            dbDialPortInfo.default_value(-10.0);
            dbDialPortInfo.is_input(true);
            auto dialLdb = CreatePortView(dbDialPortInfo);

            dbDialPortInfo.name("(DB VAL R)");
            auto dialRdb = CreatePortView(dbDialPortInfo);

            {

                Lv2PortInfo portInfo;
                portInfo.name("L");
                portInfo.min_value(-35);
                portInfo.max_value(5);
                portInfo.default_value(-10);
                portInfo.is_input(false);
                portInfo.is_output(true);
                portInfo.units(Lv2Units::db);

                auto portView = CreatePortView(portInfo);
                dialLdb->ValueProperty.Bind(portView->ValueProperty);
                main->AddChild(portView);

                portInfo.name("OUT");
                portInfo.designation(LV2_PORT_GROUPS__left);
                portView = CreateStereoPortView("OUT",dialLdb->ValueProperty,dialRdb->ValueProperty, portInfo);
                main->AddChild(portView);
            }
            LvtkValueElement::ptr statusDial;
            {
                Lv2PortInfo portInfo; 
                portInfo.name("STATUS");
                portInfo.min_value(0);
                portInfo.max_value(3);
                portInfo.default_value(0);
                portInfo.is_input(true);
                portInfo.integer_property(true);
                statusDial = CreatePortView(portInfo);
            }
            {
                Lv2PortInfo portInfo;
                portInfo.name("STATUS");
                portInfo.min_value(0);
                portInfo.max_value(3);
                portInfo.default_value(0);
                portInfo.is_output(true);
                portInfo.enumeration_property(true);
                std::vector<Lv2ScalePoint> scalePoints;
                scalePoints.push_back(Lv2ScalePoint(0, "Idle"));
                scalePoints.push_back(Lv2ScalePoint(1, "Loading"));
                scalePoints.push_back(Lv2ScalePoint(2, "Ready"));
                scalePoints.push_back(Lv2ScalePoint(3, "Error"));
                portInfo.scale_points(scalePoints);

                auto portView = CreatePortView(statusDial->ValueProperty, portInfo);
                main->AddChild(portView);
            }

            {
                auto portGroup = LvtkGroupElement::Create();
                portGroup->Text("Port Group");
                Lv2PortInfo portInfo;
                portInfo.name("BASS");
                portInfo.min_value(0);
                portInfo.max_value(10);
                portInfo.default_value(5);
                portInfo.is_input(true);

                portGroup->AddChild(CreatePortView(portInfo));

                portInfo.name("MID");
                portGroup->AddChild(CreatePortView(portInfo));

                portInfo.name("TREBLE");
                portGroup->AddChild(CreatePortView(portInfo));

                auto frame = LvtkContainerElement::Create();
                // frame->Style().Background(LvtkColor(0.5,1,0.5,0.2));
                frame->AddChild(portGroup);
                main->AddChild(frame);
            }
            {
                auto div = LvtkElement::Create();
                div->Style()
                    .Width(LvtkMeasurement::Percent(100))
                    .Height(1);
                main->AddChild(div);
            }

            {
                auto portGroup = LvtkGroupElement::Create();
                portGroup->Text("VU Values");
                main->AddChild(portGroup);

                {

                    dialL->Value(0.3);
                    portGroup->AddChild(dialL);
                    dialR->Value(0.8);
                    portGroup->AddChild(dialR);

                    lampToggle->Value(1);
                    portGroup->AddChild(lampToggle);

                    dialLdb->Value(3.0);
                    dialRdb->Value(-14);
                    portGroup->AddChild(dialLdb);
                    portGroup->AddChild(dialRdb);

                    portGroup->AddChild(statusDial);
                }
            }
        }
        scroll->Child(main);
    }
    return scroll;
}
