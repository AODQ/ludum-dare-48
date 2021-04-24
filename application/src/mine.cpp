#include <mine.hpp>

#include <enum.hpp>

#include <raylib.h>

#include <algorithm>


namespace
{
  constexpr ld::MineRock basicRock{
    ld::RockType   ::Dirt,
    ld::RockTier   ::Base1,
    ld::RockGemType::Empty,
    10,
  };
}


ld::MineChasm ld::MineChasm::Initialize(
  std::size_t columns,
  std::size_t rows
)
{
  auto self = ld::MineChasm{
    columns,
    decltype(ld::MineChasm::rocks)(columns * rows, basicRock)
  };
  auto noise = ::GenImagePerlinNoise(columns, rows, 0, 0, 10.f);

  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t col = 0; col < columns; ++col) {
      const auto i = row * columns + col;
      auto& rock = self.rocks[i];
      const auto nv = reinterpret_cast<const ::Color*>(noise.data)[i].r / 255.f;

      rock.type = static_cast<ld::RockType>(
        static_cast<std::size_t>(nv * Idx(RockTier::Size))
      );

      rock.tier = static_cast<ld::RockTier>(::GetRandomValue(
        0,
        Idx(RockTier::Mined)-1
      ));

      rock.gem  = ld::RockGemType::Empty;

      // first row is walkable
      if (row < 1) {
        rock.type = ld::RockType::Sand;
        rock.tier = ld::RockTier::Mined;
      }

      rock.durability = 10;
    }
  }

  return self;
}

bool ld::MineRock::receiveDamage(int32_t damage) {
  auto & self = *this;

  // -abs(damage)
  if (damage < 0) damage = -damage;

  durability = std::max(0, durability - damage);
  if (durability == 0) {
    self.tier = RockTier::Mined;
  }

  return self.isMined();
}
