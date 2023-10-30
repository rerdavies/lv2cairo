#include <catch2/catch.hpp>
#include "lv2c/Lv2cNumericEditBoxElement.hpp"
using namespace lv2c;

TEST_CASE( "NiceEditString test", "[nice_edit_string]" )   {
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(123456,Lv2cValueType::Uint16) == "123456");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(999.951,Lv2cValueType::Double) == "1000");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(199.95,Lv2cValueType::Double) == "200.0");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(12.3446,Lv2cValueType::Double) == "12.34");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(12.34556,Lv2cValueType::Double) == "12.35");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(9.876556,Lv2cValueType::Double) == "9.877");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(9.876056,Lv2cValueType::Double) == "9.876");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.9876056,Lv2cValueType::Double) == "0.9876");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.98765056,Lv2cValueType::Double) == "0.9877");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.09876056,Lv2cValueType::Double) == "0.09876");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.098765056,Lv2cValueType::Double) == "0.09877");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.009876056,Lv2cValueType::Double) == "0.009876");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.0098765056,Lv2cValueType::Double) == "0.009877");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.0009876056,Lv2cValueType::Double) == "0.0009876");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.00098765056,Lv2cValueType::Double) == "0.0009877");

    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(1234567,Lv2cValueType::Uint16) == "1234567");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(0.000098765056,Lv2cValueType::Double) == "9.8765e-05");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(12345678,Lv2cValueType::Uint16) == "12345678");
    REQUIRE( Lv2cNumericEditBoxElement::NiceEditText(123456789,Lv2cValueType::Uint16) == "123456789");
}
 