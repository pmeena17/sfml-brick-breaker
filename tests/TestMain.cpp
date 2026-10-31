#include <cstdlib>
#include <exception>
#include <iostream>

#include "TestFramework.hpp"

int main()
{
    int executed = 0;
    for (const bb::test::TestCase& test : bb::test::Registry())
    {
        const int failuresBefore = bb::test::FailureCount();
        try
        {
            test.function();
        }
        catch (const bb::test::AbortTest&)
        {
            // REQUIRE already reported the failure.
        }
        catch (const std::exception& exception)
        {
            bb::test::ReportFailure(test.name, 0, std::string("unexpected exception: ") + exception.what());
        }

        ++executed;
        std::cout << (bb::test::FailureCount() == failuresBefore ? "[ PASS ] " : "[ FAIL ] ") << test.name << "\n";
    }

    std::cout << "\n" << executed << " test(s), " << bb::test::FailureCount() << " failure(s)\n";
    return bb::test::FailureCount() == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
