#pragma once

#include "core.h"
#include "stream.h"

template <byte_stream_writer writer>
struct value_writer
{
  writer *pWriter = nullptr;
};

template <byte_stream_writer writer>
lsResult value_writer_init(value_writer<writer> &vw, writer *pWriter)
{
  lsResult result = lsR_Success;

  lsAssert(vw.pWriter == nullptr);
  vw.pWriter = pWriter;

  goto epilogue;
epilogue:
  return result;
}

template <typename T, byte_stream_writer writer>
  requires (!std::is_array_v<T>)
lsResult value_writer_write(value_writer<writer> &vw, const T &v)
{
  lsResult result = lsR_Success;

  lsAssert(vw.pWriter);
  LS_ERROR_CHECK(write_byte_stream_append(*vw.pWriter, reinterpret_cast<const uint8_t *>(&v), sizeof(T)));

epilogue:
  return result;
}

template <typename T, byte_stream_writer writer>
  requires (!std::is_array_v<T>)
lsResult value_writer_write(value_writer<writer> &vw, const T *pV, const size_t count)
{
  lsResult result = lsR_Success;

  lsAssert(vw.pWriter);
  LS_ERROR_CHECK(write_byte_stream_append(*vw.pWriter, reinterpret_cast<const uint8_t *>(pV), sizeof(T) * count));

epilogue:
  return result;
}

template <byte_stream_reader reader>
struct value_reader
{
  reader *pReader = nullptr;
};

template <byte_stream_reader reader>
lsResult value_reader_init(value_reader<reader> &vr, reader *pReader)
{
  lsResult result = lsR_Success;

  lsAssert(vr.pReader == nullptr);
  vr.pReader = pReader;

  goto epilogue;
epilogue:
  return result;

}

template <typename T, byte_stream_reader reader>
  requires (!std::is_const_v<T> && !std::is_array_v<T>)
lsResult value_reader_read(value_reader<reader> &vr, T &v)
{
  lsResult result = lsR_Success;

  lsAssert(vr.pReader);
  LS_ERROR_CHECK(read_byte_stream_read(*vr.pReader, reinterpret_cast<uint8_t *>(&v), sizeof(T)));

epilogue:
  return result;
}

template <typename T, byte_stream_reader reader>
  requires (!std::is_const_v<T> && !std::is_array_v<T>)
lsResult value_reader_read(value_reader<reader> &vr, T *pV, const size_t count)
{
  lsResult result = lsR_Success;

  lsAssert(vr.pReader);
  LS_ERROR_CHECK(read_byte_stream_read(*vr.pReader, reinterpret_cast<uint8_t *>(pV), sizeof(T) * count));

epilogue:
  return result;
}
