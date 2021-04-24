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

  constexpr float lerp(float n1, float n2, float f)
  {
    return (n2 * f) + (n1 * (1.f - f));
  }
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
  auto pn1 = ::GenImagePerlinNoise(columns, rows, 0, 0, 10.f);
  auto cn1 = ::GenImageCellular(columns, rows, 5);

  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t col = 0; col < columns; ++col) {
      const auto i = row * columns + col;
      auto& rock = self.rocks[i];
      auto nv = (
          reinterpret_cast<const ::Color*>(pn1.data)[i].r / 256.f
        + reinterpret_cast<const ::Color*>(cn1.data)[i].r / 256.f
      ) / 2.f;

      nv = (
        (row < rows / 2)
        ? lerp(0.f,  nv, (static_cast<float>(row             ) / (rows / 2)))
        : lerp(nv , 1.f, (static_cast<float>(row - (rows / 2)) / (rows / 2)))
      );

      rock.type = static_cast<ld::RockType>(
        static_cast<std::size_t>(
          nv
          * Idx(RockTier::Size)
        )
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
