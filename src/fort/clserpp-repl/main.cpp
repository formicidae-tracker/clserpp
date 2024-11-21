#include "fort/clserpp/clser.h"
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>

#include <cpptrace/exceptions.hpp>
#include <cpptrace/utils.hpp>

#include <argparse/argparse.hpp>

#include <fort/clserpp/clserpp.hpp>

using namespace fort::clserpp;

struct Opts : public argparse::Args {
	clBaudrate_e &baudrate =
	    kwarg("b,baudrate", "Baudrate to use").set_default(CL_BAUDRATE_19200);
	int &interface = kwarg("i,idx"
};

void execute(int argc, char **argv) {
	auto args = argparse::parse<Opts>(argc, argv);
	args.print();

	std::cout << "clserpp-repl" << std::endl;
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
	size_t idx    = std::stoi(line);
	auto   serial = Serial::Open(idx);

	const auto baudrates = serial->SupportedBaudrates();
	std::cout << "Supported Baudrate for " << descriptions[idx].info
	          << std::endl;
	for (size_t i = 0; i < baudrates.size(); ++i) {
		std::cout << "[" << i << "]: " << baudrates[i] << std::endl;
	}
	std::cout << "Please choose a baudrate: " << std::endl;
	std::getline(std::cin, line);
	size_t bdIdx = std::stoi(line);
	if (bdIdx >= baudrates.size()) {
		throw cpptrace::invalid_argument(
		    "Invalid index " + std::to_string(bdIdx)
		);
	}
	serial->SetBaudrate(baudrates[bdIdx]);

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
