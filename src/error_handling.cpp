#include "error_handling.hpp"

namespace godot {

namespace {

/**
 * @brief State of the error channel.
 *
 * The state is kept in plain C++ types, so that it stays valid for the whole
 * lifetime of the program, independently of the Godot runtime.
 */
struct ErrorChannelState {
	bool has_error = false;
	std::string message;
	ryml::Location location;
};

ErrorChannelState &channel_state() {
	static ErrorChannelState state;
	return state;
}

} // namespace

void YAMLErrorChannel::report(const YAMLException &p_error) {
	ErrorChannelState &state = channel_state();
	if (state.has_error) {
		return;
	}

	state.has_error = true;
	state.message = p_error.what();
	state.location = p_error.get_location();
}

bool YAMLErrorChannel::has_error() {
	return channel_state().has_error;
}

const std::string &YAMLErrorChannel::message() {
	return channel_state().message;
}

ryml::Location YAMLErrorChannel::location() {
	return channel_state().location;
}

void YAMLErrorChannel::clear() {
	ErrorChannelState &state = channel_state();
	state.has_error = false;
	state.message.clear();
	state.location = ryml::Location();
}

YAMLErrorFallback YAMLErrorChannel::fail(const YAMLException &p_error) {
	report(p_error);
	return YAMLErrorFallback();
}

YAMLErrorFallback YAMLErrorChannel::fallback() {
	return YAMLErrorFallback();
}

const YAMLException &YAMLErrorChannel::unreachable_exception() {
	// Deliberately never destroyed: the catch handlers of a build without
	// exception support bind a reference to it that has to stay valid for the
	// rest of the program.
	static const YAMLException *placeholder = new YAMLException("YAML error");
	return *placeholder;
}

} // namespace godot
