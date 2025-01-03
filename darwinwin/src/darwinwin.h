#pragma once

#include "core.h"

enum tileFlag : uint8_t
{
  tf_Underwater = 1,
  tf_Protein = 1 << 1,
  tf_Sugar = 1 << 2,
  tf_Vitamin = 1 << 3,
  tf_Fat = 1 << 4,
  tf_Collidable = 1 << 5,
  tf_OtherActor = 1 << 6, // not on the map
  tf_Hidden = 1 << 7, // not on the map
};

void tileFlag_toTempString(const uint8_t flag, char(&out)[9]);
void tileFlag_print(const uint8_t flag);

struct level
{
  static constexpr size_t width = 32;
  static constexpr size_t height = 32;

  static constexpr uint8_t wallThickness = 3; // this needs a shorter name

  uint8_t grid[width * height];
};

enum lookDirection
{
  ld_left,
  ld_up,
  ld_right,
  ld_down,

  _lookDirection_Count,
};

const char *lookDirection_name(const lookDirection dir);

struct actor
{
  vec2u pos; // which size is best?
  lookDirection look_at_dir;

  actor(const vec2u8 pos, const lookDirection dir) : pos(pos), look_at_dir(dir) { lsAssert(pos.x >= level::wallThickness && pos.x < (level::width - level::wallThickness) && pos.y >= level::wallThickness && pos.y < (level::height - level::wallThickness)); }
};

enum viewConePosition
{
  vcp_self,
  vcp_nearLeft,
  vcp_nearCenter,
  vcp_nearRight,
  vcp_midLeft,
  vcp_midCenter,
  vcp_midRight,
  vcp_farCenter,

  _viewConePosition_Count,
};

struct viewCone
{
  uint8_t values[_viewConePosition_Count];

  uint8_t operator [](const viewConePosition pos) const
  {
    lsAssert(pos < LS_ARRAYSIZE(values));
    return values[pos];
  }
};

struct actorStats
{
  static constexpr uint8_t _maxLevel = 128;

  uint8_t energy = 100; // TODO: figure out start values, but we probably want to custimize the values anyways when training.
  uint8_t air = 100;
  uint8_t protein = 100;
  uint8_t sugar = 100;
  uint8_t vitamin = 100;
  uint8_t fat = 100;
  // room for atleast two more attributes
};

void level_initLinear(level *pLevel);
void level_print(const level &level);

viewCone viewCone_get(const level &lvl, const actor &actor);
void viewCone_print(const viewCone &values, const actor &actor);

void actor_move(actor *pActor, actorStats *pStats, const level &lvl);
void actor_turnAround(actor *pActor, const lookDirection targetDir);
void actor_eat(actor *pActor, actorStats *pStats, level *pLvl, const viewCone cone);
