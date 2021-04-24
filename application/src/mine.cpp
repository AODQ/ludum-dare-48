#include <mine.hpp>

ld::MineChasm ld::MineChasm::Initialize()
{
  ld::MineChasm self;

  self.columns = 32;
  self.rocks.resize(self.columns * 50);

  for (size_t i = 0; i < self.rocks.size(); ++ i) {
    auto & rock = self.rocks[i];

    rock.type = ld::RockType::Sand;
    rock.tier = ld::RockTier::Base1;
    rock.gem  = ld::RockGemType::Empty;

    rock.durability = 10;
  }

  return self;
}
