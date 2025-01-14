#pragma once

#include "core.h"
#include "stream.h"

template <byte_stream_writer writer>
struct value_writer
{
  writer *pWriter = nullptr;
};

template <byte_stream_writer writer>
lsResult value_writer_init(value_writer<writer> &vw, writer *pWriter); // TODO: init byte_stream

template <typename T, byte_stream_writer writer>
lsResult value_writer_write(value_writer<writer> &vw, const T &v)
{
  lsResult result = lsR_Success;

  lsAssert(vw.pWriter);

  // TODO: ...
  // append write_byte_stream_append(vw.pWriter, ) cast to uint8_t

epilogue:
  return result;
}

template <typename T, byte_stream_writer writer>
lsResult value_writer_write(value_writer<writer> &vw, const T *pV, const size_t count);

template <byte_stream_reader reader>
struct value_reader
{
  reader *pReader = nullptr;
};

template <byte_stream_reader reader>
lsResult value_reader_init(value_reader<reader> &vr, reader *pReader);

template <typename T, byte_stream_reader reader>
lsResult value_reader_read(value_reader<reader> &vr, const T &v);

template <typename T, byte_stream_reader reader>
lsResult value_reader_read(value_reader<reader> &vr, const T *pV, const size_t count);
