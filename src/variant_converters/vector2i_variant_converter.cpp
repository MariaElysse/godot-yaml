#include "vector2i_variant_converter.hpp"
#include "../exception.hpp"
#include "../util/ryml_flags.hpp"
#include "../util/util_numeric.hpp"
#include "../error_handling.hpp"

using namespace godot;

void YAMLVector2iVariantConverter::encode(ryml::NodeRef &node, const Variant &v, const YAMLStyle::View &style) const {
	const Vector2i vec = v.operator Vector2i();

	if (style.is_valid()) {
		style.apply_flow_style(node);
	} else {
		add_node_flags(node, ryml::FLOW_SL);
	}

	ryml::NodeRef x_node;
	ryml::NodeRef y_node;

	if (!style.is_valid() || style.get_container_form() != YAMLStyle::FORM_ARRAY) {
		add_node_flags(node, ryml::MAP);
		x_node = node["x"];
		y_node = node["y"];
	} else {
		add_node_flags(node, ryml::SEQ);
		x_node = node.append_child();
		y_node = node.append_child();
	}

	x_node.save(int_to_string(vec.x, style.has_child("x") ? style.get_child("x").get_integer_format() : style.get_integer_format()));
	y_node.save(int_to_string(vec.y, style.has_child("y") ? style.get_child("y").get_integer_format() : style.get_integer_format()));
}

Variant YAMLVector2iVariantConverter::decode(const ryml::ConstNodeRef &node, YAMLParserContext *context) const {
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

Vector2i YAMLVector2iVariantConverter::decode_from_map(const ryml::ConstNodeRef &node, YAMLParserContext *context) const {
	if (!check_required_fields(node, { "x", "y" })) {
		return YAMLErrorChannel::fallback();
	}

	const bool detect_style = context->detect_style;

	if (detect_style) {
		Ref<YAMLStyle> style = context->current_style();
		YAMLStyle::detect_flow_style(node, style);
		style->set_container_form(YAMLStyle::FORM_DICTIONARY);

		context->push_style("x");
	}

	YAMLStyle::IntegerFormat x_format;
	int32_t x = string_to_int<int32_t>(node[0].val(), detect_style ? &x_format : nullptr);

	if (detect_style) {
		context->current_style()->set_integer_format(x_format);
		context->pop_style();

		context->push_style("y");
	}

	YAMLStyle::IntegerFormat y_format;
	int32_t y = string_to_int<int32_t>(node["y"].val(), detect_style ? &y_format : nullptr);

	if (detect_style) {
		context->current_style()->set_integer_format(y_format);
		context->pop_style();
	}

	return Vector2i(x, y);
}

Vector2i YAMLVector2iVariantConverter::decode_from_sequence(const ryml::ConstNodeRef &node, YAMLParserContext *context) const {
	if (node.num_children() != 2) {
		YAML_THROW(create_invalid_sequence_length_exception(2, node));
	}

	const bool detect_style = context->detect_style;

	if (detect_style) {
		Ref<YAMLStyle> style = context->current_style();
		YAMLStyle::detect_flow_style(node, style);
		style->set_container_form(YAMLStyle::FORM_ARRAY);

		context->push_style("x");
	}

	YAMLStyle::IntegerFormat x_format;
	int32_t x = string_to_int<int32_t>(node[0].val(), detect_style ? &x_format : nullptr);

	if (detect_style) {
		context->current_style()->set_integer_format(x_format);
		context->pop_style();

		context->push_style("y");
	}

	YAMLStyle::IntegerFormat y_format;
	int32_t y = string_to_int<int32_t>(node[1].val(), detect_style ? &y_format : nullptr);

	if (detect_style) {
		context->current_style()->set_integer_format(y_format);
		context->pop_style();
	}

	return Vector2i(x, y);
}
