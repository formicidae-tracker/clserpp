#include <gtest/gtest.h>

#include <sstream>

#include "buffer.hpp"
#include "details.hpp"

TEST(Buffer, Escaping) {

	std::string escaped{R"(\n\r\t\foo)"}, parsed{"\n\r\t\\foo"};

	EXPECT_EQ(fort::clserpp::details::escape(parsed), escaped);
	EXPECT_EQ(fort::clserpp::details::parse_ascii(escaped), parsed);
}

TEST(Buffer, Formatting) {

	std::ostringstream out;

	fort::clserpp::Buffer buf{4};
	out << buf;
	EXPECT_EQ(
	    out.str(),
	    "buffer 4 bytes:\n"
	    "0000 | 2e2e2e2e          .                   | ....\n"
	);
}

TEST(Buffer, FormattingString) {

	std::ostringstream out;

	fort::clserpp::Buffer buf{"Hello world!! This is a new buffer."};
	out << buf;
	EXPECT_EQ(
	    out.str(),
	    "buffer 35 bytes:\n"
	    "0000 | 48656c6c 6f20776f . 726c6421 21205468 | Hello world!! Th\n"
	    "0016 | 69732069 73206120 . 6e657720 62756666 | is is a new buff\n"
	    "0032 | 65722e            .                   | er.\n"
	);
}

TEST(Buffer, FormattingNonPrintableCharacter) {

	std::ostringstream out;

	fort::clserpp::Buffer buf{4};
	buf[0] = '\r';
	buf[1] = '\n';
	buf[2] = 0;
	buf[3] = 0xaf;
	out << buf;
	EXPECT_EQ(
	    out.str(),
	    "buffer 4 bytes:\n"
	    "0000 | 0d0a00af          .                   | \\r\\n\\x0\\x175\n"
	);
}
