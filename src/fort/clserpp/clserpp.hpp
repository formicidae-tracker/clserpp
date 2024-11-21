#pragma once

#include <cctype>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <vector>

#include <cpptrace/exceptions.hpp>

#include <magic_enum/magic_enum.hpp>

#include "clser.h"

#include "details.hpp"

namespace fort {
namespace clserpp {

struct SerialDescription {
	uint32_t    index;
	std::string info;
};

struct ManufacturerInfo {
	std::string name;
	std::string version;
};

class IOTimeout : public cpptrace::runtime_error {
public:
	IOTimeout(uint32_t bytes) noexcept
	    : cpptrace::runtime_error{"timouted after " + std::to_string(bytes)}
	    , d_bytes{bytes} {}

	uint32_t bytes() const noexcept {
		return d_bytes;
	}

private:
	uint32_t d_bytes;
};

class Buffer : public std::vector<char> {
public:
	Buffer(size_t i)
	    : std::vector<char>(i, '.') {}

	Buffer(const std::string &value)
	    : std::vector<char>(value.size(), 0) {
		std::copy(value.cbegin(), value.cend(), this->begin());
	}
};

class Serial {

public:
	static std::unique_ptr<Serial> Open(uint32_t idx) {
		auto *res = new Serial(idx);
		return std::unique_ptr<Serial>(res);
	}

	static uint32_t NumSerial() {
		uint32_t res = 0;
		details::call(clGetNumSerialPorts, &res);
		return res;
	}

	static std::vector<SerialDescription> GetDescriptions() {
		const auto size = NumSerial();

		std::vector<SerialDescription> res(size);
		for (uint32_t i = 0; i < size; i++) {
			char     buffer[DefaultBufferSize];
			uint32_t size = DefaultBufferSize;
			details::call(clGetSerialPortIdentifier, i, buffer, &size);
			res[i] = {.index = i, .info = buffer};
		}
		return res;
	}

	static ManufacturerInfo GetManufacturerInfos() {
		char     buffer[DefaultBufferSize];
		uint32_t size = DefaultBufferSize;
		uint32_t version;
		details::call(clGetManufacturerInfo, buffer, &size, &version);
		return {
		    .name    = buffer,
		    .version = std::string{magic_enum::enum_name(clVersion_e(version))},
		};
	}

	~Serial() {
		clSerialClose(d_serial);
	}

	void Flush() const {
		details::call(clFlushPort, d_serial);
	}

	template <typename Container>
	void Read(Container &buf, uint32_t timeout_ms) {
		uint32_t read = 0;
		while (read < buf.size()) {
			uint32_t size = buf.size() - read;
			try {
				details::call(
				    clSerialRead,
				    d_serial,
				    &buf[read],
				    &size,
				    timeout_ms
				);
			} catch (const details::clserException &e) {
				if (e.code() == -10004) {
					throw IOTimeout(read);
				}
				throw;
			}
			if (size == 0) {
				throw IOTimeout(read);
			}
			read += size;
		}
	}

	template <typename Container>
	void Write(const Container &buf, uint32_t timeout_ms) {
		uint32_t written = 0;
		while (written < buf.size()) {
			uint32_t size = buf.size() - written;
			try {
				details::call(
				    clSerialWrite,
				    d_serial,
				    &buf[written],
				    &size,
				    timeout_ms
				);
			} catch (const details::clserException &e) {
				if (e.code() == -10004) {
					throw IOTimeout(written);
				}
				throw;
			}

			if (size == 0) {
				throw IOTimeout(written);
			}
			written += size;
		}
	}

	uint32_t ByteAvailable() const {
		uint32_t res = 0;
		details::call(clGetNumBytesAvail, d_serial, &res);
		return res;
	}

	std::vector<clBaudrate_e> SupportedBaudrates() const {
		uint32_t baudrates = 0;
		details::call(clGetSupportedBaudRates, d_serial, &baudrates);

		std::vector<clBaudrate_e> res;
		res.reserve(32);
		for (int i = 0; i < 32; i++) {
			clBaudrate_e bd = clBaudrate_e(1 << i);
			if ((baudrates & bd) != 0) {
				res.push_back(bd);
			}
		}
		return res;
	}

	void SetBaudrate(clBaudrate_e bd) {
		details::call(clSetBaudRate, d_serial, bd);
	}

private:
	Serial(uint32_t idx) {
		details::call(clSerialInit, idx, &d_serial);
	}

	Serial(const Serial &other)            = delete;
	Serial &operator=(const Serial &other) = delete;
	Serial(Serial &&other)                 = delete;
	Serial &operator=(Serial &&other)      = delete;

	void *d_serial = nullptr;

	const static uint32_t DefaultBufferSize = 300;
};
} // namespace clserpp
} // namespace fort

std::ostream &operator<<(std::ostream &out, clBaudrate_e e) {
	return out << magic_enum::enum_name(e);
}

std::ostream &operator<<(std::ostream &out, const fort::clserpp::Buffer &buf) {
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
		    struct EscapableChar {
			public:
			    char        value;
			    const char *escaped;
		    };

		    const std::array<EscapableChar, 2> escapables = {
		        EscapableChar{.value = '\r', .escaped = "\\r"},
		        EscapableChar{.value = '\n', .escaped = "\\n"},
		    };
		    for (auto iter = start; iter != end; ++iter) {
			    if (std::isprint(*iter)) {
				    out << char(*iter);
				    continue;
			    }
			    auto pos = std::find_if(

			        escapables.begin(),
			        escapables.end(),
			        [v = *iter](const EscapableChar &c) { return c.value == v; }
			    );

			    if (pos != escapables.end()) {
				    out << pos->escaped;
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
