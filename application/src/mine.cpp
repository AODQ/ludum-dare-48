#include <mine.hpp>

#include <enum.hpp>

#include <raylib.h>

ld::MineChasm ld::MineChasm::Initialize()
{
  ld::MineChasm self;

  self.columns = 30;
  self.rocks.resize(self.columns * 50);

  for (size_t i = 0; i < self.rocks.size(); ++ i) {
    auto & rock = self.rocks[i];

    rock.type = static_cast<ld::RockType>(i / 500);
    rock.tier =
      static_cast<ld::RockTier>(::GetRandomValue(0, Idx(RockTier::Mined)-1));
    rock.gem  = ld::RockGemType::Empty;

    // first row is walkable
    if (i < 30) {
      rock.type = ld::RockType::Sand;
      rock.tier = ld::RockTier::Mined;
    }

    rock.durability = 10;
  }

  return self;
}
