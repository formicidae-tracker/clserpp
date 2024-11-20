#pragma once

#include "clser.h"
#include <memory>
#include <vector>

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
			char     buffer[2000];
			uint32_t size = 2000;
			details::call(clGetSerialPortIdentifier, i, buffer, &size);
			res[i] = {.index = i, .info = buffer};
		}
		return res;
	}

	static ManufacturerInfo GetManufacturerInfos() {
		char     buffer[2000];
		uint32_t size = 2000;
		uint32_t version;
		details::call(clGetManufacturerInfo, buffer, &size, &version);
		return {
		    .name    = buffer,
		    .version = details::version_name(clVersion_e(version))};
	}

	~Serial() {
		clSerialClose(d_serial);
	}

private:
	Serial(uint32_t idx) {}

	Serial(const Serial &other)            = delete;
	Serial &operator=(const Serial &other) = delete;
	Serial(Serial &&other)                 = delete;
	Serial &operator=(Serial &&other)      = delete;

	void *d_serial = nullptr;
};
} // namespace clserpp
} // namespace fort
