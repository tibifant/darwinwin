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

#include "core.h"
#include "darwinwin.h"
#include "io.h"

//////////////////////////////////////////////////////////////////////////

static std::atomic<bool> _IsRunning = true;
static level _WebLevel;
static actor _WebActor(vec2u8(level::width / 2, level::height / 2), ld_up);

//////////////////////////////////////////////////////////////////////////

int32_t main(const int32_t argc, const char **pArgv)
{
  (void)pArgv;

  if (argc > 1)
  {
    print_error_line("Unsupported Argument!");
    return EXIT_FAILURE;
  }

  cpu_info::DetectCpuFeatures();

  if (!(cpu_info::avx2Supported && cpu_info::avxSupported))
  {
    print_error_line("CPU Platform does not provide support for AVX/AVX2!");
    return EXIT_FAILURE;
  }

  sformatState_ResetCulture();
  print("DarWinWin (built " __DATE__ " " __TIME__ ") running on ", cpu_info::GetCpuName(), ".\n");
  print("\nConfiguration:\n");
  print("Level size: ", FF(Group, Frac(3), AllFrac)(sizeof(level) / 1024.0), " KiB\n");
  print("Actor size: ", FF(Group, Frac(3), AllFrac)(sizeof(actor) / 1024.0), " KiB\n");
  print("\n");

  level lvl;
  actor actr(vec2u8(level::width / 2, level::height / 2), ld_up);
  actorStats stats;

  level_initLinear(&lvl);
  level_print(lvl);

  viewCone cone = viewCone_get(lvl, actr);
  viewCone_print(cone, actr);

  actor_move(&actr, &stats, lvl);
  actor_turnAround(&actr, ld_left);
  cone = viewCone_get(lvl, actr);
  viewCone_print(cone, actr);

  crow::App<crow::CORSHandler> app;

  auto &cors = app.get_middleware<crow::CORSHandler>();
#ifndef DARWINWIN_LOCALHOST
  cors.global().origin(DARWINWIN_HOSTNAME);
#else
  cors.global().origin("*");
#endif

  app.port(21110).multithreaded().run();

  // TODO: /get_level
  // TODO: /set_tile (x, y, value)
  // TODO: /manual_act (action_id)

  _IsRunning = false;
}

//////////////////////////////////////////////////////////////////////////

