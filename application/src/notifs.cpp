#include <notifs.hpp>

#include <gamestate.hpp>

void ld::NotifGroup::Update(ld::GameState & state)
{
  for (
    int32_t it = 0;
    it < static_cast<int32_t>(state.notifGroup.notifs.size());
    ++ it
  ) {
    auto & notif = state.notifGroup.notifs[it];
    notif.timer -= 1;
    if (notif.timer <= 0) {
      state.notifGroup.notifs.erase(state.notifGroup.notifs.begin() + it);
      it -= 1;
    }

    notif.positionY -= 1;
  }
}

void ld::NotifGroup::AddNotif(
  ld::NotifType const notifType, int32_t const x, int32_t const y
) {
  auto & self = *this;

  self.notifs.push_back({
    .type = notifType,
    .timer = 120,
    .positionX = x, .positionY = y,
  });
}
