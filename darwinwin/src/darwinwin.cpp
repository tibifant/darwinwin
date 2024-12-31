#include "darwinwin.h"

void level_initLinear(level *pLevel)
{
  for (size_t i = 0; i < level::width * level::height; i++)
    pLevel->grid[i] = (uint8_t)(i);

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
  // view cone: (0 is the current pos)
  //  14
  // 0257
  //  36

  viewCone viewCone;

  size_t currentIdx = animal.pos.y * level::width + animal.pos.x;
  viewCone.viewCone[0] = lvl.grid[currentIdx];

  // TODO: lookup for viewcone values

  switch (animal.look_at_dir)
  {
  case d_up:
  {
    size_t leftNear = currentIdx - level::width - 1;
    viewCone.viewCone[1] = lvl.grid[leftNear]; // near left
    viewCone.viewCone[2] = lvl.grid[leftNear + 1]; // straight ahead near
    viewCone.viewCone[3] = lvl.grid[leftNear + 2]; // near right
    size_t leftFar = leftNear - level::width;
    viewCone.viewCone[4] = lvl.grid[leftFar]; // far left
    viewCone.viewCone[5] = lvl.grid[leftFar + 1]; // straight ahead middle
    viewCone.viewCone[6] = lvl.grid[leftFar + 2]; // far right
    viewCone.viewCone[7] = lvl.grid[leftFar + 1 - level::width]; // straight ahead far

    break;
  }

  case d_down:
  {
    size_t rightNear = currentIdx + level::width - 1;
    viewCone.viewCone[3] = lvl.grid[rightNear]; // near right
    viewCone.viewCone[2] = lvl.grid[rightNear + 1]; // straight ahead near
    viewCone.viewCone[1] = lvl.grid[rightNear + 2]; // near left
    size_t rightFar = rightNear + level::width;
    viewCone.viewCone[6] = lvl.grid[rightFar]; // far right
    viewCone.viewCone[5] = lvl.grid[rightFar + 1]; // straight ahead middle
    viewCone.viewCone[4] = lvl.grid[rightFar + 2]; // far left
    viewCone.viewCone[7] = lvl.grid[rightFar + 1 + level::width]; // straight ahead far

    break;
  }

  case d_right:
  {
    size_t leftNear = currentIdx + 1 - level::width;
    viewCone.viewCone[1] = lvl.grid[leftNear]; // near left
    viewCone.viewCone[4] = lvl.grid[leftNear + 1]; // far left
    viewCone.viewCone[2] = lvl.grid[currentIdx + 1]; // straight ahead near
    viewCone.viewCone[5] = lvl.grid[currentIdx + 2]; // straight ahead middle
    viewCone.viewCone[7] = lvl.grid[currentIdx + 3]; // straight ahead far
    size_t rightNear = currentIdx + 1 + level::width;
    viewCone.viewCone[3] = lvl.grid[rightNear]; // near right
    viewCone.viewCone[6] = lvl.grid[rightNear + 1]; // far right

    break;
  }

  case d_left:
  {
    size_t rightFar = currentIdx - 2 - level::width;
    viewCone.viewCone[6] = lvl.grid[rightFar]; // far right
    viewCone.viewCone[3] = lvl.grid[rightFar + 1]; // near right
    viewCone.viewCone[7] = lvl.grid[currentIdx - 3]; // straight ahead far
    viewCone.viewCone[5] = lvl.grid[currentIdx - 2]; // straight ahead middle
    viewCone.viewCone[2] = lvl.grid[currentIdx - 1]; // straight ahead near
    size_t leftFar = currentIdx - 2 + level::width;
    viewCone.viewCone[4] = lvl.grid[leftFar]; // far left
    viewCone.viewCone[1] = lvl.grid[leftFar + 1]; // near left

    break;
  }
  }

  // TODO: hidden flag
  // TODO: other animal flag

  return viewCone;
}

void printEmpty()
{
  print("         ");
}

void printValue(const uint8_t val)
{
  //print(FU(Bin, Min(8), Fill0)(val), ' ');
  print(FU(Min(8))(val), ' ');
}

void viewCone_print(const viewCone &viewCone, const animal &animal)
{
  print("VIEWCONE from pos ", animal.pos, " with looking direction: ", (uint8_t)animal.look_at_dir, '\n');

  printEmpty();                     printValue(viewCone.viewCone[1]);  printValue(viewCone.viewCone[4]);  print('\n');
  printValue(viewCone.viewCone[0]); printValue(viewCone.viewCone[2]);  printValue(viewCone.viewCone[5]);  printValue(viewCone.viewCone[7]);  print('\n');
  printEmpty();                     printValue(viewCone.viewCone[3]);  printValue(viewCone.viewCone[6]);  print('\n');
}
