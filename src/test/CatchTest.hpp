
#ifndef CATCH2_VERSION
#define CATCH2_VERSION 3
#endif

#if (CATCH2_VERSION == 3)
#include <catch2/catch_test_macros.hpp>
#elif CATCH2_VERSION == 2)
#include <catch2/catch.hpp>
#else
#include <catch2/catch_test_macros.hpp>
#endif
#