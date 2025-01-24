#pragma once
#include "core.h"
#include "darwinwin.h"

void level_generateDefault(level *pLvl);

//////////////////////////////////////////////////////////////////////////

lsResult train_loop(struct thread_pool *pThreadPool, const char *dir);
lsResult train_loopIndependentEvolution(struct thread_pool *pThreadPool, const char *dir);
