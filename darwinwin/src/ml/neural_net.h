#pragma once

#include "core.h"

constexpr size_t neural_net_block_size = sizeof(__m256) / sizeof(int16_t);

namespace nn_internal
{
  template <size_t ...layer_blocks>
  struct unwrap_layers_
  {
  };

  template <size_t n>
  struct unwrap_layers_<n>
  {
    constexpr static size_t self_blocks = n;
    constexpr static size_t total_blocks = n;
    constexpr static size_t self_neurons = n * neural_net_block_size;
    constexpr static size_t total_neurons = n * neural_net_block_size;
    constexpr static size_t self_biases = n * neural_net_block_size;
    constexpr static size_t total_biases = n * neural_net_block_size;
    constexpr static size_t count = 1;
    constexpr static size_t max_child_neurons = self_neurons;
    constexpr static size_t self_weights(const size_t prevLayerNeurons) { return prevLayerNeurons * self_neurons; };
    constexpr static size_t size(const size_t prevLayerNeurons) { return self_weights(prevLayerNeurons) + self_biases; };
  };

  template <size_t n, size_t... others>
  struct unwrap_layers_<n, others...>
  {
    constexpr static size_t self_blocks = unwrap_layers_<n>::self_blocks;
    constexpr static size_t total_blocks = unwrap_layers_<n>::total_blocks + unwrap_layers_<others...>::total_blocks;
    constexpr static size_t self_neurons = unwrap_layers_<n>::self_neurons;
    constexpr static size_t total_neurons = unwrap_layers_<n>::total_neurons + unwrap_layers_<others...>::total_neurons;
    constexpr static size_t self_biases = unwrap_layers_<n>::self_biases;
    constexpr static size_t total_biases = unwrap_layers_<n>::total_biases + unwrap_layers_<others...>::total_biases;
    constexpr static size_t count = unwrap_layers_<n>::count + unwrap_layers_<others...>::count;
    constexpr static size_t max_child_neurons = lsMax(self_neurons, unwrap_layers_<others...>::max_child_neurons);
    constexpr static size_t self_weights(const size_t prevLayerNeurons) { return unwrap_layers_<n>::self_weights(prevLayerNeurons); };
    constexpr static size_t size(const size_t prevLayerNeurons) { return unwrap_layers_<n>::size(prevLayerNeurons) + unwrap_layers_<others...>::size(self_neurons); };
  };

  template <size_t n, size_t... others>
  struct unwrap_layers
  {
    constexpr static size_t self_blocks = unwrap_layers_<n>::self_blocks;
    constexpr static size_t total_blocks = unwrap_layers_<others...>::total_blocks;
    constexpr static size_t self_neurons = unwrap_layers_<n>::self_neurons;
    constexpr static size_t total_neurons = unwrap_layers_<others...>::total_neurons;
    constexpr static size_t self_biases = 0;
    constexpr static size_t total_biases = unwrap_layers_<others...>::total_biases;
    constexpr static size_t count = unwrap_layers_<others...>::count;
    constexpr static size_t max_child_neurons = lsMax(self_neurons, unwrap_layers_<others...>::max_child_neurons);
    constexpr static size_t max_child_neurons_excl_first = unwrap_layers_<others...>::max_child_neurons;
    constexpr static size_t size = unwrap_layers_<others...>::size(self_neurons);
  };

  template <size_t prev_layer_neurons, size_t ...layer_blocks>
  struct layer_data_
  {
  };

  template <size_t prev_layer_neurons, size_t layer_blocks>
  struct layer_data_<prev_layer_neurons, layer_blocks>
  {
    constexpr static size_t weight_count = unwrap_layers_<layer_blocks>::self_weights(prev_layer_neurons);
    constexpr static size_t weight_blocks = weight_count / neural_net_block_size;
    constexpr static size_t bias_count = unwrap_layers_<layer_blocks>::self_biases;
    constexpr static size_t bias_blocks = bias_count / neural_net_block_size;
    constexpr static size_t neuron_count = unwrap_layers_<layer_blocks>::self_neurons;
    constexpr static size_t neuron_blocks = neuron_count / neural_net_block_size;
    constexpr static size_t layer_combined_size = bias_count + weight_count;
    constexpr static size_t total_combined_size = layer_combined_size;
    constexpr static size_t previous_layer_neuron_blocks = prev_layer_neurons / neural_net_block_size;
    constexpr static bool is_last = true;

    LS_ALIGN(32) int16_t biases[bias_count];
    LS_ALIGN(32) int16_t weights[weight_count];
  };

  template <size_t prev_layer_neurons, size_t self_layer_blocks, size_t ...layer_blocks>
  struct layer_data_<prev_layer_neurons, self_layer_blocks, layer_blocks...> : layer_data_<prev_layer_neurons, self_layer_blocks>
  {
    layer_data_<unwrap_layers_<self_layer_blocks>::self_neurons, layer_blocks...> next;

    constexpr static size_t total_combined_size = layer_data_<prev_layer_neurons, self_layer_blocks>::layer_combined_size + layer_data_<unwrap_layers_<self_layer_blocks>::self_neurons, layer_blocks...>::total_combined_size;
    constexpr static bool is_last = false;
  };

  template <size_t self_layer_blocks, size_t ...layer_blocks>
  struct layer_data : layer_data_<unwrap_layers_<self_layer_blocks>::self_neurons, layer_blocks...>
  {
  };
}

//////////////////////////////////////////////////////////////////////////

template <size_t layer_blocks>
struct neural_net_buffer;

template <size_t ...layer_blocks_per_layer>
struct neural_net
{
  using io_buffer_t = neural_net_buffer<nn_internal::unwrap_layers<layer_blocks_per_layer...>::max_child_neurons / neural_net_block_size>;

  constexpr static size_t total_value_count = nn_internal::unwrap_layers<layer_blocks_per_layer...>::size;

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif
  union
  {
    nn_internal::layer_data<layer_blocks_per_layer...> data;
    int16_t values[total_value_count];
  };
#ifdef _MSC_VER
#pragma warning(pop)
#endif
};

//////////////////////////////////////////////////////////////////////////

template <size_t layer_blocks>
struct neural_net_buffer
{
  static constexpr size_t block_size = neural_net_block_size;
  LS_ALIGN(32) int16_t data[layer_blocks * block_size];

  int16_t &operator [](const size_t idx)
  {
    lsAssert(idx < layer_blocks * block_size);
    return data[idx];
  }

  const int16_t operator [](const size_t idx) const
  {
    lsAssert(idx < layer_blocks * block_size);
    return data[idx];
  }
};

//////////////////////////////////////////////////////////////////////////

// convert any non-zero values to `lsMaxValue<int8_t>()` => ~1 in fixed point.
template <size_t layer_blocks>
inline void neural_net_buffer_prepare(neural_net_buffer<layer_blocks> &b, const size_t blockCount = layer_blocks)
{
  lsAssert(blockCount <= layer_blocks);

  __m256i *pBuffer = reinterpret_cast<__m256i *>(b.data);
  const __m256i expected = _mm256_set1_epi16(lsMaxValue<int8_t>());

  for (size_t inputBlock = 0; inputBlock < blockCount; inputBlock++)
  {
    const __m256i raw = _mm256_load_si256(pBuffer + inputBlock);
    const __m256i cmp = _mm256_cmpeq_epi16(_mm256_cmpeq_epi16(raw, _mm256_setzero_si256()), _mm256_setzero_si256());
    const __m256i out = _mm256_and_si256(cmp, expected);
    _mm256_store_si256(pBuffer + inputBlock, out);
  }
}

//////////////////////////////////////////////////////////////////////////

template <size_t ...blocks>
inline void neural_net_eval_layer_recursive_internal(const nn_internal::layer_data_<blocks...> &layer, __m256i *pIO, int16_t *pTmp)
{
  __m256i *pTmp256 = reinterpret_cast<__m256i *>(pTmp);
  const __m256i *pWeight = reinterpret_cast<const __m256i *>(layer.weights);
  const __m256i *pBias = reinterpret_cast<const __m256i *>(layer.biases);
  const __m128i _FFFF_64 = _mm_set1_epi64x(0xFFFF);
  const __m256i _min_16 = _mm256_set1_epi16(lsMinValue<int8_t>());
  const __m256i _max_16 = _mm256_set1_epi16(lsMaxValue<int8_t>());

  // Accumulate Weights.
  for (size_t neuron = 0; neuron < layer.neuron_count; neuron++)
  {
    for (size_t inputBlock = 0; inputBlock < layer.previous_layer_neuron_blocks; inputBlock++)
    {
      const __m256i weight = _mm256_load_si256(pWeight);
      pWeight++;

      const __m256i in = _mm256_load_si256(pIO);

      const __m256i resRaw = _mm256_mullo_epi16(weight, in);
      const __m256i resNormalized = _mm256_srai_epi16(resRaw, 7);

      const __m256i resAdd2 = _mm256_hadds_epi16(resNormalized, resNormalized); // ACEG....IKMO....
      const __m256i resAdd4 = _mm256_hadds_epi16(resAdd2, resAdd2); // AE......IM......
      const __m256i resAdd8 = _mm256_hadds_epi16(resAdd4, resAdd4); // A.......I.......

      pTmp[neuron] += (int16_t)_mm256_extract_epi16(resAdd8, 0) + (int16_t)_mm256_extract_epi16(resAdd8, 8);
    }
  }

  for (size_t inputBlock = 0; inputBlock < layer.bias_blocks; inputBlock++)
  {
    const __m256i bias = _mm256_load_si256(pBias);
    pBias++;

    const __m256i weightSum = _mm256_load_si256(pTmp256 + inputBlock);

    const __m256i sum = _mm256_adds_epi16(bias, weightSum);
    const __m256i res = _mm256_max_epi16(_mm256_min_epi16(sum, _max_16), _min_16);

    _mm256_store_si256(pIO + inputBlock, res);
  }

  if constexpr (!layer.is_last)
    neural_net_eval_layer_recursive_internal(layer.next, pIO, pTmp);
}

template <size_t ...layer_blocks_per_layer>
inline void neural_net_eval(const neural_net<layer_blocks_per_layer...> &nn, typename neural_net<layer_blocks_per_layer...>::io_buffer_t &io)
{
  static_assert(nn.data.total_combined_size == nn.total_value_count);
  LS_ALIGN(32) int16_t tmp[nn_internal::unwrap_layers<layer_blocks_per_layer...>::max_child_neurons_excl_first] = {};

  neural_net_eval_layer_recursive_internal(nn.data, reinterpret_cast<__m256i *>(io.data), tmp);
}
