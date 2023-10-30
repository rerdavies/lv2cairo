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

#include <catch2/catch.hpp>
#include <string>
#include <sstream>
#include <locale>
#include "lv2c/JsonVariant.hpp"
#include "lv2c/JsonIo.hpp"
#include <iostream>

using namespace lv2c;

void SerializationTest(const json_variant &variant)
{
    std::stringstream output;
    output << variant;

    //std::cout << output.str() << std::endl;
    std::stringstream input(output.str());
    json_variant t;
    input >> t;

    REQUIRE(t == variant);
}

class JsonSerializable
{
public:
    void Read(json_variant &v)
    {
        value = v["value"].as_number();
    }
    void Write(json_variant &v) const
    {
        v = json_variant::object();
        v["value"] = value;
    }

    double value = 0.0;
};
TEST_CASE("JsonVariant, JsonIO test", "[json]")
{
    {
        json_variant v;
        REQUIRE(v.is_null() == true);
        v = true;
        REQUIRE(v.is_bool() == true);
        REQUIRE(v.as_bool() == true);
        REQUIRE(v.as<bool>() == true);

        v = 3.0;
        REQUIRE(v.as_number() == 3.0);
        REQUIRE(v.is_number() == true);
        v = 3;
        REQUIRE(v.is_number() == true);
        auto v1 = std::is_same_v<int, bool>;
        (void)v1;
        auto x = v.as<int>();
        (void)x;
        REQUIRE(v.as<int>() == 3);

        v = "abc";
        REQUIRE(v.is_number() == false);
        REQUIRE(v.is_string() == true);
        REQUIRE(v.as_string() == "abc");
        REQUIRE(v.as<std::string>() == "abc");

        {
            json_variant v = json_variant::array();
            REQUIRE(v.is_array() == true);
            v.resize(3);
            v[0] = 1;
            v[1] = "abc";
            v[2] = true;
            REQUIRE(v[0].is_number() == true);
            REQUIRE(v[1].is_string() == true);
            REQUIRE(v[2].is_bool() == true);

            json_array::ptr vp = v.as_array();
            REQUIRE(vp->at(0).is_number() == true);
            REQUIRE(vp->at(1).is_string() == true);
            REQUIRE(vp->at(2).is_bool() == true);

            SerializationTest(v);
        }
        {
            json_variant v = json_variant::object();
            REQUIRE(v.is_object() == true);
            v["a"] = 1;
            v["b"] = "abc";
            v["c"] = true;

            REQUIRE(v["a"].is_number() == true);
            REQUIRE(v["b"].is_string() == true);
            REQUIRE(v["c"].is_bool() == true);
            REQUIRE(v["d"].is_null() == true);

            json_object::ptr vp = v.as_object();
            REQUIRE(vp->at("a").is_number() == true);
            REQUIRE(vp->at("b").is_string() == true);
            REQUIRE(vp->at("c").is_bool() == true);
            REQUIRE(vp->at("d").is_null() == true);

            SerializationTest(v);

            auto t = json_variant::array();
            t.resize(2);
            t[1] = json_variant::object();
            v["b"] = t;
            v["b"][1]["a"] = json_variant::array();
            v["b"][1]["b"] = 99;
            v["b"][1]["c"] = json_variant::object();
            v["b"][1]["d"] = true;

            SerializationTest(v);
            {
                std::stringstream input;
                input << v;
                v["b"][1]["b"] = 100;

                std::stringstream output(input.str());

                json_variant t;
                output >> t;
                REQUIRE(t != v);
            }

            {
                enum class TEnum
                {
                    A,
                    B,
                    C
                };

                json_variant t;
                t = TEnum::B;

                REQUIRE(t.as<TEnum>() == TEnum::B);
            }
        }
    }
    {
        // jason as<array>.
        json_variant v;
        std::vector<int> intValues {1,2,3};
        v = intValues;

        std::vector<int> out = v.as<std::vector<int>>();
        REQUIRE(out == intValues);
    }
    {
        // check Json serialization.

        JsonSerializable serializable;
        serializable.value = 1;

        json_variant variant;
        variant = serializable;

        JsonSerializable serializableOut;

        serializableOut.Read(variant);
        

    }
}
