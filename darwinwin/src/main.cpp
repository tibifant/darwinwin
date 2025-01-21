#include <stdio.h>
#include <exception>

#define ASIO_STANDALONE 1
#define ASIO_NO_EXCEPTIONS 1

#define DARWINWIN_LOCALHOST
#define DARWINWIN_HOSTNAME "https://hostname_not_configured"

namespace asio
{
  namespace detail
  {
    template <typename Exception>
    void throw_exception(const Exception &e)
    {
#ifdef _MSC_VER
      __debugbreak(); // windows only, sorry!
#endif
      printf("Exception thrown: %s.\n", e.what());
    }
  }
}

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4702) // unreachable (somewhere in json.h)
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (push, 0)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
#include "crow.h"
#include "crow/middlewares/cors.h"
#ifdef _MSC_VER
#pragma warning (pop)
#else
#pragma GCC diagnostic pop
#endif

//////////////////////////////////////////////////////////////////////////

#include "darwinwin.h"
#include "testable.h"
#include "thread_pool.h"
#include "io.h"

//////////////////////////////////////////////////////////////////////////

crow::response handle_getLevel(const crow::request &req);
crow::response handle_setTile(const crow::request &req);
crow::response handle_manualAct(const crow::request &req);
crow::response handle_aiStep(const crow::request &req);
crow::response handle_levelGenerate(const crow::request &req);
crow::response handle_startTraining();
crow::response handle_stopTraining();
crow::response handle_isTraining();
crow::response handle_loadTrainingLevel();
crow::response handle_loadTrainingActor();
crow::response handle_resetStats();

//////////////////////////////////////////////////////////////////////////

static bool parse_args(const char **pArgs, const ptrdiff_t count);
static void print_args();

//////////////////////////////////////////////////////////////////////////

static std::atomic<bool> _IsRunning = true;
static level _WebLevel;
static actor _WebActor(vec2u8(level::width / 2, level::height / 2), ld_up);
static std::thread *_pTrainingThread = nullptr;
static thread_pool *_pThreadPool = nullptr;
static const char *_TrainingDirectory = "training/";

static struct {
  bool runTests = true;
  bool runServer = true;
} _Args;

//////////////////////////////////////////////////////////////////////////
#include "level_generator.h"

int32_t main(const int32_t argc, const char **pArgv)
{
  sformatState_ResetCulture();

  cpu_info::DetectCpuFeatures();

  if (!(cpu_info::avx2Supported && cpu_info::avxSupported && cpu_info::aesNiSupported))
  {
    print_error_line("CPU Platform does not provide support for AVX/AVX2/AES-NI!");
    return EXIT_FAILURE;
  }

  if (!parse_args(pArgv + 1, argc - 1))
    return EXIT_FAILURE;

  // Set Working Directory.
  do
  {
    wchar_t filePath[MAX_PATH];
    GetModuleFileNameW(nullptr, filePath, sizeof(filePath) / sizeof(wchar_t));

    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
      print_error_line("Insufficient Buffer for Module File Name Retrieval.");
      break;
    }

    wchar_t *lastSlash = nullptr;

    for (size_t i = 0; i < sizeof(filePath) / sizeof(wchar_t) && filePath[i] != L'\0'; i++)
      if (filePath[i] == L'\\')
        lastSlash = filePath + i;

    if (lastSlash != nullptr)
      *lastSlash = L'\0';

    if (0 == SetCurrentDirectoryW(filePath))
      print_error_line("Failed to set working directory.");

  } while (false);

  print("DarWinWin (built " __DATE__ " " __TIME__ ") running on ", cpu_info::GetCpuName(), ".\n");
  print("\nConfiguration:\n");
  print("Level size: ", FF(Group, Frac(3), AllFrac)(sizeof(level) / 1024.0), " KiB\n");
  print("Actor size: ", FF(Group, Frac(3), AllFrac)(sizeof(actor) / 1024.0), " KiB\n");
  print("\n");

  if (_Args.runTests)
  {
    print("Running tests...\n");
    run_testables();
    print("\n");
  }

  if (_Args.runServer)
  {
    if (!lsFileExists(_TrainingDirectory))
      if (LS_FAILED(lsCreateDirectory(_TrainingDirectory)))
        print_error_line("Failed to create training directory at '", _TrainingDirectory, "'.");

    _pThreadPool = thread_pool_new(lsMax(1, thread_pool_max_threads()));

    if (!_pThreadPool)
      print_error_line("Failed to initialize thread pool.");

    crow::App<crow::CORSHandler> app;

    level_generateDefault(&_WebLevel);
    actor_initStats(&_WebActor);

    print_error_line("WARNING: STARTING TRAINING!");
    handle_startTraining();

    auto &cors = app.get_middleware<crow::CORSHandler>();
#ifndef DARWINWIN_LOCALHOST
    cors.global().origin(DARWINWIN_HOSTNAME);
#else
    cors.global().origin("*");
#endif

    CROW_ROUTE(app, "/getLevel").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_getLevel(req); });
    CROW_ROUTE(app, "/setTile").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_setTile(req); });
    CROW_ROUTE(app, "/manualAct").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_manualAct(req); });
    CROW_ROUTE(app, "/ai_step").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_aiStep(req); });
    CROW_ROUTE(app, "/level_generate").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_levelGenerate(req); });
    CROW_ROUTE(app, "/start_training").methods(crow::HTTPMethod::POST)([](const crow::request &) { return handle_startTraining(); });
    CROW_ROUTE(app, "/stop_training").methods(crow::HTTPMethod::POST)([](const crow::request &) { return handle_stopTraining(); });
    CROW_ROUTE(app, "/is_training").methods(crow::HTTPMethod::POST)([](const crow::request &) { return handle_isTraining(); });
    CROW_ROUTE(app, "/load_training_level").methods(crow::HTTPMethod::POST)([](const crow::request &) { return handle_loadTrainingLevel(); });
    CROW_ROUTE(app, "/load_training_actor").methods(crow::HTTPMethod::POST)([](const crow::request &) { return handle_loadTrainingActor(); });
    CROW_ROUTE(app, "/reset_stats").methods(crow::HTTPMethod::POST)([](const crow::request &) { return handle_resetStats(); });

    app.port(21110).multithreaded().run();

    _IsRunning = false;
  }

  return EXIT_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

crow::response handle_getLevel(const crow::request &req)
{
  auto body = crow::json::load(req.body);

  if (!body)
    return crow::response(crow::status::BAD_REQUEST);

  crow::json::wvalue ret;

  ret["level"]["width"] = _WebLevel.width;
  ret["level"]["height"] = _WebLevel.height;

  for (size_t i = 0; i < LS_ARRAYSIZE(_WebLevel.grid); i++)
  {
    auto &item = ret["level"]["grid"][(uint32_t)i];
    item = _WebLevel.grid[i];
  }

  ret["actor"][0]["posX"] = _WebActor.pos.x;
  ret["actor"][0]["posY"] = _WebActor.pos.y;
  ret["actor"][0]["lookDir"] = _WebActor.look_dir;

  for (size_t i = 0; i < _actorStats_Count; i++)
  {
    auto &item = ret["actor"][0]["stats"][(uint32_t)i];
    item = _WebActor.stats[i];
  }

  viewCone cone = viewCone_get(_WebLevel, _WebActor);

  for (size_t i = 0; i < _viewConePosition_Count; i++)
  {
    auto &item = ret["actor"][0]["viewcone"][(uint32_t)i];
    item = cone[(viewConePosition)i];
  }

  return ret;
}

crow::response handle_setTile(const crow::request &req)
{
  auto body = crow::json::load(req.body);

  if (!body || !body.has("x") || !body.has("y") || !body.has("value"))
    return crow::response(crow::status::BAD_REQUEST);

  const size_t x = body["x"].i();
  const size_t y = body["y"].i();
  const uint8_t val = (uint8_t)body["value"].i();

  if (x >= level::width || y >= level::height)
    return crow::response(crow::status::BAD_REQUEST);

  _WebLevel.grid[y * level::width + x] = (tileFlag)val;

  return crow::response(crow::status::OK);
}

crow::response handle_manualAct(const crow::request &req)
{
  auto body = crow::json::load(req.body);

  if (!body || !body.has("actionId"))
    return crow::response(crow::status::BAD_REQUEST);

  const uint8_t id = (uint8_t)body["actionId"].i();

  if (id > _actorAction_Count)
    return crow::response(crow::status::BAD_REQUEST);

  viewCone cone = viewCone_get(_WebLevel, _WebActor);
  actor_updateStats(&_WebActor, cone);
  actor_act(&_WebActor, &_WebLevel, cone, actorAction(id));

  return crow::response(crow::status::OK);
}

crow::response handle_aiStep(const crow::request &)
{
  level_performStep(_WebLevel, &_WebActor, 1);

  return crow::response(crow::status::OK);
}

crow::response handle_levelGenerate(const crow::request &)
{
  level_generateDefault(&_WebLevel);

  return crow::response(crow::status::OK);
}

crow::response handle_startTraining()
{
  if (_TrainingRunning)
    return crow::response(crow::status::CONFLICT);

  if (_pTrainingThread)
  {
    if (_pTrainingThread->joinable())
      _pTrainingThread->join();

    delete _pTrainingThread;
    _pTrainingThread = nullptr;
  }

  _DoTraining = true;
  _TrainingRunning = true;
  _pTrainingThread = new std::thread(train_loop, _pThreadPool, _TrainingDirectory);

  return crow::response(crow::status::OK);
}

crow::response handle_stopTraining()
{
  _DoTraining = false;

  return crow::response(crow::status::OK);
}

crow::response handle_isTraining()
{
  if (_pTrainingThread == nullptr)
    return crow::json::wvalue({ "result", false });

  if (_TrainingRunning)
    return crow::json::wvalue({ "result", true });

  return crow::json::wvalue({ "result", false });
}

crow::response handle_loadTrainingLevel()
{
  _WebLevel = _CurrentLevel;

  return crow::response(crow::status::OK);
}

crow::response handle_loadTrainingActor()
{
  actor_loadNewestBrain(_TrainingDirectory, _WebActor);

  return crow::response(crow::status::OK);
}

crow::response handle_resetStats()
{
  actor_initStats(&_WebActor);

  return crow::response(crow::status::OK);
}

//////////////////////////////////////////////////////////////////////////

static const char _ArgNoServer[] = "--no-server";
static const char _ArgNoTest[] = "--no-test";
static const char _ArgTestOnly[] = "--test-only";

static bool parse_args(const char **pArgs, const ptrdiff_t count)
{
  ptrdiff_t argsRemaining = count;

  while (argsRemaining > 0)
  {
    if (lsStringEquals(_ArgNoServer, *pArgs))
    {
      _Args.runServer = false;
      argsRemaining--;
      pArgs++;
    }
    else if (lsStringEquals(_ArgNoTest, *pArgs))
    {
      _Args.runTests = false;
      argsRemaining--;
      pArgs++;
    }
    else if (lsStringEquals(_ArgTestOnly, *pArgs))
    {
      _Args.runServer = false;
      argsRemaining--;
      pArgs++;
    }
    else
    {
      print_error_line("Invalid Parameter '", *pArgs, "'. Aborting.");
      print_args();
      return false;
    }
  }

  return true;
}

void print_args()
{
  print("Usage: \n");
  print("\t", FS(_ArgNoServer, Min(12)), ": Disable running Webserver.\n");
  print("\t", FS(_ArgNoTest, Min(12)), ": Disable running Unit-Tests.\n");
  print("\t", FS(_ArgTestOnly, Min(12)), ": Disable running everything except Unit-Tests (for CI).\n");
}
