#pragma once

#include <enum.hpp>
#include <items.hpp>

#include <raylib.h>

#include <array>
#include <string>
#include <vector>

namespace ld { struct GameState; }

namespace ld {

  struct Miner {
      size_t minerId;
      // position is in texels, not tiles
      int32_t xPosition = 0, yPosition = 0;
      int32_t prevXPosition = 0, prevYPosition = 0;
      int32_t energy = 5;
      std::array<ld::Item, Idx(ld::ItemType::Size)> inventory = {{
        { .type = ld::ItemType::Pickaxe1, .owns = false },
        { .type = ld::ItemType::Pickaxe2, .owns = false },
        { .type = ld::ItemType::Armor,    .owns = false },
      }};

      void moveTowards(int32_t x, int32_t y);

      bool animationFinishesThisFrame();
      void reduceEnergy(int32_t units);
      void kill();

      uint32_t cargoCapacity = 5u;
      uint32_t currentCargoCapacity = 0u;
      std::array<ld::Valuable, Idx(ld::ValuableType::Size)> cargo = {{
        {
          .type = ld::ValuableType::Stone,
          .ownedUnits = 0,
        }, {
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
        Dying,
      };

      AnimationState animationState = AnimationState::Idling;

      // this syncs with the miner.png file horizontally, scaled by 60Hz
      int32_t animationIdx = 0;

      enum class AiState {
        Mining,
        Attacking,
        Traversing,
        Idling,
        Surfaced,
        Dying,
      };

      enum class AiStateSurfaced {
        Surfacing,
        MovingToBase,
        DumpingMaterial,
        PurchasingUpgrades,
        BackToMine,
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
          int32_t waitTimer = 0;
        };

        struct Idling {
        };

        struct Dying {
        };

        struct Surfaced {
          AiStateSurfaced state = AiStateSurfaced::Surfacing;

          int32_t waitTimer = -1;
        };

        Mining     mining;
        Attacking  attacking;
        Traversing traversing;
        Idling     idling;
        Surfaced   surfaced;
        Dying      dying;
      };

      AiState aiState;
      AiStateInternal aiStateInternal;

      bool isSurfaced = true;
  };

  struct MinerGroup {
      std::vector<ld::Miner> miners;

      static MinerGroup Initialize();

      static void Update(ld::GameState & state);

      void addMiner();
  };
}
