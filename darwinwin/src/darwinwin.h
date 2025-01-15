#pragma once

#include "core.h"
#include "neural_net.h"
#include "evolution.h"

//////////////////////////////////////////////////////////////////////////

enum actorStats
{
  as_Air,
  _actorStats_FoodBegin,
  as_Protein = _actorStats_FoodBegin,
  as_Sugar,
  as_Vitamin,
  as_Fat,
  _actorStats_FoodEnd = as_Fat,
  as_Energy,

  _actorStats_Count
};

enum tileFlag_ : uint8_t
{
  tf_Underwater = 1ULL << as_Air,
  tf_Protein = 1ULL << as_Protein,
  tf_Sugar = 1ULL << as_Sugar,
  tf_Vitamin = 1ULL << as_Vitamin,
  tf_Fat = 1ULL << as_Fat,
  tf_Collidable = 1ULL << 5,
  tf_OtherActor = 1ULL << 6, // not on the map
  tf_Hidden = 1ULL << 7, // not on the map
};

using tileFlag = uint8_t;

void tileFlag_toTempString(const uint8_t flag, char(&out)[9]);
void tileFlag_print(const uint8_t flag);

//////////////////////////////////////////////////////////////////////////

struct level
{
  static constexpr size_t width = 32;
  static constexpr size_t height = 32;
  static constexpr size_t total = width * height;

  static constexpr uint8_t wallThickness = 3; // this needs a shorter name

  uint8_t grid[width * height];
};

void level_initLinear(level *pLevel);
void level_print(const level &level);

struct actor;

bool level_performStep(level &lvl, actor *pActors, const size_t actorCount);

//////////////////////////////////////////////////////////////////////////

enum lookDirection
{
  ld_left,
  ld_up,
  ld_right,
  ld_down,

  _lookDirection_Count,
};

const char *lookDirection_name(const lookDirection dir);

//////////////////////////////////////////////////////////////////////////

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

viewCone viewCone_get(const level &lvl, const actor &actor);
void viewCone_print(const viewCone &values, const actor &actor);

//////////////////////////////////////////////////////////////////////////

struct actor
{
  vec2u16 pos;
  lookDirection look_at_dir;
  uint8_t stats[_actorStats_Count];
  uint8_t stomach_remaining_capacity;
  neural_net<(_viewConePosition_Count * 8 + _actorStats_Count + (neural_net_block_size - 1)) / neural_net_block_size, 2, 1> brain;

  actor(const vec2u8 pos, const lookDirection dir) : pos(pos), look_at_dir(dir) { lsAssert(pos.x >= level::wallThickness && pos.x < (level::width - level::wallThickness) && pos.y >= level::wallThickness && pos.y < (level::height - level::wallThickness)); }
};

enum actorAction
{
  aa_Move,
  aa_Move2,
  aa_TurnLeft,
  aa_TurnRight,
  aa_Eat,

  _actorAction_Count
};

void actor_updateStats(actor *pActor, const viewCone &cone);
void actor_act(actor *pActor, level *pLevel, const viewCone &cone, const actorAction action);
