#pragma once

#include "clser.h"
#include <cpptrace/exceptions.hpp>
#include <string>
#include <utility>

#include <cpptrace/cpptrace.hpp>

namespace fort {
namespace clserpp {
namespace details {

template <typename Fnct, typename... Args>
void call(Fnct &&fnct, Args &&...args) {
	int32_t res = std::forward<Fnct>(fnct)(std::forward<Args>(args)...);
	if (res != 0) {
		char     buffer[2000];
		uint32_t size     = 2000;
		int32_t  errorRes = clGetErrorText(res, buffer, &size);
		if (errorRes != 0) {
			throw cpptrace::logic_error(
			    "clser error (" + std::to_string(res) +
			    "): could not get error text: error " + std::to_string(errorRes)
			);
		}
		throw cpptrace::runtime_error("clser error: " + std::string(buffer));
	}
}

inline const char *version_name(clVersion_e version) {
	switch (version) {
	case CL_VERSION_NONE:
		return "Invalid";
	case CL_VERSION_1_0:
		return "CL v1.0";
	case CL_VERSION_1_1:
		return "CL v1.1";
	default:
		return "Unknown version";
	}
}

} // namespace details
} // namespace clserpp
} // namespace fort
