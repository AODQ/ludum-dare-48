#pragma once

#include <enum.hpp>

#include <array>
#include <cstdint>

namespace ld {

  // if this changes, must update below and in miner.hpp inventory
  enum class ItemType {
    Pickaxe,
    Armor,
    Speed,
    Size,
  };

  struct Item {
      ld::ItemType type;
      bool owns;
      int32_t durability;
      uint32_t level;
  };

  struct ItemInfoLookup {
    ld::ItemType type;
    int32_t cost;
    int32_t value;
  };

  constexpr std::array<ld::ItemInfoLookup, Idx(ld::ItemType::Size)>
    itemInfoLookup = {{
      { .type = ld::ItemType::Pickaxe, .cost = 20, .value = 50 },
      { .type = ld::ItemType::Armor,   .cost = 30, .value = 5'000 },
      { .type = ld::ItemType::Speed,   .cost = 50, .value = 10'500 },
    }};

  // if this changes, must update below and in miner.hpp cargo
  enum class ValuableType {
    Stone,
    Food,
    Tin,
    Ruby,
    Emerald,
    Sapphire,
    Size,
  };

  enum class ValuableCurrencyType {
    Gold, Food
  };

  struct ValuableInfoLookup {
    ld::ValuableType type;
    float weightUnitMultiplier = 1.0f;
    int32_t value = 0;
    ValuableCurrencyType currencyType;
  };

  constexpr std::array<ld::ValuableInfoLookup, Idx(ld::ValuableType::Size)>
    valuableInfoLookup = {{
      {
        .type = ld::ValuableType::Stone,
        .weightUnitMultiplier = 1.0f,
        .value = 1,
        .currencyType = ld::ValuableCurrencyType::Gold,
      }, {
        .type = ld::ValuableType::Food,
        .weightUnitMultiplier = 5.0f,
        .value = 500,
        .currencyType = ld::ValuableCurrencyType::Food,
      }, {
        .type = ld::ValuableType::Tin,
        .weightUnitMultiplier = 1.0f,
        .value = 10,
        .currencyType = ld::ValuableCurrencyType::Gold,
      }, {
        .type = ld::ValuableType::Ruby,
        .weightUnitMultiplier = 5.0f,
        .value = 50,
        .currencyType = ld::ValuableCurrencyType::Food,
      }, {
        .type = ld::ValuableType::Emerald,
        .weightUnitMultiplier = 5.0f,
        .value = 100,
        .currencyType = ld::ValuableCurrencyType::Food,
      }, {
        .type = ld::ValuableType::Sapphire,
        .weightUnitMultiplier = 5.0f,
        .value = 150,
        .currencyType = ld::ValuableCurrencyType::Food,
      },


    }};

  struct Valuable {
    ValuableType type;
    int32_t ownedUnits = 0;
  };
}
