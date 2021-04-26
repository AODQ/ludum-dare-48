#pragma once

#include <cstdint>

#include <vector>

namespace ld { struct GameState; }

namespace ld {

  enum class NotifType {
    ItemSold,
    FoodGot,
    PickaxeGot,
    DrillGot,
    ArmorGot,
    ThrowAway,
    Size,
  };

  struct Notif {
    NotifType type;

    int32_t timer;

    int32_t positionX, positionY;
  };

  struct NotifGroup {
    std::vector<Notif> notifs;

    static void Update(ld::GameState & state);

    void AddNotif(ld::NotifType const, int32_t const x, int32_t const y);
  };
}
