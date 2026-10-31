#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>

#include "app/Application.hpp"
#include "app/AssetLocator.hpp"

int main(int argc, char** argv)
{
    // C++ Concept: Exceptions end at main() - one top-level handler
    // Anything that throws below (missing font, window creation failure, ...) unwinds the stack, running every
    // destructor on the way (so the window and files are closed properly), and lands here, where it becomes a
    // readable message and a non-zero exit code instead of a crash.
    try
    {
        const std::filesystem::path executable = (argc > 0 && argv[0] != nullptr) ? std::filesystem::path(argv[0]) : std::filesystem::path();

        const std::optional<bb::app::AssetPaths> assets = bb::app::LocateAssets(executable);
        if (!assets)
        {
            std::cerr << "Cannot find the 'assets' folder (it must contain a 'levels' sub-folder).\n";
            std::cerr << "Put it next to the executable, run from the project directory, or set BRICK_BREAKER_ASSETS.\n";
            return EXIT_FAILURE;
        }

        bb::app::Application application(*assets);
        return application.Run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Fatal error: " << exception.what() << '\n';
        return EXIT_FAILURE;
    }
}
