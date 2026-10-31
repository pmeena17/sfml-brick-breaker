#pragma once

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// A deliberately tiny test framework so the project builds and tests with NO third-party download.
// The macros mirror Catch2/doctest/GoogleTest (TEST_CASE, CHECK, REQUIRE), so migrating later is mostly a
// search-and-replace of the include line.
//
//   TEST_CASE("name") { ... }     registers a test
//   CHECK(cond)                   records a failure and keeps going
//   REQUIRE(cond)                 records a failure and aborts the current test
//   CHECK_EQ(a, b)                equality with both values printed on failure (needs operator<<)
//   CHECK_NEAR(a, b, tolerance)   floating-point comparison
//   CHECK_THROWS_AS(expr, Type)   expects `expr` to throw `Type`
namespace bb::test
{
struct TestCase
{
    const char* name;
    void (*function)();
};

inline std::vector<TestCase>& Registry()
{
    static std::vector<TestCase> registry;
    return registry;
}

inline int& FailureCount()
{
    static int failures = 0;
    return failures;
}

// C++ Concept: Static registration through a constructor
// TEST_CASE defines a global `Registrar` object; its constructor runs before main() and adds the test to the
// registry. That is how test frameworks "discover" tests without a hand-maintained list.
struct Registrar
{
    Registrar(const char* name, void (*function)())
    {
        Registry().push_back({name, function});
    }
};

struct AbortTest
{
};

inline void ReportFailure(const char* file, int line, const std::string& message)
{
    ++FailureCount();
    std::cerr << file << ":" << line << ": FAILED: " << message << "\n";
}

template <typename A, typename B>
void CheckEqual(const A& actual, const B& expected, const char* actualText, const char* expectedText, const char* file, int line)
{
    if (!(actual == expected))
    {
        std::ostringstream message;
        message << actualText << " == " << expectedText << "  (actual: " << actual << ", expected: " << expected << ")";
        ReportFailure(file, line, message.str());
    }
}

inline void CheckNear(double actual, double expected, double tolerance, const char* actualText, const char* expectedText, const char* file, int line)
{
    if (!(std::abs(actual - expected) <= tolerance))
    {
        std::ostringstream message;
        message << actualText << " ~= " << expectedText << "  (actual: " << actual << ", expected: " << expected << ", tolerance: " << tolerance << ")";
        ReportFailure(file, line, message.str());
    }
}
} // namespace bb::test

#define BB_CONCAT_INNER(a, b) a##b
#define BB_CONCAT(a, b) BB_CONCAT_INNER(a, b)

// __COUNTER__ (supported by GCC, Clang and MSVC) gives every test a unique function name even when two tests are
// declared on the same line or MSVC's /ZI debug mode makes __LINE__ unusable for token pasting.
#define BB_TEST_CASE_IMPL(testName, functionName) static void functionName(); static const bb::test::Registrar BB_CONCAT(functionName, _registrar)(testName, &functionName); static void functionName()

#define TEST_CASE(testName) BB_TEST_CASE_IMPL(testName, BB_CONCAT(TestBody_, __COUNTER__))

#define CHECK(condition) do { if (!(condition)) { bb::test::ReportFailure(__FILE__, __LINE__, #condition); } } while (false)

#define REQUIRE(condition) do { if (!(condition)) { bb::test::ReportFailure(__FILE__, __LINE__, #condition); throw bb::test::AbortTest{}; } } while (false)

#define CHECK_EQ(actual, expected) bb::test::CheckEqual((actual), (expected), #actual, #expected, __FILE__, __LINE__)

#define CHECK_NEAR(actual, expected, tolerance) bb::test::CheckNear(static_cast<double>(actual), static_cast<double>(expected), static_cast<double>(tolerance), #actual, #expected, __FILE__, __LINE__)

#define CHECK_THROWS_AS(expression, ExceptionType) do { bool bbThrown = false; try { (void)(expression); } catch (const ExceptionType&) { bbThrown = true; } catch (...) { } if (!bbThrown) { bb::test::ReportFailure(__FILE__, __LINE__, #expression " did not throw " #ExceptionType); } } while (false)
