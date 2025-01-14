#pragma once

#include "darwinwin.h"

void generator_generate_level(level *pLvl, const size_t width, const size_t height, const uint8_t base_flag, const uint8_t islands_flag /*please decide on a better name*/, const uint8_t exculded_flag)
{

}

void generator_fill_with_base_flag(level *pLvl, const uint8_t tile_flag);
void generator_fill_via_random_walk(level *pLvl, const uint8_t tile_flag);

void generator_set_borders(level *pLvl)
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
