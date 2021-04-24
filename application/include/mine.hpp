#pragma once

#include <stdint.h>
#include <vector>

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
    Empty, Tin,
    Size,
  };

  struct MineRock {
    ld::RockType type;
    ld::RockTier tier;
    ld::RockGemType gem;

    int32_t durability;

    bool isMined() const { return this->tier == ld::RockTier::Mined; }
  };

  struct MineChasm {
    uint32_t columns;
    std::vector<ld::MineRock> rocks;

    static ld::MineChasm Initialize();

    ld::MineRock GetRock(uint32_t rockId);
  };
}
