#pragma once

#include <string>
#include <vector>

#include "core/Brick.hpp"

namespace bb::core
{
// An immutable description of one level: its name and the bricks it starts with.
//
// This is the seam that makes adding levels cheap. Gameplay code (GameSession) only ever sees a
// LevelData; it neither knows nor cares whether it came from a text file (today), a JSON file, a level
// editor or was generated procedurally. The original program instead had one hard-coded `if (level == N)`
// block per level - roughly 100-170 lines each, copy-pasted, with the same bricks/colours/hit-points logic
// repeated five times (and one block mislabelled "level 5 starts" inside `if (level == 4)`).
struct LevelData
{
    std::string name;
    std::vector<Brick> bricks;
};
} // namespace bb::core
