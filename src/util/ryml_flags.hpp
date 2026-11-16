/**
 * @file ryml_flags.hpp
 * @brief Helpers for setting rapidyaml node types without the deprecated operators.
 *
 * rapidyaml 0.16 deprecated `NodeRef::operator|=` in favour of the
 * `set_map()`/`set_seq()`/`set_*_style()` setters. Those setters assert that the
 * node already carries the corresponding part (a container or a value), but the
 * emitter applies style flags before the node type and the value are known, so
 * the flags have to be added verbatim.
 */
#pragma once

#include <ryml.hpp>

namespace godot {

/**
 * @brief Creates the node if needed and adds the given node type flags.
 *
 * Equivalent to the deprecated `node |= flags`, including the implicit
 * creation of a seeded node (one obtained from a map lookup).
 *
 * @param node The node to modify
 * @param flags The node type flags to add
 */
inline void add_node_flags(ryml::NodeRef &node, ryml::NodeType flags) {
	node.create();
	node._add_flags(flags);
}

} // namespace godot