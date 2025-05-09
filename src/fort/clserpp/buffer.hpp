#pragma once

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <vector>

#include <spdlog/spdlog.h>

#include "details.hpp"

namespace fort {
namespace clserpp {

namespace details {
const static std::array<std::string, 5> terminations = {
    "",
    "\n",
    "\r",
    "\r\n",
    "\x0",
};
}

class Buffer : public std::vector<char> {

public:
	Buffer(size_t i)
	    : std::vector<char>(i, '.') {}

	Buffer(
	    const std::string &value,
	    LineTermination    termination = LineTermination::NONE
	)
	    : std::vector<char>(
	          value.size() +
	              details::terminations.at(size_t(termination)).size(),
	          0
	      ) {
		std::copy(value.cbegin(), value.cend(), this->begin());
		const auto &terminationStr =
		    details::terminations.at(size_t(termination));
		std::copy(
		    terminationStr.begin(),
		    terminationStr.end(),
		    this->begin() + value.size()
		);
	}
};

} // namespace clserpp
} // namespace fort

inline std::ostream &
operator<<(std::ostream &out, const fort::clserpp::Buffer &buf) {
	using namespace fort::clserpp;
	static const auto print4Bytes = [](std::ostream                 &out,
	                                   const Buffer::const_iterator &start,
	                                   Buffer::const_iterator        end
	                                ) -> Buffer::const_iterator {
		end = std::min(end, start + 4);

		for (auto c = start; c != end; ++c) {
			out << std::hex << std::setw(2) << (int)(*c & 0xff);
		}
		for (int rem = 4 - std::distance(start, end); rem > 0; --rem) {
			out << "  ";
		}
		return end;
	};

	static const auto print16BytesAscii =
	    [](std::ostream                 &out,
	       const Buffer::const_iterator &start,
	       const Buffer::const_iterator &end) {
		    for (auto iter = start; iter != end; ++iter) {
			    if (std::isprint(*iter)) {
				    out << char(*iter);
				    continue;
			    }

			    const auto *escaped = details::escape_char(*iter);
			    if (escaped != nullptr) {
				    out << escaped;
				    continue;
			    }

			    out << "\\x" << std::dec << (int)(*iter & 0xff);
		    }

		    int left = std::distance(start, end) - 16;
		    if (left > 0) {
			    out << std::string(left, ' ');
		    }
	    };

	auto flags = out.flags();

	out << "buffer " << buf.size() << " bytes:" << std::endl;

	for (auto current = buf.cbegin(); current != buf.cend();) {
		auto linestart = current;
		out << std::right << std::dec << std::setw(4) << std::setfill('0')
		    << std::distance(buf.cbegin(), current) << " | ";

		current = print4Bytes(out, current, buf.cend());
		out << " ";
		current = print4Bytes(out, current, buf.cend());
		out << " . ";
		current = print4Bytes(out, current, buf.cend());
		out << " ";
		current = print4Bytes(out, current, buf.cend());
		out << " | ";
		print16BytesAscii(out, linestart, std::min(linestart + 16, buf.cend()));
		out << std::endl;
	}
	out.flags(flags);

	return out;
}

template <>
struct fmt::formatter<fort::clserpp::Buffer> : fmt::formatter<std::string> {
	auto format(const fort::clserpp::Buffer &buf, format_context &ctx) const
	    -> decltype(ctx.out()) {
		std::ostringstream out;
		out << buf;

		return fmt::format_to(ctx.out(), "{}", out.str());
	}
};
