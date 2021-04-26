#pragma once

namespace ld {
  enum class SoundType {
    RockHit,
    RockHit2,
    RockHit3,
    RockHit4,
    Size,
  };


  void SoundInitialize();
  void SoundShutdown();

  void SoundUpdate();

  void SoundPlay(SoundType const, float distance);
}
