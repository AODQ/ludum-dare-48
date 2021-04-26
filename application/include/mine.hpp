#pragma once

#include <stdint.h>
#include <vector>

namespace ld { struct MobGroup; }
namespace ld { struct GameState; }

namespace ld {

  // this since with the rocks.png file vertically
  enum class RockType {
    Sand, Dirt, Rock, Granite,
    Size,
  };

  // this since with the rocks.png file horizontally
  enum class RockTier {
    Base1, Base2, Hard, Mined,
    Size,
  };

  enum class RockGemType {
    Empty, Tin, Ruby, Emerald, Sapphire, Food,
    Size,
  };

  struct MineRock {
    ld::RockType type;
    ld::RockTier tier;
    ld::RockGemType gem;

    int32_t durability = 5u;
    int32_t baseDurability = 5u;

    int32_t rockId = 0;

    bool isMined() const { return this->tier == ld::RockTier::Mined; }

    // returns if this destroyed rock
    bool receiveDamage(int32_t damage);
  };

  struct MineChasm {
    uint32_t columns;
    std::vector<ld::MineRock> rocks;
    std::vector<float> rockFow;

    template <typename T>
    uint8_t fowU8(T const & thing) const {
      float x = rockFow[(thing.positionY/32)*columns + thing.positionX/32];
      return static_cast<uint8_t>(x > 1.0f ? 255 : (x < 0.0f ? 0 : x*255.0f));
    }

    static ld::MineChasm Initialize(
      ld::MobGroup & group,
      uint32_t columns =  30,
      uint32_t rows    = 250
    );

    ld::MineRock & rock(uint32_t rockId) { return rocks[rockId]; }
    ld::MineRock const & rock(uint32_t rockId) const { return rocks[rockId]; }
    ld::MineRock & rock(uint32_t x, uint32_t y) {
      return rocks[rockId(x, y)];
    }
    ld::MineRock const & rock(uint32_t x, uint32_t y) const {
      return rocks[rockId(x, y)];
    }
    uint32_t rockId(uint32_t x, uint32_t y) const {
      return y*columns + x;
    }

    int32_t rockPositionX(uint32_t rockId) const { return rockId % columns; }
    int32_t rockPositionY(uint32_t rockId) const { return rockId / columns; }

    int32_t rockPathValue(int32_t x, int32_t y) const;
    int32_t rockPathDurability(int32_t x, int32_t y) const;

    static void Update(ld::GameState & state);

    int32_t limitX(int32_t x, bool weirdOff=false) {
      if (x < 0) return 0;
      if (x >= static_cast<int32_t>(columns))
        return columns + (weirdOff ? 0 : -1);
      return x;
    }

    int32_t limitY(int32_t y) {
      if (y <= 0) return 1;
      if (y >= static_cast<int32_t>(rocks.size() / columns))
        return (rocks.size() / columns) - 1;
      return y;
    }
  };

  int32_t baseRockDurability(
    RockType const type,
    RockTier const tier,
    RockGemType const gem
  );
}
