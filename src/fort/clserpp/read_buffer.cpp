#include <gtest/gtest.h>

#include "buffer.hpp"
#include "buffered_io.hpp"

using namespace fort::clserpp;

class MockReader {
public:
	MockReader(const Buffer &data)
	    : d_data{data}
	    , d_next{d_data.begin()} {}

	template <typename Container>
	void Read(Container &buf, uint32_t timeout_ms) {
		const auto end      = std::min(d_data.cend(), d_next + buf.size());
		bool       complete = end <= d_data.end();
		std::copy(d_next, end, &buf[0]);
		size_t read = std::distance(d_next, end);
		if (!complete) {
			throw IOTimeout(read);
		}
		d_next += read;
	}

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
