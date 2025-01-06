#pragma once

#include "core.h"

template <size_t layer_blocks_, size_t layers_>
struct neural_net
{
  static constexpr size_t layer_blocks = layer_blocks_;
  static constexpr size_t layers = layers_;

  static constexpr size_t block_size = sizeof(__m256) / sizeof(int8_t);
  static constexpr size_t neurons_per_layer = layer_blocks * block_size;
  static constexpr size_t weights_per_neuron = neurons_per_layer;
  static constexpr size_t weights_per_layer = neurons_per_layer * neurons_per_layer;
  static constexpr size_t biases_per_layer = neurons_per_layer;

  // data layout:
  //   sequential layers with:
  //     weights_per_neuron weights for each neuron
  //     biases_per_layer bias values (one for each neuron)
  LS_ALIGN(32) int8_t data[(weights_per_layer + biases_per_layer) * layers];
};

template <size_t layer_blocks>
struct neural_net_buffer
{
  static constexpr size_t block_size = sizeof(__m256) / sizeof(int8_t);
  LS_ALIGN(32) int8_t data[layer_blocks * block_size];
};

// convert any non-zero values to `lsMaxValue<int8_t>()` => ~1 in fixed point.
template <size_t layer_blocks>
inline void neural_net_buffer_prepare(neural_net_buffer<layer_blocks> &b, const size_t blockCount = layer_blocks)
{
  lsAssert(blockCount <= layer_blocks);

  __m256i *pBuffer = reinterpret_cast<__m256i *>(b.data);
  const __m256i expected = _mm256_set1_epi8(lsMaxValue<int8_t>());

  for (size_t inputBlock = 0; inputBlock < blockCount; inputBlock++)
  {
    const __m256i raw = _mm256_load_si256(pBuffer + inputBlock);
    const __m256i cmp = _mm256_cmpeq_epi8(_mm256_cmpeq_epi8(raw, _mm256_setzero_si256()), _mm256_setzero_si256());
    const __m256i out = _mm256_and_si256(cmp, expected);
    _mm256_store_si256(pBuffer + inputBlock, out);
  }
}

template <size_t layer_blocks, size_t layers>
inline void neural_net_eval(const neural_net<layer_blocks, layers> &nn, neural_net_buffer<layer_blocks> &io)
{
  LS_ALIGN(32) int16_t tmp[layer_blocks * nn.block_size];

  __m256i *pOut = reinterpret_cast<__m256i *>(io.data);
  __m256i *pTmp = reinterpret_cast<__m256i *>(tmp);
  const __m256i *pLayer = reinterpret_cast<const __m256i *>(nn.data);
  const __m128i _FFFF_64 = _mm_set1_epi64x(0xFFFF);
  const __m256i _min_16 = _mm256_set1_epi8(lsMinValue<int8_t>());
  const __m256i _max_16 = _mm256_set1_epi8(lsMaxValue<int8_t>());

  for (size_t layer = 0; layer < layers; layer++)
  {
    // Accumulate Weights.
    for (size_t neuron = 0; neuron < nn.neurons_per_layer; neuron++)
    {
      for (size_t inputBlock = 0; inputBlock < nn.layer_blocks; inputBlock++)
      {
        const __m128i *pLayer128 = reinterpret_cast<const __m128i *>(pLayer);
        const __m128i weightLo = _mm_load_si128(pLayer128);
        const __m128i weighsHi = _mm_load_si128(pLayer128 + 1);
        pLayer++;

        const __m128i *pOut128 = reinterpret_cast<const __m128i *>(pOut + inputBlock);
        const __m128i inLo = _mm_load_si128(pOut128);
        const __m128i inHi = _mm_load_si128(pOut128 + 1);

        const __m256i weightLo16 = _mm256_cvtepi8_epi16(weightLo);
        const __m256i weightHi16 = _mm256_cvtepi8_epi16(weighsHi);

        const __m256i inLo16 = _mm256_cvtepi8_epi16(inLo);
        const __m256i inHi16 = _mm256_cvtepi8_epi16(inHi);

        const __m256i res16Lo16 = _mm256_mullo_epi16(weightLo16, inLo16);
        const __m256i res16Hi16 = _mm256_mullo_epi16(weightHi16, inHi16);

        const __m256i resLo16 = _mm256_srai_epi16(res16Lo16, 7);
        const __m256i resHi16 = _mm256_srai_epi16(res16Hi16, 7);

        const __m256i res16Add2 = _mm256_hadds_epi16(resLo16, resHi16); // aaaabbbbAAAABBBB
        const __m128i res16Add2Lo = _mm256_extractf128_si256(res16Add2, 0); // aaaabbbb
        const __m128i res16Add2Hi = _mm256_extractf128_si256(res16Add2, 1); // AAAABBBB

        const __m128i res16Add4 = _mm_hadds_epi16(res16Add2Lo, res16Add2Hi); // aaAAbbBB
        const __m128i res16Add4Lo = _mm_and_si128(res16Add4, _FFFF_64); // aa__bb__
        const __m128i res16Add4Hi = _mm_srli_si128(res16Add4, 4); // AA__BB__
        const __m128i res16Add8 = _mm_adds_epi16(res16Add4Lo, res16Add4Hi); // aA__bB__

        const __m128i res16Add16 = _mm_hadds_epi16(res16Add8, res16Add8); // a_a_b_b_
        const __m128i res16Add16Lo = _mm_srli_si128(res16Add16, 64); // b_b_____
        const __m128i res16Add32 = _mm_adds_epi16(res16Add16, res16Add16Lo);

        tmp[neuron] += (int16_t)_mm_extract_epi16(res16Add32, 0);
      }
    }

    for (size_t inputBlock = 0; inputBlock < layer_blocks; inputBlock++)
    {
      const __m128i *pLayer128 = reinterpret_cast<const __m128i *>(pLayer);
      const __m128i biasLo = _mm_load_si128(pLayer128);
      const __m128i biasHi = _mm_load_si128(pLayer128 + 1);
      pLayer++;

      const __m256i tmpLo16 = _mm256_load_si256(pTmp + inputBlock * 2);
      const __m256i tmpHi16 = _mm256_load_si256(pTmp + inputBlock * 2 + 1);

      const __m256i biasLo16 = _mm256_cvtepi8_epi16(biasLo);
      const __m256i biasHi16 = _mm256_cvtepi8_epi16(biasHi);

      const __m256i sumLo16 = _mm256_adds_epi16(biasLo16, tmpLo16);
      const __m256i sumHi16 = _mm256_adds_epi16(biasHi16, tmpHi16);
      const __m256i resLo16 = _mm256_max_epi16(_mm256_min_epi16(sumLo16, _max_16), _min_16);
      const __m256i resHi16 = _mm256_max_epi16(_mm256_min_epi16(sumHi16, _max_16), _min_16);

      const __m128i sumLo8 = _mm256_cvtepi16_epi8(resLo16); // !!! this is AVX-512 VL, rewrite via AVX2, but potentially make all buffers `int16_t` anyways.
      const __m128i sumHi8 = _mm256_cvtepi16_epi8(resHi16); // !!! this is AVX-512 VL, rewrite via AVX2, but potentially make all buffers `int16_t` anyways.

      const __m256i sum8 = _mm256_set_m128i(sumHi8, sumLo8);
      _mm256_store_si256(pOut + inputBlock, sum8);
    }
  }
}
