#include <mine.hpp>

#include <enum.hpp>
#include <mob.hpp>

#include <raylib.h>

#include <algorithm>
#include <cmath>    // round


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

  template<typename... T> constexpr auto average(T... vs)
  {
    return (0.f + ... + vs) / static_cast<float>(sizeof...(vs));
  }

  constexpr float vertGrade(float z, float maxZ, float value)
  {
    return (
      (z < maxZ / 2)
      ? lerp(  0.f, value, ((z             ) / (maxZ / 2)))
      : lerp(value,   1.f, ((z - (maxZ / 2)) / (maxZ / 2)))
    );
  }

  float fval(const ::Image& img, std::size_t i)
  {
    return reinterpret_cast<const ::Color*>(img.data)[i].r / 255.f;
  }
}


ld::MineChasm ld::MineChasm::Initialize(
  ld::MobGroup & group,
  std::size_t columns,
  std::size_t rows
)
{
  auto self = ld::MineChasm{
    columns,
    decltype(ld::MineChasm::rocks)(columns * rows, basicRock)
  };
  auto pn1 = ::GenImagePerlinNoise(columns, rows, 0, 0, 10.f);
  auto pn2 = ::GenImagePerlinNoise(columns, rows, 0, 0, 20.f);
  auto cn1 = ::GenImageCellular(columns, rows, 5);

  for (std::size_t row = 0; row < rows; ++row) {
    for (std::size_t col = 0; col < columns; ++col) {
      const auto i = row * columns + col;
      auto& rock = self.rocks[i];

      const auto nv = vertGrade(
        row,
        rows,
        average(
          fval(pn1, i),
          fval(cn1, i)
        )
      );

      rock.type = static_cast<ld::RockType>(
        static_cast<std::size_t>(std::round(
          nv
          * (Idx(RockType::Size) - 1)
        ))
      );

      // fade in gem distribution for the first few rows
      const auto gv = (
        row < 20
        ? lerp(0.5f, fval(pn2, i), static_cast<float>(row) / 20.f)
        : fval(pn2, i)
      );
      if (gv > 0.6f) {
        rock.gem = static_cast<ld::RockGemType>(::GetRandomValue(
          1,
          Idx(ld::RockGemType::Size) * (static_cast<float>(row) / rows)
        ));
      }

      // first row is walkable
      if (row < 1) {
        rock.type = ld::RockType::Sand;
        rock.tier = ld::RockTier::Mined;
      }

      rock.durability = 1;
    }
  }

  // grade rock tiers in each column, leaving the first row walkable
  for (std::size_t col = 0; col < columns; ++col) {
    std::size_t topRow = 1;
    for (std::size_t row = 1; row <= rows; ++row) {
      auto&  top = self.rocks[topRow * columns + col];
      if (row == rows || top.type != self.rocks[row * columns + col].type) {
        for (std::size_t i = topRow; i < row; ++i) {
          self.rocks[i * columns + col].tier = static_cast<ld::RockTier>(
            std::round(
              // skips Mined -- that's added separately
              (Idx(ld::RockTier::Size) - 2) * vertGrade(
                i - topRow,
                row - topRow,
                fval(pn2, i)
              )
            )
          );
        }
        topRow = row;
      }
    }
  }

  // TODO add mobs to empty rooms
  group.slimes.push_back({
    .positionX = 400, .positionY = 200
  });
  group.poisonClouds.push_back({
    .positionX = 400, .positionY = 200
  });

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

int32_t ld::MineChasm::rockPathValue(int32_t x, int32_t y) const {
  auto & self = *this;

  auto const & target = self.rock(self.rockId(x, y));

  if (target.isMined()) { return 100; }

  int32_t value = 0;
  switch (target.type) {
    default: break;
    case ld::RockType::Sand:   value += 5;  break;
    case ld::RockType::Dirt:   value -= 5;  break;
    case ld::RockType::Rock:   value -= 25; break;
    case ld::RockType::Gravel: value -= 50; break;
  }

  switch (target.tier) {
    default: break;
    case ld::RockTier::Base1: case ld::RockTier::Base2: break;
    case ld::RockTier::Hard: value -= 5;
  }

  switch (target.gem) {
    default: break;
    case ld::RockGemType::Empty: break;
    case ld::RockGemType::Tin:      value += 10; break;
    case ld::RockGemType::Ruby:     value += 100; break;
    case ld::RockGemType::Emerald:  value += 250; break;
    case ld::RockGemType::Sapphire: value += 500; break;
  }

  return value;
}
