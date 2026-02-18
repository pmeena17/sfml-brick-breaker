
#include <memory>
#include "Engine/Engine.h"
#include "Game/GameScene.h"

int main()
{
    const int SCREEN_WIDTH = 800;
    const int SCREEN_HEIGHT = 600;

    // 1. Create the engine
    Engine::GameEngine engine(SCREEN_WIDTH, SCREEN_HEIGHT, "Retro Brick Breaker");

    // 2. Create the game's starting scene
    auto gameScene = std::make_unique<Game::GameScene>(SCREEN_WIDTH, SCREEN_HEIGHT);

    // 3. Tell the engine to use that scene
    engine.setScene(std::move(gameScene));

    // 4. Start the engine's main loop
    engine.run();

    return 0;
}
