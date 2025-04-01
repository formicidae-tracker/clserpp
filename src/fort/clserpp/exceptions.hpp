#pragma once

#include <cpptrace/exceptions.hpp>

namespace fort {
namespace clserpp {
class IOTimeout : public cpptrace::runtime_error {
public:
	IOTimeout(uint32_t bytes) noexcept
	    : cpptrace::
	          runtime_error{"timouted after " + std::to_string(bytes) + " bytes"}
	    , d_bytes{bytes} {}

	uint32_t bytes() const noexcept {
		return d_bytes;
	}

private:
	uint32_t d_bytes;
};

} // namespace clserpp
} // namespace fort
