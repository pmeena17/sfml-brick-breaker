
#pragma once

#include "../Engine/GameObject.h"

namespace Game
{
    class Ball : public Engine::GameObject
    {
    public:
        sf::CircleShape shape;
        float radius = 10.f;

        Ball(float startX, float startY)
        {
            position = {startX, startY};
            velocity = {200.0f, -200.0f}; // Pixels per second
            shape.setRadius(radius);
            shape.setOrigin(radius, radius);
            shape.setFillColor(sf::Color::Cyan);
            shape.setPosition(position.x, position.y);
        }

        void update(float dt) override
        {
            position.x += velocity.x * dt;
            position.y += velocity.y * dt;
            shape.setPosition(position.x, position.y);
        }
        
        void render(sf::RenderWindow& window) override
        {
            if (isActive)
            {
                window.draw(shape);
            }
        }
    };
}
