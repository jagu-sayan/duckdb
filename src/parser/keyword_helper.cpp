#include "duckdb/parser/keyword_helper.hpp"
#include "duckdb/parser/parser.hpp"
#include "duckdb/common/string_util.hpp"

namespace duckdb {

bool KeywordHelper::IsKeyword(const string &text) {
	return Parser::IsKeyword(text);
}

bool KeywordHelper::RequiresQuotes(const string &text, bool allow_caps) {
	for (size_t i = 0; i < text.size(); i++) {
		if (i > 0 && (text[i] >= '0' && text[i] <= '9')) {
			continue;
		}
		if (text[i] >= 'a' && text[i] <= 'z') {
			continue;
		}
		if (allow_caps) {
			if (text[i] >= 'A' && text[i] <= 'Z') {
				continue;
			}
		}
		if (text[i] == '_') {
			continue;
		}
		return true;
	}
	return IsKeyword(text);
}

string KeywordHelper::WriteOptionallyQuoted(const string &text, char quote, bool allow_caps) {
	if (!RequiresQuotes(text, allow_caps)) {
		return text;
	}
	return string(1, quote) + StringUtil::Replace(text, string(1, quote), string(2, quote)) + string(1, quote);
}

} // namespace duckdb
