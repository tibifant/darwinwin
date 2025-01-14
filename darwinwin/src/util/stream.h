#pragma once

#include "core.h"

//////////////////////////////////////////////////////////////////////////

template <typename T>
concept byte_stream_reader = requires (T & t, uint8_t & v, uint8_t * pD, size_t s)
{
  { read_byte_stream_read(t, v) } -> std::same_as<lsResult>;
  { read_byte_stream_read(t, pD, s) } -> std::same_as<lsResult>;
  { read_byte_stream_pos(std::as_const(t)) } -> std::same_as<size_t>;
  { read_byte_stream_size(std::as_const(t)) } -> std::same_as<size_t>;
};

//////////////////////////////////////////////////////////////////////////

template <byte_stream_reader source_stream, size_t buffer_size_bits = 9>
struct cached_byte_stream_reader
{
  uint8_t buffer[1ULL << buffer_size_bits] = {};
  static_assert(sizeof(buffer) > sizeof(uint8_t));

  size_t position = 0;
  size_t consumed_bytes = 0;
  size_t buffer_size = 0;
  source_stream *pSource = nullptr;
};

template <byte_stream_reader source_stream, size_t buffer_size_bits>
inline void read_byte_stream_init(cached_byte_stream_reader<source_stream, buffer_size_bits> &stream, source_stream *pSrc)
{
  lsAssert(pSrc);
  stream.pSource = pSrc;
}

template <byte_stream_reader source_stream, size_t buffer_size_bits>
inline size_t read_byte_stream_pos(const cached_byte_stream_reader<source_stream, buffer_size_bits> &stream)
{
  return stream.position + stream.consumed_bytes;
}

template <byte_stream_reader source_stream, size_t buffer_size_bits>
inline size_t read_byte_stream_size(const cached_byte_stream_reader<source_stream, buffer_size_bits> &stream)
{
  return read_byte_stream_size(*stream.pSource);
}

template <byte_stream_reader source_stream, size_t buffer_size_bits>
inline lsResult read_byte_stream_pull_internal(cached_byte_stream_reader<source_stream, buffer_size_bits> &stream)
{
  lsResult result = lsR_Success;

  lsAssert(stream.position == stream.buffer_size);

  const size_t pos = read_byte_stream_pos(*stream.pSource);
  const size_t size = read_byte_stream_size(*stream.pSource);
  const size_t pullSize = lsMin(LS_ARRAYSIZE(stream.buffer), size - pos);
  LS_ERROR_IF(pullSize == 0, lsR_EndOfStream);
  LS_ERROR_CHECK(read_byte_stream_read(*stream.pSource, stream.buffer, pullSize));
  stream.consumed_bytes += stream.buffer_size;
  stream.buffer_size = pullSize;
  stream.position = 0;

epilogue:
  return result;
}

template <byte_stream_reader source_stream, size_t buffer_size_bits>
inline lsResult read_byte_stream_read(cached_byte_stream_reader<source_stream, buffer_size_bits> &stream, uint8_t &value)
{
  lsResult result = lsR_Success;

  if (stream.position == stream.buffer_size) _UNLIKELY
    LS_ERROR_CHECK(read_byte_stream_pull_internal(stream));

  value = stream.buffer[stream.position];
  stream.position++;

epilogue:
  return result;
}

template <byte_stream_reader source_stream, size_t buffer_size_bits>
inline lsResult read_byte_stream_read(cached_byte_stream_reader<source_stream, buffer_size_bits> &stream, uint8_t *pData, const size_t size)
{
  lsResult result = lsR_Success;

  const size_t initiallyAvailableSize = stream.buffer_size - stream.position;

  if (initiallyAvailableSize >= size) _LIKELY
  {
    lsMemcpy(pData, stream.buffer + stream.position, size);
    stream.position += size;
  }
  else if (size < LS_ARRAYSIZE(stream.buffer) + initiallyAvailableSize)
  {
    lsMemcpy(pData, stream.buffer + stream.position, initiallyAvailableSize);
    stream.position = stream.buffer_size;
    LS_ERROR_CHECK(read_byte_stream_pull_internal(stream));
    const size_t remainingSize = size - initiallyAvailableSize;
    LS_ERROR_IF(stream.buffer_size < remainingSize, lsR_EndOfStream);
    lsMemcpy(pData + initiallyAvailableSize, stream.buffer, remainingSize);
    stream.position = remainingSize;
  }
  else
  {
    lsMemcpy(pData, stream.buffer + stream.position, initiallyAvailableSize);
    const size_t remainingSize = size - initiallyAvailableSize;
    const size_t pos = read_byte_stream_pos(*stream.pSource);
    const size_t totalSize = read_byte_stream_size(*stream.pSource);
    const size_t remainingStreamSize = totalSize - pos;
    stream.position = stream.buffer_size;
    LS_ERROR_IF(remainingStreamSize < remainingSize, lsR_EndOfStream);
    LS_ERROR_CHECK(read_byte_stream_read(*stream.pSource, pData + initiallyAvailableSize, remainingSize));
    stream.consumed_bytes += remainingSize;
  }

epilogue:
  return result;
}

//////////////////////////////////////////////////////////////////////////

struct raw_file_byte_stream_reader
{
  void *pHandle = nullptr;
  size_t position = 0;
  size_t size = 0;

  ~raw_file_byte_stream_reader();
};

lsResult read_byte_stream_init(raw_file_byte_stream_reader &stream, const char *filename);
void read_byte_stream_destroy(raw_file_byte_stream_reader &stream);

lsResult read_byte_stream_read(raw_file_byte_stream_reader &stream, uint8_t &value);
lsResult read_byte_stream_read(raw_file_byte_stream_reader &stream, uint8_t *pData, const size_t size);

inline size_t read_byte_stream_pos(const raw_file_byte_stream_reader &stream)
{
  return stream.position;
}

inline size_t read_byte_stream_size(const raw_file_byte_stream_reader &stream)
{
  return stream.size;
}

//////////////////////////////////////////////////////////////////////////

template <size_t buffer_size_bits = 10>
struct cached_file_byte_stream_reader : cached_byte_stream_reader<raw_file_byte_stream_reader, buffer_size_bits>
{
  raw_file_byte_stream_reader raw;
};

template <size_t buffer_size_bits>
lsResult read_byte_stream_init(cached_file_byte_stream_reader<buffer_size_bits> &reader, const char *filename)
{
  const lsResult result = read_byte_stream_init(reader.raw, filename);
  read_byte_stream_init(reader, &reader.raw);
  return result;
}

template <size_t buffer_size_bits>
void read_byte_stream_destroy(cached_file_byte_stream_reader<buffer_size_bits> &reader)
{
  read_byte_stream_destroy(reader.raw);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
concept byte_stream_writer = requires (T & t, uint8_t * pD, size_t s)
{
  { write_byte_stream_append(t, pD, s) } -> std::same_as<lsResult>;
  { write_byte_stream_flush(t) } -> std::same_as<lsResult>;
};

//////////////////////////////////////////////////////////////////////////

template <byte_stream_writer target_stream, size_t buffer_size_bits = 9>
struct cached_byte_stream_writer
{
  uint8_t buffer[1ULL << buffer_size_bits] = {};
  static_assert(sizeof(buffer) > sizeof(uint8_t));

  size_t position = 0;
  target_stream *pTarget = nullptr;
};

template <byte_stream_writer target_stream, size_t buffer_size_bits>
inline void write_byte_stream_init(cached_byte_stream_writer<target_stream, buffer_size_bits> &writer, target_stream *pTarget)
{
  lsAssert(pTarget != nullptr);

  writer.pTarget = pTarget;
  writer.position = 0;
}

template <byte_stream_writer target_stream, size_t buffer_size_bits>
inline lsResult write_byte_stream_append(cached_byte_stream_writer<target_stream, buffer_size_bits> &writer, const uint8_t *pData, const size_t size)
{
  lsResult result = lsR_Success;

  lsAssert(writer.pTarget != nullptr);

  if (size >= sizeof(writer.buffer))
  {
    if (writer.position > 0)
    {
      LS_ERROR_CHECK(write_byte_stream_append(*writer.pTarget, writer.buffer, writer.position));
      writer.position = 0;
    }

    LS_ERROR_CHECK(write_byte_stream_append(*writer.pTarget, pData, size));
  }
  else
  {
    if (writer.position + size > sizeof(writer.buffer))
    {
      LS_ERROR_CHECK(write_byte_stream_append(*writer.pTarget, writer.buffer, writer.position));
      writer.position = size;
      lsMemcpy(writer.buffer, pData, size);
    }
    else
    {
      lsMemcpy(writer.buffer + writer.position, pData, size);
      writer.position += size;
    }
  }

epilogue:
  return result;
}

template <byte_stream_writer target_stream, size_t buffer_size_bits>
inline lsResult write_byte_stream_flush(cached_byte_stream_writer<target_stream, buffer_size_bits> &writer)
{
  lsResult result = lsR_Success;

  lsAssert(writer.pTarget != nullptr);

  if (writer.position > 0)
    LS_ERROR_CHECK(write_byte_stream_append(*writer.pTarget, writer.buffer, writer.position));

  writer.position = 0;
  LS_ERROR_CHECK(write_byte_stream_flush(*writer.pTarget));

epilogue:
  return result;
}

//////////////////////////////////////////////////////////////////////////

struct raw_file_byte_stream_writer
{
  void *pHandle = nullptr;

  ~raw_file_byte_stream_writer();
};

lsResult write_byte_stream_init(raw_file_byte_stream_writer &writer, const char *filename, const bool append = false);
void write_byte_stream_destroy(raw_file_byte_stream_writer &writer);
lsResult write_byte_stream_append(raw_file_byte_stream_writer &writer, const uint8_t *pData, const size_t size);
lsResult write_byte_stream_flush(raw_file_byte_stream_writer &writer);

//////////////////////////////////////////////////////////////////////////

template <size_t buffer_size_bits = 10>
struct cached_file_byte_stream_writer : cached_byte_stream_writer<raw_file_byte_stream_writer, buffer_size_bits>
{
  raw_file_byte_stream_writer raw;
};

template <size_t buffer_size_bits>
lsResult write_byte_stream_init(cached_file_byte_stream_writer<buffer_size_bits> &writer, const char *filename, const bool append = false)
{
  const lsResult result = write_byte_stream_init(writer.raw, filename, append);
  write_byte_stream_init(writer, &writer.raw);
  return result;
}

template <size_t buffer_size_bits>
void write_byte_stream_destroy(cached_file_byte_stream_writer<buffer_size_bits> &writer)
{
  write_byte_stream_flush(writer);
  write_byte_stream_destroy(writer.raw);
}
