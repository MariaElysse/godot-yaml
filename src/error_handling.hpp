/**
 * @file error_handling.hpp
 * @brief Error reporting that works with and without C++ exception support.
 *
 * Godot's web runtime is built without exception handling: its main module
 * exports no __cpp_exception tag and its JS runtime only provides aborting
 * stubs for the JS exception helpers. A web side module that uses C++
 * exceptions therefore cannot even be instantiated by the engine
 * ("import object field '__cpp_exception' is not a Tag"), and emscripten
 * rejects JS-based exceptions together with the wasm longjmp mode that
 * godot-cpp turns on for web builds.
 *
 * Web builds consequently compile this extension with -fno-exceptions and
 * report errors through the channel implemented here. The macros below keep a
 * single set of sources for both variants:
 *
 * - YAML_TRY, YAML_CATCH, YAML_CATCH_VAR and YAML_CATCH_ANY map onto the
 *   equivalent C++ exception constructs, or onto blocks that are never entered.
 * - YAML_THROW and YAML_THROW_VOID report an error to the channel and return
 *   from the enclosing function instead of throwing it.
 *
 * Native builds keep using C++ exceptions, so the channel stays empty there.
 */
#pragma once

#include "exception.hpp"

#include <ryml.hpp>

#include <string>

namespace godot {

/**
 * @class YAMLErrorFallback
 * @brief Value returned by a function that failed in a build without exceptions.
 *
 * The class converts to any type the enclosing function returns, which lets
 * the error reporting macros return a default value without having to know
 * the return type of the function they are used in.
 */
class YAMLErrorFallback {
public:
	/**
	 * @brief Converts the fallback to the return type of the enclosing function.
	 *
	 * @tparam T The return type of the enclosing function
	 * @return T A default constructed value
	 */
	template <typename T>
	operator T() const { return T(); }
};

/**
 * @class YAMLErrorChannel
 * @brief Holds the error of the operation that is currently running.
 *
 * The channel is only used in builds without C++ exception support, where it
 * replaces throwing an exception. The first reported error wins, so that the
 * error closest to the cause is reported instead of a follow-up error caused
 * by the failed operation.
 */
class YAMLErrorChannel {
public:
	/**
	 * @brief Reports an error to the channel.
	 *
	 * Errors that are already reported are kept, so the first error is the one
	 * that gets reported.
	 *
	 * @param p_error The error to report
	 */
	static void report(const YAMLException &p_error);

	/**
	 * @brief Checks whether an error has been reported.
	 *
	 * @return bool True if an error has been reported
	 */
	static bool has_error();

	/**
	 * @brief Gets the message of the reported error.
	 *
	 * @return const std::string & The error message
	 */
	static const std::string &message();

	/**
	 * @brief Gets the location of the reported error.
	 *
	 * @return ryml::Location The location in the source document
	 */
	static ryml::Location location();

	/**
	 * @brief Discards the reported error.
	 */
	static void clear();

	/**
	 * @brief Reports an error and returns a value to return from the failed function.
	 *
	 * @param p_error The error to report
	 * @return YAMLErrorFallback A value that converts to the return type of the calling function
	 */
	static YAMLErrorFallback fail(const YAMLException &p_error);

	/**
	 * @brief Gets a value to return from a function that failed.
	 *
	 * @return YAMLErrorFallback A value that converts to the return type of the calling function
	 */
	static YAMLErrorFallback fallback();

	/**
	 * @brief Gets an exception to bind the handlers of a catch clause to.
	 *
	 * The catch handlers of a build without exception support are never
	 * entered, but they still have to compile. The returned exception is
	 * deliberately never destroyed, because the handlers hold a reference to
	 * it for the rest of the program.
	 *
	 * @return const YAMLException & An exception that is never thrown
	 */
	static const YAMLException &unreachable_exception();
};

} // namespace godot

#if defined(__cpp_exceptions)

/**
 * @brief Starts a block that catches errors.
 */
#define YAML_TRY try

/**
 * @brief Catches errors of a given type.
 *
 * @param type The type of the exception to catch
 */
#define YAML_CATCH(type) catch (type)

/**
 * @brief Catches errors of a given type and binds them to a name.
 *
 * @param type The type of the exception to catch
 * @param name The name to bind the caught exception to
 */
#define YAML_CATCH_VAR(type, name) catch (type name)

/**
 * @brief Catches errors of any type.
 */
#define YAML_CATCH_ANY catch (...)

/**
 * @brief Throws an error.
 *
 * @param exception The exception to throw
 */
#define YAML_THROW(exception) throw exception

/**
 * @brief Throws an error from a function that returns nothing.
 *
 * @param exception The exception to throw
 */
#define YAML_THROW_VOID(exception) throw exception

/**
 * @brief Rethrows the error that is currently handled.
 */
#define YAML_RETHROW throw

#else

/**
 * @brief Starts a block that catches errors.
 */
#define YAML_TRY

/**
 * @brief Catches errors of a given type.
 *
 * @param type The type of the exception to catch
 */
#define YAML_CATCH(type) if (false)

/**
 * @brief Catches errors of a given type and binds them to a name.
 *
 * @param type The type of the exception to catch
 * @param name The name to bind the caught exception to
 */
#define YAML_CATCH_VAR(type, name) type name = godot::YAMLErrorChannel::unreachable_exception(); if (false)

/**
 * @brief Catches errors of any type.
 */
#define YAML_CATCH_ANY if (false)

/**
 * @brief Reports an error and returns from the enclosing function.
 *
 * @param exception The exception to report
 */
#define YAML_THROW(exception) return godot::YAMLErrorChannel::fail(exception)

/**
 * @brief Reports an error and returns from the enclosing function.
 *
 * @param exception The exception to report
 */
#define YAML_THROW_VOID(exception) \
	do {                            \
		godot::YAMLErrorChannel::report(exception); \
		return;                    \
	} while (false)

/**
 * @brief Rethrows the error that is currently handled.
 */
#define YAML_RETHROW (void)0

#endif
