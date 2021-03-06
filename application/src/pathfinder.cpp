#include <pathfinder.hpp>

#include <gamestate.hpp>
#include <micropather.hpp>

#include <algorithm>
#include <array>

namespace {
class GraphPathFinder : public micropather::Graph {
public:
  ld::GameState * gameState;
  bool canMine;
  ld::Miner * miner = nullptr;

  float LeastCostEstimate(void * stateStart, void * stateEnd) {
    uint32_t startId = reinterpret_cast<ld::MineRock *>(stateStart)->rockId;
    uint32_t endId   = reinterpret_cast<ld::MineRock *>(stateEnd)->rockId;

    int32_t startX = startId % 30, startY = startId / 30;
    int32_t endX   = endId   % 30, endY   = endId   / 30;

    return std::abs(endX - startX) + std::abs(endY - startY);
  }

  void AdjacentCost(void * state, MP_VECTOR<micropather::StateCost> * adjacent)
  {
    int32_t startId = reinterpret_cast<ld::MineRock *>(state)->rockId;
    int32_t startX = startId % 30, startY = startId / 30;

    // we add 4 adjacencies
    struct Offsets {
      int32_t x, y;
    };

    constexpr std::array<Offsets, 4> offsets = {{
      { -1, 0 }, { +1, 0 }, {  0, +1 }, {  0, -1 },
    }};

    for (auto const offset : offsets) {

      int32_t x = startX + offset.x;
      int32_t y = startY + offset.y;

      if (
          x < 0 || x >= 30
       || y < 0
       || y > static_cast<int32_t>(gameState->mineChasm.rocks.size()/30)
      ) {
        continue;
      }

      ld::MineRock & rock =
        gameState->mineChasm.rock(startX + offset.x, startY + offset.y);

      if (!canMine && !rock.isMined()) { continue; }

      /* // only allow 30 swings */
      /* if ( */
      /*     !rock.isMined() */
      /*  && miner */
      /*  && ( */
      /*       rock.durability */
      /*     / ld::PickLevelDamage( */
      /*         miner->inventory[Idx(ld::ItemType::Pickaxe)].level */
      /*       ) */
      /*     > 20 */
      /*     ) */
      /* ) { */
      /*   continue; */
      /* } */

      float minerValue = ::GetRandomValue(0.0f, +3.0f);
      if (miner) {
        minerValue += gameState->mineChasm.rockPathDurability(x, y);
        minerValue /=
            ld::PickLevelDamage(
              miner->inventory[Idx(ld::ItemType::Pickaxe)].level
            )
        ;

        auto isValuable = rock.gem != ld::RockGemType::Empty;

        minerValue -= gameState->mineChasm.rockPathValue(x, y);

        minerValue = std::clamp(minerValue, isValuable ? 0.1f : 2.0f, 20.0f);
      }

      adjacent->push_back(micropather::StateCost {
        .state = reinterpret_cast<void *>(&rock),
        .cost = rock.isMined() ? 1.0f : (minerValue)
        ,
      });
    }
  }

  void PrintStateInfo(void * /*state*/)
  {
  }
};

GraphPathFinder graphMinable;
GraphPathFinder graphUnminable;

// couldnt use unique ptr here?
micropather::MicroPather * patherMinable = nullptr;
micropather::MicroPather * patherUnminable = nullptr;
} // -- namespace

void ld::pathFindInitialize(ld::GameState * state) {
  graphMinable.gameState = state;
  graphUnminable.gameState = state;

  graphMinable.canMine = true;
  graphUnminable.canMine = false;

  if (patherMinable) {
    delete patherMinable;
  }

  if (patherUnminable) {
    delete patherUnminable;
  }

  // couldnt use make_unique
  patherMinable =
    new micropather::MicroPather(micropather::MicroPather(&graphMinable));
  patherUnminable =
    new micropather::MicroPather(micropather::MicroPather(&graphUnminable));
}

uint32_t ld::pathFind(
  ld::GameState const & state,
  std::array<::Vector2, 4> & path, size_t & pathSize,
  int32_t const origTileX,   int32_t const origTileY,
  int32_t       targetTileX, int32_t       targetTileY,
  ld::Miner * miner
) {
  micropather::MPVector<void *> microPath;
  auto & startTile = state.mineChasm.rock(origTileX, origTileY);

  patherMinable->Reset();
  patherUnminable->Reset();

  /* //  TODO limit the end tile to a radius of 4 */
  /* if (std::abs(origTileX - targetTileX) + std::abs(origTileY - targetTileY)) { */
  /*   float theta = atan2(origTileY - targetTileY, origTileX - targetTileX); */
  /*   targetTileX = origTileX - cos(theta)*5; */
  /*   targetTileY = origTileY - sin(theta)*5; */
  /* } */

  auto & endTile   = state.mineChasm.rock(targetTileX, targetTileY);

  float totalCost;

  graphMinable.miner = miner;

  (miner ? patherMinable : patherUnminable)
    ->Solve(
      reinterpret_cast<void *>(const_cast<ld::MineRock *>(&startTile)),
      reinterpret_cast<void *>(const_cast<ld::MineRock *>(&endTile)),
      &microPath,
      &totalCost
    );

  pathSize = std::min(4u, microPath.size());
  for (uint32_t i = 0u; i < pathSize; ++ i) {
    uint32_t pathId = reinterpret_cast<ld::MineRock *>(microPath[i])->rockId;
    path[i] =
      ::Vector2{
        static_cast<float>(pathId % 30),
        static_cast<float>(pathId / 30)
      };
  }

  return microPath.size();
}

void ld::pathClear() {
  patherMinable->Reset();
  patherUnminable->Reset();
}
