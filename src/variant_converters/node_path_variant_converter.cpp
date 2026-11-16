#include "node_path_variant_converter.hpp"
#include "../exception.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include "../error_handling.hpp"

using namespace godot;

void YAMLNodePathVariantConverter::encode(ryml::NodeRef &node, const Variant &v, const YAMLStyle::View &style) const {
	const NodePath path = v.operator NodePath();

	style.apply_string_style(node);
	const CharString utf8 = String(path).utf8();
	node.save(c4::csubstr(utf8.get_data(), utf8.length()));
}

Variant YAMLNodePathVariantConverter::decode(const ryml::ConstNodeRef &node, YAMLParserContext *context) const {
	YAML_TRY {
		if (!node.has_val()) {
			YAML_THROW(create_invalid_format_exception(node));
		}

		if (node.val_is_null()) {
			return NodePath(); // Return empty NodePath
		}

		if (context->detect_style) {
			YAMLStyle::detect_string_style(node, context->current_style());
		}

		return NodePath(from_ryml_str(node.val()));
	} YAML_CATCH(const YAMLException &) {
		YAML_RETHROW; // Re-throw YAML exceptions
	} YAML_CATCH_VAR(const std::exception &, e) {
		YAML_THROW(create_decode_error_exception(e.what(), node));
	}
}
