#pragma once

namespace fort {
namespace clserpp {

enum class LineTermination {
	NONE     = 0,
	LF       = 1,
	CR       = 2,
	CRLF     = 3,
	NULLCHAR = 4,
};
}
} // namespace fort
