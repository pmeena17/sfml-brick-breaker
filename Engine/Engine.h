
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include "Scene.h"

namespace Engine
{
    class GameEngine
    {
    public:
        GameEngine(int width, int height, const std::string& title)
            : window(sf::VideoMode(width, height), title)
        {
            window.setFramerateLimit(60);
        }

        void run()
        {
            sf::Clock clock;
            while (window.isOpen())
            {
                // Use a delta time for frame-rate independent physics
                float dt = clock.restart().asSeconds();
                
                sf::Event event;
                while (window.pollEvent(event))
                {
                    if (event.type == sf::Event::Closed)
                    {
                        window.close();
                    }
                    if (currentScene)
                    {
                        currentScene->handleInput(event, window);
                    }
                }

                if (currentScene)
                {
                    currentScene->update(dt);
                }
                
                window.clear(sf::Color(10, 10, 40));
                if (currentScene)
                {
                    currentScene->render(window);
                }
                window.display();
            }
        }
        
        void setScene(std::unique_ptr<Scene> scene)
        {
            currentScene = std::move(scene);
        }

    private:
        sf::RenderWindow window;
        std::unique_ptr<Scene> currentScene;
    };
}
