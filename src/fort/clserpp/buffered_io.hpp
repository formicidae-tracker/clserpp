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

	BufferView(Buffer &buf, Buffer::iterator begin, Buffer::iterator end)
	    : d_begin{begin}
	    , d_end{end} {

		if (end < begin || begin > buf.end() || end > buf.end()) {
			throw cpptrace::logic_error(
			    "invalid buffer view [" +
			    std::to_string(std::distance(buf.begin(), begin)) + ", " +
			    std::to_string(std::distance(buf.begin(), end)) +
			    "[ for buffer of size " + std::to_string(buf.size())
			);
		}
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

	bool BytesAvailable() const {
		return std::distance(
		           d_buffer.begin(),
		           clserpp::Buffer::const_iterator{d_tail}
		       ) + d_reader->BytesAvailable() >
		       0;
	}

	std::string Flush() {
		if (d_head == d_tail) {
			return "";
		}

		std::string res{d_head, d_tail};
		d_head = d_buffer.begin();
		d_tail = d_buffer.begin();
		return res;
	}

	std::string ReadLine(uint32_t timeout_ms, char delim = '\n') {
		static const size_t segmentRead = 20;

		bool timeouted = false;
		while (true) {
			// test if we can send back data
			const auto pos = std::find(d_head, d_tail, delim);
			if (pos != d_tail) {
				std::string res{d_head, pos + 1};
				d_head = pos + 1;
				return res;
			} else if (timeouted) {
				throw IOTimeout(std::distance(d_head, d_tail));
			}

			// make room if possible
			if (std::distance(d_tail, d_buffer.end()) < segmentRead &&
			    d_head != d_buffer.begin()) {
				size_t available = std::distance(d_head, d_tail);
				std::copy(d_head, d_tail, d_buffer.begin());
				d_head = d_buffer.begin();
				d_tail = d_head + available;
			}

			// read more if possible
			details::BufferView segment{d_buffer, d_tail, d_tail + segmentRead};

			try {
				d_reader->Read(segment, timeout_ms);
				d_tail += segmentRead;
				timeouted = false;
			} catch (const IOTimeout &timeout) {
				if (timeout.bytes() == 0) {
					throw IOTimeout(std::distance(d_head, d_tail));
				}
				timeouted = true;
				d_tail += timeout.bytes();
			}
		}
	}

	const clserpp::Buffer &Bytes() const {
		return d_buffer;
	}

private:
	std::shared_ptr<Reader> d_reader = nullptr;

	clserpp::Buffer           d_buffer = clserpp::Buffer{4096};
	clserpp::Buffer::iterator d_head   = d_buffer.begin(),
	                          d_tail   = d_buffer.begin();
};
} // namespace clserpp
} // namespace fort
