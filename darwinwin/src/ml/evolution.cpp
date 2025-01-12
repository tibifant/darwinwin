#include "evolution.h"
#include "testable.h"

REGISTER_TESTABLE_FILE(1);

template <typename crossbreeder>
void crossbreed(vec2i8 &val, const vec2i8 parentA, const vec2i8 parentB, const crossbreeder &c)
{
  crossbreeder_eval(c, val.x, parentA.x, parentB.x);
  crossbreeder_eval(c, val.y, parentA.y, parentB.y);
}

template <typename mutator>
void mutate(vec2i8 &target, const mutator &m)
{
  mutator_eval(m, target.x);
  mutator_eval(m, target.y);
}

struct test_config
{
  using mutator = mutator_naive;
  using crossbreeder = crossbreeder_naive;

  static constexpr size_t survivingGenes = 4;
  static constexpr size_t newGenesPerGeneration = 16;
};

size_t test_eval_func(const vec2i8 &val)
{
  return 1000 - lsAbs((int64_t)val.x) - lsAbs((int64_t)val.y);
}

DEFINE_TESTABLE(evolution_basic_test)
{
  lsResult result = lsR_Success;

  {
    const vec2i8 startPos(121, -72);
    evolution<vec2i8, test_config> evolver;

    evolution_init(evolver, startPos, test_eval_func);

    size_t prevBestScore = 0;

    for (size_t i = 0; i < 100; i++)
    {
      evolution_generation(evolver, test_eval_func);

      const vec2i8 *pBestValue = nullptr;
      size_t bestScore;

      evolution_get_best(evolver, &pBestValue, bestScore);

      TESTABLE_ASSERT_TRUE(prevBestScore <= bestScore);
      TESTABLE_ASSERT_NOT_EQUAL(pBestValue, nullptr);
      prevBestScore = bestScore;
    }

    const vec2i8 *pBestValue = nullptr;
    size_t bestScore;

    evolution_get_best(evolver, &pBestValue, bestScore);

    print("best: ", *pBestValue, " with score ", bestScore, "\n");

    TESTABLE_ASSERT_TRUE(test_eval_func(startPos) <= bestScore);
  }

epilogue:
  return result;
}

DEFINE_TESTABLE(evolution_mt_test)
{
  lsResult result = lsR_Success;

  const size_t maxThreads = thread_pool_max_threads();
  thread_pool *pThreadPool = nullptr;
  // thread_pool_new(maxThreads);

  print("max thread count: ", maxThreads, "\n");

  if (pThreadPool)
  {
    const vec2i8 startPos(121, -72);
    evolution<vec2i8, test_config> evolver;

    evolution_init(evolver, startPos, test_eval_func);

    size_t prevBestScore = 0;

    for (size_t i = 0; i < 100; i++)
    {
      evolution_generation(evolver, test_eval_func, pThreadPool);

      const vec2i8 *pBestValue = nullptr;
      size_t bestScore;

      evolution_get_best(evolver, &pBestValue, bestScore);

      TESTABLE_ASSERT_TRUE(prevBestScore <= bestScore);
      TESTABLE_ASSERT_NOT_EQUAL(pBestValue, nullptr);
      prevBestScore = bestScore;
    }

    const vec2i8 *pBestValue = nullptr;
    size_t bestScore;

    evolution_get_best(evolver, &pBestValue, bestScore);

    print("best: ", *pBestValue, " with score ", bestScore, "\n");

    TESTABLE_ASSERT_TRUE(test_eval_func(startPos) <= bestScore);
  }

epilogue:
  thread_pool_destroy(&pThreadPool);
  return result;
}
