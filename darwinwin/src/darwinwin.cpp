#include "darwinwin.h"

//////////////////////////////////////////////////////////////////////////

level _CurrentLevel;
volatile bool _DoTraining = false;
volatile bool _TrainingRunning = false;

//////////////////////////////////////////////////////////////////////////

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

  for (size_t i = 0; i < level::width; i++)
  {
    pLevel->grid[i] = tf_Collidable;
    pLevel->grid[i + level::width] = tf_Collidable;
    pLevel->grid[i + level::width * 2] = tf_Collidable;

    pLevel->grid[i + level::width * level::height - 3 * level::width] = tf_Collidable;
    pLevel->grid[i + level::width * level::height - 2 * level::width] = tf_Collidable;
    pLevel->grid[i + level::width * level::height - 1 * level::width] = tf_Collidable;
  }

  for (size_t i = 0; i < level::height; i++)
  {
    pLevel->grid[i * level::width] = tf_Collidable;
    pLevel->grid[i * level::width + 1] = tf_Collidable;
    pLevel->grid[i * level::width + 2] = tf_Collidable;

    pLevel->grid[i * level::width + level::width - 1] = tf_Collidable;
    pLevel->grid[i * level::width + level::width - 2] = tf_Collidable;
    pLevel->grid[i * level::width + level::width - 3] = tf_Collidable;
  }
}

void printEmptyTile()
{
  lsSetConsoleColor(lsCC_DarkGray, lsCC_Black);
  print("        |");
  lsResetConsoleColor();
}

void printTile(const tileFlag val)
{
  const char lut[9] = "UPSVFCOH";
  const lsConsoleColor fg[8] = { lsCC_BrightBlue, lsCC_BrightMagenta, lsCC_White, lsCC_BrightGreen, lsCC_BrightYellow, lsCC_BrightGray, lsCC_BrightCyan, lsCC_BrightRed };

  for (size_t i = 0, mask = 1; i < 8; mask <<= 1, i++)
  {
    lsSetConsoleColor(fg[i], lsCC_Black);
    print((val & mask) ? lut[i] : ' ');
  }

  lsSetConsoleColor(lsCC_DarkGray, lsCC_Black);
  print('|');
  lsResetConsoleColor();
}

void level_print(const level &level)
{
  print("Level \n");

  lsSetConsoleColor(lsCC_DarkGray, lsCC_Black);
  for (size_t x = 0; x < level::width; x++)
    print("        |");

  print('\n');
  lsResetConsoleColor();

  for (size_t y = 0; y < level::height; y++)
  {
    for (size_t x = 0; x < level::width; x++)
      printTile(level.grid[y * level::width + x]);

    print('\n');

    lsSetConsoleColor(lsCC_DarkGray, lsCC_Black);
    for (size_t x = 0; x < level::width; x++)
      print("--------|");

    print('\n');
    lsResetConsoleColor();
  }

  print('\n');
}

//////////////////////////////////////////////////////////////////////////

bool level_performStep(level &lvl, actor *pActors, const size_t actorCount)
{
  // TODO: optional level internal step. (grow plants, etc.)

  bool anyAlive = false;

  for (size_t i = 0; i < actorCount; i++)
  {
    if (!pActors[i].stats[as_Energy])
      continue;

    anyAlive = true;

    const viewCone cone = viewCone_get(lvl, pActors[i]);
    actor_updateStats(&pActors[i], cone);

    decltype(actor::brain)::io_buffer_t ioBuffer;

    for (size_t j = 0; j < LS_ARRAYSIZE(cone.values); j++)
      for (size_t k = 0, bit = 1; k < 8; k++, bit <<= 1)
        ioBuffer[j * 8 + k] = (int8_t)(cone[(viewConePosition)j] & bit);

    neural_net_buffer_prepare(ioBuffer, (LS_ARRAYSIZE(cone.values) * 8) / ioBuffer.block_size);

    for (size_t j = 0; j < _actorStats_Count; j++)
      ioBuffer[LS_ARRAYSIZE(cone.values) * 8 + j] = (int8_t)((int64_t)pActors[i].stats[j] - 128);

    lsMemcpy(&ioBuffer[pActors[i].brain.first_layer_count - LS_ARRAYSIZE(pActors[i].previous_feedback_output)], pActors[i].previous_feedback_output, LS_ARRAYSIZE(pActors[i].previous_feedback_output));

    neural_net_eval(pActors[i].brain, ioBuffer);

    size_t actionValueCount = 0;

    static_assert(_actorAction_Count <= LS_ARRAYSIZE(ioBuffer.data));

    for (size_t j = 0; j < _actorAction_Count; j++)
      actionValueCount += ioBuffer[j];

    size_t bestActionIndex = 0;

    const size_t rand = lsGetRand() % lsMax(actionValueCount, 1);

    for (size_t actionIndex = 0; actionIndex < _actorAction_Count; actionIndex++)
    {
      const int16_t val = ioBuffer[actionIndex];

      if (val < rand)
      {
        bestActionIndex = actionIndex;
        break;
      }

      actionValueCount -= val;
    }

    pActors[i].last_action = (actorAction)bestActionIndex;
    actor_act(&pActors[i], &lvl, cone, pActors[i].last_action);

    lsMemcpy(pActors[i].previous_feedback_output, &ioBuffer[pActors[i].brain.last_layer_count - LS_ARRAYSIZE(pActors[i].previous_feedback_output)], LS_ARRAYSIZE(pActors[i].previous_feedback_output));
  }

  return anyAlive;
}

//////////////////////////////////////////////////////////////////////////

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
    ret.values[i] = lvl.grid[currentIdx + lut[a.look_dir][i]];

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

void viewCone_print(const viewCone &v, const actor &actor)
{
  print("VIEWCONE from pos ", actor.pos, " with look direction: ", lookDirection_name(actor.look_dir), '\n');

  printEmptyTile();        printTile(v[vcp_nearLeft]);    printTile(v[vcp_midLeft]);    print('\n');
  printTile(v[vcp_self]);  printTile(v[vcp_nearCenter]);  printTile(v[vcp_midCenter]);  printTile(v[vcp_farCenter]);  print('\n');
  printEmptyTile();        printTile(v[vcp_nearRight]);   printTile(v[vcp_midRight]);   print('\n');
}

//////////////////////////////////////////////////////////////////////////

void actor_move(actor *pActor, const level &lvl);
void actor_moveTwo(actor *pActor, const level &lvl);
void actor_turnLeft(actor *pActor);
void actor_turnRight(actor *pActor);
void actor_eat(actor *pActor, level *pLvl, const viewCone &cone);
void actor_wait(actor *pActor);
void actor_moveDiagonalLeft(actor *pActor, const level lvl);
void actor_moveDiagonalRight(actor *pActor, const level lvl);

//////////////////////////////////////////////////////////////////////////

template <typename T>
  requires (std::is_integral_v<T> && (sizeof(T) < sizeof(int64_t) || std::is_same_v<T, int64_t>))
inline T modify_with_clamp(T &value, const int64_t diff, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>())
{
  const int64_t val = (int64_t)value + diff;
  const T prevVal = value;
  value = (T)lsClamp<int64_t>(val, min, max);
  return value - prevVal;
}

void actor_act(actor *pActor, level *pLevel, const viewCone &cone, const actorAction action)
{
  switch (action)
  {
  case aa_Move:
    actor_move(pActor, *pLevel);
    break;

  case aa_Move2:
    actor_moveTwo(pActor, *pLevel);
    break;

  case aa_TurnLeft:
    actor_turnLeft(pActor);
    break;

  case aa_TurnRight:
    actor_turnRight(pActor);
    break;

  case aa_Eat:
    actor_eat(pActor, pLevel, cone);
    break;

  case aa_Wait:
    actor_wait(pActor);
    break;

  case aa_DiagonalMoveLeft:
    actor_moveDiagonalLeft(pActor, *pLevel);
    break;

  case aa_DiagonalMoveRight:
    actor_moveDiagonalRight(pActor, *pLevel);
    break;

  default:
    lsFail(); // not implemented.
    break;
  }
}

constexpr uint8_t MinStatsValue = 0;
constexpr uint8_t MaxStatsValue = 127;

void actor_initStats(actor *pActor)
{
  for (size_t i = 0; i < _actorStats_Count; i++)
    pActor->stats[i] = 0;

  pActor->stats[as_Air] = MaxStatsValue;
  pActor->stats[as_Energy] = 64;
}

void actor_updateStats(actor *pActor, const viewCone &cone)
{
  // Check air
  constexpr int64_t UnderwaterAirCost = 4;
  constexpr int64_t SurfaceAirAmount = 24;
  constexpr int64_t NoAirEnergyCost = 24;

  if (cone[vcp_self] & tf_Underwater)
    modify_with_clamp(pActor->stats[as_Air], -UnderwaterAirCost, MinStatsValue, MaxStatsValue);
  else
    modify_with_clamp(pActor->stats[as_Air], SurfaceAirAmount, MinStatsValue, MaxStatsValue);

  if (!pActor->stats[as_Air])
    modify_with_clamp(pActor->stats[as_Energy], -NoAirEnergyCost, MinStatsValue, MaxStatsValue);

  // Digest
  constexpr int64_t FoodEnergyAmount = 1;
  constexpr int64_t FoodDigestionAmount = 1;

  size_t count = 0;

  for (size_t i = _actorStats_FoodBegin; i <= _actorStats_FoodEnd; i++)
  {
    if (pActor->stats[i])
    {
      modify_with_clamp(pActor->stats[i], -FoodDigestionAmount, MinStatsValue, MaxStatsValue);
      count++;
    }
  }

  if (count > 0)
    modify_with_clamp(pActor->stats[as_Energy], (1ULL << (count - 1)) * FoodEnergyAmount, MinStatsValue, MaxStatsValue);
}

//////////////////////////////////////////////////////////////////////////

constexpr int64_t CollideEnergyCost = 2;

void actor_move(actor *pActor, const level &lvl)
{
  constexpr int64_t MovementEnergyCost = 3;
  constexpr vec2i16 lut[_lookDirection_Count] = { vec2i16(-1, 0), vec2i16(0, -1), vec2i16(1, 0), vec2i16(0, 1) };

  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);
  lsAssert(!(lvl.grid[pActor->pos.y * level::width + pActor->pos.x] & tf_Collidable));

  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -MovementEnergyCost, MinStatsValue, MaxStatsValue);

  if (oldEnergy < MovementEnergyCost)
    return;

  const vec2u16 newPos = vec2u16(vec2i16(pActor->pos) + lut[pActor->look_dir]);
  const size_t newIdx = newPos.y * level::width + newPos.x;

  if (!!(lvl.grid[newIdx] & tf_Collidable))
  {
    modify_with_clamp(pActor->stats[as_Energy], -CollideEnergyCost, MinStatsValue, MaxStatsValue);
    return;
  }

  lsAssert(!(lvl.grid[newIdx] & tf_Collidable));
  lsAssert(newPos.x < level::width - level::wallThickness && newPos.y < level::height - level::wallThickness && newPos.x >= level::wallThickness && newPos.y >= level::wallThickness);

  pActor->pos = newPos;
}

void actor_moveTwo(actor *pActor, const level &lvl)
{
  constexpr int64_t DoubleMovementEnergyCost = 7;
  constexpr vec2i16 LutPosDouble[_lookDirection_Count] = { vec2i16(-2, 0), vec2i16(0, -2), vec2i16(2, 0), vec2i16(0, 2) };
  constexpr int8_t LutIdxSingle[_lookDirection_Count] = { -1, -(int64_t)level::width, 1, level::width };

  const size_t currentIdx = pActor->pos.y * level::width + pActor->pos.x;
  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);
  lsAssert(!(lvl.grid[currentIdx] & tf_Collidable));

  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -DoubleMovementEnergyCost, MinStatsValue, MaxStatsValue);

  if (oldEnergy < DoubleMovementEnergyCost)
    return;

  const size_t nearIdx = currentIdx + LutIdxSingle[pActor->look_dir];
  const size_t newPosIdx = currentIdx + 2 * LutIdxSingle[pActor->look_dir];

  if (!!(lvl.grid[newPosIdx] & tf_Collidable) || !!(lvl.grid[nearIdx] & tf_Collidable))
  {
    modify_with_clamp(pActor->stats[as_Energy], -CollideEnergyCost, MinStatsValue, MaxStatsValue);
    return;
  }

  const vec2u16 newPos = (vec2u16)((vec2i16)(pActor->pos) + LutPosDouble[pActor->look_dir]);

  lsAssert(newPosIdx == newPos.y * level::width + newPos.x);
  lsAssert(!(lvl.grid[newPosIdx] & tf_Collidable));
  lsAssert(newPos.x < level::width - level::wallThickness && newPos.y < level::height - level::wallThickness && newPos.x >= level::wallThickness && newPos.y >= level::wallThickness);

  pActor->pos = newPos;
}

constexpr int64_t TurnEnergy = 2;

void actor_turnLeft(actor *pActor)
{
  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -TurnEnergy, MinStatsValue, MaxStatsValue);

  if (oldEnergy < TurnEnergy)
    return;

  pActor->look_dir = pActor->look_dir == ld_left ? ld_down : (lookDirection)(pActor->look_dir - 1);
  lsAssert(pActor->look_dir < _lookDirection_Count);
}

void actor_turnRight(actor *pActor)
{
  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -TurnEnergy, MinStatsValue, MaxStatsValue);

  if (oldEnergy < TurnEnergy)
    return;

  pActor->look_dir = pActor->look_dir == ld_down ? ld_left : (lookDirection)(pActor->look_dir + 1);
  lsAssert(pActor->look_dir < _lookDirection_Count);
}

void actor_eat(actor *pActor, level *pLvl, const viewCone &cone)
{
  static constexpr int64_t EatEnergyCost = 2;
  static constexpr int64_t NoFoodFoundEnergyCost = 2;
  static constexpr int64_t FoodAmount = 16;
  static constexpr uint8_t StomachCapacity = 127;

  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);

  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -EatEnergyCost, MinStatsValue, MaxStatsValue);

  if (oldEnergy < EatEnergyCost)
    return;

  size_t stomachFoodCount = 0;

  for (size_t i = _actorStats_FoodBegin; i <= _actorStats_FoodEnd; i++)
    stomachFoodCount += pActor->stats[i];

  lsAssert(stomachFoodCount <= StomachCapacity);
  bool anyFood = false;

  for (size_t i = _actorStats_FoodBegin; i <= _actorStats_FoodEnd; i++)
  {
    if (cone[vcp_self] & (1ULL << i))
    {
      stomachFoodCount += modify_with_clamp(pActor->stats[i], FoodAmount, lsMinValue<uint8_t>(), (uint8_t)((StomachCapacity - stomachFoodCount) + pActor->stats[i]));
      pLvl->grid[pActor->pos.y * level::width + pActor->pos.x] &= ~(1ULL << i); // yes, actor stats & tile masks SHARE the masks for foods.
      anyFood = true;
    }
  }

  if (!anyFood)
    modify_with_clamp(pActor->stats[as_Energy], -NoFoodFoundEnergyCost, MinStatsValue, MaxStatsValue);
}

void actor_wait(actor *pActor)
{
  constexpr int64_t WaitEnergyCost = 1;
  modify_with_clamp(pActor->stats[as_Energy], -WaitEnergyCost, MinStatsValue, MaxStatsValue);
}

static constexpr int64_t MoveDiagonalCost = 4;

void actor_moveDiagonalLeft(actor *pActor, const level lvl)
{
  const size_t currentIdx = pActor->pos.y * level::width + pActor->pos.x;
  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);
  lsAssert(!(lvl.grid[currentIdx] & tf_Collidable));

  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -MoveDiagonalCost, MinStatsValue, MaxStatsValue);

  if (oldEnergy < MoveDiagonalCost)
    return;

  constexpr vec2i16 lut[_lookDirection_Count] = { vec2i16(-1, 1), vec2i16(-1, -1), vec2i16(1, -1), vec2i16(1, 1) };

  const vec2u16 newPos = (vec2u16)((vec2i16)(pActor->pos) + lut[pActor->look_dir]);
  const size_t newIdx = newPos.y * level::width + newPos.x;

  if (!!(lvl.grid[newIdx] & tf_Collidable))
  {
    modify_with_clamp(pActor->stats[as_Energy], -CollideEnergyCost, MinStatsValue, MaxStatsValue);
    return;
  }

  lsAssert(!(lvl.grid[newIdx] & tf_Collidable));
  lsAssert(newPos.x < level::width - level::wallThickness && newPos.y < level::height - level::wallThickness && newPos.x >= level::wallThickness && newPos.y >= level::wallThickness);

  pActor->pos = newPos;
}

void actor_moveDiagonalRight(actor *pActor, const level lvl)
{
  const size_t currentIdx = pActor->pos.y * level::width + pActor->pos.x;
  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);
  lsAssert(!(lvl.grid[currentIdx] & tf_Collidable));

  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -MoveDiagonalCost, MinStatsValue, MaxStatsValue);

  if (oldEnergy < MoveDiagonalCost)
    return;

  constexpr vec2i16 lut[_lookDirection_Count] = { vec2i16(-1, -1), vec2i16(1, -1), vec2i16(1, 1), vec2i16(-1, 1) };

  const vec2u16 newPos = (vec2u16)((vec2i16)(pActor->pos) + lut[pActor->look_dir]);
  const size_t newIdx = newPos.y * level::width + newPos.x;

  if (!!(lvl.grid[newIdx] & tf_Collidable))
  {
    modify_with_clamp(pActor->stats[as_Energy], -CollideEnergyCost, MinStatsValue, MaxStatsValue);
    return;
  }

  lsAssert(!(lvl.grid[newIdx] & tf_Collidable));
  lsAssert(newPos.x < level::width - level::wallThickness && newPos.y < level::height - level::wallThickness && newPos.x >= level::wallThickness && newPos.y >= level::wallThickness);

  pActor->pos = newPos;
}
