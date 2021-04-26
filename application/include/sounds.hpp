#pragma once

namespace ld { struct GameState; }

namespace ld {
  enum class SoundType {
    RockHit,
    Slime,
    SlimeDie,
    MinerDie,
    Explosion,
    Size,
  };

  void SoundInitialize();
  void SoundShutdown();

  void SoundUpdate(ld::GameState const & state);

  void SoundPlay(SoundType const, float distance);

  void ToggleMuteSound();
  void ToggleMuteMedia();
}
