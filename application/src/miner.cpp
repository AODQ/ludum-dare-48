#include <miner.hpp>

#include <gamestate.hpp>
#include <pathfinder.hpp>
#include <sounds.hpp>

#include <algorithm>

int32_t ld::PickLevelDamage(int32_t level) {
  switch (level) {
    default: return 5;
    case 1:  return 10;
    case 2:  return 15;
    case 3:  return 20;
    case 4:  return 25;
    case 5:  return 30;
    case 6:  return 35;
    case 7:  return 40;
    case 8:  return 50;
    case 9:  return 60;
    case 10: return 75;
  }
}

ld::MinerGroup ld::MinerGroup::Initialize() {
  ld::MinerGroup self;

  self.addMiner();

  return self;
}

void ld::Miner::damageEquipment(ld::ItemType type)
{
  auto & self = *this;
  auto & item = self.inventory[Idx(type)];
  if (item.owns && item.level > 0) {
    item.durability -= 1;
    if (item.durability <= 0) {
      item.level = std::max(0, (int32_t)(item.level)-1);
      item.durability = item.level > 0 ? 10 : 0;
      if (item.level == 0) {
        item.owns = false;
      }
    }
  }
}

void ld::Miner::moveTowards(int32_t x, int32_t y)
{
  auto & self = *this;
  self.prevXPosition = self.xPosition;
  self.prevYPosition = self.yPosition;

  uint32_t speedLevel = inventory[Idx(ld::ItemType::Speed)].level + 1;
  for (uint32_t i = 0; i < speedLevel; ++ i) {
    self.xPosition -= ld::sgn(self.xPosition - x);
    self.yPosition -= ld::sgn(self.yPosition - y);
  }
}

int32_t ld::Miner::animationFrames() const
{
  auto & self = *this;
  return self.animationState == ld::Miner::AnimationState::Idling ? 3 : 4;
}

bool ld::Miner::animationFinishesThisFrame()
{
  auto & self = *this;
  return
    (self.animationIdx + 5) % (self.animationFrames() * 60) < self.animationIdx
  ;
}

void ld::Miner::reduceEnergy(int32_t units)
{

  auto & self = *this;
  self.energy = std::max(0, self.energy - std::abs(units));

  if (self.energy == 0) { self.kill(); }
}

void ld::Miner::kill()
{
  auto & self = *this;
  if (self.aiState == ld::Miner::AiState::Dying) { return; }

  self.aiState = ld::Miner::AiState::Dying;
  self.energy = -1; //  state doesnt matter if energy is 0
  self.aiStateInternal.dying = {};
  self.applyAnimationState(ld::Miner::AnimationState::Dying);

  ld::SoundPlay(
    ld::SoundType::MinerDie,
    1.0f // always audible
  );
}

void ld::Miner::chooseNewTarget(ld::GameState & gameState)
{
  auto & self = *this;
  auto & state = self.aiStateInternal.traversing;

  int32_t targetX = self.xPosition / 32;
  int32_t targetY = self.xPosition / 32;

  if (gameState.targetX >= 0 && gameState.targetY >= 0) {
    targetX = gameState.targetX;
    targetY = gameState.targetY;
  }

  state.targetTileX =
    gameState.mineChasm.limitX(
      targetX + ::GetRandomValue(-10, +10)
    );

  state.targetTileY =
    gameState.mineChasm.limitY(
      targetY + ::GetRandomValue(-10, +10)
    );
}

void ld::Miner::surfaceMiner()
{
  auto & self = *this;
  if (self.aiState == ld::Miner::AiState::Surfaced) { return; }
  self.resetToTraversal();
  self.aiStateInternal.traversing.wantsToSurface = true;
  self.aiStateInternal.traversing.targetTileX = self.xPosition/32;
  self.aiStateInternal.traversing.targetTileY = 0;

  if (self.yPosition < 48) {
    self.aiState = ld::Miner::AiState::Surfaced;
    self.aiStateInternal.surfaced.state =
      ld::Miner::AiStateSurfaced::Surfacing;
    self.aiStateInternal.surfaced.waitTimer = -1;
  }
}

namespace {

void MinerPickLocation(
  ld::Miner & miner,
  int32_t const targetTileX,
  int32_t const targetTileY,
  ld::GameState const & gameState
) {
  miner.resetToTraversal();
  auto & state = miner.aiStateInternal.traversing;

  ld::pathFind(
    gameState,
    state.path, state.pathSize,
    miner.xPosition / 32, miner.yPosition / 32,
    targetTileX, targetTileY,
    state.wantsToSurface ? nullptr : &miner
  );
}

void UpdateMinerCargo(ld::Miner & miner) {
  miner.currentCargoCapacity = 0;
  for (size_t it = 0u; it < miner.cargo.size(); ++ it) {
    miner.currentCargoCapacity += miner.cargo[it].ownedUnits;
  }
}

void UpdateMinerInventory(ld::Miner & miner, ld::MineRock const & rock)
{
  switch (rock.gem) {
    default:
      miner.cargo[Idx(ld::ValuableType::Stone)].ownedUnits += 2;
      break;
    case ld::RockGemType::Tin:
      miner.cargo[Idx(ld::ValuableType::Tin)].ownedUnits += 5;
      break;
    case ld::RockGemType::Ruby:
      miner.cargo[Idx(ld::ValuableType::Ruby)].ownedUnits += 10;
      break;
    case ld::RockGemType::Emerald:
      miner.cargo[Idx(ld::ValuableType::Emerald)].ownedUnits += 20;
      break;
    case ld::RockGemType::Sapphire:
      miner.cargo[Idx(ld::ValuableType::Sapphire)].ownedUnits += 25;
      break;
    case ld::RockGemType::Food:
      miner.cargo[Idx(ld::ValuableType::Food)].ownedUnits += 25;
      break;
  }

  UpdateMinerCargo(miner);
}

void UpdateMinerAiInventorying(ld::Miner & miner, ld::GameState & gameState) {
  miner.applyAnimationState(ld::Miner::AnimationState::Idling);

  auto & state = miner.aiStateInternal.inventorying;
  if (state.waitTimer < 0) {
    state.waitTimer = 40;
    return;
  }

  state.waitTimer -= 1;

  if (state.waitTimer >= 0) { return; }

  UpdateMinerCargo(miner);

  if (miner.currentCargoCapacity <= miner.cargoCapacity) {
    miner.resetToTraversal();
    return;
  }

  for (size_t i = 0; i < Idx(ld::RockGemType::Size); ++ i) {
    if (miner.cargo[i].ownedUnits > 0) {
      -- miner.cargo[i].ownedUnits;
      UpdateMinerCargo(miner);
      break;
    }
  }

  gameState.notifGroup.AddNotif(
    ld::NotifType::ThrowAway, miner.xPosition-8, miner.yPosition-16
  );
}

void UpdateMinerAiMining(ld::Miner & miner, ld::GameState & state) {
  auto rockId = miner.aiStateInternal.mining.targetRockId;
  auto & rock = state.mineChasm.rock(rockId);

  if (miner.currentCargoCapacity >= miner.cargoCapacity) {
    miner.surfaceMiner();
  }

  // reset back to traversing as before, no need to clear traversal
  if (rock.isMined()) {
    miner.aiState = ld::Miner::AiState::Traversing;
    return;
  }

  if (
      rock.durability
    / ld::PickLevelDamage(miner.inventory[Idx(ld::ItemType::Pickaxe)].level)
    > 20
  )
  {
    miner.resetToTraversal();

    // panic if stuck
    if (!state.mineChasm.rock(miner.xPosition/32, miner.yPosition/32).isMined())
    {
      miner.surfaceMiner();
    }
  }

  miner.applyAnimationState(ld::Miner::AnimationState::Mining);

  if (miner.animationFinishesThisFrame()) {
    miner.reduceEnergy(10);
    miner.damageEquipment(ld::ItemType::Pickaxe);
    rock.receiveDamage(
      -ld::PickLevelDamage(miner.inventory[Idx(ld::ItemType::Pickaxe)].level)
    );

    if (rock.isMined()) {
      UpdateMinerInventory(miner, rock);
    }

    ld::SoundPlay(
      ld::SoundType::RockHit,
      miner.yPosition - state.camera.y
    );

    if (
        miner.currentCargoCapacity >= miner.cargoCapacity
     || miner.wantsToSurface()
    ) {
      miner.surfaceMiner();
    }
  }
}

void UpdateMinerAiTraversing(ld::Miner & miner, ld::GameState & gameState)
{
  auto & state = miner.aiStateInternal.traversing;

  if (state.pathIdx >= state.pathSize) {

    MinerPickLocation(
      miner,
      state.targetTileX,
      state.targetTileY,
      gameState
    );

    // if no path was selected just change target
    if (state.pathIdx >= state.pathSize) {
      miner.chooseNewTarget(gameState);
      state.panic += 1;
      if (state.panic + 1 > 5) {
        miner.surfaceMiner();
      }
      return;
    } else {
      state.panic = 0;
    }
  }

  auto & path = state.path[state.pathIdx];

  miner.applyAnimationState(ld::Miner::AnimationState::Travelling);
  miner.moveTowards(
    path.x*32.0f + state.targetPosOffX,
    path.y*32.0f + state.targetPosOffY
  );

  if (miner.animationFinishesThisFrame()) {
    miner.reduceEnergy(5);
    miner.damageEquipment(ld::ItemType::Speed);
  }

  ::Rectangle rect = {
    .x = path.x*32.0f, .y = path.y*32.0f,
    .width = 32.0f, .height = 32.0f,
  };

  if (!state.wantsToSurface && miner.wantsToSurface()) {
    miner.surfaceMiner();
    return;
  }

  if (
    ::CheckCollisionCircleRec(
      ::Vector2 {
        static_cast<float>(miner.xPosition),
        static_cast<float>(miner.yPosition),
      },
      8.0f,
      rect
    )
  ) {
    state.pathIdx += 1;

    if (state.wantsToSurface && miner.yPosition < 48.0f) {
      // surface
      miner.aiState = ld::Miner::AiState::Surfaced;
      miner.aiStateInternal.surfaced.state =
        ld::Miner::AiStateSurfaced::Surfacing;
      miner.aiStateInternal.surfaced.waitTimer = -1;
    }

    state.targetPosOffX = ::GetRandomValue(0, 16);
    state.targetPosOffY = ::GetRandomValue(0, 16);

    auto rockId = gameState.mineChasm.rockId(path.x, path.y);
    auto & rock = gameState.mineChasm.rock(rockId);
    if (!rock.isMined() && !miner.aiStateInternal.traversing.wantsToSurface) {
      miner.aiState = ld::Miner::AiState::Mining;
      miner.aiStateInternal.mining.targetRockId = rockId;
    }

    if (state.targetTileX == path.x && state.targetTileY == path.y) {

      // should just leave
      if (miner.wantsToSurface()) {
        miner.surfaceMiner();
        return;
      }

      miner.chooseNewTarget(gameState);
    }
  }
}

void UpdateMinerAiSurfaced(ld::Miner & miner, ld::GameState & gameState)
{
  auto & state = miner.aiStateInternal.surfaced;
  switch (state.state) {
    case ld::Miner::AiStateSurfaced::Surfacing:

      miner.animationState = ld::Miner::AnimationState::Idling;

      if (state.waitTimer < 0) {
        state.waitTimer = 4*60;
      }

      miner.alpha =
        state.waitTimer > 120
          ? (1.0f - (120 - state.waitTimer)/120.0f) * 255
          : (1.0f - state.waitTimer/120.0f) * 255
      ;

      if (state.waitTimer <= 120) {
        miner.xPosition = 700;
        miner.yPosition = -80;
      }

      if (state.waitTimer == 0) {
        miner.alpha = 255;
        state.state = ld::Miner::AiStateSurfaced::MovingToBase;
        miner.animationState = ld::Miner::AnimationState::Travelling;
        miner.animationIdx = 0;

        state.targetX = ::GetRandomValue( 30,  70);
        state.targetY = ::GetRandomValue(-90, -80);
      }

      state.waitTimer -= 1;
    break;
    case ld::Miner::AiStateSurfaced::MovingToBase:
      miner.moveTowards(state.targetX, state.targetY);

      if (miner.animationFinishesThisFrame()) {
        miner.reduceEnergy(5);
        miner.damageEquipment(ld::ItemType::Speed);
      }

      if (
          miner.xPosition == state.targetX
       && miner.yPosition == state.targetY
      ) {
        miner.animationState = ld::Miner::AnimationState::Idling;
        miner.animationIdx = 0;
        state.waitTimer = 50;
        state.state = ld::Miner::AiStateSurfaced::DumpingMaterial;
      }
    break;
    case ld::Miner::AiStateSurfaced::DumpingMaterial: {
      state.waitTimer -= 1;

      if (state.waitTimer > 0) { break; }

      // sell item
      state.waitTimer = 20;

      bool hasSold = false;

      for (auto & cargo : miner.cargo) {
        if (cargo.ownedUnits == 0) { continue; }

        if (cargo.type == ld::ValuableType::Food) {
          gameState.food += 25;
          hasSold = true;
          cargo.ownedUnits -= 1;
          UpdateMinerCargo(miner);
          gameState.notifGroup.AddNotif(
            ld::NotifType::FoodGot, miner.xPosition-8, miner.yPosition-16
          );
          break;
        }

        auto value = ld::valuableInfoLookup[Idx(cargo.type)].value;
        miner.netValue += value;
        gameState.gold += value;
        hasSold = true;
        cargo.ownedUnits -= 1;
        UpdateMinerCargo(miner);
        gameState.notifGroup.AddNotif(
          ld::NotifType::ItemSold, miner.xPosition-8, miner.yPosition-16
        );
        break;
      }

      if (!hasSold) {
        state.state = ld::Miner::AiStateSurfaced::PurchasingUpgrades;
        state.hasPurchasedFood = false;
        state.waitTimer = 80;
      }

    } break;
    case ld::Miner::AiStateSurfaced::PurchasingUpgrades: {
      state.waitTimer -= 1;
      if (state.waitTimer > 0) { break; }
      state.waitTimer = 20;

      bool readyToContinue = true;

      // Find the highest upgrade that they can purchase
      // and that the bank can afford
      // TODO add additional purchase states
      // Buy highest / cheapest
      // Conserve money, etc
      bool canBuyUpgrade = false;
      if (readyToContinue)
      {
        canBuyUpgrade = false;
        uint32_t highestCost = 0;
        size_t selectedUpgradeType = -1;
        for (size_t i = 0; i < Idx(ld::ItemType::Size); ++i) {
          uint32_t cost = ld::itemInfoLookup[i].cost;
          if (
                (miner.netValue >= cost)
             && (gameState.gold >= static_cast<int32_t>(cost))
             && (highestCost < cost)
             && (miner.inventory[i].level < gameState.researchItems[i].level)
          ) {
            highestCost = cost;
            canBuyUpgrade = true;
            selectedUpgradeType = i;
          }
        }

        // Purchase highest upgrade
        if (canBuyUpgrade) {
          readyToContinue = false;
          miner.netValue -= highestCost;
          gameState.gold -= highestCost;
          miner.inventory[selectedUpgradeType].owns = true;
          miner.inventory[selectedUpgradeType].level++;

          gameState.notifGroup.AddNotif(
            static_cast<ld::NotifType>(
              Idx(ld::NotifType::PickaxeGot)+selectedUpgradeType
            ),
            miner.xPosition-8, miner.yPosition-16
          );
          state.waitTimer = 80;
        }
      }

      // be conservative ; don't eat unless there is 100% efficiency
      if (
          readyToContinue
       &&
          ( // conserve if they have purchased food, otherwise must waste
            state.hasPurchasedFood
              ? (miner.energy < miner.maxEnergy)
              : (miner.energy <= miner.maxEnergy - miner.foodToEnergyRatio)
          )
       && gameState.food > 0
      ) {
        state.hasPurchasedFood = true;
        gameState.food -= 1;
        miner.energy =
          std::min(miner.maxEnergy, miner.energy + miner.foodToEnergyRatio);
        readyToContinue = false;
        gameState.notifGroup.AddNotif(
          ld::NotifType::FoodGot, miner.xPosition-8, miner.yPosition-16
        );
      }

      // top off for free only if they have already purchased
      if (
          readyToContinue
       && state.hasPurchasedFood
       && miner.energy + miner.foodToEnergyRatio >= miner.maxEnergy
      ) {
        miner.netValue = 0;
        miner.energy = miner.maxEnergy;
        readyToContinue = false;
        gameState.notifGroup.AddNotif(
          ld::NotifType::FoodGot, miner.xPosition-8, miner.yPosition-16
        );
        state.hasPurchasedFood = false;
      }

      // waste food
      if (gameState.food > gameState.MaxFood()) {
        readyToContinue = false;
        gameState.food -= 20;

        if (gameState.food < gameState.MaxFood()) {
          gameState.food = gameState.MaxFood();
        }
        gameState.notifGroup.AddNotif(
          ld::NotifType::ThrowAway, miner.xPosition-8, miner.yPosition-16
        );
      }


      if (readyToContinue) {
        state.state = ld::Miner::AiStateSurfaced::BackToMine;
        miner.animationState = ld::Miner::AnimationState::Travelling;
        miner.animationIdx = 0;
      }
    } break;
    case ld::Miner::AiStateSurfaced::BoughtFromMine:
      miner.moveTowards(700, -80);
      miner.applyAnimationState(ld::Miner::AnimationState::Travelling);
      miner.alpha = miner.alpha == 255 ? 255 : miner.alpha + 1;
      if (miner.xPosition == 700 && miner.yPosition == -80) {
        miner.aiState = ld::Miner::AiState::Idling;
        miner.aiStateInternal.idling.waitTimer = -1;
        miner.alpha = 255;
      }
    break;
    case ld::Miner::AiStateSurfaced::BackToMine:
      miner.moveTowards(700, -80);
      if (miner.xPosition == 700 && miner.yPosition == -80) {
        miner.aiState = ld::Miner::AiState::Idling;
        miner.aiStateInternal.idling.waitTimer = -1;
      }
    break;
  }
}

void UpdateMinerAiIdling(ld::Miner & miner, ld::GameState & gameState)
{
  miner.animationState = ld::Miner::AnimationState::Idling;

  if (
      miner.currentCargoCapacity >= miner.cargoCapacity
   || miner.wantsToSurface()
  ) {
    miner.surfaceMiner();
  }

  auto & state = miner.aiStateInternal.idling;

  if (miner.yPosition <= 0 && state.waitTimer < 0) {
    state.waitTimer = 4*60;
  }

  state.waitTimer -= 1;

  miner.alpha =
    state.waitTimer > 120
      ? (1.0f - (120 - state.waitTimer)/120.0f) * 255
      : (1.0f - state.waitTimer/120.0f) * 255
  ;

  if (miner.yPosition <= 0 && state.waitTimer <= 120) {
      miner.xPosition = ::GetRandomValue(100, 700);
      miner.yPosition = ::GetRandomValue(10, 30);
  }

  if (miner.animationFinishesThisFrame()) {
    miner.reduceEnergy(1);
  }

  if (state.waitTimer > 0) { return; }

  miner.alpha = 255;

  // draw
  if (miner.minerId == gameState.minerSelection) {
    if (::IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
    {
      auto mousePos = ::GetMousePosition();
      miner.animationIdx = 0;
      miner.animationState = ld::Miner::AnimationState::Travelling;
      miner.resetToTraversal();
      miner.aiStateInternal.traversing.targetTileX = mousePos.x / 32.0f;
      miner.aiStateInternal.traversing.targetTileY =
        (mousePos.y + gameState.camera.y) / 32.0f;
      miner.aiStateInternal.traversing.wantsToSurface = false;
    }
  } else if (gameState.targetX >= 0 && gameState.targetY >= 0)
  {
      miner.animationIdx = 0;
      miner.animationState = ld::Miner::AnimationState::Travelling;
      miner.resetToTraversal();
      miner.aiStateInternal.traversing.wantsToSurface = false;
      miner.chooseNewTarget(gameState);
  }

}

} // -- namespace


void ld::Miner::AddUnit(ld::ValuableType const type, int32_t units) {
  auto & self = *this;
  self.cargo[Idx(type)].ownedUnits += units;

  UpdateMinerCargo(self);
}

void ld::MinerGroup::Update(ld::GameState & state) {
  auto & self = state.minerGroup;

  // Update miner stats based on upgrades
  for (int64_t i = 0; i < self.miners.size(); ++ i) {
    auto & miner = self.miners[i];
    // Passive buffs that don't need to be purchased
    miner.cargoCapacity = state.MaxCargoCapacity();
    miner.foodToEnergyRatio = state.FoodToEnergyRatio();
    // Vision
  }

  // selecting a miner via mouse click
  if (::IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    auto mousePos = ::GetMousePosition();
    for (int64_t i = 0; i < self.miners.size(); ++ i) {
      auto & miner = self.miners[i];
      if(
        ::CheckCollisionPointCircle(
          mousePos,
          ::Vector2 {
            miner.xPosition + 8.0f,
            miner.yPosition + 8.0f - state.camera.y,
          },
          9.0f
        )
      ) {
        state.minerSelection = miner.minerId;
        break;
      }
    }
  }

  for (int64_t i = 0; i < self.miners.size(); ++ i) {
    auto & miner = self.miners[i];

    if (
        miner.aiState != ld::Miner::AiState::Dying
     && miner.aiState != ld::Miner::AiState::Fighting
    ) {
      if (miner.currentCargoCapacity > miner.cargoCapacity) {
        miner.aiState = ld::Miner::AiState::Inventorying;
      }
    }

    // force dying state (in case overriden by inventorying / fighting)
    if (miner.energy <= 0) {
      miner.aiState = ld::Miner::AiState::Dying;
      miner.applyAnimationState(ld::Miner::AnimationState::Dying);
    }

    switch (miner.aiState) {
      case ld::Miner::AiState::Inventorying:
        UpdateMinerAiInventorying(miner, state);
      break;
      case ld::Miner::AiState::Mining:
        UpdateMinerAiMining(miner, state);
      break;
      case ld::Miner::AiState::Idling:
        UpdateMinerAiIdling(miner, state);
      break;
      case ld::Miner::AiState::Fighting:
        // don't do anything special, let the slime take care of it
        // this is important since multiple slimes can be fighting at
        // a time
        miner.applyAnimationState(ld::Miner::AnimationState::Fighting);
        miner.aiStateInternal.fighting.hasSwung = false;
        miner.alpha = 255; // in case slime attacks miner while surfacing

        // force break off after one swing
        if (miner.aiStateInternal.fighting.prevFrameFinished) {
          miner.resetToTraversal();
        }
        miner.aiStateInternal.fighting.prevFrameFinished =
          miner.animationFinishesThisFrame();
      break;
      case ld::Miner::AiState::Surfaced:
        UpdateMinerAiSurfaced(miner, state);
      break;
      case ld::Miner::AiState::Traversing:
        UpdateMinerAiTraversing(miner, state);
      break;
      case ld::Miner::AiState::Dying:
        if (miner.animationFinishesThisFrame()) {
          self.miners.erase(self.miners.begin() + i);
          i -= 1;
          continue;
        }
      break;
    }
  }

  for (auto & miner : self.miners) {
    miner.animationIdx =
      (miner.animationIdx + 5) % (miner.animationFrames() * 60);
  }

  // update fog of war
  for (auto & miner : self.miners) {
    if (miner.aiState == ld::Miner::AiState::Surfaced) { continue; }

    int32_t const minBoundsX = state.mineChasm.limitX(miner.xPosition/32 - 3);
    int32_t const minBoundsY = state.mineChasm.limitY(miner.yPosition/32 - 3);
    int32_t const maxBoundsX = state.mineChasm.limitX(miner.xPosition/32 + 3,1);
    int32_t const maxBoundsY = state.mineChasm.limitY(miner.yPosition/32 + 3);

    for (int32_t x = minBoundsX; x < maxBoundsX; ++ x)
    for (int32_t y = minBoundsY; y < maxBoundsY; ++ y) {
      auto & fow = state.mineChasm.rockFow[state.mineChasm.rockId(x, y)];

      float len =
        std::clamp(
            (3*::sqrt(2.0f))
          - static_cast<float>(
              ::sqrt(
                (miner.xPosition/32 - x)*(miner.xPosition/32 - x)
              + (miner.yPosition/32 - y)*(miner.yPosition/32 - y)
              )
            )
          , 0.0f, (3*::sqrt(2.0f))
        ) * (miner.alpha / 255.0f)
      ;

      fow = std::clamp(len/(3.0f), fow, 1.0f);
    }
  }
}

void ld::MinerGroup::addMiner()
{
  auto & self = *this;

  auto id = self.miners.size();

  self.miners.push_back(
    ld::Miner {
      .minerId = static_cast<int32_t>(id),
      .xPosition = 300,
      .yPosition = -150,
      .alpha = 0,

      .aiState = ld::Miner::AiState::Surfaced,
      .aiStateInternal = {
        .surfaced = {
          .state = ld::Miner::AiStateSurfaced::BoughtFromMine,
          .waitTimer = -1,
        },
      },
    }
  );
}
