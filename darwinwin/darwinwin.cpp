#include "darwinwin.h"

void getViewcone(level lvl, animal animal)
{
  size_t currentIdx = animal.pos.y * lvl.width + animal.pos.x;

  // view cone:
  //  xx
  // Xxxx
  //  xx

  animal.viewCone[0] = lvl.grid[currentIdx];

  if (animal.look_at_dir == d_up)
  {
    currentIdx -= lvl.width;
    animal.viewCone[1] = lvl.grid[currentIdx]; // straight ahead near
    animal.viewCone[2] = lvl.grid[currentIdx - 1]; // near left
    animal.viewCone[3] = lvl.grid[currentIdx + 1]; // near right
    currentIdx -= lvl.width;
    animal.viewCone[4] = lvl.grid[currentIdx]; // straight ahead middle
    animal.viewCone[5] = lvl.grid[currentIdx - 1]; // far left
    animal.viewCone[6] = lvl.grid[currentIdx + 1]; // far right
    animal.viewCone[7] = lvl.grid[currentIdx - lvl.width]; // straight ahead far
  }
  else if (animal.look_at_dir == d_down)
  {
    currentIdx += lvl.width;
    animal.viewCone[1] = lvl.grid[currentIdx]; // straight ahead near
    animal.viewCone[2] = lvl.grid[currentIdx + 1]; // near left
    animal.viewCone[3] = lvl.grid[currentIdx - 1]; // near right
    currentIdx += lvl.width;
    animal.viewCone[4] = lvl.grid[currentIdx]; // straight ahead middle
    animal.viewCone[5] = lvl.grid[currentIdx + 1]; // far left
    animal.viewCone[6] = lvl.grid[currentIdx - 1]; // far right
    animal.viewCone[7] = lvl.grid[currentIdx + lvl.width]; // straight ahead far
  }
  else if (animal.look_at_dir == d_right)
  {
    size_t left = currentIdx + 1 - lvl.width;
    size_t right = currentIdx + 1 + lvl.width;
    animal.viewCone[1] = lvl.grid[currentIdx + 1]; // straight ahead near
    animal.viewCone[2] = lvl.grid[left]; // near left
    animal.viewCone[3] = lvl.grid[right]; // near right
    animal.viewCone[4] = lvl.grid[currentIdx + 2]; // straight ahead middle
    animal.viewCone[5] = lvl.grid[left + 1]; // far left
    animal.viewCone[6] = lvl.grid[right +  1]; // far right
    animal.viewCone[7] = lvl.grid[currentIdx + 3]; // straight ahead far
  }
  else if (animal.look_at_dir == d_left)
  {
    size_t left = currentIdx - 1 + lvl.width;
    size_t right = currentIdx - 1 - lvl.width;
    animal.viewCone[1] = lvl.grid[currentIdx - 1]; // straight ahead near
    animal.viewCone[2] = lvl.grid[left]; // near left
    animal.viewCone[3] = lvl.grid[right]; // near right
    animal.viewCone[4] = lvl.grid[currentIdx - 2]; // straight ahead middle
    animal.viewCone[5] = lvl.grid[left - 1]; // far left
    animal.viewCone[6] = lvl.grid[right - 1]; // far right
    animal.viewCone[7] = lvl.grid[currentIdx - 3]; // straight ahead far
  }
}
