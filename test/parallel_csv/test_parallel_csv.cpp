#include "catch.hpp"
#include "duckdb/common/types/date.hpp"
#include "duckdb/common/types/time.hpp"
#include "duckdb/common/types/timestamp.hpp"
#include "duckdb/execution/operator/persistent/csv_reader_options.hpp"
#include "duckdb/main/appender.hpp"
#include "test_helpers.hpp"
#include "duckdb/main/client_data.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using namespace duckdb;
using namespace std;

//! CSV Files
const string csv = "*.csv";
const string tsv = "*.tsv";
const string csv_gz = "csv.gz";
const string csv_zst = "csv.zst";
const string tbl_zst = "tbl.zst";

const string csv_extensions[5] = {csv, tsv, csv_gz, csv_zst, tbl_zst};

bool debug = true;
bool RunFull(std::string &path, duckdb::Connection &conn, std::set<std::string> *skip = nullptr,
             const string &add_parameters = "") {
	bool single_threaded_passed;
	// Here we run the csv file first with the single thread reader.
	// Then the parallel csv reader with a combination of multiple threads and buffer sizes.
	if (skip) {
		if (skip->find(path) != skip->end()) {
			// Gotta skip this
			return true;
		}
	}
	if (debug) {
		std::cout << path << std::endl;
	}

	// Set max line length to 0 when starting a ST CSV Read
	conn.context->client_data->max_line_length = 0;
	duckdb::unique_ptr<MaterializedQueryResult> single_threaded_res;
	ColumnDataCollection *ground_truth = nullptr;
	single_threaded_res =
	    conn.Query("SELECT * FROM read_csv_auto('" + path + add_parameters + "', parallel = 0) ORDER BY ALL");
	if (single_threaded_res->HasError()) {
		single_threaded_passed = false;
	} else {
		single_threaded_passed = true;
		ground_truth = &single_threaded_res->Collection();
	}

	// For parallel CSV Reading the buffer must be at least the size of the biggest line in the File.
	idx_t min_buffer_size = conn.context->client_data->max_line_length + 2;
	// So our tests don't take infinite time, we will go till a max buffer size of 5 positions higher than the minimum.
	idx_t max_buffer_size = min_buffer_size + 5;
	// Let's go from 1 to 8 threads.
	for (auto thread_count = 1; thread_count <= 8; thread_count++) {
		DuckDB db(nullptr);
		Connection multi_conn(db);
		for (auto buffer_size = min_buffer_size; buffer_size < max_buffer_size; buffer_size++) {
			if (debug) {
				std::cout << " Thread count: " << to_string(thread_count) << " Buffer Size: " << to_string(buffer_size)
				          << std::endl;
			}
			multi_conn.Query("SET preserve_insertion_order=false;");
			multi_conn.Query("PRAGMA threads=" + to_string(thread_count));
			duckdb::unique_ptr<MaterializedQueryResult> multi_threaded_result =
			    multi_conn.Query("SELECT * FROM read_csv_auto('" + path + add_parameters +
			                     "', buffer_size = " + to_string(buffer_size) + ") ORDER BY ALL");
			bool multi_threaded_passed;
			ColumnDataCollection *result = nullptr;
			if (multi_threaded_result->HasError()) {
				multi_threaded_passed = false;
			} else {
				multi_threaded_passed = true;
				result = &multi_threaded_result->Collection();
			}
			if (!single_threaded_passed && !multi_threaded_passed) {
				// Two wrongs can make a right
				continue;
			}
			if (!single_threaded_passed && multi_threaded_passed) {
				//! oh oh, this should not pass
				std::cout << path << " Failed on single threaded but succeeded on parallel reading" << std::endl;
				return false;
			}
			if (!multi_threaded_passed) {
				std::cout << path << " Multithreaded failed" << std::endl;
				std::cout << multi_threaded_result->GetError() << std::endl;
				return false;
			}
			// Results do not match
			string error_message;
			if (!ColumnDataCollection::ResultEquals(*ground_truth, *result, error_message, false)) {
				std::cout << path << " Thread count: " << to_string(thread_count)
				          << " Buffer Size: " << to_string(buffer_size) << std::endl;
				std::cout << error_message << std::endl;
				return false;
			}
		}
	}

	return true;
}

// Collects All CSV-Like files from folder and execute Parallel Scans on it
void RunTestOnFolder(const string &path, std::set<std::string> *skip = nullptr, const string &add_parameters = "") {
	DuckDB db(nullptr);
	Connection con(db);

	auto &fs = duckdb::FileSystem::GetFileSystem(*con.context);
	for (auto &ext : csv_extensions) {
		auto csv_files = fs.Glob(path + "*" + ext);
		for (auto &csv_file : csv_files) {
			REQUIRE(RunFull(csv_file, con, skip, add_parameters));
		}
	}
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/auto", "[parallel-csv][.]") {
	std::set<std::string> skip;
	// This file is from a 'mode skip' test
	skip.insert("test/sql/copy/csv/data/auto/titlebasicsdebug.tsv");
	RunTestOnFolder("test/sql/copy/csv/data/auto/", &skip);
}
//! Test case with specific parameters that allow us to run the titlebasicsdebug.tsv we were skipping
TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/auto/titlebasicsdebug.tsv", "[parallel-csv][.]") {
	DuckDB db(nullptr);
	Connection con(db);
	string add_parameters = ", nullstr=\'\\N\', sample_size = -1";
	string file = "test/sql/copy/csv/data/auto/titlebasicsdebug.tsv";
	REQUIRE(RunFull(file, con, nullptr, add_parameters));
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/auto/glob", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/auto/glob/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/error/date_multiple_file", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/error/date_multiple_file/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/a1", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/glob/a1/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/a2", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/glob/a2/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/a3", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/glob/a3/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/empty", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/glob/empty/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/glob/i1", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/glob/i1/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/real", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/real/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/test", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/test/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/zstd", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/zstd/");
}

TEST_CASE("Test Parallel CSV All Files - data/csv", "[parallel-csv][.]") {
	RunTestOnFolder("data/csv/");
}

TEST_CASE("Test Parallel CSV All Files - data/csv/decimal_separators", "[parallel-csv][.]") {
	RunTestOnFolder("data/csv/decimal_separators/");
}

TEST_CASE("Test Parallel CSV All Files - data/csv/中文", "[parallel-csv][.]") {
	RunTestOnFolder("data/csv/中文/");
}

TEST_CASE("Test Parallel CSV All Files - test/sql/copy/csv/data/abac", "[parallel-csv][.]") {
	RunTestOnFolder("test/sql/copy/csv/data/abac/");
}
