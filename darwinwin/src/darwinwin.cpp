#include "darwinwin.h"

void level_initLinear(level *pLevel)
{
  for (size_t i = 0; i < level::width * level::height; i++)
    pLevel->grid[i] = (uint8_t)(i);

  // Setting things to collidable for testing
  pLevel->grid[6 * level::width + 6] = tf_Collidable;
  pLevel->grid[5 * level::width + 7] = tf_Collidable;
  pLevel->grid[6 * level::width + 8] = tf_Collidable;

  // Making the Borders collidable
  for (size_t i = 0; i < level::width; i++)
  {
    pLevel->grid[i] = tf_Collidable;
    pLevel->grid[i + level::width] = tf_Collidable;
    pLevel->grid[i + level::width * 2] = tf_Collidable;

    pLevel->grid[i + level::width * level::height - 4 * level::width + 1] = tf_Collidable;
    pLevel->grid[i + level::width * level::height - 3 * level::width + 1] = tf_Collidable;
    pLevel->grid[i + level::width * level::height - 2 * level::width + 1] = tf_Collidable;
  }

  for (size_t i = 0; i < level::height; i++)
  {
    pLevel->grid[i * level::width] = tf_Collidable;
    pLevel->grid[i * level::width + 1] = tf_Collidable;
    pLevel->grid[i * level::width + 2] = tf_Collidable;

    pLevel->grid[i * level::width + level::width] = tf_Collidable;
    pLevel->grid[i * level::width + level::width - 1] = tf_Collidable;
    pLevel->grid[i * level::width + level::width - 2] = tf_Collidable;
  }
}

void level_print(const level &level)
{
  print("Level \n");

  for (size_t y = 0; y < level::height; y++)
  {
    for (size_t x = 0; x < level::width; x++)
      print(FU(Min(4))(level.grid[y * level::width + x]));

    print('\n');
  }
}

viewCone viewCone_get(const level &lvl, const animal &animal)
{
  // view cone: (0 = current pos)
  //  14
  // 0257
  //  36

  size_t currentIdx = animal.pos.y * level::width + animal.pos.x;
  constexpr ptrdiff_t width = (ptrdiff_t)level::width;
  static const ptrdiff_t lut[d_Count][8] = {
    { 0, width - 1, -1, -width - 1, width - 2, -2, -width - 2, -3 },
    { 0, -width - 1, -width, -width + 1, -width * 2 - 1, -width * 2, -width * 2 + 1, -width * 3 },
    { 0, -width + 1, 1, width + 1, -width + 2, 2, width + 2, 3 },
    { 0, width + 1, width, width - 1, width * 2 + 1, width * 2, width * 2 - 1, width * 3 },
  };

  viewCone viewCone;
  
  for (size_t i = 0; i < 8; i++)
    viewCone.viewCone[i] = lvl.grid[currentIdx + lut[animal.look_at_dir][i]];

  // hidden flags
  if (viewCone.viewCone[1] & tf_Collidable)
    viewCone.viewCone[4] = tf_Hidden;

  if (viewCone.viewCone[2] & tf_Collidable)
  {
    viewCone.viewCone[5] = tf_Hidden;
    viewCone.viewCone[7] = tf_Hidden;
  }
  else if (viewCone.viewCone[5] & tf_Collidable)
  {
    viewCone.viewCone[7] = tf_Hidden;
  }

  if (viewCone.viewCone[3] & tf_Collidable)
    viewCone.viewCone[6] = tf_Hidden;

  // TODO: other animal flag

  return viewCone;
}

void printEmpty()
{
  print("         ");
}

void printValue(const uint8_t val)
{
  print(FU(Bin, Min(8), Fill0)(val), ' ');
  //print(FU(Min(8))(val), ' ');
}

void viewCone_print(const viewCone &viewCone, const animal &animal)
{
  print("VIEWCONE from pos ", animal.pos, " with looking direction: ", (uint8_t)animal.look_at_dir, '\n');

  printEmpty();                     printValue(viewCone.viewCone[1]);  printValue(viewCone.viewCone[4]);  print('\n');
  printValue(viewCone.viewCone[0]); printValue(viewCone.viewCone[2]);  printValue(viewCone.viewCone[5]);  printValue(viewCone.viewCone[7]);  print('\n');
  printEmpty();                     printValue(viewCone.viewCone[3]);  printValue(viewCone.viewCone[6]);  print('\n');
}
