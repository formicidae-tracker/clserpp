#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>

#include <cpptrace/exceptions.hpp>
#include <cpptrace/utils.hpp>

#include <argparse/argparse.hpp>

#ifndef NDEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_ERROR
#endif

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

#include "fort/clserpp/buffered_io.hpp"
#include "fort/clserpp/details.hpp"
#include <fort/clserpp/buffer.hpp>
#include <fort/clserpp/clserpp.hpp>

template <>
struct fmt::formatter<fort::clserpp::Buffer> : fmt::formatter<std::string> {
	auto format(const fort::clserpp::Buffer &buf, format_context &ctx) const
	    -> decltype(ctx.out()) {
		std::ostringstream out;
		out << buf;

		return fmt::format_to(ctx.out(), "{}", out.str());
	}
};

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
	auto fileLogger = spdlog::basic_logger_mt("file", "logs.txt");
	spdlog::set_default_logger(fileLogger);
#ifndef NDEBUG
	spdlog::set_level(spdlog::level::debug);
#else
	spdlog::set_level(spdlog::level::info);
#endif

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

	spdlog::info("using delim '{}'", delim);
	std::string line;

	while (true) {
		while (buffer.BytesAvailable() > 0) {
			try {
				std::cout << buffer.ReadLine(1000, delim) << std::flush;
			} catch (const IOTimeout &e) {
				if (e.bytes() > 0) {
					std::cout << buffer.Flush() << std::endl;
				} else {
					throw;
				}
			}
		}

		line.clear();
		std::cout << ">>> " << std::flush;
		if (!std::getline(std::cin, line)) {
			break;
		}

		Buffer out{line, termination};
		spdlog::info("sending {}", out);
		serial->Write(out, 1000);

		try {
			std::string res =
			    buffer.ReadLine(10000, delims.at(size_t(termination)));

			std::cout << "<<< " << res << std::endl;
		} catch (const IOTimeout &e) {
			SPDLOG_DEBUG("got timeout: {}, {}", e, buffer);
			continue;
		}
	}
}

int main(int argc, char **argv) {
	cpptrace::register_terminate_handler();
	execute(argc, argv);
	return 0;
}
