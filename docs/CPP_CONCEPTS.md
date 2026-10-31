# C++ concept index

Every place in the code base that uses a modern C++ concept carries a comment starting with `C++ Concept:` that explains
what it is, why it is used there and what to watch out for. This index is generated from those comments
(`grep -rn "C++ Concept:" src tests` gives the same list with line numbers).


## `src/app/Application.cpp`

- A fixed time-step game loop ("Fix Your Timestep")

## `src/app/Application.hpp`

- Deleting copy and move operations on an object that other objects point into
- Member initialisation order = declaration order

## `src/app/AssetLocator.cpp`

- Conditional compilation driven by the build system

## `src/app/FontLocator.cpp`

- Platform differences behind a preprocessor switch, hidden inside one function

## `src/app/SfmlInput.cpp`

- SFML 3's event model is itself built on std::variant / std::optional

## `src/app/SfmlRenderer.hpp`

- Holding references to objects you do not own

## `src/app/main.cpp`

- Exceptions end at main() - one top-level handler

## `src/core/Ball.hpp`

- Member initializer lists

## `src/core/Brick.hpp`

- class vs struct, invariants and encapsulation
- Constructors with default arguments and std::optional parameters

## `src/core/Collision.cpp`

- Anonymous namespaces (internal linkage)

## `src/core/Config.hpp`

- `inline constexpr` variables (C++17) instead of #define or `static const`
- static_assert - compile-time sanity checks

## `src/core/EffectTracker.cpp`

- Returning containers by value (move semantics / copy elision)
- std::ranges algorithms and lambdas (C++20)

## `src/core/GameEvents.hpp`

- std::variant - a type-safe union ("sum type") - and `using` type aliases

## `src/core/GameSession.cpp`

- `if` with an initializer (C++17)
- std::erase_if (C++20) and lambda captures

## `src/core/GameSession.hpp`

- `explicit` constructors

## `src/core/LevelParser.cpp`

- [[noreturn]]
- Constrained templates with `concepts` (C++20), std::from_chars and structured bindings
- std::string_view as a non-owning "window" into text you already have

## `src/core/LevelParser.hpp`

- Custom exception types derived from std::runtime_error

## `src/core/LevelRepository.cpp`

- std::filesystem (C++17) and the std::error_code overloads
- std::ranges::sort - sorting without iterator pairs
- Sink parameters - pass by value, then std::move

## `src/core/LevelRepository.hpp`

- The Rule of Zero
- std::span (C++20) - a non-owning view over contiguous elements

## `src/core/Paddle.hpp`

- std::clamp (C++17) and assert

## `src/core/PowerUp.hpp`

- Scoped enumerations (`enum class`)
- Class template argument deduction (CTAD, C++17), std::array and std::string_view
- std::optional - "a value or nothing"

## `src/core/Rect.hpp`

- Designated initializers (C++20)
- Named constructors (static factory functions)

## `src/core/Vec2.hpp`

- Nested namespace definitions (C++17)
- Aggregates, default member initializers and value types
- constexpr, noexcept and [[nodiscard]]
- Defaulted comparison operators (C++20)

## `src/ui/Color.hpp`

- Fixed-width integer types (<cstdint>)

## `src/ui/GameplayScreen.cpp`

- std::visit with a function object (the visitor pattern for std::variant)
- std::holds_alternative

## `src/ui/IRenderer.hpp`

- Abstract interfaces, virtual functions and dependency inversion
- Deleting copy/move to prevent slicing

## `src/ui/InstructionsScreen.cpp`

- User-defined literals (the `sv` suffix) combined with CTAD

## `src/ui/MenuScreen.cpp`

- std::get_if - "is this variant currently holding a T?"

## `src/ui/MenuScreen.hpp`

- `static constexpr` class members

## `src/ui/ResultScreen.cpp`

- emplace_back - constructing elements in place

## `src/ui/Screen.hpp`

- Polymorphism with `override` and `final` (see MenuScreen, GameplayScreen, ...)

## `src/ui/ScreenNavigator.hpp`

- std::unique_ptr, RAII and make_unique - ownership you can see in the type

## `src/ui/Theme.hpp`

- switch over an enum class WITHOUT a `default:` label

## `tests/LevelTests.cpp`

- RAII for test fixtures

## `tests/TestFramework.hpp`

- Static registration through a constructor
