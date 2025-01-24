#pragma once

#include "core.h"
#include "pool.h"
#include "small_list.h"
#include "thread_pool.h"
#include <random>

//////////////////////////////////////////////////////////////////////////

struct mutator_naive
{
  using state_type = std::nullptr_t;
};

inline void mutator_init(mutator_naive &mut, const size_t generation, std::nullptr_t &)
{
  (void)mut;
  (void)generation;
}

template <typename T>
  requires (std::is_integral_v<T>)
inline void mutator_eval(const mutator_naive &m, T &val, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>())
{
  (void)m;
  val = (T)lsClamp<int64_t>(val + (int64_t)(lsGetRand() % 5) - 2, min, max);
}

template <typename mutator, typename T>
inline void mutator_eval(const mutator &m, T *pVal, const size_t count, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>())
{
  for (size_t i = 0; i < count; i++)
    mutator_eval(m, pVal[i], min, max);
}

template <typename config>
struct mutator_chance
{
};

template <typename config>
inline void mutator_init(mutator_chance<config> &mut, const size_t generation)
{
  (void)mut;
  (void)generation;
}

template <typename T, typename config>
  requires (std::is_integral_v<T>)
inline void mutator_eval(const mutator_chance<config> &m, T &val, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>())
{
  (void)m;

  const uint64_t rand = lsGetRand();

  if ((rand & 1024) > config::chanceOf1024)
    return;

  val = (T)lsClamp<int64_t>(val + (int64_t)(lsGetRand() % 5) - 2, min, max);
}

//////////////////////////////////////////////////////////////////////////

struct mutator_random
{
  using state_type = std::nullptr_t;
};

inline void mutator_init(mutator_random &mut, const size_t generation, std::nullptr_t &)
{
  (void)mut;
  (void)generation;
}

template <typename mutator, typename T>
  requires (std::is_integral_v<T>)
inline void mutator_eval(const mutator_random &m, T &val, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>())
{
  (void)m;

  val = lsClamp((T)lsGetRand(), min, max);
}

inline void mutator_eval(const mutator_random &m, int16_t *pVal, const size_t count, const int16_t min, const int16_t max)
{
  (void)m;
  (void)min;
  (void)max;
  lsAssert(min == lsMinValue<int8_t>() && max == lsMaxValue<int8_t>());

  size_t i = 0;
  uint64_t rand;

  for (; i + 7 < count; i += 8)
  {
    rand = lsGetRand();

    for (size_t j = 0; j < 8; j++)
      pVal[i + j] = (int8_t)((rand >> (j * 8)) & 0xFF);
  }

  rand = lsGetRand();

  for (; i < count; i++)
  {
    pVal[i] = (int8_t)(rand & 0xFF);
    rand >>= 8;
  }
}

//////////////////////////////////////////////////////////////////////////

template <typename config>
struct mutator_zero_chance
{
  using state_type = std::nullptr_t;
};

template <typename config>
inline void mutator_init(mutator_zero_chance<config> &mut, const size_t generation, std::nullptr_t &)
{
  (void)mut;
  (void)generation;
}

template <typename T, typename config>
  requires (std::is_integral_v<T>)
inline void mutator_eval(const mutator_zero_chance<config> &m, T &val, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>())
{
  (void)m;
  lsAssert(min == lsMinValue<int8_t>() && max == lsMaxValue<int8_t>());

  const uint64_t rand = lsGetRand();

  if ((rand & 1024) > config::chanceOf1024)
    return;

  val = lsClamp(lsGetRand(), min, max);
}

// THIS IS NOT ZERO CHANCE!
template <typename config>
inline void mutator_eval(const mutator_zero_chance<config> &m, int16_t *pVal, const size_t count, const int16_t min, const int16_t max)
{
  (void)m;
  (void)min;
  (void)max;
  lsAssert(min == lsMinValue<int8_t>() && max == lsMaxValue<int8_t>());

  size_t i = 0;
  uint64_t rand;

  for (; i + 7 < count; i += 8)
  {
    rand = lsGetRand();

    for (size_t j = 0; j < 8; j++)
      pVal[i + j] = (int8_t)((rand >> (j * 8)) & 0xFF);
  }

  rand = lsGetRand();

  for (; i < count; i++)
  {
    pVal[i] = (int8_t)(rand & 0xFF);
    rand >>= 8;
  }
}

//////////////////////////////////////////////////////////////////////////

struct smart_mutator_mutation_params
{
  float mutation_chance = 1, mutation_rate = 1;
};

struct smart_mutator_state
{
  small_list<smart_mutator_mutation_params> gene_data;
  smart_mutator_mutation_params selected;
};

inline lsResult mutator_state_init(smart_mutator_state &ms, const size_t genesPerGeneration)
{
  return list_reserve(&ms.gene_data, genesPerGeneration);
}

inline void mutator_state_next_generation(smart_mutator_state &ms)
{
  list_clear(&ms.gene_data);
}

inline smart_mutator_mutation_params mutator_state_get_params(smart_mutator_state &ms)
{
  return ms.selected;
}

inline void mutator_state_add_params(smart_mutator_state &ms, const smart_mutator_mutation_params &params)
{
  LS_DEBUG_ERROR_ASSERT(list_add(&ms.gene_data, params));
}

inline void mutator_state_select(smart_mutator_state &ms, const size_t mutationIdx)
{
  ms.selected = ms.gene_data[mutationIdx];
}

template <double chance>
constexpr uint16_t smart_mutator_make_chance()
{
  static_assert(chance > 0 && chance < 1);
  constexpr double unitSize = (double)0xFFFF;
  return lsMax<uint16_t>(1, (uint16_t)(chance * unitSize + 0.5));
}

template <typename config>
struct smart_mutator
{
  using state_type = smart_mutator_state;
  smart_mutator_mutation_params params;

#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
  LS_ALIGN(16) mutable uint64_t rndLast[2];
  LS_ALIGN(16) mutable uint64_t rndLast2[2];
#ifdef _MSC_VER
#pragma warning(pop)
#endif

  int8_t precalculatedChanges[256];
  uint16_t chance;
};

template <typename config>
inline void mutator_init(smart_mutator<config> &mut, const size_t generation, typename smart_mutator<config>::state_type &state)
{
  (void)generation;

  mut.rndLast[0] = lsGetCurrentTimeNs();
  mut.rndLast[1] = __rdtsc();
  mut.rndLast2[0] = ~mut.rndLast[1];
  mut.rndLast2[1] = ~mut.rndLast[0];

  mut.params = mutator_state_get_params(state);

  const float minChance = mut.params.mutation_chance * config::param_mutation_min_fac; // ~1.1
  const float maxChance = mut.params.mutation_chance * config::param_mutation_max_fac; // ~1 / 1.1
  const float minRate = mut.params.mutation_rate * config::param_mutation_min_fac; // ~1.1
  const float maxRate = mut.params.mutation_rate * config::param_mutation_max_fac; // ~1 / 1.1

  std::mt19937 rnd;
  std::uniform_real_distribution<float> chanceDist(minChance, maxChance);
  std::uniform_real_distribution<float> rateDist(minRate, maxRate);

  mut.params.mutation_chance = chanceDist(rnd);
  mut.params.mutation_rate = rateDist(rnd);

  mut.params = mutator_state_get_params(state);

  mut.chance = (uint16_t)lsClamp<int64_t>((int64_t)lsRound((config::mutationChanceBase * mut.params.mutation_chance) / (float)0xFFFF), 0, 0xFFFF);

  const float mutationRate = config::mutationRateBase * mut.params.mutation_rate;
  std::normal_distribution dist(0.f, mutationRate);

  // precalculate changes.
  for (size_t i = 0; i < LS_ARRAYSIZE(mut.precalculatedChanges); i++)
    mut.precalculatedChanges[i] = (int8_t)lsClamp((int64_t)lsRound(dist(rnd)), (int64_t)lsMinValue<int8_t>(), (int64_t)lsMaxValue<int8_t>());
}

template <typename T, typename config>
  requires (std::is_integral_v<T>)
inline void mutator_eval(const smart_mutator<config> &m, T &val, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>())
{
  const __m128i a = _mm_load_si128(reinterpret_cast<__m128i *>(m.rndLast));
  const __m128i b = _mm_load_si128(reinterpret_cast<__m128i *>(m.rndLast2));
  const __m128i r = _mm_aesdec_si128(a, b);

  _mm_store_si128(reinterpret_cast<__m128i *>(m.rndLast), b);
  _mm_store_si128(reinterpret_cast<__m128i *>(m.rndLast2), r);

  const uint64_t rand = m.rndLast[1] ^ m.rndLast[0];

  if ((rand & 0xFFFF) > m.chance)
    return;

  const int8_t diff = m.precalculatedChanges[(rand >> 16) % LS_ARRAYSIZE(m.precalculatedChanges)];
  val = (T)lsClamp<int64_t>((int64_t)val + diff, min, max);
}

template <typename config>
inline void mutator_eval(const smart_mutator<config> &m, int16_t *pVal, const size_t count, const int16_t min, const int16_t max)
{
  for (size_t i = 0; i < count; i++)
    mutator_eval(m, pVal[i], min, max);
}

//////////////////////////////////////////////////////////////////////////

struct crossbreeder_naive
{
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
  LS_ALIGN(16) mutable uint64_t rndLast[2];
  LS_ALIGN(16) mutable uint64_t rndLast2[2];
#ifdef _MSC_VER
#pragma warning(pop)
#endif
};

inline void crossbreeder_init(crossbreeder_naive &cb, const size_t scoreParentA, const size_t scoreParentB)
{
  (void)scoreParentA;
  (void)scoreParentB;

  cb.rndLast[0] = lsGetCurrentTimeNs();
  cb.rndLast[1] = __rdtsc();
  cb.rndLast2[0] = ~cb.rndLast[1];
  cb.rndLast2[1] = ~cb.rndLast[0];
}

template <typename T>
  requires (std::is_integral_v<T>)
inline void crossbreeder_eval(const crossbreeder_naive &c, T &val, const T parentA, const T parentB)
{
  (void)c;
  val = lsGetRand() & 1 ? parentA : parentB;
}

template <typename T>
  requires (std::is_integral_v<T>)
inline void crossbreeder_eval(const crossbreeder_naive &c, T *pVal, const size_t count, const T *pParentA, const T *pParentB)
{
  __m128i a = _mm_load_si128(reinterpret_cast<__m128i *>(c.rndLast));
  __m128i b = _mm_load_si128(reinterpret_cast<__m128i *>(c.rndLast2));

  size_t i = 0;
  for (; i + 63 < count; i += 64)
  {
    const __m128i r = _mm_aesdec_si128(a, b); 

    _mm_store_si128(reinterpret_cast<__m128i *>(c.rndLast), b);
    b = r;

    const uint64_t rand = c.rndLast[1] ^ c.rndLast[0];

    for (size_t j = 0; j < 64; j++)
      pVal[i + j] = ((rand >> j) & 1) ? pParentA[i + j] : pParentB[i + j];
  }

  const __m128i r = _mm_aesdec_si128(a, b);

  _mm_store_si128(reinterpret_cast<__m128i *>(c.rndLast), b);
  _mm_store_si128(reinterpret_cast<__m128i *>(c.rndLast2), r);

  uint64_t rand = c.rndLast[1] ^ c.rndLast[0];

  for (; i < count; i++)
  {
    pVal[i] = (rand & 1) ? pParentA[i] : pParentB[i];
    rand >>= 1;
  }
}

//////////////////////////////////////////////////////////////////////////

struct crossbreeder_copy
{
};

inline void crossbreeder_init(crossbreeder_copy &cb, const size_t scoreParentA, const size_t scoreParentB)
{
  (void)cb;
  (void)scoreParentA;
  (void)scoreParentB;
}

template <typename T>
  requires (std::is_integral_v<T>)
inline void crossbreeder_eval(const crossbreeder_copy &c, T &val, const T parentA, const T parentB)
{
  (void)c;
  (void)parentB;
  val = parentA;
}

template <typename T>
  requires (std::is_integral_v<T>)
inline void crossbreeder_eval(const crossbreeder_copy &c, T *pVal, const size_t count, const T *pParentA, const T *pParentB)
{

  (void)c;
  (void)pParentB;
  lsMemcpy(pVal, pParentA, count);
}

//////////////////////////////////////////////////////////////////////////

template <typename target, typename config>
struct evolution
{
  struct gene
  {
    target t;
    size_t score;
  };

  pool<gene> genes;
  small_list<size_t, config::survivingGenes + config::newGenesPerGeneration> bestGeneIndices;
  size_t generationIndex = 0;

  static constexpr bool has_mutator_state = !std::is_same_v<std::nullptr_t, typename config::mutator::state_type>;
  typename config::mutator::state_type mutatorState;

  typedef size_t callback_type(const target &);
};

template <typename target, typename config>
lsResult evolution_init_empty(evolution<target, config> &e)
{
  lsResult result = lsR_Success;

  LS_ERROR_CHECK(pool_reserve(&e.genes, config::survivingGenes + config::newGenesPerGeneration));

  if constexpr (e.has_mutator_state)
    LS_ERROR_CHECK(mutator_state_init(e.mutatorState, config::newGenesPerGeneration));

epilogue:
  return result;
}

template <typename target, typename config>
lsResult evolution_init(evolution<target, config> &e, const target &t, typename evolution<target, config>::callback_type *pEvalFunc)
{
  lsResult result = lsR_Success;

  typename evolution<target, config>::gene g;
  g.t = t;
  g.score = pEvalFunc(t);

  LS_ERROR_CHECK(evolution_init_empty(e));

  size_t index;
  LS_DEBUG_ERROR_ASSERT(pool_add(&e.genes, std::move(g), &index));

  LS_DEBUG_ERROR_ASSERT(list_add(&e.bestGeneIndices, index));
  e.generationIndex++;

epilogue:
  return result;
}

//////////////////////////////////////////////////////////////////////////

template <typename target, typename config>
void evolution_generation_finalize_internal(evolution<target, config> &e)
{
  // Only let the best survive.
  const std::function<int64_t(const size_t &index)> &idxToScore = [&e](const size_t &index) -> int64_t {
    return -(int64_t)pool_get(e.genes, index)->score;
    };

  list_sort<int64_t>(e.bestGeneIndices, idxToScore);

  // Remove everyone from pool, who is lower than best 4 genes
  {
    for (size_t i = config::survivingGenes; i < e.bestGeneIndices.count; i++)
    {
      pool_remove(e.genes, e.bestGeneIndices[i]);

#ifdef _DEBUG
      e.bestGeneIndices[i] = (size_t)-1;
#endif
    }

    e.bestGeneIndices.count = config::survivingGenes;
  }

  e.generationIndex++;
}

template <typename target, typename config, typename func>
void evolution_generation_make_and_eval_baby_internal(evolution<target, config> &e, func evalFunc, typename evolution<target, config>::gene &baby, const size_t maxParentIndex)
{
  // Choose parents
  const typename evolution<target, config>::gene &mama = *pool_get(e.genes, e.bestGeneIndices[lsGetRand() % maxParentIndex]);
  const typename evolution<target, config>::gene &papa = *pool_get(e.genes, e.bestGeneIndices[lsGetRand() % maxParentIndex]);

  typename config::crossbreeder crossbreeder;
  crossbreeder_init(crossbreeder, mama.score, papa.score);
  crossbreed(baby.t, mama.t, papa.t, crossbreeder);

  typename config::mutator mutator;
  mutator_init(mutator, e.generationIndex, e.mutatorState);
  mutate(baby.t, mutator);

  baby.score = evalFunc(baby.t);
}

//////////////////////////////////////////////////////////////////////////

template <typename target, typename config, typename func>
void evolution_generation(evolution<target, config> &e, func evalFunc)
{
  lsAssert(e.genes.count <= config::survivingGenes);
  lsAssert(e.genes.count > 0);

  const size_t maxParentIndex = e.genes.count;
  size_t bestScore = 0;
  size_t bestScoreMutatorIdx = (size_t)-1;

  if constexpr (e.has_mutator_state)
  {
    mutator_state_next_generation(e.mutatorState);
    bestScore = pool_get(e.genes, e.bestGeneIndices[0])->score;
  }

  for (size_t i = 0; i < config::newGenesPerGeneration; i++)
  {
    typename evolution<target, config>::gene baby;
    evolution_generation_make_and_eval_baby_internal(e, evalFunc, baby, maxParentIndex);

    if constexpr (e.has_mutator_state)
    {
      if (baby.score > bestScore)
      {
        bestScore = baby.score;
        bestScoreMutatorIdx = i;
      }
    }

    // Add Baby to pool and bestGeneIndices
    size_t babyIndex;
    LS_DEBUG_ERROR_ASSERT(pool_add(&e.genes, std::move(baby), &babyIndex));
    LS_DEBUG_ERROR_ASSERT(list_add(&e.bestGeneIndices, babyIndex));
  }

  if constexpr (e.has_mutator_state)
  {
    if (bestScoreMutatorIdx != (size_t)-1)
      mutator_state_select(e.mutatorState, bestScoreMutatorIdx);
  }

  evolution_generation_finalize_internal(e);
}

template <typename target, typename config, typename func>
void evolution_generation(evolution<target, config> &e, func evalFunc, thread_pool *pThreads)
{
  static_assert(!e.has_mutator_state);

  lsAssert(e.genes.count <= config::survivingGenes);
  lsAssert(e.genes.count > 0);

  const size_t maxParentIndex = e.genes.count;

  for (size_t i = 0; i < config::newGenesPerGeneration; i++)
  {
    typename evolution<target, config>::gene uninitialized_baby;
    size_t babyIndex;
    LS_DEBUG_ERROR_ASSERT(pool_add(&e.genes, std::move(uninitialized_baby), &babyIndex));
    LS_DEBUG_ERROR_ASSERT(list_add(&e.bestGeneIndices, babyIndex));

    const auto &eval = [=, &e]()
      {
        typename evolution<target, config>::gene &baby = *pool_get(e.genes, babyIndex);

        // Should be fine to be used without mutexes in a multithreaded context, as the pool should never realloc anyways, as we've reserved the amount that will *EVER* be needed in advance.
        evolution_generation_make_and_eval_baby_internal(e, evalFunc, baby, maxParentIndex);
      };

    thread_pool_add(pThreads, eval);
  }

  thread_pool_await(pThreads);

  evolution_generation_finalize_internal(e);
}

template <typename target, typename config>
void evolution_get_best(const evolution<target, config> &e, const target **ppTarget, size_t &best_score)
{
  lsAssert(e.genes.count > 0);

  const typename evolution<target, config>::gene *pBestGene = pool_get(&e.genes, e.bestGeneIndices[0]);

  *ppTarget = &pBestGene->t;
  best_score = pBestGene->score;
}

template <typename target, typename config>
void evolution_get_at(evolution<target, config> &e, const size_t at, size_t &index, size_t &score)
{
  lsAssert(at < e.bestGeneIndices.count);

  index = *list_get(&e.bestGeneIndices, at);
  score = pool_get(e.genes, index)->score;
}

template <typename target, typename config>
size_t evolution_get_count(evolution<target, config> &e)
{
  return e.bestGeneIndices.count;
}

template <typename target, typename config, typename func>
void evolution_reevaluate(evolution<target, config> &e, func evalFunc)
{
  for (auto g : e.genes)
    g.pItem->score = evalFunc(g.pItem->t);

  const std::function<int64_t(const size_t &index)> &idxToScore = [&e](const size_t &index) -> int64_t {
    return -(int64_t)pool_get(e.genes, index)->score;
    };

  list_sort<int64_t>(e.bestGeneIndices, idxToScore);
}

template <typename target, typename config, typename func>
void evolution_for_each(evolution<target, config> &e, func f)
{
  for (auto g : e.genes)
    f(g.pItem->t);
}

// Only use this if you know what you are doing! Target must always be evaluated afterwards!
template <typename target, typename config>
lsResult evolution_add_unevaluated_target(evolution<target, config> &e, target &&t)
{
  lsResult result = lsR_Success;

  typename evolution<target, config>::gene g;
  g.t = std::move(t);

  size_t idx;
  LS_ERROR_CHECK(pool_add(&e.genes, std::move(g), &idx));
  LS_ERROR_CHECK(list_add(&e.bestGeneIndices, idx));

epilogue:
  return result;
}

// Only use this if you know what you are doing! Target must always be evaluated afterwards!
template <typename target, typename config>
lsResult evolution_add_unevaluated_target(evolution<target, config> &e, const target &t)
{
  lsResult result = lsR_Success;

  typename evolution<target, config>::gene g;
  g.t = t;

  size_t idx;
  LS_ERROR_CHECK(pool_add(&e.genes, std::move(g), &idx));
  LS_ERROR_CHECK(list_add(&e.bestGeneIndices, idx));

epilogue:
  return result;
}

template <typename target, typename config>
void evolution_clear(evolution<target, config> &e)
{
  pool_clear(&e.genes);
  list_clear(&e.bestGeneIndices);
}
