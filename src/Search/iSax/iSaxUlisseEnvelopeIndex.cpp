#include "Search/iSax/iSaxUlisseEnvelopeIndex.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Summarization/iSaxWord.hpp"

#include <iostream>

iSaxUlisseEnvelopeIndex::iSaxUlisseEnvelopeIndex(SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                                                 std::unique_ptr<IiSaxBreakpointStrategy> breakpoint_strategy,
                                                 std::unique_ptr<IiSaxSplitStrategy> split_strategy)
    : m_first_layer_num_bits(first_layer_num_bits),
      m_alphabet_num_bits(first_layer_num_bits),
      m_leaf_capacity(leaf_capacity),
      m_breakpoint_strategy(std::move(breakpoint_strategy)),
      m_split_strategy(std::move(split_strategy)),
      m_breakpoints(m_breakpoint_strategy->get_breakpoints(1 << m_alphabet_num_bits)) {}

void iSaxUlisseEnvelopeIndex::split_leaf(iSaxWord &isax_min, vec<float> &env_min, std::unique_ptr<iSaxNode> &node_ref) {
    // Split the leaf
    SaxSplitIndT split_ind = m_split_strategy->get_split_ind();
    SaxNumBitsT split_seg_bits = isax_min.get_num_bits()[split_ind];
    // If cannot split further, return
    if (split_seg_bits == MAX_SYMBOL_BITS) return;

    // // The alphabet size was doubled, update the iSAX word
    // isax_min = iSaxWord(env_min, isax_min.get_num_bits(), m_alphabet_num_bits, m_breakpoints);
    // isax_update_required = false;

    // Find the breakpoint in the middle of the symbol at the split index
    SaxSymbolT breakpoint_alphabet_size = (m_breakpoints.size() + 1);
    SaxSymbolT alphabet_size_ratio = breakpoint_alphabet_size / (1 << split_seg_bits);

    bool isax_update_required = false;
    // If the segment already used the full alphabet, double the alphabet size
    // Do not update the iSAX word yet, it may not be necessary
    if (alphabet_size_ratio == 1) {
        m_breakpoints = m_breakpoint_strategy->get_breakpoints(breakpoint_alphabet_size * 2);
        alphabet_size_ratio = 2;
        ++m_alphabet_num_bits;
        isax_update_required = true;
    }
    // `isax_min[split_ind] * 2 + 1` goes to the upper interval in the next resolution
    // `* (alphabet_size_ratio >> 1)` goes to the lowest portion of the upper interval
    // (i.e. just above the mid breakpoint) in the desired resolution
    // `-1` adjusts for the fact that the breakpoints have an implicit -inf at the beginning
    auto mid_breakpoint = m_breakpoints[(isax_min[split_ind] * 2 + 1) * (alphabet_size_ratio >> 1) - 1];

    // Distribute the envelopes across the two new leaves
    vec<FilePositionT> left_file_positions, right_file_positions;
    vec<UlisseEnvelope> left_envelopes, right_envelopes;

    auto *leaf = static_cast<iSaxSplittableLeaf *>(node_ref.get());
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
    size_t left_size = left_file_positions.size(), right_size = right_file_positions.size();
    auto new_internal = std::make_unique<iSaxInternalNode>(split_ind);
    new_internal->left =
        std::make_unique<iSaxSplittableLeaf>(std::move(left_file_positions), std::move(left_envelopes));
    new_internal->right =
        std::make_unique<iSaxSplittableLeaf>(std::move(right_file_positions), std::move(right_envelopes));

    // Replace the leaf with the new internal node
    // auto &node_ref = parent ? (new_bit ? parent->right : parent->left) : node_it->second;
    node_ref = std::move(new_internal);

    // Check if further splitting is necessary
    bool split_left = left_size > m_leaf_capacity, split_right = right_size > m_leaf_capacity;
    if (split_left || split_right) {
        if (isax_update_required)
            isax_min = iSaxWord(env_min, isax_min.get_num_bits(), m_alphabet_num_bits, m_breakpoints);

        auto &parent = reinterpret_cast<std::unique_ptr<iSaxInternalNode> &>(node_ref);

        if (split_left) {
            uint8_t new_bit = 0;
            isax_min.append_to_symbol(split_ind, new_bit);
            leaf = static_cast<iSaxSplittableLeaf *>(parent->left.get());
            split_leaf(isax_min, env_min, parent->left);
            isax_min.remove_from_symbol(split_ind);
        }
        if (split_right) {
            uint8_t new_bit = 1;
            isax_min.append_to_symbol(split_ind, new_bit);
            leaf = static_cast<iSaxSplittableLeaf *>(parent->right.get());
            split_leaf(isax_min, env_min, parent->right);
            isax_min.remove_from_symbol(split_ind);
        }
    }
}

void iSaxUlisseEnvelopeIndex::insert(const vec<UlisseEnvelope> &envelopes, FilePositionT file_pos) {
    size_t num_symbols = envelopes.size() * envelopes[0].first.size();
    vec<float> env_min, env_max;

    for (auto &[channel_min, channel_max] : envelopes) {
        env_min.insert(env_min.end(), channel_min.begin(), channel_min.end());
        env_max.insert(env_max.end(), channel_max.begin(), channel_max.end());
    }

    iSaxWord isax_min(env_min, vec<SaxNumBitsT>(env_min.size(), m_first_layer_num_bits), m_alphabet_num_bits,
                      m_breakpoints);

    auto node_it = m_first_layer.find(isax_min);
    if (node_it == m_first_layer.end()) {
        m_first_layer.emplace(isax_min, std::make_unique<iSaxSplittableLeaf>(vec<FilePositionT>{file_pos},
                                                                             vec<UlisseEnvelope>{{env_min, env_max}}));
    } else {
        auto node = node_it->second.get();
        iSaxInternalNode *parent = nullptr;
        uint8_t new_bit = 0;
        // Traverse tree until a leaf is reached
        while (!(node->is_leaf())) {
            new_bit = isax_min.apply_split(node->get_split_ind());
            node = const_cast<iSaxNode *>(new_bit ? node->get_children().second : node->get_children().first);
        }
        // Reached a leaf => insert
        auto *leaf = static_cast<iSaxSplittableLeaf *>(node);
        leaf->m_file_positions.push_back(file_pos);
        leaf->m_envelopes.push_back({env_min, env_max});

        // Split if needed
        if (leaf->m_file_positions.size() > m_leaf_capacity) {
            auto &node_ref = parent ? (new_bit ? parent->right : parent->left) : node_it->second;
            split_leaf(isax_min, env_min, node_ref);
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
