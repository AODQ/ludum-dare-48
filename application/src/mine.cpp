#include <mine.hpp>

#include <enum.hpp>
#include <gamestate.hpp>
#include <mob.hpp>
#include <pathfinder.hpp>

#include <raylib.h>

#include <algorithm>
#include <cmath>    // round, floor
#include <memory>

namespace {
  constexpr ld::MineRock basicDirt{
    ld::RockType   ::Dirt,
    ld::RockTier   ::Base1,
    ld::RockGemType::Empty,
    5,
  };

  constexpr uint32_t startingRows = 2;

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

  float fval(const ::Image& img, uint32_t x, uint32_t y)
  {
    if (
      img.width <= 0 || img.height <= 0
      || x >= static_cast<uint32_t>(img.width)
      || y >= static_cast<uint32_t>(img.height)
    ) {
      return 0.f;
    }
    else {
      return reinterpret_cast<const ::Color*>(img.data)[
        y * img.width + x
      ].r / 255.f;
    }
  }

  // raylib Perlin noise size
  auto perlinSize(ld::MineChasm const& self)
  {
    return std::max(
      static_cast<uint32_t>(self.rocks.size() / self.columns),
      self.columns
    );
  }

  // raylib Perlin noise coordinate
  auto pc()
  {
    return ::GetRandomValue(0, 197);
  }

  int32_t MobPos(uint32_t cell)
  {
    return cell * 32 + 16 / 2;
  }
}


namespace { // generate passes
  // grade rock tiers in each column
  void GenerateTiers(ld::MineChasm& self)
  {
    const auto rows = static_cast<uint32_t>(self.rocks.size() / self.columns);
    const auto ps = perlinSize(self);
    auto perlin = ::GenImagePerlinNoise(ps, ps, pc(), pc(), 100.f);

    for (uint32_t col = 0; col < self.columns; ++col) {
      uint32_t topRow = 0;
      for (uint32_t row = 0; row <= rows; ++row) {
        auto& top = self.rock(col, topRow);
        if (
          row == rows
          || top.type != self.rocks[row * self.columns + col].type
        ) {
          for (uint32_t i = topRow; i < row; ++i) {
            self.rock(col, i).tier = static_cast<ld::RockTier>(
              std::round(
                // skips Mined -- that's added separately
                (Idx(ld::RockTier::Size) - 2) * vertGrade(
                  i - topRow,
                  row - topRow,
                  fval(perlin, col, i)
                )
              )
            );
          }
          topRow = row;
        }
      }
    }

    ::UnloadImage(perlin);
  }

  // basic earth pattern
  void GenerateEarth(ld::MineChasm& self)
  {
    const auto rows = static_cast<uint32_t>(self.rocks.size() / self.columns);
    const auto ps = perlinSize(self);
    auto perlin = ::GenImagePerlinNoise(ps, ps, pc(), pc(), 10.f);
    auto cells  = ::GenImageCellular(self.columns, rows, 5);

    for (uint32_t row = 0; row < rows; ++row) {
      for (uint32_t col = 0; col < self.columns; ++col) {
        const auto pv = average(
          fval(perlin, col, row),
          fval(cells , col, row)
        );
        const auto nv = vertGrade(row, rows, pv);

        if (pv < 0.55f) {
          self.rock(col, row).type = static_cast<ld::RockType>(
            static_cast<uint32_t>(std::round(
              nv
              * (Idx(ld::RockType::Size) - 1)
            ))
          );
        }
        else {
          self.rock(col, row).type = ld::RockType::Granite;
        }
      }
    }

    ::UnloadImage(perlin);
    ::UnloadImage(cells );

    GenerateTiers(self);
  }

  void GenerateGems(ld::MineChasm& self)
  {
    const auto rows = static_cast<uint32_t>(self.rocks.size() / self.columns);
    const auto ps = perlinSize(self);
    auto perlin = ::GenImagePerlinNoise(ps, ps, pc(), pc(), 80.f);

    for (uint32_t row = 0; row < rows; ++row) {
      for (uint32_t col = 0; col < self.columns; ++col) {
        // fade in gem distribution for the first few rows
        const auto gv = (
          row < 20
          ? lerp(0.5f, fval(perlin, col, row), static_cast<float>(row) / 20.f)
          : fval(perlin, col, row)
        );
        if (gv > 0.7f) {
          if (row < 20) {
            self.rock(col, row).gem = ld::RockGemType::Tin;
          }
          else {
            self.rock(col, row).gem = static_cast<ld::RockGemType>(
              1 + std::floor(
                fval(perlin, col, row)
                * static_cast<float>(row - 20) / (rows - 20)
                * Idx(ld::RockGemType::Food) // dont gen food here
              )
            );
          }
        }
      }
    }
  }

  // who put food in the walls??
  void GenerateFood(ld::MineChasm& self)
  {
    const auto rows = static_cast<uint32_t>(self.rocks.size() / self.columns);
    const auto ps = perlinSize(self);
    auto perlin = ::GenImagePerlinNoise(ps, ps, pc(), pc(), 7.f);
    auto cells  = ::GenImageCellular(self.columns, rows, 10);

    for (uint32_t row = 0; row < rows; ++row) {
      for (uint32_t col = 0; col < self.columns; ++col) {
        auto& rock = self.rock(col, row);
        const auto vv = average(
          fval(perlin, col, row),
          fval(cells , col, row)
        );
        if (
            ::GetRandomValue(0, 100) > 95 && vv < 0.35f && !rock.isMined()
         && row > 8 // ban from first few rows
        ) {
          rock.gem = ld::RockGemType::Food;
        }
      }
    }

    ::UnloadImage(perlin);
    ::UnloadImage(cells );
  }

  void GenerateSlimePockets(ld::MineChasm& self, ld::MobGroup & group)
  {
    // bluenoise would be noice
    const auto rows = static_cast<uint32_t>(self.rocks.size() / self.columns);

    int32_t gen = 0;

    for (uint32_t row = 8; row < rows-1; ++row) {
      for (uint32_t col = 1; col < self.columns-1; ++col) {
        auto& rock = self.rock(col, row);
        gen += 1 + row/3;
        if (::GetRandomValue(0, 100) < (gen/100) && !rock.isMined()) {
          gen = 0;
          rock.tier = ld::RockTier::Mined;
          rock.gem  = ld::RockGemType::Empty;
          group.slimes.push_back({
            .positionX = MobPos(col),
            .positionY = MobPos(row)
          });
        }
      }
    }
  }

  // add caves & caverns
  [[nodiscard]] auto GenerateCaves(ld::MineChasm& self)
  {
    const auto rows = static_cast<uint32_t>(self.rocks.size() / self.columns);
    const auto ps = perlinSize(self);
    auto perlin = ::GenImagePerlinNoise(ps, ps, pc(), pc(), 7.f);
    auto cells  = ::GenImageCellular(self.columns, rows, 10);

    std::vector<std::pair<uint32_t, uint32_t>> emptySpaces;

    for (uint32_t row = 0; row < rows; ++row) {
      for (uint32_t col = 0; col < self.columns; ++col) {
        auto& rock = self.rock(col, row);
        const auto vv = average(
          fval(perlin, col, row),
          fval(cells , col, row)
        );
        // fade in caverns for the first few rows
        const auto cv = (
          row < 5
          ? lerp(0.5f, vv, static_cast<float>(row) / 5.f)
          : vv
        );
        if (cv < 0.3f) {
          rock.tier = ld::RockTier::Mined;
          rock.gem  = ld::RockGemType::Empty;
          if (row >= startingRows) {
            emptySpaces.emplace_back(row, col);
          }
        }
      }
    }

    ::UnloadImage(perlin);
    ::UnloadImage(cells );

    return emptySpaces;
  }

  void GenerateMobs(
    ld::MineChasm& self,
    ld::MobGroup & group,
    std::vector<std::pair<uint32_t, uint32_t>> const& emptySpaces
  )
  {
    const auto numTnts = static_cast<uint32_t>(
      self.rocks.size() / 100
    );
    const auto numSlimes = static_cast<uint32_t>(
      self.rocks.size() / 375
    );
    const auto numClouds = static_cast<uint32_t>(
      self.rocks.size() / 375
    );

    for (uint32_t i = 0; i < numTnts; ++i) {
      auto spot = emptySpaces.begin() + ::GetRandomValue(
        0,
        emptySpaces.size() - 1
      );
      group.tnts.push_back({
        .positionX = MobPos(spot->second),
        .positionY = MobPos(spot->first )
      });
    }

    for (uint32_t i = 0; i < numSlimes; ++i) {
      auto spot = emptySpaces.begin() + ::GetRandomValue(
        0,
        emptySpaces.size() - 1
      );
      group.slimes.push_back({
        .positionX = MobPos(spot->second),
        .positionY = MobPos(spot->first )
      });
    }

    // poison clouds only in lower half
    for (uint32_t i = 0; i < numClouds; ++i) {
      auto spot = emptySpaces.begin() + ::GetRandomValue(
        emptySpaces.size()/2,
        emptySpaces.size() - 1
      );
      group.poisonClouds.push_back({
        .positionX = (int32_t)(spot->second)*32,
        .positionY = (int32_t)(spot->first )*32,
        .numInstances = std::make_shared<int32_t>(1),
      });
    }
  }
}


ld::MineChasm ld::MineChasm::Initialize(
  ld::MobGroup & group,
  uint32_t columns,
  uint32_t rows
)
{
  auto self = ld::MineChasm{
    columns,
    decltype(ld::MineChasm::rocks  )(columns * rows, basicDirt),
    decltype(ld::MineChasm::rockFow)(columns * rows, 0.0f     ),
    decltype(ld::MineChasm::poison )(columns * rows, 0.0f     )
  };

  GenerateEarth(self);
  GenerateGems (self);
  GenerateFood (self);
  GenerateSlimePockets(self, group);
  const auto emptySpaces = GenerateCaves(self);

  // first few rows are walkable
  for (uint32_t i = 0; i < columns * startingRows; ++i) {
    self.rock(i).type = ld::RockType::Sand;
    self.rock(i).tier = ld::RockTier::Mined;
    self.rock(i).gem = ld::RockGemType::Empty;
  }

  // and very last is walkable
  for (uint32_t i = (rows-2)*columns; i < rows*columns; ++i) {
    self.rock(i).type = ld::RockType::Sand;
    self.rock(i).tier = ld::RockTier::Mined;
    self.rock(i).gem = ld::RockGemType::Empty;
  }

  GenerateMobs (self, group, emptySpaces);

  // compute durabilities & ids
  for (uint32_t id = 0; id < self.rocks.size(); ++ id) {
    auto & rock = self.rocks[id];
    rock.rockId = id;
    rock.baseDurability = rock.durability =
      ld::baseRockDurability(rock.type, rock.tier, rock.gem);
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
    ld::pathClear();
  }

  return self.isMined();
}

int32_t ld::MineChasm::rockPathValue(int32_t x, int32_t y) const {
  auto & self = *this;

  if (y < 0) { return 0; }

  if (x < 0 || x > static_cast<int32_t>(columns)) { return 0; }

  auto rockId = self.rockId(x, y);

  auto const & target = self.rock(self.rockId(x, y));

  if (target.isMined()) { return 0.0f; }

  int32_t value = ::GetRandomValue(-5.0f, 5.0f);

  // can only see rocks if visible
  if (self.rockFow[rockId] >= 0.1f) {
    switch (target.gem) {
      default: break;
      case ld::RockGemType::Empty: break;
      case ld::RockGemType::Tin:      value += 20; break;
      case ld::RockGemType::Ruby:     value += 200; break;
      case ld::RockGemType::Emerald:  value += 350; break;
      case ld::RockGemType::Sapphire: value += 400; break;
      case ld::RockGemType::Food:     value += 200; break;
    }
  }

  return std::clamp(1.0f + value, 0.5f, 60.0f);
}

int32_t ld::MineChasm::rockPathDurability(int32_t x, int32_t y) const {
  auto & self = *this;

  if (y < 0) { return 5000; }

  if (x < 0 || x > static_cast<int32_t>(columns)) { return 5000; }

  auto rock = self.rock(x, y);
  return ld::baseRockDurability(rock.type, rock.tier, rock.gem);
}

void ld::MineChasm::Update(ld::GameState & state)
{
  // update FOW
  for (size_t i = 0; i < state.mineChasm.rocks.size(); ++ i) {
    state.mineChasm.rockFow[i] =
      std::max(
        std::clamp(1.0f - std::clamp(i/30, 0ul, 4ul) / 4.0f, 0.05f, 1.0f),
        state.mineChasm.rockFow[i]
       - (
         0.00005f

         + 0.005f * (1.0f - state.researchItems[Idx(ld::ResearchType::Vision)].level / 10.0f)
         )
      );
  }
}

int32_t ld::baseRockDurability(
  ld::RockType const type,
  ld::RockTier const tier,
  ld::RockGemType const gem
) {
  int32_t durability = 0;
  switch (type) {
    default: break;
    case ld::RockType::Sand:    durability = 10;  break;
    case ld::RockType::Dirt:    durability = 50;  break;
    case ld::RockType::Rock:    durability = 150; break;
    case ld::RockType::Granite: durability = 500; break;
  }

  switch (tier) {
    default: break;
    case ld::RockTier::Hard:   durability *= 2;
  }

  switch (gem) {
    default: break;
    case ld::RockGemType::Empty: break;
    case ld::RockGemType::Tin:      durability += 20; break;
    case ld::RockGemType::Ruby:     durability += 70; break;
    case ld::RockGemType::Emerald:  durability += 130; break;
    case ld::RockGemType::Sapphire: durability += 230; break;
    case ld::RockGemType::Food:     durability += 20; break;
  }

  return durability;
}
