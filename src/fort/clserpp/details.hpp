#pragma once

#include "clser.h"

#include "types.hpp"

#include <algorithm>
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

inline const char *escape_char(char c) {
	struct EscapableChar {
	public:
		char        value;
		const char *escaped;
	};

	const static std::array<EscapableChar, 4> escapables = {
	    EscapableChar{.value = '\r', .escaped = "\\r"},
	    EscapableChar{.value = '\n', .escaped = "\\n"},
	    EscapableChar{.value = '\\', .escaped = R"(\)"},
	    EscapableChar{.value = '\t', .escaped = "\\t"},
	};

	auto pos = std::find_if(
	    escapables.begin(),
	    escapables.end(),
	    [v = c](const EscapableChar &c) { return c.value == v; }
	);

	if (pos != escapables.end()) {
		return pos->escaped;
	}
	return nullptr;
}

inline std::string parse_ascii(const std::string &str) {
	std::string res;
	res.reserve(str.size());
	for (auto it = str.begin(); it != str.end(); ++it) {
		auto found = std::find(it, str.end(), '\\');
		res.insert(res.end(), it, found);
		if (found == str.end()) {
			break;
		}
		if (found == str.end() - 1) {
			res.push_back('\\');
			break;
		}

		it = found + 1;
		std::cerr << "got: " << *it << std::endl;
		switch (*(found + 1)) {
		case 'n':
			res.push_back('\n');
			break;
		case 'r':
			res.push_back('\r');
			break;
		case 't':
			res.push_back('\t');
			break;
		default:
			res.push_back('\\');
			--it;
			break;
		}
	}
	return res;
}

} // namespace details
} // namespace clserpp
} // namespace fort
