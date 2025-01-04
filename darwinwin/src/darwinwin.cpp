#include "darwinwin.h"

static constexpr size_t _movementEnergyCost = 10; // maybe these should live in the level or the stats?
static constexpr size_t _doubleMovementEnergyCost = 17; // maybe these should live in the level or the stats?
static constexpr size_t _idleEnergyCost = 1;
static constexpr size_t _underwaterAirCost = 5;

const char *lookDirection_toName[] =
{
  "left",
  "up",
  "right",
  "down",
};

static_assert(LS_ARRAYSIZE(lookDirection_toName) == _lookDirection_Count);

void tileFlag_toTempString(const uint8_t flag, char(&out)[9])
{
  const char lut[9] = "UPSVFCOH";

  for (size_t i = 0, mask = 1; i < 8; mask <<= 1, i++)
    out[i] = (flag & mask) ? lut[i] : ' ';

  out[LS_ARRAYSIZE(out) - 1] = '\0';
}

void tileFlag_print(const uint8_t flag)
{
  char tmp[9];
  tileFlag_toTempString(flag, tmp);
  print(tmp);
}

const char *lookDirection_name(const lookDirection dir)
{
  lsAssert(dir < LS_ARRAYSIZE(lookDirection_toName));
  return lookDirection_toName[dir];
}

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

viewCone viewCone_get(const level &lvl, const actor &a)
{
  lsAssert(a.pos.x > 0 && a.pos.y < level::width);

  viewCone ret;

  size_t currentIdx = a.pos.y * level::width + a.pos.x;
  constexpr ptrdiff_t width = (ptrdiff_t)level::width;
  static const ptrdiff_t lut[_lookDirection_Count][LS_ARRAYSIZE(ret.values)] = {
    { 0, width - 1, -1, -width - 1, width - 2, -2, -width - 2, -3 },
    { 0, -width - 1, -width, -width + 1, -width * 2 - 1, -width * 2, -width * 2 + 1, -width * 3 },
    { 0, -width + 1, 1, width + 1, -width + 2, 2, width + 2, 3 },
    { 0, width + 1, width, width - 1, width * 2 + 1, width * 2, width * 2 - 1, width * 3 },
  };

  for (size_t i = 0; i < LS_ARRAYSIZE(ret.values); i++)
    ret.values[i] = lvl.grid[currentIdx + lut[a.look_at_dir][i]];

  // hidden flags
  if (ret.values[vcp_nearLeft] & tf_Collidable)
    ret.values[vcp_midLeft] = tf_Hidden;

  if (ret.values[vcp_nearCenter] & tf_Collidable)
  {
    ret.values[vcp_midCenter] = tf_Hidden;
    ret.values[vcp_farCenter] = tf_Hidden;
  }
  else if (ret.values[vcp_midCenter] & tf_Collidable)
  {
    ret.values[vcp_farCenter] = tf_Hidden;
  }

  if (ret.values[vcp_nearRight] & tf_Collidable)
    ret.values[vcp_midRight] = tf_Hidden;

  // TODO: other actor flag

  return ret;
}

void printEmpty()
{
  print("         ");
}

void printValue(const uint8_t val)
{
  tileFlag_print(val);
  print(' ');
  //print(FU(Bin, Min(8), Fill0)(val), ' ');
  //print(FU(Min(8))(val), ' ');
}

void viewCone_print(const viewCone &v, const actor &actor)
{
  print("VIEWCONE from pos ", actor.pos, " with look direction: ", lookDirection_name(actor.look_at_dir), '\n');

  printEmpty();             printValue(v[vcp_nearLeft]);    printValue(v[vcp_midLeft]);    print('\n');
  printValue(v[vcp_self]);  printValue(v[vcp_nearCenter]);  printValue(v[vcp_midCenter]);  printValue(v[vcp_farCenter]);  print('\n');
  printEmpty();             printValue(v[vcp_nearRight]);   printValue(v[vcp_midRight]);   print('\n');
}

///////////////////////////////////////////////////////////////////////////////////////////

void actor_updateStats(actorStats *pStats, const viewCone cone)
{
  // Always update view cone first!
  
  // Remove energy
  if (pStats->energy)
    pStats->energy = pStats->energy > _idleEnergyCost ? (pStats->energy - _idleEnergyCost) : 0;

  // Check underwater
  if (pStats->air && cone[vcp_self] & tf_Underwater)
    pStats->energy = pStats->air > _underwaterAirCost ? (pStats->air - _underwaterAirCost) : 0;
}

void actor_move(actor *pActor, actorStats *pStats, const level &lvl)
{
  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);
  //lsAssert(!(lvl.grid[pActor->pos.y * level::width + pActor->pos.x] & tf_Collidable));

  static const vec2i8 lut[_lookDirection_Count] = { vec2i8(-1, 0), vec2i8(0, -1), vec2i8(1, 0), vec2i8(0, -1) };

  if (pStats->energy >= _movementEnergyCost)
  {
    vec2u newPos = vec2u(pActor->pos.x + lut[pActor->look_at_dir].x, pActor->pos.y + lut[pActor->look_at_dir].y); // is it ok to add the i to the ui?

    if (!(lvl.grid[newPos.y * level::width + newPos.x] & tf_Collidable) && newPos.x > 0 && newPos.x < level::width && newPos.y > 0 && newPos.y < level::height)
    {
      pActor->pos = newPos;
      pStats->energy -= _movementEnergyCost;
    }
  }
}

void actor_moveTwo(actor *pActor, actorStats *pStats, const level &lvl)
{
  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);
  //lsAssert(!(lvl.grid[pActor->pos.y * level::width + pActor->pos.x] & tf_Collidable));

  static const vec2i8 lut[_lookDirection_Count] = { vec2i8(-1, 0), vec2i8(0, -1), vec2i8(1, 0), vec2i8(0, -1) };

  if (pStats->energy >= _doubleMovementEnergyCost)
  {
    vec2u newPos = vec2u(pActor->pos.x + 2 * lut[pActor->look_at_dir].x, pActor->pos.y + 2 * lut[pActor->look_at_dir].y); // is it ok to add the i to the ui?

    if (!(lvl.grid[newPos.y * level::width + newPos.x] & tf_Collidable) && newPos.x > 0 && newPos.x < level::width && newPos.y > 0 && newPos.y < level::height)
    {
      pActor->pos = newPos;
      pStats->energy -= _doubleMovementEnergyCost;
    }
  }
}

void actor_turnAround(actor *pActor, const lookDirection targetDir)
{
  lsAssert(targetDir < _lookDirection_Count);

  pActor->look_at_dir = targetDir; // Or should dir be the turn we want to do and we need to change the current look_dir to be turned in the dir?
}

void actor_eat(actor *pActor, actorStats *pStats, level *pLvl, const viewCone cone)
{
  // View cone must be updated before calling this!

  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);

  size_t currentIdx = pActor->pos.y * level::width + pActor->pos.x;

  if (cone[vcp_self] & tf_Protein && pStats->protein < actorStats::_maxLevel)
  {
    pStats->protein++;
    pLvl->grid[currentIdx] &= !tf_Protein; // This will remove the protein completely...
  }
  if (cone[vcp_self] & tf_Sugar && pStats->sugar < actorStats::_maxLevel)
  {
    pStats->sugar++;
    pLvl->grid[currentIdx] &= !tf_Sugar; // This will remove the protein completely...
  }
  if (cone[vcp_self] & tf_Vitamin && pStats->vitamin < actorStats::_maxLevel)
  {
    pStats->vitamin++;
    pLvl->grid[currentIdx] &= !tf_Vitamin; // This will remove the protein completely...
  }
  if (cone[vcp_self] & tf_Fat && pStats->fat < actorStats::_maxLevel)
  {
    pStats->fat ++;
    pLvl->grid[currentIdx] &= !tf_Vitamin; // This will remove the protein completely...
  }
}
