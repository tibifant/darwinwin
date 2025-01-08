#pragma once

#include "core.h"
#include "pool.h"
#include "small_list.h"

struct mutator_naive
{
};

inline void mutator_init(mutator_naive &mut, const size_t generation)
{
  (void)mut;
  (void)generation;
}

template <typename T>
  requires (std::is_integral_v<T>)
inline void mutator_eval(const mutator_naive &m, T &val, const T min = lsMinValue<T>(), const T max = lsMaxValue<T>()) // probably worth using a larger integer type than provided internally, to prevent integer overflows.
{
  (void)m;
  val = (T)lsClamp<int64_t>(val + (int64_t)(lsGetRand() % 5) - 2, min, max);
}

struct crossbreeder_naive
{
};

inline void crossbreeder_init(crossbreeder_naive &cb, const size_t scoreParentA, const size_t scoreParentB)
{
  (void)cb;
  (void)scoreParentA;
  (void)scoreParentB;
}

template <typename T>
  requires (std::is_integral_v<T>)
inline void crossbreeder_eval(const crossbreeder_naive &c, T &val, const T parentA, const T parentB)
{
  (void)c;
  val = lsGetRand() & 1 ? parentA : parentB;
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
  small_list<size_t, config::survivingGenes + config::newGenesPerGeneration> bestGeneIndices;
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
  LS_DEBUG_ERROR_ASSERT(pool_add(&e.genes, std::move(g), &index));

  LS_DEBUG_ERROR_ASSERT(list_add(&e.bestGeneIndices, index));
  e.generationIndex++;

epilogue:
  return result;
}

template <typename target, typename config>
void evolution_generation(evolution<target, config> &e, typename evolution<target, config>::callback_type *pEvalFunc)
{
  lsAssert(e.genes.count <= config::survivingGenes);
  lsAssert(e.genes.count > 0);

  const size_t maxParentIndex = e.genes.count;

  for (size_t i = 0; i < config::newGenesPerGeneration; i++)
  {
    // Choose parents
    const typename evolution<target, config>::gene &mama = *pool_get(e.genes, e.bestGeneIndices[lsGetRand() % maxParentIndex]);
    const typename evolution<target, config>::gene &papa = *pool_get(e.genes, e.bestGeneIndices[lsGetRand() % maxParentIndex]);

    typename evolution<target, config>::gene baby;

    typename config::crossbreeder crossbreeder;
    crossbreeder_init(crossbreeder, mama.score, papa.score);
    crossbreed(baby.t, mama.t, papa.t, crossbreeder);

    typename config::mutator mutator;
    mutator_init(mutator, e.generationIndex);
    mutate(baby.t, mutator);

    // Add Baby to pool and bestGeneIndices
    baby.score = (*pEvalFunc)(baby.t);

    size_t babyIndex;
    LS_DEBUG_ERROR_ASSERT(pool_add(&e.genes, std::move(baby), &babyIndex));
    LS_DEBUG_ERROR_ASSERT(list_add(&e.bestGeneIndices, babyIndex));
  }

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

template <typename target, typename config>
void evolution_get_best(const evolution<target, config> &e, const target **ppTarget, size_t &best_score)
{
  lsAssert(e.genes.count > 0);

  const typename evolution<target, config>::gene *pBestGene = pool_get(&e.genes, e.bestGeneIndices[0]);

  *ppTarget = &pBestGene->t;
  best_score = pBestGene->score;
}
