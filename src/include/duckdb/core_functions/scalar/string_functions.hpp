//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/core_functions/scalar/string_functions.hpp
//
//
//===----------------------------------------------------------------------===//
// This file is automatically generated by scripts/generate_functions.py
// Do not edit this file manually, your changes will be overwritten
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/function/function_set.hpp"

namespace duckdb {

struct StartsWithOperatorFun {
	static constexpr const char *Name = "^@";
	static constexpr const char *Parameters = "string,search_string";
	static constexpr const char *Description = "Returns true if string begins with search_string";
	static constexpr const char *Example = "starts_with('abc','a')";

	static ScalarFunction GetFunction();
};

struct StartsWithFun {
	using ALIAS = StartsWithOperatorFun;

	static constexpr const char *Name = "starts_with";
};

struct ASCIIFun {
	static constexpr const char *Name = "ascii";
	static constexpr const char *Parameters = "string";
	static constexpr const char *Description = "Returns an integer that represents the Unicode code point of the first character of the string";
	static constexpr const char *Example = "ascii('Ω')";

	static ScalarFunction GetFunction();
};

struct BarFun {
	static constexpr const char *Name = "bar";
	static constexpr const char *Parameters = "x,min,max,width";
	static constexpr const char *Description = "Draws a band whose width is proportional to (x - min) and equal to width characters when x = max. width defaults to 80";
	static constexpr const char *Example = "bar(5, 0, 20, 10)";

	static ScalarFunctionSet GetFunctions();
};

struct BinFun {
	static constexpr const char *Name = "bin";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Converts the value to binary representation";
	static constexpr const char *Example = "bin(42)";

	static ScalarFunctionSet GetFunctions();
};

struct ToBinaryFun {
	using ALIAS = BinFun;

	static constexpr const char *Name = "to_binary";
};

struct ChrFun {
	static constexpr const char *Name = "chr";
	static constexpr const char *Parameters = "code_point";
	static constexpr const char *Description = "Returns a character which is corresponding the ASCII code value or Unicode code point";
	static constexpr const char *Example = "chr(65)";

	static ScalarFunction GetFunction();
};

struct DamerauLevenshteinFun {
	static constexpr const char *Name = "damerau_levenshtein";
	static constexpr const char *Parameters = "str1,str2";
	static constexpr const char *Description = "Extension of Levenshtein distance to also include transposition of adjacent characters as an allowed edit operation. In other words, the minimum number of edit operations (insertions, deletions, substitutions or transpositions) required to change one string to another. Different case is considered different";
	static constexpr const char *Example = "damerau_levenshtein('hello', 'world')";

	static ScalarFunction GetFunction();
};

struct FormatFun {
	static constexpr const char *Name = "format";
	static constexpr const char *Parameters = "format,parameters...";
	static constexpr const char *Description = "Formats a string using fmt syntax";
	static constexpr const char *Example = "format('Benchmark \"{}\" took {} seconds', 'CSV', 42)";

	static ScalarFunction GetFunction();
};

struct FormatBytesFun {
	static constexpr const char *Name = "format_bytes";
	static constexpr const char *Parameters = "bytes";
	static constexpr const char *Description = "Converts bytes to a human-readable presentation (e.g. 16000 -> 16KB)";
	static constexpr const char *Example = "format_bytes(1000 * 16)";

	static ScalarFunction GetFunction();
};

struct FormatreadabledecimalsizeFun {
	using ALIAS = FormatBytesFun;

	static constexpr const char *Name = "formatReadableDecimalSize";
};

struct HammingFun {
	static constexpr const char *Name = "hamming";
	static constexpr const char *Parameters = "str1,str2";
	static constexpr const char *Description = "The number of positions with different characters for 2 strings of equal length. Different case is considered different";
	static constexpr const char *Example = "hamming('duck','luck')";

	static ScalarFunction GetFunction();
};

struct MismatchesFun {
	using ALIAS = HammingFun;

	static constexpr const char *Name = "mismatches";
};

struct HexFun {
	static constexpr const char *Name = "hex";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Converts the value to hexadecimal representation";
	static constexpr const char *Example = "hex(42)";

	static ScalarFunctionSet GetFunctions();
};

struct ToHexFun {
	using ALIAS = HexFun;

	static constexpr const char *Name = "to_hex";
};

struct InstrFun {
	static constexpr const char *Name = "instr";
	static constexpr const char *Parameters = "haystack,needle";
	static constexpr const char *Description = "Returns location of first occurrence of needle in haystack, counting from 1. Returns 0 if no match found";
	static constexpr const char *Example = "instr('test test','es')";

	static ScalarFunction GetFunction();
};

struct StrposFun {
	using ALIAS = InstrFun;

	static constexpr const char *Name = "strpos";
};

struct PositionFun {
	using ALIAS = InstrFun;

	static constexpr const char *Name = "position";
};

struct JaccardFun {
	static constexpr const char *Name = "jaccard";
	static constexpr const char *Parameters = "str1,str2";
	static constexpr const char *Description = "The Jaccard similarity between two strings. Different case is considered different. Returns a number between 0 and 1";
	static constexpr const char *Example = "jaccard('duck','luck')";

	static ScalarFunction GetFunction();
};

struct JaroSimilarityFun {
	static constexpr const char *Name = "jaro_similarity";
	static constexpr const char *Parameters = "str1,str2";
	static constexpr const char *Description = "The Jaro similarity between two strings. Different case is considered different. Returns a number between 0 and 1";
	static constexpr const char *Example = "jaro_similarity('duck','duckdb')";

	static ScalarFunction GetFunction();
};

struct JaroWinklerSimilarityFun {
	static constexpr const char *Name = "jaro_winkler_similarity";
	static constexpr const char *Parameters = "str1,str2";
	static constexpr const char *Description = "The Jaro-Winkler similarity between two strings. Different case is considered different. Returns a number between 0 and 1";
	static constexpr const char *Example = "jaro_winkler_similarity('duck','duckdb')";

	static ScalarFunction GetFunction();
};

struct LeftFun {
	static constexpr const char *Name = "left";
	static constexpr const char *Parameters = "string,count";
	static constexpr const char *Description = "Extract the left-most count characters";
	static constexpr const char *Example = "left('Hello🦆', 2)";

	static ScalarFunction GetFunction();
};

struct LeftGraphemeFun {
	static constexpr const char *Name = "left_grapheme";
	static constexpr const char *Parameters = "string,count";
	static constexpr const char *Description = "Extract the left-most count grapheme clusters";
	static constexpr const char *Example = "left_grapheme('🤦🏼‍♂️🤦🏽‍♀️', 1)";

	static ScalarFunction GetFunction();
};

struct LevenshteinFun {
	static constexpr const char *Name = "levenshtein";
	static constexpr const char *Parameters = "str1,str2";
	static constexpr const char *Description = "The minimum number of single-character edits (insertions, deletions or substitutions) required to change one string to the other. Different case is considered different";
	static constexpr const char *Example = "levenshtein('duck','db')";

	static ScalarFunction GetFunction();
};

struct Editdist3Fun {
	using ALIAS = LevenshteinFun;

	static constexpr const char *Name = "editdist3";
};

struct LpadFun {
	static constexpr const char *Name = "lpad";
	static constexpr const char *Parameters = "string,count,character";
	static constexpr const char *Description = "Pads the string with the character from the left until it has count characters";
	static constexpr const char *Example = "lpad('hello', 10, '>')";

	static ScalarFunction GetFunction();
};

struct LtrimFun {
	static constexpr const char *Name = "ltrim";
	static constexpr const char *Parameters = "string,characters";
	static constexpr const char *Description = "Removes any occurrences of any of the characters from the left side of the string";
	static constexpr const char *Example = "ltrim('>>>>test<<', '><')";

	static ScalarFunctionSet GetFunctions();
};

struct MD5Fun {
	static constexpr const char *Name = "md5";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Returns the MD5 hash of the value as a string";
	static constexpr const char *Example = "md5('123')";

	static ScalarFunction GetFunction();
};

struct MD5NumberFun {
	static constexpr const char *Name = "md5_number";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Returns the MD5 hash of the value as an INT128";
	static constexpr const char *Example = "md5_number('123')";

	static ScalarFunction GetFunction();
};

struct MD5NumberLowerFun {
	static constexpr const char *Name = "md5_number_lower";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Returns the MD5 hash of the value as an INT128";
	static constexpr const char *Example = "md5_number_lower('123')";

	static ScalarFunction GetFunction();
};

struct MD5NumberUpperFun {
	static constexpr const char *Name = "md5_number_upper";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Returns the MD5 hash of the value as an INT128";
	static constexpr const char *Example = "md5_number_upper('123')";

	static ScalarFunction GetFunction();
};

struct PrintfFun {
	static constexpr const char *Name = "printf";
	static constexpr const char *Parameters = "format,parameters...";
	static constexpr const char *Description = "Formats a string using printf syntax";
	static constexpr const char *Example = "printf('Benchmark \"%s\" took %d seconds', 'CSV', 42)";

	static ScalarFunction GetFunction();
};

struct RepeatFun {
	static constexpr const char *Name = "repeat";
	static constexpr const char *Parameters = "string,count";
	static constexpr const char *Description = "Repeats the string count number of times";
	static constexpr const char *Example = "repeat('A', 5)";

	static ScalarFunctionSet GetFunctions();
};

struct ReplaceFun {
	static constexpr const char *Name = "replace";
	static constexpr const char *Parameters = "string,source,target";
	static constexpr const char *Description = "Replaces any occurrences of the source with target in string";
	static constexpr const char *Example = "replace('hello', 'l', '-')";

	static ScalarFunction GetFunction();
};

struct ReverseFun {
	static constexpr const char *Name = "reverse";
	static constexpr const char *Parameters = "string";
	static constexpr const char *Description = "Reverses the string";
	static constexpr const char *Example = "reverse('hello')";

	static ScalarFunction GetFunction();
};

struct RightFun {
	static constexpr const char *Name = "right";
	static constexpr const char *Parameters = "string,count";
	static constexpr const char *Description = "Extract the right-most count characters";
	static constexpr const char *Example = "right('Hello🦆', 3)";

	static ScalarFunction GetFunction();
};

struct RightGraphemeFun {
	static constexpr const char *Name = "right_grapheme";
	static constexpr const char *Parameters = "string,count";
	static constexpr const char *Description = "Extract the right-most count grapheme clusters";
	static constexpr const char *Example = "right_grapheme('🤦🏼‍♂️🤦🏽‍♀️', 1)";

	static ScalarFunction GetFunction();
};

struct RpadFun {
	static constexpr const char *Name = "rpad";
	static constexpr const char *Parameters = "string,count,character";
	static constexpr const char *Description = "Pads the string with the character from the right until it has count characters";
	static constexpr const char *Example = "rpad('hello', 10, '<')";

	static ScalarFunction GetFunction();
};

struct RtrimFun {
	static constexpr const char *Name = "rtrim";
	static constexpr const char *Parameters = "string,characters";
	static constexpr const char *Description = "Removes any occurrences of any of the characters from the right side of the string";
	static constexpr const char *Example = "rtrim('>>>>test<<', '><')";

	static ScalarFunctionSet GetFunctions();
};

struct SHA256Fun {
	static constexpr const char *Name = "sha256";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Returns the SHA256 hash of the value";
	static constexpr const char *Example = "sha256('hello')";

	static ScalarFunction GetFunction();
};

struct StringSplitFun {
	static constexpr const char *Name = "string_split";
	static constexpr const char *Parameters = "string,separator";
	static constexpr const char *Description = "Splits the string along the separator";
	static constexpr const char *Example = "string_split('hello-world', '-')";

	static ScalarFunction GetFunction();
};

struct StrSplitFun {
	using ALIAS = StringSplitFun;

	static constexpr const char *Name = "str_split";
};

struct StringToArrayFun {
	using ALIAS = StringSplitFun;

	static constexpr const char *Name = "string_to_array";
};

struct SplitFun {
	using ALIAS = StringSplitFun;

	static constexpr const char *Name = "split";
};

struct StringSplitRegexFun {
	static constexpr const char *Name = "string_split_regex";
	static constexpr const char *Parameters = "string,separator";
	static constexpr const char *Description = "Splits the string along the regex";
	static constexpr const char *Example = "string_split_regex('hello␣world; 42', ';?␣')";

	static ScalarFunctionSet GetFunctions();
};

struct StrSplitRegexFun {
	using ALIAS = StringSplitRegexFun;

	static constexpr const char *Name = "str_split_regex";
};

struct RegexpSplitToArrayFun {
	using ALIAS = StringSplitRegexFun;

	static constexpr const char *Name = "regexp_split_to_array";
};

struct TranslateFun {
	static constexpr const char *Name = "translate";
	static constexpr const char *Parameters = "string,from,to";
	static constexpr const char *Description = "Replaces each character in string that matches a character in the from set with the corresponding character in the to set. If from is longer than to, occurrences of the extra characters in from are deleted";
	static constexpr const char *Example = "translate('12345', '143', 'ax')";

	static ScalarFunction GetFunction();
};

struct TrimFun {
	static constexpr const char *Name = "trim";
	static constexpr const char *Parameters = "string,characters";
	static constexpr const char *Description = "Removes any occurrences of any of the characters from either side of the string";
	static constexpr const char *Example = "trim('>>>>test<<', '><')";

	static ScalarFunctionSet GetFunctions();
};

struct UnbinFun {
	static constexpr const char *Name = "unbin";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Converts a value from binary representation to a blob";
	static constexpr const char *Example = "unbin('0110')";

	static ScalarFunction GetFunction();
};

struct FromBinaryFun {
	using ALIAS = UnbinFun;

	static constexpr const char *Name = "from_binary";
};

struct UnhexFun {
	static constexpr const char *Name = "unhex";
	static constexpr const char *Parameters = "value";
	static constexpr const char *Description = "Converts a value from hexadecimal representation to a blob";
	static constexpr const char *Example = "unhex('2A')";

	static ScalarFunction GetFunction();
};

struct FromHexFun {
	using ALIAS = UnhexFun;

	static constexpr const char *Name = "from_hex";
};

struct UnicodeFun {
	static constexpr const char *Name = "unicode";
	static constexpr const char *Parameters = "str";
	static constexpr const char *Description = "Returns the unicode codepoint of the first character of the string";
	static constexpr const char *Example = "unicode('ü')";

	static ScalarFunction GetFunction();
};

struct OrdFun {
	using ALIAS = UnicodeFun;

	static constexpr const char *Name = "ord";
};

struct ToBaseFun {
	static constexpr const char *Name = "to_base";
	static constexpr const char *Parameters = "number,radix,min_length";
	static constexpr const char *Description = "Converts a value to a string in the given base radix, optionally padding with leading zeros to the minimum length";
	static constexpr const char *Example = "to_base(42, 16)";

	static ScalarFunctionSet GetFunctions();
};

} // namespace duckdb
