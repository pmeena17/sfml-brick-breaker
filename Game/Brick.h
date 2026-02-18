
#pragma once

#include "../Engine/GameObject.h"

namespace Game
{
    class Brick : public Engine::GameObject
    {
    public:
        sf::RectangleShape shape;
        int health = 1;

        Brick(float x, float y, float width, float height)
        {
            position = {x, y};
            shape.setPosition(position.x, position.y);
            shape.setSize({width, height});
            shape.setFillColor(sf::Color::Magenta);
        }

        // Brick is static, so update does nothing
        void update(float dt) override {}

        void render(sf::RenderWindow& window) override
        {
            if (isActive)
            {
                window.draw(shape);
            }
        }
    };
}
