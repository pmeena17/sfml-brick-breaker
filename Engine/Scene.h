
#pragma once

#include <SFML/Graphics.hpp>

namespace Engine
{
    // Abstract base class for a game screen (e.g., main menu, level)
    class Scene
    {
    public:
        virtual ~Scene() = default;

        virtual void handleInput(const sf::Event& event, sf::RenderWindow& window) = 0;
        virtual void update(float dt) = 0;
        virtual void render(sf::RenderWindow& window) = 0;
    };
}
