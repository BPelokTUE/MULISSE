#include "Search/iSax/iSaxUlisseEnvelopeIndex.hpp"
#include "Search/iSax/iSaxNode.hpp"
#include "Summarization/iSaxWord.hpp"

iSaxUlisseEnvelopeIndex::iSaxUlisseEnvelopeIndex(SaxNumBitsT first_layer_num_bits, size_t leaf_capacity,
                                                 std::unique_ptr<IiSaxBreakpointStrategy> breakpoint_strategy,
                                                 std::unique_ptr<IiSaxSplitStrategy> split_strategy)
    : m_first_layer_num_bits(first_layer_num_bits),
      m_leaf_capacity(leaf_capacity),
      m_breakpoint_strategy(std::move(breakpoint_strategy)),
      m_split_strategy(std::move(split_strategy)),
      m_breakpoints(m_breakpoint_strategy->get_breakpoints(1 << m_first_layer_num_bits)) {}

void iSaxUlisseEnvelopeIndex::insert(const vec<UlisseEnvelope> &envelopes, FilePositionT file_pos) {
    size_t num_symbols = envelopes.size() * envelopes[0].first.size();
    vec<float> env_min(num_symbols), env_max(num_symbols);

    for (auto &[channel_min, channel_max] : envelopes) {
        env_min.insert(env_min.end(), channel_min.begin(), channel_min.end());
        env_max.insert(env_max.end(), channel_max.begin(), channel_max.end());
    }

    iSaxWord isax_min(env_min, m_first_layer_num_bits, m_breakpoints),
        isax_max(env_max, m_first_layer_num_bits, m_breakpoints);

    auto node_it = m_first_layer.find(isax_min);
    if (node_it == m_first_layer.end()) {
        m_first_layer.emplace(isax_min,
                              std::make_unique<iSaxNode>(iSaxSplittableLeaf({file_pos}, {{env_min, env_max}})));
    } else {
        iSaxNode *node = node_it->second.get();
        iSaxInternalNode *parent = nullptr;
        uint8_t new_bit = 0;
        // Traverse tree until a leaf is reached
        while (auto *internal = dynamic_cast<iSaxInternalNode *>(node)) {
            SaxSplitIndT split_ind = internal->m_split_ind;
            new_bit = isax_min.apply_split(env_min, m_breakpoints, split_ind);
            isax_max.apply_split(env_max, m_breakpoints, split_ind);

            parent = internal;
            node = new_bit ? internal->right.get() : internal->left.get();
        }
        // Reached a leaf => insert and check if split is needed
        auto *leaf = dynamic_cast<iSaxSplittableLeaf *>(node);
        if (leaf->m_file_positions.size() < m_leaf_capacity) {
            leaf->m_file_positions.push_back(file_pos);
        } else {
            // Split the leaf
            SaxSplitIndT split_ind = m_split_strategy->get_split_ind();

            // Find the breakpoint in the middle of the symbol at the split index
            SaxNumBitsT num_bits = isax_min.get_num_bits(split_ind);
            SaxSymbolT breakpoint_alphabet_size = (m_breakpoints.size() - 1);
            SaxSymbolT alphabet_size_ratio = breakpoint_alphabet_size / (1 << num_bits);

            if (alphabet_size_ratio == 1) {
                m_breakpoints = m_breakpoint_strategy->get_breakpoints(breakpoint_alphabet_size * 2);
                alphabet_size_ratio = 2;
            }
            auto mid_breakpoint = m_breakpoints[(isax_min[split_ind] * 2 + 1) * (alphabet_size_ratio >> 1)];

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
            iSaxInternalNode *new_internal = new iSaxInternalNode(split_ind);
            // Reuse the old leaf for the left child
            leaf->m_file_positions = std::move(left_file_positions);
            leaf->m_envelopes = std::move(left_envelopes);
            new_internal->left = std::unique_ptr<iSaxNode>(leaf);
            // Create a new leaf for the right child
            new_internal->right = std::make_unique<iSaxNode>(
                iSaxSplittableLeaf(std::move(right_file_positions), std::move(right_envelopes)));

            // Cast to iSaxNode
            auto new_internal_as_base = std::unique_ptr<iSaxNode>(new_internal);
            // Insert the new node into the tree
            if (parent) {
                if (new_bit) {
                    parent->right = std::move(new_internal_as_base);
                } else {
                    parent->left = std::move(new_internal_as_base);
                }
            } else {
                m_first_layer[isax_min] = std::move(new_internal_as_base);
            }
        }
    }
}

vec<FilePositionT> iSaxUlisseEnvelopeIndex::search(vec<vec<float>> mts, const SearchOptions &search_options) const {
    return {};
};
