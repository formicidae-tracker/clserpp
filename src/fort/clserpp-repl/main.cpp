#include <cpptrace/exceptions.hpp>
#include <cpptrace/utils.hpp>
#include <fort/clserpp/clserpp.hpp>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>

using namespace fort::clserpp;

void execute(int argc, char **argv) {
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
			serial->ReadAll(data, 1000);
			std::cout << "<<< " << data << std::endl;
		}

		Buffer res{1000};
		line.clear();
		std::cout << ">>> " << std::flush;
		if (!std::getline(std::cin, line)) {
			break;
		}
		std::cerr << "sending '" << line << "'" << std::endl;
		serial->WriteAll(line, 1000);
		try {

			serial->ReadLine(res);
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
