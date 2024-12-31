#include "darwinwin.h"

void getViewcone(const level &lvl, const animal &animal, viewCone *pViewCone)
{
  // view cone: (0 is the current pos)
  //  14
  // 0257
  //  36

  size_t currentIdx = animal.pos.y * lvl.width + animal.pos.x;
  pViewCone->viewCone[0] = lvl.grid[currentIdx];

  if (animal.look_at_dir == d_up)
  {
    size_t leftNear = currentIdx - lvl.width - 2;
    pViewCone->viewCone[1] = lvl.grid[leftNear]; // near left
    pViewCone->viewCone[2] = lvl.grid[leftNear + 1]; // straight ahead near
    pViewCone->viewCone[3] = lvl.grid[leftNear + 2]; // near right
    size_t leftFar = leftNear - lvl.width;
    pViewCone->viewCone[4] = lvl.grid[leftFar]; // far left
    pViewCone->viewCone[5] = lvl.grid[leftFar + 1]; // straight ahead middle
    pViewCone->viewCone[6] = lvl.grid[leftFar + 2]; // far right
    pViewCone->viewCone[7] = lvl.grid[leftFar + 1 - lvl.width]; // straight ahead far
  }
  else if (animal.look_at_dir == d_down)
  {
    size_t rightNear = currentIdx + lvl.width - 1;
    pViewCone->viewCone[3] = lvl.grid[rightNear]; // near right
    pViewCone->viewCone[2] = lvl.grid[rightNear + 1]; // straight ahead near
    pViewCone->viewCone[1] = lvl.grid[rightNear + 2]; // near left
    size_t rightFar = rightNear + lvl.width;
    pViewCone->viewCone[6] = lvl.grid[rightFar]; // far right
    pViewCone->viewCone[5] = lvl.grid[rightFar + 1]; // straight ahead middle
    pViewCone->viewCone[4] = lvl.grid[rightFar + 2]; // far left
    pViewCone->viewCone[7] = lvl.grid[rightFar + 1 + lvl.width]; // straight ahead far
  }
  else if (animal.look_at_dir == d_right)
  {
    size_t leftNear = currentIdx + 1 - lvl.width;
    pViewCone->viewCone[1] = lvl.grid[leftNear]; // near left
    pViewCone->viewCone[4] = lvl.grid[leftNear + 1]; // far left
    pViewCone->viewCone[2] = lvl.grid[currentIdx + 1]; // straight ahead near
    pViewCone->viewCone[5] = lvl.grid[currentIdx + 2]; // straight ahead middle
    pViewCone->viewCone[7] = lvl.grid[currentIdx + 3]; // straight ahead far
    size_t rightNear = currentIdx + 1 + lvl.width;
    pViewCone->viewCone[3] = lvl.grid[rightNear]; // near right
    pViewCone->viewCone[6] = lvl.grid[rightNear +  1]; // far right
  }
  else if (animal.look_at_dir == d_left)
  {
    size_t rightFar = currentIdx - 2 - lvl.width;
    pViewCone->viewCone[6] = lvl.grid[rightFar]; // far right
    pViewCone->viewCone[3] = lvl.grid[rightFar + 1]; // near right
    pViewCone->viewCone[7] = lvl.grid[currentIdx - 3]; // straight ahead far
    pViewCone->viewCone[5] = lvl.grid[currentIdx - 2]; // straight ahead middle
    pViewCone->viewCone[2] = lvl.grid[currentIdx - 1]; // straight ahead near
    size_t leftFar = currentIdx - 2 + lvl.width;
    pViewCone->viewCone[4] = lvl.grid[leftFar]; // far left
    pViewCone->viewCone[1] = lvl.grid[leftFar + 1]; // near left
  }

  // TODO: hidden flag
  // TODO: other animal flag

  // TODO: print
}
