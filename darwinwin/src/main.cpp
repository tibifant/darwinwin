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

crow::response handle_getLevel(const crow::request &req);
crow::response handle_setTile(const crow::request &req);

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

  crow::App<crow::CORSHandler> app;

  auto &cors = app.get_middleware<crow::CORSHandler>();
#ifndef DARWINWIN_LOCALHOST
  cors.global().origin(DARWINWIN_HOSTNAME);
#else
  cors.global().origin("*");
#endif

  CROW_ROUTE(app, "/getLevel").methods(crow::HTTPMethod::POST)([](const crow::request &req) { return handle_setTile(req); });

  app.port(21110).multithreaded().run();

  // TODO: /get_level
  // TODO: /set_tile (x, y, value)
  // TODO: /manual_act (action_id)

  _IsRunning = false;
}

//////////////////////////////////////////////////////////////////////////

crow::response handle_getLevel(const crow::request &req)
{
  auto body = crow::json::load(req.body);

  if (!body)
    return crow::response(crow::status::BAD_REQUEST);

  crow::json::wvalue ret;

  ret["width"] = _WebLevel.width;
  ret["height"] = _WebLevel.height;

  for (size_t i = 0; i < LS_ARRAYSIZE(_WebLevel.grid); i++)
    ret["grid"][i] = _WebLevel.grid[i];

  return ret;
}

crow::response handle_setTile(const crow::request &req)
{
  auto body = crow::json::load(req.body);

  if (!body || body.has("x") || body.has("y") || body.has("value"))
    return crow::response(crow::status::BAD_REQUEST);

  const size_t x = body["x"].i();
  const size_t y = body["y"].i();
  const uint8_t val = (uint8_t)body["value"].i();

  if (x >= level::width || y >= level::height)
    return crow::response(crow::status::BAD_REQUEST);

  // TODO: when actually doing something with the level: thredlock?
  _WebLevel.grid[y * level::width + x] = (tileFlag)val;

  return crow::response(crow::status::NOT_IMPLEMENTED);
}

