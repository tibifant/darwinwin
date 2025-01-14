#include "io.h"

#include "shlwapi.h"

//////////////////////////////////////////////////////////////////////////

lsResult lsReadFileBytes(const char *filename, uint8_t **ppData, const size_t elementSize, size_t *pCount)
{
  lsResult result = lsR_Success;

  FILE *pFile = nullptr;
  size_t length, readLength;

  LS_ERROR_IF(filename == nullptr || ppData == nullptr || pCount == nullptr, lsR_ArgumentNull);

  pFile = fopen(filename, "rb");

  LS_ERROR_IF(pFile == nullptr, lsR_ResourceNotFound);

  LS_ERROR_IF(0 != fseek(pFile, 0, SEEK_END), lsR_IOFailure);

  length = ftell(pFile);

  LS_ERROR_IF(0 != fseek(pFile, 0, SEEK_SET), lsR_IOFailure);

  LS_ERROR_CHECK(lsAlloc(ppData, length + (elementSize > 2 ? 0 : elementSize)));

  if (elementSize <= 2)
    lsZeroMemory(&((*ppData)[length]), elementSize); // To zero terminate strings. This is out of bounds for all other data types anyways.

  readLength = fread(*ppData, 1, length, pFile);

  *pCount = readLength / elementSize;

epilogue:
  if (pFile != nullptr)
    fclose(pFile);

  return result;
}

lsResult lsReadFileBytesSized(const char *filename, _Out_ uint8_t *pData, const size_t elementSize, const size_t count)
{
  lsResult result = lsR_Success;

  const size_t requestedBytes = elementSize * count;
  FILE *pFile = nullptr;
  size_t length, readLength;

  LS_ERROR_IF(filename == nullptr || pData == nullptr, lsR_ArgumentNull);

  pFile = fopen(filename, "rb");

  LS_ERROR_IF(pFile == nullptr, lsR_ResourceNotFound);

  LS_ERROR_IF(0 != fseek(pFile, 0, SEEK_END), lsR_IOFailure);

  length = ftell(pFile);
  LS_ERROR_IF(length < requestedBytes, lsR_EndOfStream);

  LS_ERROR_IF(0 != fseek(pFile, 0, SEEK_SET), lsR_IOFailure);

  readLength = fread(pData, 1, requestedBytes, pFile);

  LS_ERROR_IF(readLength != requestedBytes, lsR_IOFailure);

epilogue:
  if (pFile != nullptr)
    fclose(pFile);

  return result;
}

//////////////////////////////////////////////////////////////////////////

#ifdef LS_PLATFORM_WINDOWS
static lsResult lsWriteFileBytesToHandle(HANDLE fileHandle, const uint8_t *pData, const size_t size)
{
  lsResult result = lsR_Success;

  size_t bytesRemaining = size;
  size_t offset = 0;

  LS_ERROR_IF(fileHandle == INVALID_HANDLE_VALUE, lsR_InternalError);

  while (bytesRemaining > 0)
  {
    const DWORD bytesToWrite = (DWORD)lsMin((size_t)MAXDWORD, bytesRemaining);
    DWORD bytesWritten = 0;

    if (0 == WriteFile(fileHandle, pData + offset, bytesToWrite, &bytesWritten, nullptr))
    {
      const DWORD error = GetLastError();
      (void)error;
      LS_ERROR_SET(lsR_IOFailure);
    }

    LS_ERROR_IF(bytesWritten == 0, lsR_IOFailure);

    offset += bytesWritten;
    bytesRemaining -= bytesWritten;
  }

epilogue:
  return result;
}

lsResult lsWriteFileBytes(const wchar_t *filename, const uint8_t *pData, const size_t size)
{
  lsResult result = lsR_Success;

  HANDLE fileHandle = INVALID_HANDLE_VALUE;
  DWORD error;

  LS_ERROR_IF(filename == nullptr || pData == nullptr, lsR_ArgumentNull);

  fileHandle = CreateFileW(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.

  if (fileHandle == INVALID_HANDLE_VALUE && error == ERROR_PATH_NOT_FOUND)
  {
    wchar_t parentDirectory[MAX_PATH + 1];
    LS_ERROR_IF(!lsCopyString(parentDirectory, LS_ARRAYSIZE(parentDirectory), filename, MAX_PATH), lsR_ArgumentOutOfBounds);

    size_t lastSlash = (size_t)-1;
    for (size_t i = 0; i < LS_ARRAYSIZE(parentDirectory); i++)
    {
      if (parentDirectory[i] == '\0')
        break;
      else if (parentDirectory[i] == '\\' || parentDirectory[i] == '/')
        lastSlash = i;
    }

    LS_ERROR_IF(lastSlash == (size_t)-1, lsR_ResourceNotFound);
    parentDirectory[lastSlash] = '\0';

    LS_ERROR_CHECK(lsCreateDirectory(parentDirectory));

    fileHandle = CreateFileW(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.
  }

  LS_ERROR_CHECK(lsWriteFileBytesToHandle(fileHandle, pData, size));

epilogue:
  if (fileHandle != INVALID_HANDLE_VALUE)
    CloseHandle(fileHandle);

  return result;
}
#endif

lsResult lsWriteFileBytes(const char *filename, const uint8_t *pData, const size_t size)
{
  lsResult result = lsR_Success;

#ifndef LS_PLATFORM_WINDOWS
  FILE *pFile = nullptr;
#else
  HANDLE fileHandle = INVALID_HANDLE_VALUE;
#endif

  LS_ERROR_IF(filename == nullptr || pData == nullptr, lsR_ArgumentNull);

#ifndef LS_PLATFORM_WINDOWS
  pFile = fopen(filename, "wb");
  LS_ERROR_IF(pFile == nullptr, lsR_IOFailure);

  LS_ERROR_IF(size != fwrite(pData, 1, size, pFile), lsR_IOFailure);
#else
  {
    fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    DWORD error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.

    if (fileHandle == INVALID_HANDLE_VALUE && error == ERROR_PATH_NOT_FOUND)
    {
      char parentDirectory[MAX_PATH + 1];
      LS_ERROR_IF(!lsCopyString(parentDirectory, LS_ARRAYSIZE(parentDirectory), filename, MAX_PATH), lsR_ArgumentOutOfBounds);

      size_t lastSlash = (size_t)-1;
      for (size_t i = 0; i < LS_ARRAYSIZE(parentDirectory); i++)
      {
        if (parentDirectory[i] == '\0')
          break;
        else if (parentDirectory[i] == '\\' || parentDirectory[i] == '/')
          lastSlash = i;
      }

      LS_ERROR_IF(lastSlash == (size_t)-1, lsR_ResourceNotFound);
      parentDirectory[lastSlash] = '\0';

      LS_ERROR_CHECK(lsCreateDirectory(parentDirectory));

      fileHandle = CreateFileA(filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
      error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.
    }

    LS_ERROR_CHECK(lsWriteFileBytesToHandle(fileHandle, pData, size));
  }
#endif

epilogue:
#ifndef LS_PLATFORM_WINDOWS
  if (pFile != nullptr)
    fclose(pFile);
#else
  if (fileHandle != INVALID_HANDLE_VALUE)
    CloseHandle(fileHandle);
#endif

  return result;
}

//////////////////////////////////////////////////////////////////////////

#ifdef LS_PLATFORM_WINDOWS
bool lsFileExists(const char *filename)
{
  return (PathFileExistsA(filename) != FALSE);
}
#endif

//////////////////////////////////////////////////////////////////////////

#ifdef LS_PLATFORM_WINDOWS
lsResult lsCreateDirectory(const wchar_t *directory)
{
  lsResult result = lsR_Success;

  if (0 == CreateDirectoryW(directory, nullptr))
  {
    wchar_t parentDirectory[MAX_PATH + 1];
    LS_ERROR_IF(!lsCopyString(parentDirectory, LS_ARRAYSIZE(parentDirectory), directory, MAX_PATH), lsR_ArgumentOutOfBounds);

    size_t lastSlash = (size_t)-1;
    for (size_t i = 0; i < LS_ARRAYSIZE(parentDirectory); i++)
    {
      if (parentDirectory[i] == '\0')
        break;
      else if (parentDirectory[i] == '\\' || parentDirectory[i] == '/')
        lastSlash = i;
    }

    LS_ERROR_IF(lastSlash == (size_t)-1, lsR_ResourceNotFound);
    parentDirectory[lastSlash] = '\0';

    LS_ERROR_CHECK(lsCreateDirectory(parentDirectory));

    if (0 == CreateDirectoryW(directory, nullptr))
    {
      const DWORD error = GetLastError();
      LS_ERROR_IF(error != ERROR_ALREADY_EXISTS, lsR_InternalError);
    }
  }

epilogue:
  return result;
}

lsResult lsCreateDirectory(const char *directory)
{
  lsResult result = lsR_Success;

  if (0 == CreateDirectoryA(directory, nullptr))
  {
    char parentDirectory[MAX_PATH + 1];
    LS_ERROR_IF(!lsCopyString(parentDirectory, LS_ARRAYSIZE(parentDirectory), directory, MAX_PATH), lsR_ArgumentOutOfBounds);

    size_t lastSlash = (size_t)-1;
    for (size_t i = 0; i < LS_ARRAYSIZE(parentDirectory); i++)
    {
      if (parentDirectory[i] == '\0')
        break;
      else if (parentDirectory[i] == '\\' || parentDirectory[i] == '/')
        lastSlash = i;
    }

    LS_ERROR_IF(lastSlash == (size_t)-1, lsR_ResourceNotFound);
    parentDirectory[lastSlash] = '\0';

    LS_ERROR_CHECK(lsCreateDirectory(parentDirectory));

    if (0 == CreateDirectoryA(directory, nullptr))
    {
      const DWORD error = GetLastError();
      LS_ERROR_IF(error != ERROR_ALREADY_EXISTS, lsR_InternalError);
    }
  }

epilogue:
  return result;
}
#endif
