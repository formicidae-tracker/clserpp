#pragma once

#include "clser.h"
#include "fort/clserpp/buffer.hpp"
#include <cpptrace/exceptions.hpp>
#include <optional>
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
	      )
	    , d_code{code} {}

	int32_t code() const noexcept {
		return d_code;
	}

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

	int32_t d_code;
};

template <typename Fnct, typename... Args>
void call(Fnct &&fnct, Args &&...args) {
	int32_t res = std::forward<Fnct>(fnct)(std::forward<Args>(args)...);
	if (res != 0) {
		throw clserException(res);
	}
}

inline const char *version_name(clVersion_e e) {
	switch (e) {
	case CL_VERSION_NONE:
		return "CL_VERSION_NONE";
	case CL_VERSION_1_0:
		return "CL_VERSION_1_0";
	case CL_VERSION_1_1:
		return "CL_VERSION_1_1";
	default:
		throw cpptrace::out_of_range(
		    "Unknown baudrate value " + std::to_string(int(e))
		);
	}
}

inline const char *baudrate_name(clBaudrate_e e) {
	switch (e) {
	case CL_BAUDRATE_9600:
		return " CL_BAUDRATE_9600";
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
		throw cpptrace::out_of_range(
		    "Unknown baudrate value " + std::to_string(int(e))
		);
	}
}

inline std::optional<clBaudrate_e> baudrate_cast(const std::string &bd) {
	if (bd == "CL_BAUDRATE_9600") {
		return CL_BAUDRATE_9600;
	}
	if (bd == "CL_BAUDRATE_19200") {
		return CL_BAUDRATE_19200;
	}
	if (bd == "CL_BAUDRATE_38400") {
		return CL_BAUDRATE_38400;
	}
	if (bd == "CL_BAUDRATE_57600") {
		return CL_BAUDRATE_57600;
	}
	if (bd == "CL_BAUDRATE_115200") {
		return CL_BAUDRATE_115200;
	}
	if (bd == "CL_BAUDRATE_230400") {
		return CL_BAUDRATE_230400;
	}
	if (bd == "CL_BAUDRATE_460800") {
		return CL_BAUDRATE_460800;
	}
	if (bd == "CL_BAUDRATE_921600") {
		return CL_BAUDRATE_921600;
	}
	return std::nullopt;
}

inline std::optional<LineTermination> termination_cast(const std::string &s) {
	if (s == "none") {
		return LineTermination::NONE;
	}
	if (s == "lf") {
		return LineTermination::LF;
	}
	if (s == "cr") {
		return LineTermination::CR;
	}
	if (s == "crlf") {
		return LineTermination::CRLF;
	}
	if (s == "null") {
		return LineTermination::NULLCHAR;
	}
	return std::nullopt;
}

} // namespace details
} // namespace clserpp
} // namespace fort
