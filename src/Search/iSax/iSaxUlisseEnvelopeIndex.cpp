#include "Search/iSax/iSaxUlisseEnvelopeIndex.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Summarization/iSaxWord.hpp"

#include <iostream>

iSaxUlisseEnvelopeIndex::iSaxUlisseEnvelopeIndex(SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                                                 std::unique_ptr<IiSaxBreakpointStrategy> breakpoint_strategy,
                                                 std::unique_ptr<IiSaxSplitStrategy> split_strategy)
    : m_first_layer_num_bits(first_layer_num_bits),
      m_leaf_capacity(leaf_capacity),
      m_breakpoint_strategy(std::move(breakpoint_strategy)),
      m_split_strategy(std::move(split_strategy)),
      m_breakpoints(m_breakpoint_strategy->get_breakpoints(1 << m_first_layer_num_bits)),
      m_first_layer_breakpoints(m_breakpoints) {}

void iSaxUlisseEnvelopeIndex::insert(const vec<UlisseEnvelope> &envelopes, FilePositionT file_pos) {
    size_t num_symbols = envelopes.size() * envelopes[0].first.size();
    vec<float> env_min, env_max;

    for (auto &[channel_min, channel_max] : envelopes) {
        env_min.insert(env_min.end(), channel_min.begin(), channel_min.end());
        env_max.insert(env_max.end(), channel_max.begin(), channel_max.end());
    }

    SaxWord sax_min(env_min, m_first_layer_num_bits, m_first_layer_breakpoints);

    auto node_it = m_first_layer.find(sax_min);
    if (node_it == m_first_layer.end()) {
        m_first_layer.emplace(sax_min, std::make_unique<iSaxSplittableLeaf>(vec<FilePositionT>{file_pos},
                                                                            vec<UlisseEnvelope>{{env_min, env_max}}));
    } else {
        iSaxWord isax_min(env_min, m_first_layer_num_bits, m_breakpoints);

        auto node = node_it->second.get();
        iSaxInternalNode *parent = nullptr;
        uint8_t new_bit = 0;
        // Traverse tree until a leaf is reached
        while (!(node->is_leaf())) {
            new_bit = isax_min.apply_split(env_min, m_breakpoints, node->get_split_ind());
            node = const_cast<iSaxNode *>(new_bit ? node->get_children().second : node->get_children().first);
        }
        // Reached a leaf => insert
        auto *leaf = static_cast<iSaxSplittableLeaf *>(node);
        leaf->m_file_positions.push_back(file_pos);
        leaf->m_envelopes.push_back({env_min, env_max});

        // Check if split is needed
        if (leaf->m_file_positions.size() > m_leaf_capacity) {
            // Split the leaf
            SaxSplitIndT split_ind = m_split_strategy->get_split_ind();

            // Find the breakpoint in the middle of the symbol at the split index
            SaxNumBitsT num_bits = isax_min.get_num_bits(split_ind);
            SaxSymbolT breakpoint_alphabet_size = (m_breakpoints.size() + 1);
            SaxSymbolT alphabet_size_ratio = breakpoint_alphabet_size / (1 << num_bits);

            if (alphabet_size_ratio == 1) {
                m_breakpoints = m_breakpoint_strategy->get_breakpoints(breakpoint_alphabet_size * 2);
                alphabet_size_ratio = 2;
            }
            auto mid_breakpoint = m_breakpoints[(isax_min[split_ind] * 2 + 1) * (alphabet_size_ratio >> 1) - 1];

            // Distribute the envelopes across the two new leaves
            vec<FilePositionT> left_file_positions, right_file_positions;
            vec<UlisseEnvelope> left_envelopes, right_envelopes;

            for (size_t i = 0; i < leaf->m_file_positions.size(); ++i) {
                auto &channel_min = leaf->m_envelopes[i].first;
                if (channel_min[split_ind] < mid_breakpoint) {
                    left_file_positions.push_back(leaf->m_file_positions[i]);
                    left_envelopes.push_back(leaf->m_envelopes[i]);
                } else {
                    right_file_positions.push_back(leaf->m_file_positions[i]);
                    right_envelopes.push_back(leaf->m_envelopes[i]);
                }
            }

            // Create new nodes
            auto new_internal = std::make_unique<iSaxInternalNode>(split_ind);
            new_internal->left =
                std::make_unique<iSaxSplittableLeaf>(std::move(left_file_positions), std::move(left_envelopes));
            new_internal->right =
                std::make_unique<iSaxSplittableLeaf>(std::move(right_file_positions), std::move(right_envelopes));

            // Replace the leaf with the new internal node
            auto &node_ref = parent ? (new_bit ? parent->right : parent->left) : node_it->second;
            node_ref = std::move(new_internal);
        }
    }
}

vec<FilePositionT> iSaxUlisseEnvelopeIndex::search(vec<vec<float>> mts, const SearchOptions &search_options) const {
    return {};
};

const iSaxNode *iSaxUlisseEnvelopeIndex::get_first_layer_node(const SaxWord &sax_min) const {
    auto node_it = m_first_layer.find(sax_min);
    return node_it == m_first_layer.end() ? nullptr : node_it->second.get();
}
