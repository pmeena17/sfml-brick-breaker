
#pragma once

#include <SFML/Graphics.hpp>
#include "Vector2D.h"

namespace Engine
{
    // Abstract base class for any object in a scene
    class GameObject
    {
    public:
        virtual ~GameObject() = default;

        virtual void update(float dt) = 0;
        virtual void render(sf::RenderWindow& window) = 0;

        Vector2D position;
        Vector2D velocity;
        bool isActive = true;
    };
}
