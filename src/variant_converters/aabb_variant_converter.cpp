#include "aabb_variant_converter.hpp"
#include "../exception.hpp"
#include "../util/ryml_flags.hpp"
#include "variant_converter_factory.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include "../error_handling.hpp"

namespace godot {

YAMLAABBVariantConverter::YAMLAABBVariantConverter(YAMLVariantConverterFactory *factory) :
		vec3_converter(factory->create_converter_as<YAMLVector3VariantConverter>(Variant::VECTOR3)) {
	ERR_FAIL_NULL(vec3_converter);
}

void YAMLAABBVariantConverter::encode(ryml::NodeRef &node, const Variant &v, const YAMLStyle::View &style) const {
	const AABB aabb = AABB(v);

	style.apply_flow_style(node);

	ryml::NodeRef position_node;
	ryml::NodeRef size_node;

	if (!style.is_valid() || style.get_container_form() != YAMLStyle::FORM_ARRAY) {
		add_node_flags(node, ryml::MAP);
		position_node = node["position"];
		size_node = node["size"];
	} else {
		add_node_flags(node, ryml::SEQ);
		position_node = node.append_child();
		size_node = node.append_child();
	}

	auto scalar_style = style.get_scalar_view();
	vec3_converter->encode(position_node, aabb.position, style.has_child("position") ? style.get_child("position") : scalar_style);
	vec3_converter->encode(size_node, aabb.size, style.has_child("size") ? style.get_child("size") : scalar_style);
}

Variant YAMLAABBVariantConverter::decode(const ryml::ConstNodeRef &node, YAMLParserContext *context) const {
	YAML_TRY {
		if (node.is_map()) {
			return decode_from_map(node, context);
		}

		if (node.is_seq()) {
			return decode_from_sequence(node, context);
		}

		YAML_THROW(create_invalid_format_exception(node));
	} YAML_CATCH(const YAMLException &) {
		YAML_RETHROW; // Re-throw YAML exceptions
	} YAML_CATCH_VAR(const std::exception &, e) {
		YAML_THROW(create_decode_error_exception(e.what(), node));
	}
}

Variant YAMLAABBVariantConverter::decode_from_map(const ryml::ConstNodeRef &node, YAMLParserContext *context) const {
	if (!check_required_fields(node, { "position", "size" })) {
		return YAMLErrorChannel::fallback();
	}

	const bool detect_style = context->detect_style;

	if (detect_style) {
		Ref<YAMLStyle> style = context->current_style();
		YAMLStyle::detect_flow_style(node, style);
		style->set_container_form(YAMLStyle::FORM_DICTIONARY);

		context->push_style("position");
	}

	Vector3 position = vec3_converter->decode(node["position"], context).operator Vector3();

	if (detect_style) {
		context->pop_style();
		context->push_style("size");
	}

	Vector3 size = vec3_converter->decode(node["size"], context).operator Vector3();

	if (detect_style) {
		context->pop_style();
	}

	return AABB(position, size);
}

Variant YAMLAABBVariantConverter::decode_from_sequence(const ryml::ConstNodeRef &node, YAMLParserContext *context) const {
	if (node.num_children() != 2) {
		YAML_THROW(create_invalid_sequence_length_exception(2, node));
	}

	const bool detect_style = context->detect_style;

	if (detect_style) {
		Ref<YAMLStyle> style = context->current_style();
		YAMLStyle::detect_flow_style(node, style);
		style->set_container_form(YAMLStyle::FORM_ARRAY);

		context->push_style("position");
	}

	Vector3 position = vec3_converter->decode(node[0], context).operator Vector3();

	if (detect_style) {
		context->pop_style();
		context->push_style("size");
	}

	Vector3 size = vec3_converter->decode(node[1], context).operator Vector3();

	if (detect_style) {
		context->pop_style();
	}

	return AABB(position, size);
}

} // namespace godot
