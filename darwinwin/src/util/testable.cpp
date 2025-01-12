#include "testable.h"

#include <map>
#include <vector>

static std::map<std::string, testable_func> *_pTests;

_testable_init register_testable(const char *name, testable_func func)
{
  static bool initialized = false;

  if (!initialized)
  {
    initialized = true;
    _pTests = new std::map<std::string, testable_func>();
  }

  _pTests->insert(std::make_pair(name, func));

  return { _pTests->size() };
}

lsResult run_testables()
{
  register_testable_files<testable_file_count>();

  lsResult result = lsR_Success;

  size_t failed = 0;
  size_t succeeded = 0;
  std::vector<const char *> failedTests;

  if (_pTests == nullptr)
  {
    print_error_line("No tests discovered.");
    goto epilogue;
  }

  print(_pTests->size(), " test(s) discovered.\n\n");

  for (const auto &_item : *_pTests)
  {
    lsSetConsoleColor(lsCC_DarkGray, lsCC_Black);
    print("[RUNNING] ");
    lsResetConsoleColor();
    print(_item.first.c_str(), "\n");

    lsErrorPushSilentImpl _silent;

    const int64_t before = lsGetCurrentTimeNs();

    const lsResult r = _item.second();

    const int64_t after = lsGetCurrentTimeNs();

    if (LS_FAILED(r))
    {
      failed++;
      result = lsR_Failure;
      lsSetConsoleColor(lsCC_BrightRed, lsCC_Black);
      print("[XFAILED] ", _item.first.c_str(), " (in ", FF(Max(5))((after - before) * 1e-6f), " ms)\n");
      lsResetConsoleColor();
      failedTests.push_back(_item.first.c_str());
    }
    else
    {
      succeeded++;
      lsSetConsoleColor(lsCC_BrightGreen, lsCC_Black);
      print("[SUCCESS] ");
      lsResetConsoleColor();
      print(_item.first.c_str());
      lsSetConsoleColor(lsCC_DarkGray, lsCC_Black);
      print(" (in ", FF(Max(5))((after - before) * 1e-6f), " ms)\n");
      lsResetConsoleColor();
    }
  }

  print("===========================================\n");

  if (failed > 0)
  {
    lsSetConsoleColor(lsCC_BrightRed, lsCC_Black);
    print(failed, " / ", _pTests->size(), " (", FD(Max(4))(failed / (double_t)_pTests->size() * 100.0), " %) Test(s) failed.\n");
    for (const char *name : failedTests)
      print("[XFAILED] ", name, "\n");
    lsResetConsoleColor();
  }
  else
  {
    lsSetConsoleColor(lsCC_BrightGreen, lsCC_Black);
    print("All ", succeeded, " / ", _pTests->size(), " Test(s) succeeded.\n");
    lsResetConsoleColor();
  }

  print("===========================================\n");

  goto epilogue;
epilogue:
  return result;
}
