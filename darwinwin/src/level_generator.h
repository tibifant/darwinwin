#pragma once

#include "darwinwin.h"

template <double chance>
constexpr uint8_t level_gen_make_chance()
{
  static_assert(chance > 0 && chance < 1);
  return lsMax<uint8_t>(1, (uint8_t)lsRound(chance * (double)0xFF));
}

void level_gen_fill(level *pLvl, const tileFlag defaultTile);
void level_gen_random_sprinkle_replace_mask(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count);
void level_gen_random_walk_replace_mask(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count, const size_t minLength, const size_t maxLength);
void level_gen_grow(level *pLvl, const tileFlag grownValue);
void level_gen_grow_into_mask(level *pLvl, const tileFlag grownValue, const tileFlag replacableMask);
void level_gen_grow_into_inv_mask(level *pLvl, const tileFlag grownValue, const tileFlag replacableInvMask);
void level_gen_sprinkle_grow(level *pLvl, const tileFlag grownValue, const uint8_t chance);
void level_gen_sprinkle_grow_into_mask(level *pLvl, const tileFlag grownValue, const tileFlag replacableMask, const uint8_t chance);
void level_gen_sprinkle_grow_into_inv_mask(level *pLvl, const tileFlag grownValue, const tileFlag replacableInvMask, const uint8_t chance);

inline void level_gen_init(level *pLvl, const tileFlag defaultTile = tf_Underwater)
{
  level_gen_fill(pLvl, defaultTile);
}

inline void level_gen_finalize(level *pLvl)
{
  for (size_t i = 0; i < level::width; i++)
  {
    pLvl->grid[i] = tf_Collidable;
    pLvl->grid[i + level::width] = tf_Collidable;
    pLvl->grid[i + level::width * 2] = tf_Collidable;

    pLvl->grid[i + level::width * level::height - 4 * level::width + 1] = tf_Collidable;
    pLvl->grid[i + level::width * level::height - 3 * level::width + 1] = tf_Collidable;
    pLvl->grid[i + level::width * level::height - 2 * level::width + 1] = tf_Collidable;
  }

  for (size_t i = 0; i < level::height; i++)
  {
    pLvl->grid[i * level::width] = tf_Collidable;
    pLvl->grid[i * level::width + 1] = tf_Collidable;
    pLvl->grid[i * level::width + 2] = tf_Collidable;

    pLvl->grid[i * level::width + level::width] = tf_Collidable;
    pLvl->grid[i * level::width + level::width - 1] = tf_Collidable;
    pLvl->grid[i * level::width + level::width - 2] = tf_Collidable;
  }
}

inline void level_gen_water_level(level *pLvl)
{
  level_gen_init(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace_mask(pLvl, tf_Underwater, (tileFlag)0, level::total / 10);
  level_gen_grow(pLvl, (tileFlag)0);
  level_gen_sprinkle_grow_into_inv_mask(pLvl, tf_Underwater, tf_Underwater, level_gen_make_chance<0.5>());
  level_gen_finalize(pLvl);
}
