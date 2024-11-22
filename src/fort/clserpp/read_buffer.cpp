#include <gtest/gtest.h>

#include "buffer.hpp"
#include "buffered_io.hpp"

#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/spdlog.h>

using namespace fort::clserpp;

class MockReader {
public:
	MockReader(const Buffer &data)
	    : d_data{data}
	    , d_next{d_data.begin()} {}

	template <typename Container>
	void Read(Container &buf, uint32_t timeout_ms) {
		const auto end = std::min(d_data.cend(), d_next + buf.size());
		std::copy(d_next, end, &buf[0]);
		size_t read = std::distance(d_next, end);
		d_next += read;
		if (read < buf.size()) {
			throw IOTimeout(read);
		}
	}

	uint32_t BytesAvailable() const {
		return std::distance(d_next, d_data.end());
	}

	void Flush() const {}

private:
	Buffer                 d_data;
	Buffer::const_iterator d_next;
};

TEST(ReadBuffer, CanReadSmallBuffer) {
	auto reader = std::make_shared<MockReader>(
	    Buffer{std::string("this is a value "), LineTermination::CRLF}
	);
	auto buffer = ReadBuffer(reader);

	EXPECT_EQ(buffer.ReadLine(1000), "this is a value \r\n");
}

TEST(ReadBuffer, CanReadLongBuffer) {
	auto reader = std::make_shared<MockReader>(Buffer{
	    std::string(
	        "this is a very long long long buffer which spann lots of data"
	    ),
	    LineTermination::CR});
	auto buffer = ReadBuffer(reader);

	EXPECT_EQ(
	    buffer.ReadLine(1000, '\r'),
	    "this is a very long long long buffer which spann lots of data\r"
	);
}

TEST(ReadBuffer, CanReadMultipleSmallBuffers) {
	auto logger = spdlog::stderr_logger_mt("tests");
	logger->set_level(spdlog::level::debug);
	logger->flush_on(spdlog::level::debug);
	spdlog::set_default_logger(logger);

	auto reader = std::make_shared<MockReader>(
	    Buffer{std::string("a\nb\nc\nd"), LineTermination::LF}
	);
	auto buffer = ReadBuffer(reader);

	EXPECT_EQ(buffer.ReadLine(1000, '\n'), "a\n");
	EXPECT_EQ(buffer.ReadLine(1000, '\n'), "b\n");
	EXPECT_EQ(buffer.ReadLine(1000, '\n'), "c\n");
	EXPECT_EQ(buffer.ReadLine(1000, '\n'), "d\n");

	EXPECT_THROW({ buffer.ReadLine(1000); }, EndOfStream);
}
