# Brick Breaker (C++20 rewrite)

A ground-up rewrite of a C/C++ project (a 5-level brick breaker built on the `simplecpp`
teaching graphics library and Windows `conio`). Same game, same five levels, same three power-ups, but structured
like a code base you could keep adding to: data-driven levels, a pure game-logic library with unit tests, and a
thin SFML 3 front-end (free, zlib-licensed, runs on Windows, Linux and macOS).

* Language: C++20 - Build system: CMake >= 3.28 - Graphics: [SFML 3](https://www.sfml-dev.org/)
* The code is full of `C++ Concept:` comments that explain each modern C++ feature where it is used.
  Index: [`docs/CPP_CONCEPTS.md`](docs/CPP_CONCEPTS.md) (53 notes).

## Controls

| Key | Action |
| --- | --- |
| Left / Right (or A / D) | Move the paddle |
| Space | Launch the ball |
| P | Pause / resume (the game also pauses if the window loses focus) |
| Esc | Back to the menu (quits from the menu) |
| Mouse | Click menu buttons |

Blue bricks take one hit, red bricks two. Some bricks drop a falling capsule when destroyed; catch it with the
paddle: red = **slow ball**, green = **fast ball**, blue = **small paddle** (10 seconds each).

## Build and run

You need a C++20 compiler, CMake 3.28+ and Git (to download SFML). The code was developed and tested with GCC 13; Visual Studio 2022
and a recent Clang/libstdc++ should work but have not been tried.

```
cmake -S . -B build
cmake --build build --config Release
```

Run `build/bin/brick_breaker` (`build\bin\Release\brick_breaker.exe` with the Visual Studio generator). The build copies
`assets/` next to the executable, and the game also finds `assets/` from the project directory.
CMake uses an installed SFML 3 if it finds one, otherwise it downloads and builds the pinned version `BB_SFML_TAG`
(default `3.0.2`). SFML 3.1.0 has also been released; `-DBB_SFML_TAG=3.1.0` should work (the API used here is the same) but is untested.

* **Windows**: open the folder in Visual Studio 2022 ("Open a local folder") or run the commands above in a Developer
  Command Prompt. No extra libraries are needed; the game uses Segoe UI/Arial from `C:\Windows\Fonts`.
* **Linux**: SFML needs its development packages first; typically on Debian/Ubuntu
  `sudo apt install build-essential cmake git libx11-dev libxrandr-dev libxcursor-dev libxi-dev libudev-dev libfreetype-dev libgl1-mesa-dev`
  (see the SFML documentation for other distributions).
* **macOS**: install Xcode command line tools and CMake (`brew install cmake`).

### Tests (no SFML or download needed)

```
cmake --preset tests-only
cmake --build --preset tests-only
ctest --preset tests-only
```

or, without CMake: `g++ -std=c++20 -Isrc -DBB_ASSETS_DIR=\"$PWD/assets\" src/core/*.cpp src/ui/*.cpp tests/*.cpp -o bb_tests && ./bb_tests`.

### Useful options

| CMake option | Default | Meaning |
| --- | --- | --- |
| `BB_BUILD_GAME` | ON | Build the SFML game. Turn OFF to build only the logic + tests. |
| `BB_BUILD_TESTS` | ON | Build the unit tests. |
| `BB_WARNINGS_AS_ERRORS` | OFF | `-Werror` / `/WX`. |
| `BB_ENABLE_SANITIZERS` | OFF | AddressSanitizer + UBSan (GCC/Clang). |
| `BB_SFML_TAG` | 3.0.2 | SFML version downloaded when SFML 3 is not installed. |

Environment: `BRICK_BREAKER_ASSETS=/path/to/assets` overrides where levels and fonts are loaded from.

## Architecture

Dependencies point one way only; the inner layers know nothing about the outer ones.

```
 src/app   (SFML adapters + main loop)  --->  src/ui   (screens, input events, IRenderer)  --->  src/core   (rules, level loading)
   Application, SfmlRenderer, SfmlInput          MenuScreen, GameplayScreen, ResultScreen          GameSession, Ball, Paddle, Brick,
   FontLocator, AssetLocator, main               InstructionsScreen, ScreenNavigator               EffectTracker, Collision, LevelParser,
                                                                                                    LevelRepository
```

* **`core`** - all the rules of the game and nothing else: no window, no keyboard, no colours. `GameSession` takes the
  player's intent (`SetPaddleDirection`, `LaunchBall`) and elapsed time (`Update`) and exposes read-only state plus
  a list of events. Fully unit-tested.
* **`ui`** - screens written against an abstract `IRenderer` and our own `InputEvent` type. Also unit-tested (with a
  renderer that just records calls).
* **`app`** - the only code that includes SFML. To move to SDL/raylib/Win32, implement `IRenderer`, translate that
  library's events into `ui::InputEvent`, and replace `Application`. Nothing else changes.

```
assets/levels/*.level     level data (one text file per level)
assets/fonts/             optional bundled font
cmake/                    shared CMake helpers (warning flags)
docs/CPP_CONCEPTS.md      index of the "C++ Concept:" notes
src/core/                 game logic library            (bb_core)
src/ui/                   screens + renderer interface  (bb_ui)
src/app/                  SFML front-end + main()       (brick_breaker)
tests/                    unit tests + tiny test framework (bb_tests)
```

## Adding a level

Drop a new file into `assets/levels/` (for example `level_06.level`). No code change, no rebuild: levels are discovered
at start-up, sorted by file name (use zero-padded numbers), and the menu grows a button - and pages, once there are more
than six levels.

```
# comments start with #
name Zig-zag                      # shown on the menu button
brick_size 60 25                  # default brick size (optional; 60x25 if omitted)
brick 100 60                      # x y = the CENTRE of the brick, in pixels
brick 160 60 hp=2                 # hp = hits needed (1..9, default 1)
brick 220 60 power=slow_ball      # power = slow_ball | fast_ball | shrink_paddle (dropped when destroyed)
brick 280 90 w=100 h=30           # per-brick size override
```

The playfield is x 10..590, y 10..590. Bricks must lie inside it and keep one ball-diameter free above the paddle (bottom
edge at y <= 515). A broken file never stops the game: it is skipped, reported on the console as `file:line: message`,
and the menu shows how many files failed. The five shipped levels were generated from the original program's layout
code, so they reproduce the original positions, two-hit row (bricks 10-14) and power-up bricks (6, 15, 16).

## Adding a power-up

1. Add an enumerator to `PowerUpKind` in `src/core/PowerUp.hpp`.
2. Add one row to `kPowerUpDefinitions` (id, display name, description, duration, ball-speed scale, paddle-width scale).
   A `static_assert` fails the build if the table and the enum disagree.
3. Add a colour in `PowerUpColor` (`src/ui/Theme.hpp`); the compiler warns about the missing `case` if you forget.
4. Use `power=<id>` in a level file.

The instructions screen legend updates itself. Effects that need new *behaviour* (not just a speed or width multiplier)
extend `EffectModifiers` / `GameSession::ApplyEffectModifiers`.

## What was wrong with the original, and what replaced it

Facts below were checked against the original source (`140070034_275code.c`, 1189 lines).

| Original | Now |
| --- | --- |
| One `if (level == N)` block per level, 100-170 lines each, copy-pasted (one even commented "level 5 starts" inside `if (level == 4)`) | Levels are data files parsed and validated by `LevelParser`; `GameSession` is level-agnostic |
| Five parallel arrays (`xb1`, `xb2`, `yb1`, `yb2`, `arectanglereflect`) plus `Rectangle*` array, indices computed as `j+5*i` | `Brick` objects in a `std::vector`; no index arithmetic |
| Brick state as magic numbers (2/3/4); power-ups chosen by `if (bj+5*bi == 15)` in each level | `hitPoints` on `Brick`; `PowerUpKind` enum + data table; power-up is a property of a brick in the level file |
| Wall/paddle bounces by `bcx == 588 \|\| bcx == 589 ...` (float `==` against lists of nearby numbers), brick hits by hand-tuned bands | Exact circle-vs-rectangle test (`TestCircleRect`), one reaction per step, proper push-out |
| Side walls reflected when the ball's *centre* reached the border, the ceiling when its *edge* did | Walls reflect the ball's edge, consistently |
| Ball advanced a fixed distance per loop iteration (speed depended on the machine) | Fixed 120 Hz time step, speeds in pixels per second |
| Speed power-ups doubled/halved `vx`,`vy` by hand; shared counters `t`/`k`; `int k=0;` shadowed an uninitialised outer `k` that the timer used | `EffectTracker`: one timer per effect, speed/width always recomputed from base values |
| `new Rectangle(...)` for every brick, no `delete` anywhere | RAII / value types / `std::unique_ptr`; no manual `new` or `delete` |
| Navigation with `goto b;`; BACK button tested the previous click (`clickpos`, not `clickpos1`) | `ScreenNavigator` + `ScreenCommand`; each screen is a small class |
| Game over printed the score via `char sco = score;` (a character code, not digits); winning just left the loop | Result screen with real score, retry / next level / menu |
| Raw key codes (`'M'`, `'K'`, `32`) and packed mouse coordinates (`clickpos/65536`) | `ui::Key` enum, `MouseClicked{position}` |

### Deliberate behaviour changes

Paddle moves continuously while a key is held (the original jumped 30 px per key press). Pause is `P` (the original used
Space, which also launched the ball). Falling capsules no longer freeze the rest of the game and are caught on contact with
the paddle. Each power-up lasts 10 s (the original: 1000 loop iterations, i.e. machine-dependent). New: HUD (score, bricks
left, active effects), pause overlay, win screen with "next level", level paging. The playfield is the original 580x580
arena; the window is 600x640 to fit the HUD strip underneath.

## Conventions

* **Formatting**: Allman braces, 4 spaces, **no line wrapping** (function parameters, arguments and statements stay on one
  line). Enforced by `.clang-format` (`ColumnLimit: 0`, `BreakBeforeBraces: Allman`). Run `clang-format -i` on changed files.
* **Naming**: types and functions `PascalCase`, variables and parameters `camelCase`, private members `m_camelCase`,
  constants `kPascalCase`, namespaces `lower_case` (checked by `.clang-tidy`).
* **Errors**: exceptions for bad input files and fatal start-up problems (caught in `main` and per level file in
  `LevelRepository`); `std::optional` for routine "no value" results; `assert` for programmer errors.
* **Ownership**: values and RAII by default, `std::unique_ptr` for polymorphic objects, references/pointers only to observe.
* **Include style**: `#include "core/Vec2.hpp"` - paths relative to `src/`.

## Verification status (please read)

Run in the sandbox where this was written (Linux, GCC 13.3, `-std=c++20 -Wall -Wextra -Wpedantic -Wshadow -Wconversion
-Wsign-conversion -Wold-style-cast`, zero warnings):

* `src/core`, `src/ui` and `tests` compile cleanly; **45 unit tests pass**, also under AddressSanitizer + UBSan.
* The tests were mutation-checked (breaking the two-hit rule and the speed-restore rule makes them fail).
* A scripted player completes all five shipped levels in the simulation (45-75 simulated seconds each).

**Not** verified here, because SFML, CMake and a Windows toolchain were not available: the SFML layer in `src/app`
(it was only type-checked against a hand-written stand-in of the SFML 3 API, so a mismatch with the real API is possible),
the CMake files (hand-reviewed, never executed), MSVC/Clang builds, and the look and feel of the actual window. If the first
build reports an SFML API error it will be in `src/app/Sfml*.cpp` / `Application.cpp`, which are small and isolated by design.
`.clang-format` / `.clang-tidy` were not run either.
