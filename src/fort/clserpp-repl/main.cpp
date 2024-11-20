#include <cpptrace/exceptions.hpp>
#include <cpptrace/utils.hpp>
#include <fort/clserpp/clserpp.hpp>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>

using namespace fort::clserpp;

class Buffer : public std::vector<char> {
public:
	Buffer(size_t i)
	    : std::vector<char>('.', i) {}
};

Buffer::const_iterator print4Bytes(
    std::ostream                 &out,
    const Buffer::const_iterator &start,
    Buffer::const_iterator        end
) {
	end = std::min(end, start + 4);

	for (auto c = start; c != end; ++c) {
		out << std::hex << std::setw(2) << int(*c);
	}
	for (int rem = 4 - std::distance(start, end); rem > 0; --rem) {
		out << "  ";
	}
	return end;
}

std::ostream &operator<<(std::ostream &out, const Buffer &buf) {
	out << "buffer " << buf.size() << " bytes" << std::endl;
	auto flags = out.flags();

	for (auto current = buf.cbegin(); current != buf.cend();) {
		std::string ascii(current, std::min(current + 32, buf.cend()));
		out << std::dec << std::setw(4) << std::setfill('0')
		    << std::distance(buf.cbegin(), current) << " ";
		current = print4Bytes(out, current, buf.cend());
		out << " ";
		current = print4Bytes(out, current, buf.cend());
		out << " . ";
		current = print4Bytes(out, current, buf.cend());
		out << " ";
		current = print4Bytes(out, current, buf.cend());
		out << " | " << ascii << std::endl;
	}
	out.flags(flags);

	return out;
}

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

		std::string res;
		line.clear();
		std::cout << ">>> " << std::flush;
		if (!std::getline(std::cin, line)) {
			break;
		}
		std::cerr << "sending '" << line << "'" << std::endl;
		serial->WriteAll(line, 1000);
		serial->ReadLine(res);
		std::cout << "<<< " << res << std::endl;
	}
}

int main(int argc, char **argv) {
	cpptrace::register_terminate_handler();
	execute(argc, argv);
	return 0;
}
