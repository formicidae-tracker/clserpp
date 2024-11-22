#include "fort/clserpp/exceptions.hpp"
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

	std::string &delimiter =
	    kwarg("d,delimiter", "line delimiter to use").set_default("\r\n>");
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
#ifndef NDEBUG
	auto fileLogger = spdlog::basic_logger_mt("file", "logs.txt");
	spdlog::set_default_logger(fileLogger);

	spdlog::set_level(spdlog::level::debug);
	fileLogger->flush_on(spdlog::level::debug);
#else
	spdlog::set_level(spdlog::level::info);
#endif

	auto opts = argparse::parse<Opts>(argc, argv);

	auto serial =
	    std::shared_ptr<Serial>(std::move(openInterface(opts.interface)));
	setupBaudrate(*serial, opts.baudrate);

	auto buffer      = ReadBuffer<Serial>{serial};
	auto termination = details::termination_cast(opts.termination).value();

	SPDLOG_INFO("using delimiter {}", details::escape(opts.delimiter));

	std::string line;

	while (true) {
		while (buffer.BytesAvailable() > 0) {
			std::cout << buffer.ReadUntil(100, opts.delimiter) << std::flush;
		}

		line.clear();
		std::cout << ">>> " << std::flush;
		if (!std::getline(std::cin, line)) {
			break;
		}

		Buffer out{line, termination};
		SPDLOG_INFO("sending {}", out);
		serial->Write(out, 100);

		try {
			std::string res = buffer.ReadUntil(100, opts.delimiter);
			std::cout << "<<< " << res << std::endl;
		} catch (const IOTimeout &e) {
			SPDLOG_DEBUG(
			    "got timeout: {} bytes, {}",
			    e.bytes(),
			    buffer.Bytes()
			);
			continue;
		}
	}
}

int main(int argc, char **argv) {
	cpptrace::register_terminate_handler();
	execute(argc, argv);
	return 0;
}
