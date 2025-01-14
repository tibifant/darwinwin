#include "stream.h"

#include <io.h>

//////////////////////////////////////////////////////////////////////////

raw_file_byte_stream_reader::~raw_file_byte_stream_reader()
{
  read_byte_stream_destroy(*this);
}

lsResult read_byte_stream_init(raw_file_byte_stream_reader &stream, const char *filename)
{
  lsResult result = lsR_Success;

  lsAssert(stream.pHandle == nullptr);

  stream.pHandle = fopen(filename, "rb");
  LS_ERROR_IF(stream.pHandle == nullptr, lsR_IOFailure);

  stream.position = 0;

  // Determine Size.
  {
#ifdef LS_PLATFORM_WINDOWS
    const int64_t seek_ret = _fseeki64(reinterpret_cast<FILE *>(stream.pHandle), 0, SEEK_END);
    LS_ERROR_IF(seek_ret < 0, lsR_IOFailure);
    stream.size = (size_t)_ftelli64(reinterpret_cast<FILE *>(stream.pHandle));
    _fseeki64(reinterpret_cast<FILE *>(stream.pHandle), 0, SEEK_SET);
#else
    const int64_t seek_ret = fseeko(reinterpret_cast<FILE *>(stream.pHandle), 0, SEEK_END);
    LS_ERROR_IF(seek_ret != 0, lsR_IOFailure);
    stream.size = (size_t)ftello(reinterpret_cast<FILE *>(stream.pHandle));
    ftello(reinterpret_cast<FILE *>(stream.pHandle), 0, SEEK_SET);
#endif
  }

epilogue:
  return result;
}

void read_byte_stream_destroy(raw_file_byte_stream_reader &stream)
{
  if (stream.pHandle == nullptr)
    return;

  fclose(reinterpret_cast<FILE *>(stream.pHandle));
  stream.pHandle = nullptr;
}

lsResult read_byte_stream_read(raw_file_byte_stream_reader &stream, uint8_t &value)
{
  if (1 == fread(&value, 1, 1, reinterpret_cast<FILE *>(stream.pHandle))) _LIKELY
  {
    stream.position++;
    return lsR_Success;
  }

  return lsR_IOFailure;
}

lsResult read_byte_stream_read(raw_file_byte_stream_reader &stream, uint8_t *pData, const size_t size)
{
  lsAssert(size < (size_t)lsMaxValue<int32_t>());

  if ((int32_t)size == fread(pData, 1, (int32_t)size, reinterpret_cast<FILE *>(stream.pHandle)))
  {
    stream.position += size;
    return lsR_Success;
  }

  return lsR_IOFailure;
}

//////////////////////////////////////////////////////////////////////////

raw_file_byte_stream_writer::~raw_file_byte_stream_writer()
{
  write_byte_stream_destroy(*this);
}

lsResult write_byte_stream_init(raw_file_byte_stream_writer &writer, const char *filename, const bool append /* = false */)
{
  lsResult result = lsR_Success;

  lsAssert(writer.pHandle == nullptr);

  writer.pHandle = fopen(filename, "wb");
  LS_ERROR_IF(writer.pHandle == nullptr, lsR_IOFailure);

  if (append)
    fseek(reinterpret_cast<FILE *>(writer.pHandle), 0, SEEK_END);

epilogue:
  return result;
}

void write_byte_stream_destroy(raw_file_byte_stream_writer &writer)
{
  if (writer.pHandle == nullptr)
    return;

  const int32_t result = fflush(reinterpret_cast<FILE *>(writer.pHandle));
  lsAssert(result == 0);

  fclose(reinterpret_cast<FILE *>(writer.pHandle));

  writer.pHandle = nullptr;
}

lsResult write_byte_stream_append(raw_file_byte_stream_writer &writer, const uint8_t *pData, const size_t size)
{
  lsAssert(size < (size_t)lsMaxValue<int32_t>());
  const size_t written = fwrite(pData, 1, size, reinterpret_cast<FILE *>(writer.pHandle));
  return written == size ? lsR_Success : lsR_IOFailure;
}

lsResult write_byte_stream_flush(raw_file_byte_stream_writer &writer)
{
  return (fflush(reinterpret_cast<FILE *>(writer.pHandle)) == 0) ? lsR_Success : lsR_IOFailure;
}
