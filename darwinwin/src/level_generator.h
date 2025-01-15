#pragma once

#include "darwinwin.h"

template <double chance>
constexpr uint8_t level_gen_make_chance()
{
  static_assert(chance > 0 && chance < 1);
  return lsMax<uint8_t>(1, (uint8_t)lsRound(chance * (double)0xFF));
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

inline void level_gen_water_level(level *pLvl)
{
  level_gen_init(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace_mask(pLvl, tf_Underwater, 0, level::total / 10);
  level_gen_grow(pLvl, 0);
  level_gen_sprinkle_grow_into_inv_mask(pLvl, tf_Underwater, tf_Underwater, level_gen_make_chance<0.5>());
  level_gen_finalize(pLvl);
}

inline void level_gen_water_food_level(level *pLvl)
{
  level_gen_init(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace_mask(pLvl, tf_Underwater, 0, level::total / 10);
  level_gen_grow(pLvl, 0);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Vitamin | tf_Underwater, level::total / 10);
  level_gen_random_sprinkle_replace(pLvl, tf_Vitamin | tf_Underwater, tf_Vitamin | tf_Underwater | tf_Fat, level::total / 3); // UVF looks sus
  level_gen_sprinkle_grow_into_mask(pLvl, tf_Underwater | tf_Vitamin, tf_Underwater, level_gen_make_chance<0.75>());
  level_gen_sprinkle_grow_into_inv_mask(pLvl, tf_Underwater, tf_Underwater, level_gen_make_chance<0.5>());
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Protein, level::total / 10);
  level_gen_finalize(pLvl);
}
