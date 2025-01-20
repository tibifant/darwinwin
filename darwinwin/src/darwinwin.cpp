#include "darwinwin.h"
#include "io.h"
#include "level_generator.h"
#include "evolution.h"

#include <filesystem>

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

void level_gen_water_level(level *pLvl)
{
  level_gen_init(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace_mask(pLvl, tf_Underwater, 0, level::total / 10);
  level_gen_grow(pLvl, 0);
  level_gen_sprinkle_grow_into_inv_mask(pLvl, tf_Underwater, tf_Underwater, level_gen_make_chance<0.5>());
  level_gen_finalize(pLvl);
}

void level_gen_water_food_level(level *pLvl)
{
  level_gen_init(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace_mask(pLvl, tf_Underwater, 0, level::total / 10);
  level_gen_grow(pLvl, 0);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Vitamin | tf_Underwater, level::total / 10);
  level_gen_random_sprinkle_replace(pLvl, tf_Vitamin | tf_Underwater, tf_Vitamin | tf_Underwater | tf_Fat, level::total / 3); // UVF looks sus
  level_gen_sprinkle_grow_into_mask(pLvl, tf_Underwater | tf_Vitamin, tf_Underwater, level_gen_make_chance<0.75>());
  level_gen_sprinkle_grow_into_inv_mask(pLvl, tf_Underwater, tf_Underwater, level_gen_make_chance<0.5>());
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Protein, level::total / 10);
  level_gen_finalize(pLvl);
}

//////////////////////////////////////////////////////////////////////////

void level_generateDefault(level *pLvl)
{
  level_gen_water_food_level(pLvl);
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

    neural_net_eval(pActors[i].brain, ioBuffer);

    int16_t maxValue = ioBuffer.data[0];
    size_t bestActionIndex = 0;
    constexpr size_t maxActionIndex = lsMin(LS_ARRAYSIZE(ioBuffer.data), _actorAction_Count);

    for (size_t actionIndex = 1; actionIndex < maxActionIndex; actionIndex++)
    {
      if (maxValue < ioBuffer.data[actionIndex])
      {
        maxValue = ioBuffer.data[actionIndex];
        bestActionIndex = actionIndex;
      }
    }

    pActors[i].last_action = (actorAction)bestActionIndex;
    actor_act(&pActors[i], &lvl, cone, pActors[i].last_action);
  }

  lsAssert(anyAlive); // otherwise, maybe don't call us???

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

  default:
    lsFail(); // not implemented.
    break;
  }
}

void actor_initStats(actor *pActor)
{
  for (size_t i = 0; i < _actorStats_Count; i++)
    pActor->stats[i] = 32;

  pActor->stats[as_Air] = 127;
  pActor->stats[as_Energy] = 127;
}

void actor_updateStats(actor *pActor, const viewCone &cone)
{
  constexpr int64_t IdleEnergyCost = 2;

  // Remove Idle Energy
  modify_with_clamp(pActor->stats[as_Energy], -IdleEnergyCost);

  // Check air
  constexpr int64_t UnderwaterAirCost = 5;
  constexpr int64_t SurfaceAirAmount = 3;
  constexpr int64_t NoAirEnergyCost = 8;

  if (cone[vcp_self] & tf_Underwater)
    modify_with_clamp(pActor->stats[as_Air], -UnderwaterAirCost);
  else
    modify_with_clamp(pActor->stats[as_Air], SurfaceAirAmount);

  if (!pActor->stats[as_Air])
    modify_with_clamp(pActor->stats[as_Energy], -NoAirEnergyCost);

  // Digest
  constexpr int64_t FoodEnergyAmount = 5;
  constexpr int64_t FoodDigestionAmount = 1;

  size_t count = 0;

  for (size_t i = _actorStats_FoodBegin; i <= _actorStats_FoodEnd; i++)
  {
    if (pActor->stats[i])
    {
      modify_with_clamp(pActor->stats[i], -FoodDigestionAmount);
      count++;
    }
  }

  modify_with_clamp(pActor->stats[as_Energy], count * FoodEnergyAmount);
}

//////////////////////////////////////////////////////////////////////////

void actor_move(actor *pActor, const level &lvl)
{
  constexpr int64_t MovementEnergyCost = 10;
  constexpr int64_t CollideEnergyCost = 4;
  constexpr vec2i16 lut[_lookDirection_Count] = { vec2i16(-1, 0), vec2i16(0, -1), vec2i16(1, 0), vec2i16(0, -1) };

  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);
  lsAssert(!(lvl.grid[pActor->pos.y * level::width + pActor->pos.x] & tf_Collidable));

  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -MovementEnergyCost);

  if (oldEnergy < MovementEnergyCost)
    return;

  const vec2u16 newPos = vec2u16(vec2i16(pActor->pos) + lut[pActor->look_dir]);

  if (lvl.grid[newPos.y * level::width + newPos.x] & tf_Collidable)
  {
    modify_with_clamp(pActor->stats[as_Energy], -CollideEnergyCost);
    return;
  }

  pActor->pos = newPos;
}

void actor_moveTwo(actor *pActor, const level &lvl)
{
  constexpr int64_t DoubleMovementEnergyCost = 30;
  constexpr int64_t CollideEnergyCost = 4;
  constexpr vec2i16 LutDouble[_lookDirection_Count] = { vec2i16(-2, 0), vec2i16(0, -2), vec2i16(2, 0), vec2i16(0, -2) };
  constexpr int8_t LutSingle[_lookDirection_Count] = { -1, -(int64_t)level::width, 1, level::width };

  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);
  lsAssert(!(lvl.grid[pActor->pos.y * level::width + pActor->pos.x] & tf_Collidable));

  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -DoubleMovementEnergyCost);

  if (oldEnergy < DoubleMovementEnergyCost)
    return;

  const size_t nearIdx = (pActor->pos.y * level::width + pActor->pos.x) + LutSingle[pActor->look_dir];
  const size_t newPosIdx = nearIdx + LutSingle[pActor->look_dir];

  if ((lvl.grid[newPosIdx] & tf_Collidable) || (lvl.grid[nearIdx] & tf_Collidable))
  {
    modify_with_clamp(pActor->stats[as_Energy], -CollideEnergyCost);
    return;
  }

  const vec2u16 newPos = vec2u16((vec2i16)(pActor->pos) + LutDouble[pActor->look_dir]);
  pActor->pos = newPos;
}

constexpr int64_t TurnEnergy = 2;

void actor_turnLeft(actor *pActor)
{
  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -TurnEnergy);

  if (oldEnergy < TurnEnergy)
    return;

  pActor->look_dir = pActor->look_dir == ld_left ? ld_down : (lookDirection)(pActor->look_dir - 1);
  lsAssert(pActor->look_dir < _lookDirection_Count);
}

void actor_turnRight(actor *pActor)
{
  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -TurnEnergy);

  if (oldEnergy < TurnEnergy)
    return;

  pActor->look_dir = pActor->look_dir == ld_down ? ld_left : (lookDirection)(pActor->look_dir + 1);
  lsAssert(pActor->look_dir < _lookDirection_Count);
}

void actor_eat(actor *pActor, level *pLvl, const viewCone &cone)
{
  static constexpr int64_t EatEnergyCost = 3;
  static constexpr int64_t FoodAmount = 2;
  static constexpr uint8_t StomachCapacity = 255;

  lsAssert(pActor->pos.x < level::width && pActor->pos.y < level::height);

  const size_t oldEnergy = pActor->stats[as_Energy];
  modify_with_clamp(pActor->stats[as_Energy], -EatEnergyCost);

  if (oldEnergy < TurnEnergy)
    return;

  size_t stomachFoodCount = 0;

  for (size_t i = _actorStats_FoodBegin; i <= _actorStats_FoodEnd; i++)
    stomachFoodCount += pActor->stats[i];

  lsAssert(stomachFoodCount <= StomachCapacity);

  for (size_t i = _actorStats_FoodBegin; i <= _actorStats_FoodEnd; i++)
  {
    if (cone[vcp_self] & (1ULL << i))
    {
      stomachFoodCount += modify_with_clamp(pActor->stats[i], FoodAmount, lsMinValue<uint8_t>(), (uint8_t)((StomachCapacity - stomachFoodCount) + pActor->stats[i]));
      pLvl->grid[pActor->pos.y * level::width + pActor->pos.x] &= ~(1ULL << i);
    }
  }
}

//////////////////////////////////////////////////////////////////////////

struct starter_random_config
{
  using mutator = mutator_random;
  using crossbreeder = crossbreeder_naive;

  static constexpr size_t survivingGenes = 16;
  static constexpr size_t newGenesPerGeneration = 3 * 2 * 16;
};

template <typename crossbreeder>
void crossbreed(actor &val, const actor parentA, const actor parentB, const crossbreeder &c)
{
  val.look_dir = parentA.look_dir;
  val.pos = parentA.pos;

  crossbreeder_eval(c, val.brain.values, LS_ARRAYSIZE(val.brain.values), parentA.brain.values, parentB.brain.values);
}

template <typename mutator>
void mutate(actor &target, const mutator &m)
{
  mutator_eval(m, &target.brain.values[0], LS_ARRAYSIZE(target.brain.values), (int16_t)lsMinValue<uint8_t>(), (int16_t)lsMaxValue<uint8_t>());
}

// TODO: Eval Funcs... -> Give scores

size_t evaluate_actor(const actor &in)
{
  constexpr size_t cycles = 1000;

  actor actr = in;
  level lvl = _CurrentLevel;
  size_t score = 0;

  for (size_t i = 0; i < cycles; i++)
  {
    const uint8_t foodCapacityBefore = actr.stomach_remaining_capacity;

    if ((i & 32) == 0)
    {
      // TODO: Sprinkle Foods.
    }

    if (!level_performStep(lvl, &actr, 1))
      break;

    score++;
    score += ((uint8_t)(foodCapacityBefore > actr.stomach_remaining_capacity)) * 3;
  }

  return score;
}

size_t evalutate_null(const actor &)
{
  return 0;
}

lsResult train_loop(thread_pool *pThreadPool, const char *dir)
{
  lsResult result = lsR_Success;

  {
    actor actr;
    load_newest_brain(dir, actr);

    uint64_t rand = lsGetRand();
    actr.pos = vec2u16(rand % level::width, (rand >> level::widthBits) % level::height);
    actr.look_dir = (lookDirection)((rand >> (level::widthBits + level::heightBits)) % _lookDirection_Count);

    actor_initStats(&actr);

    constexpr size_t iterationsPerLevel = 1000;

    evolution<actor, starter_random_config> evl;
    evolution_init(evl, actr, evalutate_null); // because no level is generated yet!

    size_t levelIndex = 0;

    while (_DoTraining)
    {
      level_generateDefault(&_CurrentLevel);
      evolution_reevaluate(evl, evaluate_actor);

      const actor *pBest = nullptr;
      size_t score, bestScore = 0;

      for (size_t i = 0; i < iterationsPerLevel && _DoTraining; i++)
      {
        evolution_generation(evl, evaluate_actor, pThreadPool);
        evolution_get_best(evl, &pBest, score);

        if (score > bestScore)
        {
          print_log_line("New Best: Level ", levelIndex, ", Generation ", i, ": ", score);
          bestScore = score;
        }
      }


      LS_ERROR_CHECK(save_brain(dir, *pBest));
      levelIndex++;
    }
  }

epilogue:
  _TrainingRunning = false;
  return result;
}

//////////////////////////////////////////////////////////////////////////

#include <time.h>

lsResult save_brain(const char *dir, const actor &actr)
{
  lsResult result = lsR_Success;

  const uint64_t now = (uint64_t)time(nullptr);
  char filename[256];
  sformat_to(filename, LS_ARRAYSIZE(filename), dir, "/", now, ".brain");

  print("Saving brain to file: '", filename, '\n');

  {
    cached_file_byte_stream_writer<> write_stream;
    LS_ERROR_CHECK(write_byte_stream_init(write_stream, filename));
    value_writer<decltype(write_stream)> writer;
    LS_ERROR_CHECK(value_writer_init(writer, &write_stream));

    LS_ERROR_CHECK(neural_net_write(actr.brain, writer));
    LS_ERROR_CHECK(write_byte_stream_flush(write_stream));
  }

epilogue:
  return result;
}

lsResult load_brain_from_file(const char *filename, actor &actr)
{
  lsResult result = lsR_Success;

  print("Loading brain from file: ", filename, '\n');

  cached_file_byte_stream_reader<> read_stream;
  value_reader<cached_file_byte_stream_reader<>> reader;
  LS_ERROR_CHECK(read_byte_stream_init(read_stream, filename));
  LS_ERROR_CHECK(value_reader_init(reader, &read_stream));

  LS_ERROR_CHECK(neural_net_read(actr.brain, reader));
  read_byte_stream_destroy(read_stream);

epilogue:
  return result;
}

lsResult load_newest_brain(const char *dir, actor &actr)
{
  lsResult result = lsR_Success;

  const std::filesystem::path path(dir);

  int64_t bestTime = -1;
  std::string best;

  for (const std::filesystem::directory_entry &dir_entry : std::filesystem::directory_iterator(dir))
  {
    if (dir_entry.is_regular_file())
    {
      const std::filesystem::file_time_type &timestamp = dir_entry.last_write_time();
      const int64_t timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();

      if (bestTime < timeMs)
      {
        bestTime = timeMs;
        best = dir_entry.path().filename().string();
      }
    }
  }

  lsAssert(bestTime >= 0);
  LS_ERROR_IF(best.empty(), lsR_ResourceNotFound);
  LS_ERROR_CHECK(load_brain_from_file(best.c_str(), actr));

epilogue:
  return result;
}

// load specific brain: list and then select in console

// train: load actor, start training, save actor whilst training, reevaluate scores... save
