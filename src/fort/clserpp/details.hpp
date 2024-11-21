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

} // namespace details
} // namespace clserpp
} // namespace fort
