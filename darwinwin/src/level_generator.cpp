#include "level_generator.h"

void level_gen_finalize(level *pLvl)
{
  for (size_t i = 0; i < level::width; i++)
  {
    pLvl->grid[i] = tf_Collidable;
    pLvl->grid[i + level::width] = tf_Collidable;
    pLvl->grid[i + level::width * 2] = tf_Collidable;

    pLvl->grid[i + level::width * level::height - 3 * level::width] = tf_Collidable;
    pLvl->grid[i + level::width * level::height - 2 * level::width] = tf_Collidable;
    pLvl->grid[i + level::width * level::height - 1 * level::width] = tf_Collidable;
  }

  for (size_t i = 0; i < level::height; i++)
  {
    pLvl->grid[i * level::width] = tf_Collidable;
    pLvl->grid[i * level::width + 1] = tf_Collidable;
    pLvl->grid[i * level::width + 2] = tf_Collidable;

    pLvl->grid[i * level::width + level::width - 1] = tf_Collidable;
    pLvl->grid[i * level::width + level::width - 2] = tf_Collidable;
    pLvl->grid[i * level::width + level::width - 3] = tf_Collidable;
  }
}

void level_gen_fill(level *pLvl, const tileFlag defaultTile)
{
  for (size_t i = 0; i < level::total; i++)
    pLvl->grid[i] = defaultTile;
}

void level_gen_random_sprinkle_replace(level *pLvl, const tileFlag src, const tileFlag target, const size_t count)
{
  for (size_t i = 0; i < count; i++)
  {
    const size_t rand = lsGetRand() % level::total;

    if (pLvl->grid[rand] == src)
      pLvl->grid[rand] = target;
  }
}

void level_gen_random_sprinkle_replace_mask(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count)
{
  for (size_t i = 0; i < count; i++)
  {
    const size_t rand = lsGetRand() % level::total;

    if (pLvl->grid[rand] & srcMask)
      pLvl->grid[rand] = target;
  }
}

void level_gen_random_sprinkle_replace_inv_mask(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count)
{
  for (size_t i = 0; i < count; i++)
  {
    const size_t rand = lsGetRand() % level::total;

    if (~pLvl->grid[rand] & srcMask)
      pLvl->grid[rand] = target;
  }
}

void level_gen_random_sprinkle_replace_mask_count(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count, const size_t matchCount)
{
  for (size_t i = 0; i < count; i++)
  {
    const size_t rand = lsGetRand() % level::total;

    if (__popcnt64(pLvl->grid[rand] & srcMask) >= matchCount)
      pLvl->grid[rand] = target;
  }
}

void level_gen_random_sprinkle_replace_inv_mask_count(level *pLvl, const tileFlag srcMask, const tileFlag target, const size_t count, const size_t matchCount)
{
  for (size_t i = 0; i < count; i++)
  {
    const size_t rand = lsGetRand() % level::total;

    if (__popcnt64(~pLvl->grid[rand] & srcMask) >= matchCount)
      pLvl->grid[rand] = target;
  }
}

void level_gen_set_if_mask_internal(level *pLvl, const uint8_t(&maskBuffer)[level::total], const tileFlag target)
{
  static_assert(level::total < 1024 * 64, "Ensure that callers have placed their array on the heap... This is NOT an error per-se, but please make sure this has been considered before changing the level size to be abnormally large.");

  for (size_t i = 0; i < level::total; i++)
  {
    const uint8_t mask = maskBuffer[i];
    pLvl->grid[i] = (~mask & pLvl->grid[i]) | (mask & target);
  }
}

void level_gen_or_if_mask_internal(level *pLvl, const uint8_t(&maskBuffer)[level::total], const tileFlag target)
{
  static_assert(level::total < 1024 * 64, "Ensure that callers have placed their array on the heap... This is NOT an error per-se, but please make sure this has been considered before changing the level size to be abnormally large.");

  for (size_t i = 0; i < level::total; i++)
  {
    const uint8_t mask = maskBuffer[i];
    pLvl->grid[i] |= (mask & target);
  }
}

void level_gen_grow(level *pLvl, const tileFlag grownValue)
{
  uint8_t maskBuffer[level::total];

  for (size_t y = level::wallThickness; y < level::height - level::wallThickness; y++)
  {
    constexpr size_t xStart = level::wallThickness;
    const size_t lineIdx = y * level::width + xStart;
    const uint8_t *pUp = pLvl->grid + lineIdx - level::width;
    const uint8_t *pDown = pLvl->grid + lineIdx + level::width;
    const uint8_t *pLeft = pLvl->grid + lineIdx - 1;
    const uint8_t *pRight = pLvl->grid + lineIdx + 1;
    uint8_t *pOut = maskBuffer + lineIdx;

    for (size_t x = xStart; x < level::width - level::wallThickness; x++)
    {
      const bool up = *pUp == grownValue;
      const bool down = *pDown == grownValue;
      const bool left = *pLeft == grownValue;
      const bool right = *pRight == grownValue;
      const bool anyMatch = up || down || left || right;
      const uint8_t matchMask = ((uint8_t)!anyMatch) - 1; // true, false -> 0xFF, 0
      *pOut = matchMask;

      pUp++;
      pDown++;
      pLeft++;
      pRight++;
      pOut++;
    }
  }

  level_gen_set_if_mask_internal(pLvl, maskBuffer, grownValue);
}

void level_gen_grow_into_mask(level *pLvl, const tileFlag grownValue, const tileFlag replacableMask)
{
  uint8_t maskBuffer[level::total];

  for (size_t y = level::wallThickness; y < level::height - level::wallThickness; y++)
  {
    constexpr size_t xStart = level::wallThickness;
    const size_t lineIdx = y * level::width + xStart;
    const uint8_t *pSelf = pLvl->grid + lineIdx;
    const uint8_t *pUp = pSelf - level::width;
    const uint8_t *pDown = pSelf + level::width;
    const uint8_t *pLeft = pSelf - 1;
    const uint8_t *pRight = pSelf + 1;
    uint8_t *pOut = maskBuffer + lineIdx;

    for (size_t x = xStart; x < level::width - level::wallThickness; x++)
    {
      const bool up = *pUp == grownValue;
      const bool down = *pDown == grownValue;
      const bool left = *pLeft == grownValue;
      const bool right = *pRight == grownValue;
      const bool self = !!(*pSelf & replacableMask);
      const bool match = self && (up || down || left || right);
      const uint8_t matchMask = ((uint8_t)!match) - 1; // true, false -> 0xFF, 0
      *pOut = matchMask;

      pSelf++;
      pUp++;
      pDown++;
      pLeft++;
      pRight++;
      pOut++;
    }
  }

  level_gen_set_if_mask_internal(pLvl, maskBuffer, grownValue);
}

void level_gen_grow_into_inv_mask(level *pLvl, const tileFlag grownValue, const tileFlag replacableInvMask)
{
  uint8_t maskBuffer[level::total];

  for (size_t y = level::wallThickness; y < level::height - level::wallThickness; y++)
  {
    constexpr size_t xStart = level::wallThickness;
    const size_t lineIdx = y * level::width + xStart;
    const uint8_t *pSelf = pLvl->grid + lineIdx;
    const uint8_t *pUp = pSelf - level::width;
    const uint8_t *pDown = pSelf + level::width;
    const uint8_t *pLeft = pSelf - 1;
    const uint8_t *pRight = pSelf + 1;
    uint8_t *pOut = maskBuffer + lineIdx;

    for (size_t x = xStart; x < level::width - level::wallThickness; x++)
    {
      const bool up = *pUp == grownValue;
      const bool down = *pDown == grownValue;
      const bool left = *pLeft == grownValue;
      const bool right = *pRight == grownValue;
      const bool self = !!(~*pSelf & replacableInvMask);
      const bool match = self && (up || down || left || right);
      const uint8_t matchMask = ((uint8_t)!match) - 1; // true, false -> 0xFF, 0
      *pOut = matchMask;

      pSelf++;
      pUp++;
      pDown++;
      pLeft++;
      pRight++;
      pOut++;
    }
  }

  level_gen_set_if_mask_internal(pLvl, maskBuffer, grownValue);
}

void level_gen_sprinkle_grow(level *pLvl, const tileFlag grownValue, const uint8_t chance)
{
  uint8_t maskBuffer[level::total];

  for (size_t y = level::wallThickness; y < level::height - level::wallThickness; y++)
  {
    constexpr size_t xStart = level::wallThickness;
    const size_t lineIdx = y * level::width + xStart;
    const uint8_t *pUp = pLvl->grid + lineIdx - level::width;
    const uint8_t *pDown = pLvl->grid + lineIdx + level::width;
    const uint8_t *pLeft = pLvl->grid + lineIdx - 1;
    const uint8_t *pRight = pLvl->grid + lineIdx + 1;
    uint8_t *pOut = maskBuffer + lineIdx;

    for (size_t x = xStart; x < level::width - level::wallThickness; x++)
    {
      const bool up = *pUp == grownValue;
      const bool down = *pDown == grownValue;
      const bool left = *pLeft == grownValue;
      const bool right = *pRight == grownValue;
      const bool anyMatch = (up || down || left || right) && ((uint8_t)lsGetRand() <= chance);
      const uint8_t matchMask = ((uint8_t)!anyMatch) - 1; // true, false -> 0xFF, 0
      *pOut = matchMask;

      pUp++;
      pDown++;
      pLeft++;
      pRight++;
      pOut++;
    }
  }

  level_gen_set_if_mask_internal(pLvl, maskBuffer, grownValue);
}

void level_gen_sprinkle_grow_into_mask(level *pLvl, const tileFlag grownValue, const tileFlag replacableMask, const uint8_t chance)
{
  uint8_t maskBuffer[level::total];

  for (size_t y = level::wallThickness; y < level::height - level::wallThickness; y++)
  {
    constexpr size_t xStart = level::wallThickness;
    const size_t lineIdx = y * level::width + xStart;
    const uint8_t *pSelf = pLvl->grid + lineIdx;
    const uint8_t *pUp = pSelf - level::width;
    const uint8_t *pDown = pSelf + level::width;
    const uint8_t *pLeft = pSelf - 1;
    const uint8_t *pRight = pSelf + 1;
    uint8_t *pOut = maskBuffer + lineIdx;

    for (size_t x = xStart; x < level::width - level::wallThickness; x++)
    {
      const bool up = *pUp == grownValue;
      const bool down = *pDown == grownValue;
      const bool left = *pLeft == grownValue;
      const bool right = *pRight == grownValue;
      const bool self = !!(*pSelf & replacableMask);
      const bool match = self && (up || down || left || right) && ((uint8_t)lsGetRand() <= chance);
      const uint8_t matchMask = ((uint8_t)!match) - 1; // true, false -> 0xFF, 0
      *pOut = matchMask;

      pSelf++;
      pUp++;
      pDown++;
      pLeft++;
      pRight++;
      pOut++;
    }
  }

  level_gen_set_if_mask_internal(pLvl, maskBuffer, grownValue);

}

void level_gen_sprinkle_grow_into_inv_mask(level *pLvl, const tileFlag grownValue, const tileFlag replacableInvMask, const uint8_t chance)
{
  uint8_t maskBuffer[level::total];

  for (size_t y = level::wallThickness; y < level::height - level::wallThickness; y++)
  {
    constexpr size_t xStart = level::wallThickness;
    const size_t lineIdx = y * level::width + xStart;
    const uint8_t *pSelf = pLvl->grid + lineIdx;
    const uint8_t *pUp = pSelf - level::width;
    const uint8_t *pDown = pSelf + level::width;
    const uint8_t *pLeft = pSelf - 1;
    const uint8_t *pRight = pSelf + 1;
    uint8_t *pOut = maskBuffer + lineIdx;

    for (size_t x = xStart; x < level::width - level::wallThickness; x++)
    {
      const bool up = *pUp == grownValue;
      const bool down = *pDown == grownValue;
      const bool left = *pLeft == grownValue;
      const bool right = *pRight == grownValue;
      const bool self = !!(~*pSelf & replacableInvMask);
      const bool match = self && (up || down || left || right) && ((uint8_t)lsGetRand() <= chance);
      const uint8_t matchMask = ((uint8_t)!match) - 1; // true, false -> 0xFF, 0
      *pOut = matchMask;

      pSelf++;
      pUp++;
      pDown++;
      pLeft++;
      pRight++;
      pOut++;
    }
  }

  level_gen_set_if_mask_internal(pLvl, maskBuffer, grownValue);
}
