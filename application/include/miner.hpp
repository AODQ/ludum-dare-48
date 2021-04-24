#pragma once

#include <enum.hpp>
#include <items.hpp>

#include <raylib.h>

#include <array>
#include <string>
#include <vector>

namespace ld {

  struct Miner {
      // position is in texels, not tiles
      int32_t xPosition = 0, yPosition = 0;
      int32_t energy = 100;
      std::array<ld::Item, Idx(ld::ItemType::Size)> inventory = {{
        { .type = ld::ItemType::Pickaxe1, .owns = false },
        { .type = ld::ItemType::Pickaxe2, .owns = false },
        { .type = ld::ItemType::Armor,    .owns = false },
      }};

      uint32_t cargoCapacity = 100u;
      std::array<ld::Valuable, Idx(ld::ValuableType::Size)> cargo = {{
        {
          .type = ld::ValuableType::Tin,
          .ownedUnits = 0,
        }, {
          .type = ld::ValuableType::Food,
          .ownedUnits = 0,
        },
      }};

      // this syncs with the miner.png file vertically
      enum class AnimationState {
        Travelling,
        Mining,
        Fighting,
        Idling,
      };

      AnimationState animationState = AnimationState::Idling;

      // this syncs with the miner.png file horizontally, scaled by 60Hz
      int32_t animationIdx = 0;

      enum class AiState {
        Mining,
        Attacking,
        Traversing,
      };

      union AiStateInternal {
        struct Mining {
          int32_t targetRockId = 0;
        };

        struct Attacking {
          int32_t targetEnemyId = 0;
        };

        struct Traversing {
          int32_t targetTileX = 0, targetTileY = 0;

          bool wantsToSurface = false;
        };

        Mining     mining;
        Attacking  attacking;
        Traversing traversing;
      };

      AiStateInternal aiStateInternal;

      bool isSurfaced = true;
  };

  struct MinerGroup {
      std::vector<ld::Miner> miners;

      std::vector<size_t> surfacedMiners; // interacting w/ surface
      std::vector<size_t> chasmMiners; // mining down in the chasm

      static MinerGroup Initialize();

      void Update();
  };
}
