#include "training.h"
#include "level_generator.h"
#include "evolution.h"
#include "local_list.h"
#include "io.h"

#include <time.h>
#include <filesystem>

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

void level_gen_puddle_food_level(level *pLvl)
{
  level_gen_init(pLvl, 0);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Underwater, level::total / 8);
  level_gen_grow(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Vitamin, level::total / 10);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Protein, level::total / 10);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Fat, level::total / 10);
  //level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Sugar, level::total / 10);
  level_gen_finalize(pLvl);
}

void level_gen_puddle_sugar_underwater_level(level *pLvl)
{
  level_gen_init(pLvl, 0);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Underwater, level::total / 8);
  level_gen_grow(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace(pLvl, tf_Underwater, tf_Sugar | tf_Underwater, level::total / 4);
  level_gen_finalize(pLvl);
}

void level_gen_puddle_food_sugar_underwater_level(level *pLvl)
{
  level_gen_init(pLvl, 0);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Underwater, level::total / 10);
  level_gen_grow(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Vitamin, level::total / 50);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Protein, level::total / 50);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Fat, level::total / 50);
  level_gen_random_sprinkle_replace(pLvl, tf_Underwater, tf_Sugar | tf_Underwater, level::total / 10);
  //level_gen_random_sprinkle_replace(pLvl, tf_Underwater, tf_Sugar | tf_Fat | tf_Underwater, level::total / 10);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Collidable, level::total / 30);
  level_gen_sprinkle_grow(pLvl, tf_Collidable, level_gen_make_chance<0.5>());
  level_gen_finalize(pLvl);
}

void level_gen_sprinkle_collidable_level(level *pLvl)
{
  level_gen_init(pLvl, 0);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Collidable, level::total / 10);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Underwater, level::total / 10);
  level_gen_grow(pLvl, tf_Underwater);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Vitamin, level::total / 10);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Protein, level::total / 10);
  level_gen_random_sprinkle_replace_inv_mask(pLvl, tf_Underwater, tf_Fat, level::total / 10);
  level_gen_finalize(pLvl);
}

//////////////////////////////////////////////////////////////////////////

void level_generateDefault(level *pLvl)
{
  //level_gen_puddle_sugar_underwater_level(pLvl);
  level_gen_puddle_food_sugar_underwater_level(pLvl);
}

//////////////////////////////////////////////////////////////////////////

lsResult actor_saveBrain(const char *dir, const actor &actr)
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

lsResult actor_loadBrainFromFile(const char *filename, actor &actr)
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

lsResult actor_loadNewestBrain(const char *dir, actor &actr)
{
  lsResult result = lsR_Success;

  const std::filesystem::path path(dir);

  int64_t bestTime = -1;
  std::string best;

  char bestChar = '0';
  std::string bestByName;
  int64_t bestNameTime = -1;

  for (const std::filesystem::directory_entry &dir_entry : std::filesystem::directory_iterator(dir))
  {
    if (dir_entry.is_regular_file())
    {
      const std::filesystem::file_time_type &timestamp = dir_entry.last_write_time();
      const int64_t timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();

      if (!lsIsUInt(dir_entry.path().stem().c_str()))
      {
        const std::string n = dir_entry.path().stem().string();
        if (bestChar < n[0])
        {
          bestChar = n[0];
          bestByName = dir_entry.path().filename().string();
          bestNameTime = timeMs;
        }
      }

      if (bestTime < timeMs)
      {
        bestTime = timeMs;
        best = dir_entry.path().filename().string();
      }
    }
  }

  if (bestTime < bestNameTime)
    best = bestByName;

  LS_ERROR_IF(best.empty(), lsR_ResourceNotFound);
  lsAssert(bestTime >= 0);
  char filename[256];
  sformat_to(filename, LS_ARRAYSIZE(filename), dir, '/', best.c_str());
  LS_ERROR_CHECK(actor_loadBrainFromFile(filename, actr));

epilogue:
  return result;
}

// TODO load specific brain: list and then select in console

//////////////////////////////////////////////////////////////////////////

struct starter_random_config
{
  using mutator = mutator_random;
  using crossbreeder = crossbreeder_naive;

  static constexpr size_t survivingGenes = 16;
  static constexpr size_t newGenesPerGeneration = 3 * 2 * 5 * 8;
};

struct starter_random_config_independent : starter_random_config
{
  static constexpr size_t survivingGenes = 4;
  static constexpr size_t newGenesPerGeneration = 16;
};

struct smart_mutator_config
{
  static constexpr float param_mutation_min_fac = 0.975f;
  static constexpr float param_mutation_max_fac = 1.f / param_mutation_min_fac;
  static constexpr float mutationChanceBase = 0.05f;
  static constexpr float min_mutation_chance_fac = 0.1f / mutationChanceBase;
  static constexpr float max_mutation_chance_fac = 0.85f / mutationChanceBase;
  static constexpr uint16_t mutationRateBase = 32;
  static constexpr float min_mutation_rate_fac = 16.f / (float)mutationRateBase;
  static constexpr float max_mutation_rate_fac = 64.f / (float)mutationRateBase;
};

struct smart_config
{
  using mutator = smart_mutator<smart_mutator_config>;
  using crossbreeder = crossbreeder_naive;

  static constexpr size_t survivingGenes = 4;
  static constexpr size_t newGenesPerGeneration = 32;
};

//////////////////////////////////////////////////////////////////////////

template <typename crossbreeder>
void crossbreed(actor &val, const actor parentA, const actor parentB, const crossbreeder &c)
{
  val.look_dir = parentA.look_dir;
  val.pos = parentA.pos;
  lsMemcpy(val.stats, parentA.stats, LS_ARRAYSIZE(val.stats));

  crossbreeder_eval(c, val.brain.values, LS_ARRAYSIZE(val.brain.values), parentA.brain.values, parentB.brain.values);
}

template <typename mutator>
void mutate(actor &target, const mutator &m)
{
  mutator_eval(m, &target.brain.values[0], LS_ARRAYSIZE(target.brain.values), (int16_t)lsMinValue<int8_t>(), (int16_t)lsMaxValue<int8_t>());
}

//////////////////////////////////////////////////////////////////////////

constexpr size_t EvaluatingCycles = 256;

// score: more sugar = more good
// if sugar at vcp_self, then position changed, then still sugar at vcp_self

size_t evaluate_actor(const actor &in)
{
  actor actr = in;
  level lvl = _CurrentLevel;
  size_t score = 0;

  for (size_t i = 0; i < EvaluatingCycles; i++)
  {
    const bool underwaterBefore = !!(actr.last_view_cone[vcp_self] & tf_Underwater);
    const bool sugarAtPosBefore = !!(actr.last_view_cone[vcp_self] & tf_Sugar);

    uint16_t stomachFillLevelBefore = 0;

    for (size_t j = _actorStats_FoodBegin; j <= _actorStats_FoodEnd; j++)
      stomachFillLevelBefore += actr.stats[j];

    const uint16_t sugarLevelBefore = actr.stats[as_Sugar];

    level_gen_random_sprinkle_replace_mask(&lvl, tf_Underwater, tf_Underwater | tf_Sugar, level::total / 100);

    if (!level_performStep(lvl, &actr, 1))
      break;

    const bool underwaterAfter = !!(actr.last_view_cone[vcp_self] & tf_Underwater);
    const bool sugarAtPosAfter = !!(actr.last_view_cone[vcp_self] & tf_Sugar);

    const uint16_t sugarLevelAfter = actr.stats[as_Sugar];
    uint16_t stomachFillLevelAfter = 0;

    for (size_t j = _actorStats_FoodBegin; j <= _actorStats_FoodEnd; j++)
      stomachFillLevelAfter += actr.stats[j];

    uint16_t foodSeeScore = 0;
    static const uint16_t perViewConePosFoodSeeScore[] = {
        7, // vcp_self
        4, // vcp_nearLeft
        5, // vcp_nearCenter
        4, // vcp_nearRight
        2, // vcp_midLeft
        3, // vcp_midCenter
        2, // vcp_midRight
        1  // vcp_farCenter
    };

    static_assert(LS_ARRAYSIZE(actr.last_view_cone.values) == LS_ARRAYSIZE(perViewConePosFoodSeeScore));

    for (size_t j = 0; j < LS_ARRAYSIZE(actr.last_view_cone.values); j++)
      if (actr.last_view_cone.values[j] & tf_Sugar) //_tileFlag_FoodMask)
        foodSeeScore += perViewConePosFoodSeeScore[j];

    score += 30;
    score += !(actr.last_view_cone.values[vcp_self] & tf_Underwater) ? 1 : 0;
    score += foodSeeScore * 5;
    score += ((uint8_t)(stomachFillLevelAfter > stomachFillLevelBefore)) * 2000ULL;
    score += ((uint8_t)(sugarLevelAfter > sugarLevelBefore)) * 10000ULL;

    // score: if pos changed and sugar at vcp_self before and now.
    score += (size_t)(underwaterBefore && (actr.last_action == aa_DragItem) && sugarAtPosBefore && sugarAtPosAfter) * 300;
    score += ((size_t)(underwaterBefore && !underwaterAfter && sugarAtPosBefore && sugarAtPosAfter && actr.last_action == aa_DragItem)) * 800;
  }

  return score;
}

size_t evaluate_null(const actor &)
{
  return 0;
}

constexpr size_t generationsPerLevel = 1024ULL * 4;

lsResult train_loopIndependentEvolution(thread_pool *pThreadPool, const char *dir)
{
  lsResult result = lsR_Success;

  using config = smart_config;
  using evl_type = evolution<actor, config>;
  small_list<evl_type> evolutions;

  struct actor_ref
  {
    size_t score, evolution_idx, idx;
    actor_ref() = default; // actor_ref': no appropriate default constructor available -> yes?
    actor_ref(const size_t score, const size_t evolution_idx, const size_t idx) : score(score), evolution_idx(evolution_idx), idx(idx) {}
    bool operator > (const actor_ref &other) const { return score < other.score; }
    bool operator < (const actor_ref &other) const { return score > other.score; }
  };

  small_list<actor_ref> best_actor_refs;
  actor best_actors[config::survivingGenes];

  actor actr;
  actor_loadNewestBrain(dir, actr);

  uint64_t rand = lsGetRand();
  actr.pos = vec2u16((rand & 0xFFFF) % (level::width - level::wallThickness * 2), ((rand >> 16) & 0xFFFF) % (level::height - level::wallThickness * 2));
  actr.pos += vec2u16(level::wallThickness);
  actr.look_dir = (lookDirection)((rand >> 32) % _lookDirection_Count);

  actor_initStats(&actr);
  size_t trainingCycle = 0;
  const size_t geneGenerationCount = thread_pool_thread_count(pThreadPool) * config::newGenesPerGeneration * generationsPerLevel;

  print("Starting Training: ", thread_pool_thread_count(pThreadPool), " Threads x ", config::newGenesPerGeneration, " Genes x ", generationsPerLevel, " Generations / Level x ", EvaluatingCycles, " Evaluating Cycles Max = ", FU(Group)(thread_pool_thread_count(pThreadPool) * config::newGenesPerGeneration * generationsPerLevel * EvaluatingCycles), '\n');

  for (size_t i = 0; i < thread_pool_thread_count(pThreadPool); i++)
  {
    evl_type evl;
    LS_ERROR_CHECK(evolution_init_empty(evl));

    evolution_add_unevaluated_target(evl, std::move(actr));

    LS_ERROR_CHECK(list_add(&evolutions, std::move(evl)));
  }

  while (_DoTraining)
  {
    list_clear(&best_actor_refs);

    level_generateDefault(&_CurrentLevel);
    size_t maxRetries = 32;

    do
    {
      rand = lsGetRand();
      actr.pos = vec2u16((rand & 0xFFFF) % (level::width - level::wallThickness * 2), ((rand >> 16) & 0xFFFF) % (level::height - level::wallThickness * 2));
      actr.pos += vec2u16(level::wallThickness);
      actr.look_dir = (lookDirection)((rand >> 32) % _lookDirection_Count);
      maxRetries--;
    } while ((_CurrentLevel.grid[actr.pos.x + actr.pos.y * level::width] & tf_Collidable) && maxRetries);

    for (auto &evol : evolutions)
      evolution_for_each(evol, [&](actor &a) { a.pos = actr.pos; a.look_dir = actr.look_dir; });

    if (maxRetries == 0)
    {
      print_error_line("Failed to find non-collidable position in lvl.");
      continue;
    }

    for (auto &evol : evolutions)
      evolution_reevaluate(evol, evaluate_actor);

    const int64_t startNs = lsGetCurrentTimeNs();
    const int64_t startMs = lsGetCurrentTimeMs();

    for (size_t i = 0; i < evolutions.count; i++)
    {
      evl_type *pEvolution = &evolutions[i];

      std::function<void()> async_func = [=]()
        {
          for (size_t j = 0; _DoTraining && lsGetCurrentTimeMs() - startMs < 2500; j++)
            evolution_generation(*pEvolution, evaluate_actor);
        };

      thread_pool_add(pThreadPool, async_func);
    }

    thread_pool_await(pThreadPool);

    const int64_t endNs = lsGetCurrentTimeNs();

    // extract actor refs w/ score
    size_t index = 0;
    for (auto &evol : evolutions)
    {
      for (size_t j = 0; j < evolution_get_count(evol); j++)
      {
        size_t idx;
        size_t score;
        evolution_get_at(evol, j, idx, score);
        LS_ERROR_CHECK(list_add(&best_actor_refs, actor_ref(score, index, idx)));
      }

      index++;
    }

    // sort
    list_sort(best_actor_refs);

    // extract best actors to best_actors
    for (size_t i = 0; i < LS_ARRAYSIZE(best_actors); i++)
      best_actors[i] = std::move(pool_get(evolutions[best_actor_refs[i].evolution_idx].genes, best_actor_refs[i].idx)->t);

    for (auto &evol : evolutions)
    {
      // clear evolutions
      evolution_clear(evol);

      // insert best actors to evolutions
      for (size_t i = 0; i < LS_ARRAYSIZE(best_actors); i++)
        evolution_add_unevaluated_target(evol, best_actors[i]);
    }

    const actor_ref *pBestRef = list_get(&best_actor_refs, 0);
    print_log_line("Current Best: Training Cycle: ", trainingCycle, " w/ score: ", pBestRef->score, " (", FD(Group, Frac(3))(geneGenerationCount / ((endNs - startNs) * 1e-9)), " Generations/s (THIS IS NOW A LIE, AS WE'RE TIME-BASED!))");

    //if constexpr (std::remove_cvref_t<decltype(evolutions[0])>::has_mutator_state)
    //{
    //  for (size_t j = 0; j < evolutions.count; j++)
    //  {
    //    print('\t', FU(Min(3))(j), ": ");
    //    mutator_state_print(evolutions[j].mutatorState);
    //  }
    //}

    LS_ERROR_CHECK(actor_saveBrain(dir, best_actors[0]));

    trainingCycle++;
  }

epilogue:
  _TrainingRunning = false;
  return result;
}

// NOT in use!
lsResult train_loop(thread_pool *pThreadPool, const char *dir)
{
  lsResult result = lsR_Success;

  constexpr bool trainSynchronously = true;

  {
    actor actr;
    actor_loadNewestBrain(dir, actr);

    uint64_t rand = lsGetRand();
    actr.pos = vec2u16((rand & 0xFFFF) % (level::width - level::wallThickness * 2), ((rand >> 16) & 0xFFFF) % (level::height - level::wallThickness * 2));
    actr.pos += vec2u16(level::wallThickness);
    actr.look_dir = (lookDirection)((rand >> 32) % _lookDirection_Count);

    actor_initStats(&actr);

    evolution<actor, smart_config> evl;
    evolution_init(evl, actr, evaluate_null); // because no level is generated yet!

    size_t levelIndex = 0;

    while (_DoTraining)
    {
      level_generateDefault(&_CurrentLevel);
      size_t maxRetries = 32;

      do
      {
        rand = lsGetRand();
        actr.pos = vec2u16((rand & 0xFFFF) % (level::width - level::wallThickness * 2), ((rand >> 16) & 0xFFFF) % (level::height - level::wallThickness * 2));
        actr.pos += vec2u16(level::wallThickness);
        actr.look_dir = (lookDirection)((rand >> 32) % _lookDirection_Count);
        maxRetries--;
      } while ((_CurrentLevel.grid[actr.pos.x + actr.pos.y * level::width] & tf_Collidable) && maxRetries);

      evolution_for_each(evl, [&](actor &a) { a.pos = actr.pos; a.look_dir = actr.look_dir; });

      if (maxRetries == 0)
      {
        print_error_line("Failed to find non-collidable position in lvl.");
        continue;
      }

      evolution_reevaluate(evl, evaluate_actor);

      const actor *pBest = nullptr;
      size_t score, bestScore = 0;

      for (size_t i = 0; i < generationsPerLevel && _DoTraining; i++)
      {
        if constexpr (trainSynchronously)
          evolution_generation(evl, evaluate_actor);
        else
          evolution_generation(evl, evaluate_actor, pThreadPool);

        evolution_get_best(evl, &pBest, score);

        if (score > bestScore)
        {
          print_log_line("New Best: Level ", levelIndex, ", Generation ", i, ": ", score);
          bestScore = score;
        }
      }

      LS_ERROR_CHECK(actor_saveBrain(dir, *pBest));
      levelIndex++;
    }
  }

epilogue:
  _TrainingRunning = false;
  return result;
}
