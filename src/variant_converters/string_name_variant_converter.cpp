#include "string_name_variant_converter.hpp"
#include "../exception.hpp"
#include "../error_handling.hpp"

using namespace godot;

void YAMLStringNameVariantConverter::encode(ryml::NodeRef &node, const Variant &v, const YAMLStyle::View &style) const {
	const String str = v.operator String();

	style.apply_string_style(node);
	const CharString utf8 = str.utf8();
	node.save(c4::csubstr(utf8.get_data(), utf8.length()));
}

Variant YAMLStringNameVariantConverter::decode(const ryml::ConstNodeRef &node, YAMLParserContext *context) const {
	YAML_TRY {
		if (!node.has_val()) {
			YAML_THROW(create_invalid_format_exception(node));
		}

		if (node.val_is_null()) {
			return StringName(); // Return empty StringName
		}

		if (context->detect_style) {
			YAMLStyle::detect_string_style(node, context->current_style());
		}

		return StringName(from_ryml_str(node.val()));
	} YAML_CATCH(const YAMLException &) {
		YAML_RETHROW; // Re-throw YAML exceptions
	} YAML_CATCH_VAR(const std::exception &, e) {
		YAML_THROW(create_decode_error_exception(e.what(), node));
	}
}
