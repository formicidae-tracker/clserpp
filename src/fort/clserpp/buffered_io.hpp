#pragma once

#include <cstddef>
#include <iterator>
#include <memory>

#include <cpptrace/exceptions.hpp>
#include <string>

#include "buffer.hpp"
#include "exceptions.hpp"

namespace fort {
namespace clserpp {

namespace details {

class BufferView {
public:
	BufferView(Buffer &buf, size_t start, size_t end) {
		if (end > buf.size() || start > buf.size() || start > end) {
			throw cpptrace::logic_error(
			    "invalid buffer view [" + std::to_string(start) + ", " +
			    std::to_string(end) + "[ for buffer of size " +
			    std::to_string(buf.size())
			);
		}
		d_begin = buf.begin() + start;
		d_end   = buf.begin() + end;
	}

	char &operator[](size_t i) {
		return *(d_begin + i);
	}

	size_t size() const {
		return std::distance(d_begin, d_end);
	}

	std::vector<char>::iterator d_begin, d_end;
};
} // namespace details

template <typename Reader> class ReadBuffer {
public:
	ReadBuffer(std::shared_ptr<Reader> reader)
	    : d_reader{reader} {
		if (d_reader == nullptr) {
			throw cpptrace::logic_error("cannot function without a Reader");
		}
	};

	std::string ReadLine(uint32_t timeout_ms, char delim = '\n') {
		while (true) {
			bool timeouted = false;

			size_t              segmentRead = 2;
			details::BufferView segment{d_buffer, d_read, d_read + segmentRead};

			try {
				d_reader->Read(segment, timeout_ms);
			} catch (const IOTimeout &timeout) {
				if (timeout.bytes() == 0) {
					throw;
				}
				timeouted   = true;
				segmentRead = timeout.bytes();
			}

			const auto end = segment.d_begin + segmentRead;
			const auto pos = std::find(segment.d_begin, end, delim);

			if (pos != end) {
				std::string res{d_buffer.begin(), pos};
				std::copy(pos, end, d_buffer.begin());
				d_read = std::distance(pos, end);
				return res;
			} else {
				d_read += segmentRead;
				if (timeouted) {
					throw IOTimeout(d_read);
				}
			}
		}
	}

	const clserpp::Buffer &Buffer() const {
		return d_buffer;
	}

private:
	std::shared_ptr<Reader> d_reader = nullptr;

	clserpp::Buffer d_buffer = clserpp::Buffer{2000};
	size_t d_read   = 0;
};
} // namespace clserpp
} // namespace fort
