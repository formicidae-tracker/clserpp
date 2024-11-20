#pragma once

#include "clser.h"
#include <cpptrace/exceptions.hpp>
#include <functional>
#include <memory>
#include <vector>

#include "details.hpp"
#include <iostream>

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

class IOTimeout : cpptrace::runtime_error {
public:
	IOTimeout()
	    : cpptrace::runtime_error("timouted") {}
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
		    .version = details::version_name(clVersion_e(version))};
	}

	~Serial() {
		clSerialClose(d_serial);
	}

	void Flush() const {
		details::call(clFlushPort, d_serial);
	}

	void ReadAll(std::string &buf, uint32_t timeout_ms) {
		buf.clear();
		uint32_t read = 0;
		while (read < buf.capacity()) {
			uint32_t size = buf.capacity() - read;
			details::call(
			    clSerialRead,
			    d_serial,
			    &buf[read],
			    &size,
			    timeout_ms
			);
			if (size == 0) {
				throw IOTimeout();
			}
			read += size;
			buf.resize(read);
		}
	}

	void WriteAll(const std::string &buf, uint32_t timeout_ms) {
		uint32_t written = 0;
		while (written < buf.size()) {
			uint32_t size = buf.size() - written;
			details::call(
			    clSerialWrite,
			    d_serial,
			    &buf[written],
			    &size,
			    timeout_ms
			);
			if (size == 0) {
				throw IOTimeout();
			}
			written += size;
		}
	}

	void
	ReadLine(std::string &buf, char delim = '\n', uint32_t timeout_ms = 100) {
		buf.clear();
		buf.reserve(DefaultBufferSize);
		uint32_t read = 0;
		while (read < DefaultBufferSize) {
			uint32_t size = DefaultBufferSize - read;
			details::call(
			    clSerialRead,
			    d_serial,
			    &buf[read],
			    &size,
			    timeout_ms
			);
			if (size == 0) {
				throw IOTimeout();
			}

			read += size;
			buf.resize(read);

			if (buf.find(delim) != std::string::npos) {
				return;
			}
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
	return out << fort::clserpp::details::baudrate_name(e);
}
