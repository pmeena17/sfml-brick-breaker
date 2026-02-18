
#pragma once

#include <vector>
#include <memory>
#include "../Engine/Scene.h"
#include "Ball.h"
#include "Paddle.h"
#include "Brick.h"

namespace Game
{
    class GameScene : public Engine::Scene
    {
    public:
        GameScene(int screenWidth, int screenHeight)
            : screenWidth(screenWidth), screenHeight(screenHeight)
        {
            // Create Paddle
            paddle = std::make_unique<Paddle>((float)screenWidth / 2.f, (float)screenHeight - 50.f);

            // Create Ball
            ball = std::make_unique<Ball>((float)screenWidth / 2.f, (float)screenHeight / 2.f);

            // Create Bricks
            const int brickRows = 5;
            const int brickCols = 10;
            const float brickWidth = 70.f;
            const float brickHeight = 20.f;
            const float brickPadding = 5.f;
            const float brickOffset = 30.f;

            for (int i = 0; i < brickRows; ++i)
            {
                for (int j = 0; j < brickCols; ++j)
                {
                    float x = j * (brickWidth + brickPadding) + brickOffset;
                    float y = i * (brickHeight + brickPadding) + brickOffset;
                    bricks.push_back(std::make_unique<Brick>(x, y, brickWidth, brickHeight));
                }
            }
        }
        
        void handleInput(const sf::Event& event, sf::RenderWindow& window) override
        {
            // Continuous input for smooth movement
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            {
                paddle->velocity.x = -paddle->speed;
            }
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            {
                paddle->velocity.x = paddle->speed;
            }
            else
            {
                paddle->velocity.x = 0;
            }
        }

        void update(float dt) override
        {
            paddle->update(dt);
            ball->update(dt);
            checkCollisions();
        }

        void render(sf::RenderWindow& window) override
        {
            paddle->render(window);
            ball->render(window);
            for (const auto& brick : bricks)
            {
                brick->render(window);
            }
        }

    private:
        void checkCollisions()
        {
            // Paddle vs Walls
            if (paddle->position.x - paddle->width / 2.f < 0)
            {
                paddle->position.x = paddle->width / 2.f;
            }
            if (paddle->position.x + paddle->width / 2.f > screenWidth)
            {
                paddle->position.x = screenWidth - paddle->width / 2.f;
            }

            // Ball vs Walls
            if (ball->position.x - ball->radius < 0 || ball->position.x + ball->radius > screenWidth)
            {
                ball->velocity.x *= -1;
            }
            if (ball->position.y - ball->radius < 0)
            {
                ball->velocity.y *= -1;
            }
            if (ball->position.y + ball->radius > screenHeight)
            {
               ball->velocity.y *= -1; // For now, just bounce
            }

            // Ball vs Paddle
            if (ball->shape.getGlobalBounds().intersects(paddle->shape.getGlobalBounds()))
            {
                ball->position.y = paddle->position.y - paddle->height / 2.f - ball->radius;
                ball->velocity.y *= -1;
            }

            // Ball vs Bricks
            for (auto& brick : bricks)
            {
                if (brick->isActive && ball->shape.getGlobalBounds().intersects(brick->shape.getGlobalBounds()))
                {
                    brick->isActive = false;
                    ball->velocity.y *= -1;
                    break;
                }
            }
        }

        int screenWidth;
        int screenHeight;
        std::unique_ptr<Ball> ball;
        std::unique_ptr<Paddle> paddle;
        std::vector<std::unique_ptr<Brick>> bricks;
    };
}
