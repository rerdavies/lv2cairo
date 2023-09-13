#include <catch2/catch.hpp>
#include "lvtk/LvtkNumericEditBoxElement.hpp"
using namespace lvtk;

TEST_CASE( "NiceEditString test", "[nice_edit_string]" )   {
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(123456,LvtkValueType::Uint16) == "123456");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(999.951,LvtkValueType::Double) == "1000");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(199.95,LvtkValueType::Double) == "200.0");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(12.3446,LvtkValueType::Double) == "12.34");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(12.34556,LvtkValueType::Double) == "12.35");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(9.876556,LvtkValueType::Double) == "9.877");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(9.876056,LvtkValueType::Double) == "9.876");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.9876056,LvtkValueType::Double) == "0.9876");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.98765056,LvtkValueType::Double) == "0.9877");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.09876056,LvtkValueType::Double) == "0.09876");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.098765056,LvtkValueType::Double) == "0.09877");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.009876056,LvtkValueType::Double) == "0.009876");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.0098765056,LvtkValueType::Double) == "0.009877");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.0009876056,LvtkValueType::Double) == "0.0009876");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.00098765056,LvtkValueType::Double) == "0.0009877");

    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(1234567,LvtkValueType::Uint16) == "1234567");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(0.000098765056,LvtkValueType::Double) == "9.8765e-05");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(12345678,LvtkValueType::Uint16) == "12345678");
    REQUIRE( LvtkNumericEditBoxElement::NiceEditText(123456789,LvtkValueType::Uint16) == "123456789");
}
 