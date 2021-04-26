#pragma once

namespace ld {
  enum class SoundType {
    RockHit,
    Slime,
    Size,
  };


  void SoundInitialize();
  void SoundShutdown();

  void SoundUpdate();

  void SoundPlay(SoundType const, float distance);
}
