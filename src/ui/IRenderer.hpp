#pragma once

#include <string_view>

#include "core/Rect.hpp"
#include "core/Vec2.hpp"
#include "ui/Color.hpp"

namespace bb::ui
{
enum class TextAnchor
{
    Center,     // the text is centred on the given point
    MiddleLeft, // the text starts at the given point and extends to the right
    MiddleRight // the text ends at the given point
};

// Everything the game needs from a graphics library, in four functions.
//
// C++ Concept: Abstract interfaces, virtual functions and dependency inversion
// The screens depend on THIS interface, not on SFML. SfmlRenderer (src/app) implements it for the real
// window; the unit tests implement it with a class that merely records the calls. To move to SDL, raylib or
// Direct3D you write one new implementation - no screen or game rule changes. This is the "ports and
// adapters" (hexagonal) architecture: the core defines the port it needs, the outside world plugs in.
//   * `= 0` makes a function pure virtual: the class is abstract and cannot be instantiated.
//   * The destructor MUST be virtual in a polymorphic base class. Without it, `delete`-ing a derived object
//     through a base pointer (which std::unique_ptr<IRenderer> does) is undefined behaviour.
//
// C++ Concept: Deleting copy/move to prevent slicing
// Copying a derived object into a base-class object chops off the derived part ("slicing"). Interfaces are
// never meant to be copied by value, so the copy and move operations are `= delete`d: any attempt to slice
// is a compile error. The constructor is protected so only derived classes can create the base subobject.
class IRenderer
{
public:
    virtual ~IRenderer() = default;

    IRenderer(const IRenderer&) = delete;
    IRenderer& operator=(const IRenderer&) = delete;
    IRenderer(IRenderer&&) = delete;
    IRenderer& operator=(IRenderer&&) = delete;

    // Fills the entire window.
    virtual void Clear(Color color) = 0;

    virtual void FillRect(const core::Rect& rect, Color color) = 0;

    virtual void FillCircle(core::Vec2 center, float radius, Color color) = 0;

    // `fontSize` is in logical pixels. Only ASCII text is guaranteed to be supported.
    // (Not called DrawText: <windows.h> #defines that name as a macro, which would silently rename it.)
    virtual void DrawLabel(std::string_view text, core::Vec2 anchor, float fontSize, Color color, TextAnchor alignment) = 0;

protected:
    IRenderer() = default;
};
} // namespace bb::ui
