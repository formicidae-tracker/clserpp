#include <cpptrace/utils.hpp>
#include <fort/clserpp/clserpp.hpp>
#include <iostream>
#include <string>
using namespace fort::clserpp;

void execute(int argc, char **argv) {
	std::cout << "clserpp-repl" << std::endl;
	const auto descriptions = Serial::GetDescriptions();
	if (descriptions.size() == 0) {
		throw cpptrace::runtime_error("No interfaces found");
	}
	for (const auto &desc : descriptions) {
		std::cout << desc.index << ": " << desc.info << std::endl;
	}

	std::cout << "Please choose an interface:" << std::endl;
	std::string line;
	std::getline(std::cin, line);
	auto serial = Serial::Open(std::stoi(line));

	while (true) {
		std::string res;
		line.clear();
		std::cout << "<<< " << std::endl;
		if (!std::getline(std::cin, line)) {
			break;
		}
		serial->WriteAll(line, 1000);
		serial->ReadLine(res);
		std::cout << ">>> " << res << std::endl;
	}
}

int main(int argc, char **argv) {
	cpptrace::register_terminate_handler();
	execute(argc, argv);
	return 0;
}
