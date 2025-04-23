#ifndef INV_SAX_HPP
#define INV_SAX_HPP

#include <compare>

#include "Util/typedefs.hpp"
#include "Util/RunSettings.hpp"
#include "Summarization/IndexEntry.hpp"
#include "Summarization/Envelope.hpp"
#include "Summarization/Paa.hpp"
#include "Summarization/SaxWord.hpp"

template <typename T>
    requires DerivedFromEntryData<T>
class InvSax {
   public:
    InvSax() = default;

    InvSax(const vec<T>& entry_data, const SaxNumBitsT segment_num_bits) {
        assert(!entry_data.empty());

        auto& RS = RunSettings::get_instance();
        auto& breakpoints = RS.get_breakpoints();

        SaxSegIndT num_segments = static_cast<SaxSegIndT>(entry_data[0].size());
        MtsNumChannelsT num_channels = static_cast<MtsNumChannelsT>(entry_data.size());

        uint num_bits_total = static_cast<uint>(segment_num_bits * num_segments * num_channels);
        if constexpr (std::is_same_v<T, Envelope>) num_bits_total *= 2;
        m_inv_sax_value.resize((num_bits_total + 7) / 8, 0);

        uint inv_sax_ind = 0;
        uint8_t bit_ind = 0b111;

        if constexpr (std::is_same_v<T, Paa>) {
            vec<SaxWord> sax_words;
            sax_words.reserve(entry_data.size());
            for (auto& entry_channel : entry_data)
                sax_words.emplace_back(static_cast<Paa>(entry_channel).m_paa_values, segment_num_bits, breakpoints);

            for (int bit = static_cast<int>(segment_num_bits) - 1; bit >= 0; --bit) {
                for (SaxSegIndT s = 0; s < num_segments; ++s) {
                    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                        update_inv_sax_value(sax_words, inv_sax_ind, bit_ind, c, s, bit);
                    }
                }
            }
        } else {  // std::is_same_v<T, Envelope>
            vec<SaxWord> sax_lowers, sax_uppers;
            sax_lowers.reserve(entry_data.size());
            sax_uppers.reserve(entry_data.size());
            for (auto& entry_channel : entry_data) {
                sax_lowers.emplace_back(static_cast<Envelope>(entry_channel).m_lower, segment_num_bits, breakpoints);
                sax_uppers.emplace_back(static_cast<Envelope>(entry_channel).m_upper, segment_num_bits, breakpoints);
            }

            for (int bit = static_cast<int>(segment_num_bits) - 1; bit >= 0; --bit) {
                for (SaxSegIndT s = 0; s < num_segments; ++s) {
                    for (MtsNumChannelsT c = 0; c < num_channels; ++c) {
                        for (auto& sax_words : {sax_lowers, sax_uppers}) {
                            update_inv_sax_value(sax_words, inv_sax_ind, bit_ind, c, s, bit);
                        }
                    }
                }
            }
        }
    }

    std::strong_ordering operator<=>(const InvSax& other) const {
        assert(m_inv_sax_value.size() == other.m_inv_sax_value.size());
        for (size_t i = 0; i < m_inv_sax_value.size(); ++i) {
            if (m_inv_sax_value[i] < other.m_inv_sax_value[i]) return std::strong_ordering::less;
            if (m_inv_sax_value[i] > other.m_inv_sax_value[i]) return std::strong_ordering::greater;
        }
        return std::strong_ordering::equal;
    }

   private:
    vec<char> m_inv_sax_value;

    inline void update_inv_sax_value(const vec<SaxWord>& sax_words, uint& inv_sax_ind, uint8_t& bit_ind,
                                     MtsNumChannelsT c, SaxSegIndT s, int bit) {
        m_inv_sax_value[inv_sax_ind] |= static_cast<char>(((sax_words[c][s] >> bit) & 1) << bit_ind);
        if (bit_ind == 0) {
            bit_ind = 0b111;
            ++inv_sax_ind;
        } else {
            --bit_ind;
        }
    }
};

#endif  // INV_SAX_HPP
