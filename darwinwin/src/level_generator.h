#pragma once

#include "darwinwin.h"

template <double chance>
constexpr uint8_t level_gen_make_chance()
{
  static_assert(chance > 0 && chance < 1);
  return lsMax<uint8_t>(1, (uint8_t)(chance * (double)0xFF + 0.5));
}

void level_gen_finalize(level *pLvl);

void level_gen_fill(level *pLvl, const tileFlag defaultTile);
void level_gen_random_sprinkle_replace(level *pLvl, const tileFlag src, const tileFlag target, const size_t count);
void level_gen_random_sprinkle_replace_mask(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count);
void level_gen_random_sprinkle_replace_inv_mask(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count);
void level_gen_random_sprinkle_replace_mask_count(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count, const size_t matchCount);
void level_gen_random_sprinkle_replace_inv_mask_count(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count, const size_t matchCount);
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
