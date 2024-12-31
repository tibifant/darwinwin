#include "darwinwin.h"

void getViewcone(level lvl, animal animal)
{
  size_t currentIdx = animal.pos.y * lvl.width + animal.pos.x;

  // view cone: (0 is the current pos)
  //  14
  // 0257
  //  36

  animal.viewCone[0] = lvl.grid[currentIdx];

  if (animal.look_at_dir == d_up)
  {
    currentIdx -= lvl.width - 2;
    animal.viewCone[1] = lvl.grid[currentIdx]; // near left
    animal.viewCone[2] = lvl.grid[currentIdx + 1]; // straight ahead near
    animal.viewCone[3] = lvl.grid[currentIdx + 2]; // near right
    currentIdx -= lvl.width;
    animal.viewCone[4] = lvl.grid[currentIdx]; // far left
    animal.viewCone[5] = lvl.grid[currentIdx + 1]; // straight ahead middle
    animal.viewCone[6] = lvl.grid[currentIdx + 2]; // far right
    animal.viewCone[7] = lvl.grid[currentIdx + 1 - lvl.width]; // straight ahead far
  }
  else if (animal.look_at_dir == d_down)
  {
    currentIdx += lvl.width - 1;
    animal.viewCone[3] = lvl.grid[currentIdx]; // near right
    animal.viewCone[2] = lvl.grid[currentIdx + 1]; // straight ahead near
    animal.viewCone[1] = lvl.grid[currentIdx + 2]; // near left
    currentIdx += lvl.width;
    animal.viewCone[6] = lvl.grid[currentIdx]; // far right
    animal.viewCone[5] = lvl.grid[currentIdx + 1]; // straight ahead middle
    animal.viewCone[4] = lvl.grid[currentIdx + 2]; // far left
    animal.viewCone[7] = lvl.grid[currentIdx + 1 + lvl.width]; // straight ahead far
  }
  else if (animal.look_at_dir == d_right)
  {
    size_t leftNear = currentIdx + 1 - lvl.width;
    size_t rightNear = currentIdx + 1 + lvl.width;
    animal.viewCone[1] = lvl.grid[leftNear]; // near left
    animal.viewCone[4] = lvl.grid[leftNear + 1]; // far left
    animal.viewCone[2] = lvl.grid[currentIdx + 1]; // straight ahead near
    animal.viewCone[5] = lvl.grid[currentIdx + 2]; // straight ahead middle
    animal.viewCone[7] = lvl.grid[currentIdx + 3]; // straight ahead far
    animal.viewCone[3] = lvl.grid[rightNear]; // near right
    animal.viewCone[6] = lvl.grid[rightNear +  1]; // far right
  }
  else if (animal.look_at_dir == d_left)
  {
    size_t rightFar = currentIdx - 2 - lvl.width;
    size_t leftFar = currentIdx - 2 + lvl.width;
    animal.viewCone[6] = lvl.grid[rightFar]; // far right
    animal.viewCone[3] = lvl.grid[rightFar + 1]; // near right
    animal.viewCone[7] = lvl.grid[currentIdx - 3]; // straight ahead far
    animal.viewCone[5] = lvl.grid[currentIdx - 2]; // straight ahead middle
    animal.viewCone[2] = lvl.grid[currentIdx - 1]; // straight ahead near
    animal.viewCone[4] = lvl.grid[leftFar]; // far left
    animal.viewCone[1] = lvl.grid[leftFar + 1]; // near left
  }

  // TODO: hidden flag
  // TODO: other animal flag

  // TODO: print
}
