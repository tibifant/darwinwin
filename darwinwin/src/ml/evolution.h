#pragma once

#include "core.h"
#include "pool.h"

struct mutator_proto
{
};

inline void mutator_init(mutator_proto &proto, const size_t generation);

template <typename T>
  requires (std::is_integral_v<T>)
inline void mutator_eval(const mutator_proto &m, T &val, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>()); // probably worth using a larger integer type than provided internally, to prevent integer overflows.

struct crossbreeder_proto
{
};

template <typename T>
  requires (std::is_integral_v<T>)
inline void crossbreeder_eval(crossbreeder_proto &c, T &val, const T parentA, const T parentB);

struct target_proto
{
  uint8_t value;
};

template <typename mutator>
void mutate(target_proto &target, const mutator &m)
{
  mutator_eval(m, target.value);
}

template <typename target, typename config>
struct evolution
{
  struct gene
  {
    target t;
    size_t score;
  };

  pool<gene> genes;
  size_t bestGeneIndices[config::survivingGenes + config::newGenesPerGeneration];
  size_t generationIndex = 0;

  typedef size_t callback_type(const target &);
};

template <typename target, typename config>
lsResult evolution_init(evolution<target, config> &e, const target &t, typename evolution<target, config>::callback_type *pEvalFunc)
{
  lsResult result = lsR_Success;

  typename evolution<target, config>::gene g;
  g.t = t;
  g.score = pEvalFunc(t);

  LS_ERROR_CHECK(pool_reserve(&e.genes, config::survivingGenes + config::newGenesPerGeneration));

  size_t index;
  LS_DEBUG_ERROR_ASSERT(pool_add(&e.genes, std::move(g), &_unused));

  e.bestGeneIndices[0] = index;
  e.generationIndex++;

  epilogue:
  return result;
}

template <typename target, typename config>
void evolution_generation(evolution<target, config> &e, typename evolution<target, config>::callback_type *pEvalFunc)
{
  lsAssert(e.genes.count < config::survivingGenes);
  lsAssert(e.genes.count > 0);

  const size_t maxParentIndex = e.genes.count;

  for (size_t i = 0; i < config::newGenesPerGeneration; i++)
  {
    // Choose parents
    const typename evolution<target, config>::gene &mama = *pool_get(e.genes, e.bestGeneIndices[config::survivingGenes % maxParentIndex]);
    const typename evolution<target, config>::gene &papa = *pool_get(e.genes, e.bestGeneIndices[config::survivingGenes % maxParentIndex]);

    typename evolution<target, config>::gene baby;

    typename config::crossbreeder crossbreeder;
    crossbreeder_init(crossbreeder, mama.score, papa.score);
    crossbreed(baby.t, mama.t, papa.t, crossbreeder);

    typename config::mutator mutator;
    mutator_init(mutator, e.generationIndex);
    mutate(baby.t, mutator);

    // Add Baby to pool and bestGeneIndices
    baby.score = (*pEvalFunc)(baby.t);
    size_t *pBabyIndex;

    LS_DEBUG_ERROR_ASSERT(pool_add(&e.genes, baby, pBabyIndex));
    e.bestGeneIndices[e.genes.count - 1] = *pBabyIndex;
  }

  // Only let the best survive.
  std::sort(e.bestGeneIndices, e.bestGeneIndices + e.genes.count, [&e](const size_t a, const size_t b)
    {
      return pool_get(e.genes, e.bestGeneIndices[a]).score > pool_get(e.genes, e.bestGeneIndices[b]).score;
    });

  // Remove everyone from pool, who is lower than best 4 genes
  {
    const size_t count = e.genes.count;

    for (size_t i = config::survivingGenes; i < count; i++)
      pool_remove(&e.genes, e.bestGeneIndices[i]);
  }

  generationIndex++;
}

template <typename target, typename config>
void evolution_get_best(const evolution<target, config> &e, const target **ppTarget, size_t &best_score);

size_t eval_func(const vec2i8 &val)
{
  return 1000 - ((int64_t)val.x - 40) - ((int64_t)val.y - 40);
}

struct find_high_config
{
  using mutator = mutator_proto;
  using crossbreeder = crossbreeder_proto;

  static constexpr size_t survivingGenes = 4;
  static constexpr size_t newGenesPerGeneration = 4;
};

void find_high_in_abs_func()
{
  const vec2i8 startPos(10, 10);

  // Call evolution until we found the hightest point

  evolution<vec2i8, find_high_config> evolver;

  evolution_init(evolver, startPos, eval_func);

  for (size_t i = 0; i < 100; i++)
    evolution_generation(evolver, eval_func);

  vec2i8 *pBestValue = nullptr;
  size_t bestScore;

  evolution_get_best(evolver, &pBestValue, bestScore);
}
