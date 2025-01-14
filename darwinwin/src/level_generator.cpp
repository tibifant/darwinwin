#include "level_generator.h"
constexpr int64_t dir[] = { -1, -(int64_t)level::width, 1, level::width };

void level_gen_fill(level *pLvl, const tileFlag defaultTile)
{
  for (size_t i = 0; i < level::total; i++)
    pLvl->grid[i] = defaultTile;
}

void level_gen_random_sprinkle_replace_mask(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count)
{
  const size_t maxTries = count * 4;

  size_t tries = 0;
  size_t addedCount = 0;

  while (addedCount < count)
  {
    size_t rand = lsGetRand() % level::total;
    if (pLvl->grid[rand] == srcMask) // if we match with `&` we could overwrite the values we just set, if we e.g. want to replace 00000001 with 00001001
    {
      pLvl->grid[rand] = target;
      addedCount++;
    }
    tries++;

    if (tries > maxTries)
      return;
  }
}

void level_gen_random_walk_replace_mask(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count, const size_t minLength, const size_t maxLength)
{
  (void)pLvl;
  (void)srcMask;
  (void)target;
  (void)minLength;
  (void)maxLength;

  size_t addedCount = 0;

  while (addedCount < count)
  {
    // serach for startpos
    // check if level matches mask in random dir
    // replace value, count++, step++
  }
}

void level_gen_grow(level *pLvl, const tileFlag grownValue)
{  
  for (size_t i = 0; i < level::total; i++)
    if (pLvl->grid[i] & grownValue)
      for (size_t j = 0; j < LS_ARRAYSIZE(dir); j++)
        pLvl->grid[lsClamp(i + dir[j], (size_t)0, level::total)] = grownValue;
}
