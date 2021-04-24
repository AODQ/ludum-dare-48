#pragma once

#include <cstddef>
#include <stdint.h>
#include <vector>

namespace ld { struct MobGroup; }

namespace ld {

  // this since with the rocks.png file vertically
  enum class RockType {
    Sand, Dirt, Rock, Gravel,
    Size,
  };

  // this since with the rocks.png file horizontally
  enum class RockTier {
    Base1, Base2, Hard, Mined,
    Size,
  };

  enum class RockGemType {
    Empty, Tin, Ruby, Emerald, Sapphire,
    Size,
  };

  struct MineRock {
    ld::RockType type;
    ld::RockTier tier;
    ld::RockGemType gem;

    int32_t durability;

    bool isMined() const { return this->tier == ld::RockTier::Mined; }

    // returns if this destroyed rock
    bool receiveDamage(int32_t damage);
  };

  struct MineChasm {
    uint32_t columns;
    std::vector<ld::MineRock> rocks;

    static ld::MineChasm Initialize(
      ld::MobGroup & group,
      std::size_t columns = 30,
      std::size_t rows    = 50
    );

    ld::MineRock & rock(uint32_t rockId) { return rocks[rockId]; }

    int32_t rockPositionX(uint32_t rockId) { return rockId % columns; }
    int32_t rockPositionY(uint32_t rockId) { return rockId / columns; }
  };
}
