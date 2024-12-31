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
  tf_OtherAnimal = 1 << 6, // this is not permanently on the grid!
  tf_Hidden = 1 << 7, // not on the map!
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

struct animal
{
  vec2u8 pos;
  dww_direction look_at_dir;

  uint8_t viewCone[8];
};


// Beings: position, orientation, hunger, energy
