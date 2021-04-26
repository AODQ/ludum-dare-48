#pragma once

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

  void SoundUpdate();

  void SoundPlay(SoundType const, float distance);

  void ToggleMuteSound();
  void ToggleMuteMedia();
}
