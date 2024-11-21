#include "fort/clserpp/clser.h"
#include "magic_enum/magic_enum.hpp"
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include <cpptrace/exceptions.hpp>
#include <cpptrace/utils.hpp>

#include <argparse/argparse.hpp>

#include <fort/clserpp/clserpp.hpp>

using namespace fort::clserpp;

struct Opts : public argparse::Args {
	int &baudrate = kwarg("b,baudrate", "Baudrate to use").set_default(19200);
	int &interface =
	    kwarg("i,idx", "Interface to use, negative values ask for prompt")
	        .set_default(-1);
};

std::unique_ptr<Serial> openInterface(int interface) {
	if (interface < 0) {
		std::cout << "Available interfaces:" << std::endl;
		const auto descriptions = Serial::GetDescriptions();
		if (descriptions.size() == 0) {
			throw cpptrace::runtime_error("No interfaces found");
		}
		for (const auto &desc : descriptions) {
			std::cout << "[" << desc.index << "]: " << desc.info << std::endl;
		}

		std::cout << "Please choose an interface:" << std::endl;
		std::string line;
		std::getline(std::cin, line);
		interface = std::stoi(line);
	}
	return Serial::Open(interface);
}

std::string enumerateBaudrates(const std::vector<clBaudrate_e> &baudrates) {
	std::string        prefix{"{"};
	std::ostringstream oss;
	for (const auto &b : baudrates) {
		oss << prefix << b;
		prefix = ", ";
	}
	oss << "}";
	return oss.str();
}

void setupBaudrate(Serial &serial, int baudrate) {
	auto e = magic_enum::enum_cast<clBaudrate_e>(
	    "CL_BAUDRATE_" + std::to_string(baudrate)
	);

	const auto supported = serial.SupportedBaudrates();

	if (e.has_value() == false ||
	    std::find(supported.begin(), supported.end(), e.value()) ==
	        supported.end()) {
		throw cpptrace::runtime_error(
		    "Unsupported baudrate " + std::to_string(baudrate) +
		    ". Supported values are: " + enumerateBaudrates(supported)
		);
	}

	serial.SetBaudrate(e.value());
}

void execute(int argc, char **argv) {
	auto opts = argparse::parse<Opts>(argc, argv);

	auto serial = openInterface(opts.interface);
	setupBaudrate(*serial, opts.baudrate);

	std::string line;
	while (true) {
		serial->Flush();
		while (serial->ByteAvailable() > 0) {
			Buffer data(serial->ByteAvailable());
			serial->Read(data, 1000);
			std::cout << "<<< " << data << std::endl;
		}

		Buffer res{1000};
		line.clear();
		std::cout << ">>> " << std::flush;
		if (!std::getline(std::cin, line)) {
			break;
		}
		Buffer out{line};
		out.resize(out.size() + 1);
		out[out.size() - 1] = '\n';
		std::cerr << "sending '" << out << "'" << std::endl;

		serial->Write(out, 1000);
		serial->Flush();
		try {

			serial->Read(res, 1000);
		} catch (const IOTimeout &e) {
			std::cerr << "timeout: " << e.what() << std::endl;
			if (e.bytes() > 0) {
				std::cerr << res << std::endl;
			}
			continue;
		}
		std::cout << "<<< " << res << std::endl;
	}
}

int main(int argc, char **argv) {
	cpptrace::register_terminate_handler();
	execute(argc, argv);
	return 0;
}
