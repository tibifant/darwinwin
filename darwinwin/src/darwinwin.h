#pragma once

#include "core.h"

enum dww_tile_flag : uint8_t
{
  tf_Underwater = 1,
  tf_Protein = 1 << 1,
  tf_Sugar = 1 << 2,
  tf_Vitamin = 1 << 3,
  tf_Fat = 1 << 4,
  tf_Collidable = 1 << 5,
  tf_OtherAnimal = 1 << 6, // not on the map
  tf_Hidden = 1 << 7, // not on the map
};

struct level
{
  static constexpr size_t width = 32;
  static constexpr size_t height = 32;

  uint8_t grid[width * height];
};

enum dww_direction
{
  d_up,
  d_down,
  d_left,
  d_right,
};

// Beings: hunger, energy etc
struct animal
{
  vec2u8 pos;
  dww_direction look_at_dir;

  animal(const vec2u8 pos, const dww_direction dir) : pos(pos), look_at_dir(dir) { lsAssert(pos.x > 3 && pos.x < (level::width - 3) && pos.y > 3 && pos.y < (level::height - 3)); }
};

struct viewCone
{
  uint8_t viewCone[8];
};

void level_initLinear(level *pLevel);
void level_print(const level &level);

viewCone viewCone_get(const level &lvl, const animal &animal);
void viewCone_print(const viewCone &viewCone, const animal &animal);
