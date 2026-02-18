
#pragma once

#include "../Engine/GameObject.h"

namespace Game
{
    class Paddle : public Engine::GameObject
    {
    public:
        sf::RectangleShape shape;
        float speed = 400.f; // Speed in pixels per second
        float width = 100.f;
        float height = 20.f;

        Paddle(float startX, float startY)
        {
            position = {startX, startY};
            shape.setSize({width, height});
            shape.setOrigin(width / 2.f, height / 2.f);
            shape.setFillColor(sf::Color::White);
            shape.setPosition(position.x, position.y);
        }

        void update(float dt) override
        {
            // Input logic is now in the GameScene, this just applies velocity
            position.x += velocity.x * dt;
            shape.setPosition(position.x, position.y);
        }

        void render(sf::RenderWindow& window) override
        {
            window.draw(shape);
        }
    };
}
