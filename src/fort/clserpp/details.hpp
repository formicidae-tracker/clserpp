#pragma once

#include "clser.h"
#include <cpptrace/exceptions.hpp>
#include <string>
#include <utility>

#include <cpptrace/cpptrace.hpp>

namespace fort {
namespace clserpp {
namespace details {

class clserException : public cpptrace::runtime_error {

public:
	clserException(int32_t code) noexcept
	    : cpptrace::runtime_error(
	          "clser error (" + std::to_string(code) +
	          "): " + getErrorText(code)
	      ) {}

private:
	std::string getErrorText(int32_t code) {
		char     buffer[2000];
		uint32_t size     = 2000;
		int32_t  errorRes = clGetErrorText(code, buffer, &size);
		if (errorRes != 0) {
			return "could not get error text: error " +
			       std::to_string(errorRes);
		}
		return buffer;
	}
};

template <typename Fnct, typename... Args>
void call(Fnct &&fnct, Args &&...args) {
	int32_t res = std::forward<Fnct>(fnct)(std::forward<Args>(args)...);
	if (res != 0) {
		throw clserException(res);
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
		return "<unknown CL version>";
	}
}

inline const char *baudrate_name(clBaudrate_e r) {
	switch (r) {
	case CL_BAUDRATE_9600:
		return "CL_BAUDRATE_9600";
	case CL_BAUDRATE_19200:
		return "CL_BAUDRATE_19200";
	case CL_BAUDRATE_38400:
		return "CL_BAUDRATE_38400";
	case CL_BAUDRATE_57600:
		return "CL_BAUDRATE_57600";
	case CL_BAUDRATE_115200:
		return "CL_BAUDRATE_115200";
	case CL_BAUDRATE_230400:
		return "CL_BAUDRATE_230400";
	case CL_BAUDRATE_460800:
		return "CL_BAUDRATE_460800";
	case CL_BAUDRATE_921600:
		return "CL_BAUDRATE_921600";
	default:
		return "<unknown CL baudrate>";
	}
}

} // namespace details
} // namespace clserpp
} // namespace fort
