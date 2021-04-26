#include <sounds.hpp>

#include <enum.hpp>

#include <raylib.h>

#include <array>

namespace {
  std::array<::Sound, 4> rockHitSounds;
  std::array<::Sound, 4> slimeSounds;
  ::Sound slimeDie;
  ::Sound minerDie;
  ::Sound explosion;
  ::Music stream;

  bool muteSound = false; bool muteMedia = false;
}


void ld::SoundInitialize()
{
  ::SetMasterVolume(100.0f);

  TraceLog(LOG_INFO, "audio ready? %d", ::IsAudioDeviceReady());

  rockHitSounds[0] = ::LoadSound("resources/hit1.ogg");
  rockHitSounds[1] = ::LoadSound("resources/hit2.ogg");
  rockHitSounds[2] = ::LoadSound("resources/hit3.ogg");
  rockHitSounds[3] = ::LoadSound("resources/hit4.ogg");

  slimeSounds[0] = ::LoadSound("resources/slime1.ogg");
  slimeSounds[1] = ::LoadSound("resources/slime2.ogg");
  slimeSounds[2] = ::LoadSound("resources/slime3.ogg");
  slimeSounds[3] = ::LoadSound("resources/slime4.ogg");

  slimeDie = ::LoadSound("resources/slime4.ogg");

  minerDie  = ::LoadSound("resources/slime2.ogg");
  explosion = ::LoadSound("resources/hit3.ogg");

  ::SetSoundPitch(slimeDie, 0.5f);
  ::SetSoundPitch(minerDie, 0.1f);
  ::SetSoundPitch(explosion, 0.5f);

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
  if (!muteMedia)
    ::UpdateMusicStream(stream);
}

void ld::SoundPlay(ld::SoundType const type, float distance)
{
  distance = 1.0f - std::clamp(std::abs(distance) / 900.0f, 0.0f, 1.0f);

  ::Sound * sound = nullptr;

  switch (type) {
    default: break;
    case ld::SoundType::RockHit:
      sound = &rockHitSounds[::GetRandomValue(0, 3)];
    break;
    case ld::SoundType::Slime:
      sound = &slimeSounds[::GetRandomValue(0, 3)];
    break;
    case ld::SoundType::SlimeDie:
      sound = &slimeDie;
    break;
    case ld::SoundType::MinerDie:
      sound = &slimeDie;
    break;
    case ld::SoundType::Explosion:
      sound = &explosion;
    break;
  }

  ::SetSoundVolume(*sound, distance);
  if (!muteSound) {
    ::PlaySound(*sound);
  }

  if (!sound) { return; }
}

void ld::ToggleMuteSound() {
  muteSound ^= 1;

  for (auto & s : rockHitSounds) ::StopSound(s);
  for (auto & s : slimeSounds) ::StopSound(s);
}
void ld::ToggleMuteMedia() {
  muteMedia ^= 1;
  if (muteMedia)
    ::StopMusicStream(stream);
  else
    ::ResumeMusicStream(stream);
}
