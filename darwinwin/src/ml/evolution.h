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

struct example_config
{
  using mutator = mutator_proto;
  using crossbreeder = crossbreeder_proto;

  static constexpr size_t survivingGenes = 4;
  static constexpr size_t newGenesPerGeneration = 4;
};

template <typename target, typename config>
struct evolution
{
  struct gene
  {
    target t;
    size_t score;
    bool hasScore = false;
  };

  pool<gene> genes;
  size_t bestGeneIndices[config::survivingGenes];
  size_t generationIndex = 0;

  typedef size_t callback_type(const target &);
};

template <typename target, typename config>
void evolution_init(evolution<target, config> &e, const target &t);

template <typename target, typename config>
void evolution_generation(evolution<target, config> &e, typename evolution<target, config>::callback_type *pEvalFunc)
{
  // ...

  target t;

  typename config::crossbreeder crossbreeder;
  crossbreeder_init(crossbreeder, mama_score, papa_score);
  crossbreed(t, mama, papa, crossbreeder);

  typename config::mutator mutator;
  mutator_init(mutator, e.generationIndex);
  mutate(t, mutator);

  const size_t score = (*pEvalFunc)(t);

  // ...
}

template <typename target, typename config>
void evolution_get_best(const evolution<target, config> &e, target **ppTarget, size_t &best_score);
