#pragma once
#include "core.h"
#include "darwinwin.h"

void level_generateDefault(level *pLvl);

//////////////////////////////////////////////////////////////////////////

lsResult actor_saveBrain(const char *dir, const actor &actr);
lsResult actor_loadNewestBrain(const char *dir, actor &actr);
lsResult actor_loadBrainFromFile(const char *filename, actor &actr);

//////////////////////////////////////////////////////////////////////////

lsResult train_loop(struct thread_pool *pThreadPool, const char *dir);
lsResult train_loopIndependentEvolution(struct thread_pool *pThreadPool, const char *dir);
