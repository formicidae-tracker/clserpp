#include "fort/clserpp/buffered_io.hpp"
#include "fort/clserpp/details.hpp"
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include <cpptrace/exceptions.hpp>
#include <cpptrace/utils.hpp>

#include <argparse/argparse.hpp>

#include <fort/clserpp/buffer.hpp>
#include <fort/clserpp/clserpp.hpp>

using namespace fort::clserpp;

struct Opts : public argparse::Args {
	int &baudrate = kwarg("b,baudrate", "Baudrate to use").set_default(19200);
	int &interface =
	    kwarg("i,idx", "Interface to use, negative values ask for prompt")
	        .set_default(-1);
	std::string &termination =
	    kwarg(
	        "t,termination",
	        "Line termination to use [allowed: <none,lf,cr,crlf,null>]"
	    )
	        .set_default("none");

	bool &verbose = flag("v,verbose", "should I be verbose");
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
		oss << prefix << std::string(details::baudrate_name(b)).substr(12);
		prefix = ", ";
	}
	oss << "}";
	return oss.str();
}

void setupBaudrate(Serial &serial, int baudrate) {
	auto e = details::baudrate_cast("CL_BAUDRATE_" + std::to_string(baudrate));

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
	static std::array<char, 5> delims = {
	    '\n',
	    '\r',
	    '\n',
	    '\n',
	    0,
	};

	auto opts = argparse::parse<Opts>(argc, argv);

	auto serial =
	    std::shared_ptr<Serial>(std::move(openInterface(opts.interface)));
	setupBaudrate(*serial, opts.baudrate);

	auto buffer      = ReadBuffer<Serial>{serial};
	auto termination = details::termination_cast(opts.termination).value();
	auto delim       = delims.at(size_t(termination));

	if (opts.verbose) {
		std::cerr << "Using delim '" << delim << "'" << std::endl;
	}

	std::string line;

	while (true) {
		while (buffer.HasByte() > 0) {
			std::cout << buffer.ReadLine(1000, delim) << std::flush;
			if (opts.verbose) {
				std::cerr << buffer.Bytes();
			}
		}

		line.clear();
		std::cout << ">>> " << std::flush;
		if (!std::getline(std::cin, line)) {
			break;
		}

		Buffer out{line, termination};
		if (opts.verbose) {
			std::cerr << "sending " << out;
		}

		serial->Write(out, 1000);
		serial->Flush();
		try {
			std::string res =
			    buffer.ReadLine(10000, delims.at(size_t(termination)));

			std::cout << "<<< " << res << std::endl;
		} catch (const IOTimeout &e) {
			std::cerr << "timeout: " << e.what() << std::endl;
			if (e.bytes() > 0) {
				std::cerr << buffer.Bytes();
			}
			continue;
		}

		if (opts.verbose) {
			std::cerr << buffer.Bytes();
		}
	}
}

int main(int argc, char **argv) {
	cpptrace::register_terminate_handler();
	execute(argc, argv);
	return 0;
}
