#include <cpptrace/utils.hpp>
#include <fort/clserpp/clserpp.hpp>
#include <iostream>
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
}

int main(int argc, char **argv) {
	cpptrace::register_terminate_handler();
	execute(argc, argv);
	return 0;
}
