#include <sounds.hpp>

#include <enum.hpp>

#include <raylib.h>

#include <array>

namespace {
  std::array<::Sound, 4> rockHitSounds;
  ::Music stream;
}


void ld::SoundInitialize()
{
  ::SetMasterVolume(100.0f);

  TraceLog(LOG_INFO, "audio ready? %d", ::IsAudioDeviceReady());

  rockHitSounds[0] = ::LoadSound("resources/hit1.ogg");
  rockHitSounds[1] = ::LoadSound("resources/hit2.ogg");
  rockHitSounds[2] = ::LoadSound("resources/hit3.ogg");
  rockHitSounds[3] = ::LoadSound("resources/hit4.ogg");

  /* rockHitSounds[0] = ::LoadSound("resources/hit1.ogg"); */
  /* rockHitSounds[1] = ::LoadSound("resources/hit2.ogg"); */
  /* rockHitSounds[2] = ::LoadSound("resources/hit3.ogg"); */
  /* rockHitSounds[3] = ::LoadSound("resources/hit4.ogg"); */

  stream = ::LoadMusicStream("resources/mine.ogg");
  ::PlayMusicStream(stream);
}

void ld::SoundShutdown()
{
  // TODO
  ::CloseAudioDevice();
}

void ld::SoundUpdate()
{
  ::UpdateMusicStream(stream);
}

void ld::SoundPlay(ld::SoundType const type, float distance)
{
  distance = 1.0f - std::clamp(std::abs(distance) / 900.0f, 0.0f, 1.0f);

  ::Sound * sound = nullptr;

  if (type == ld::SoundType::RockHit) {
    sound = &rockHitSounds[::GetRandomValue(0, 3)];
  }

  ::SetSoundVolume(*sound, distance);
  ::PlaySound(*sound);

  if (!sound) { return; }
}
