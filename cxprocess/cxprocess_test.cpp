//-------------------------------------------------------------------------------------------------
//
//  cxprocess_test.cpp
//
//  Test suite for the CxProcess class.
//
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <cx/base/string.h>
#include <cx/process/process.h>

static int gTestsPassed = 0;
static int gTestsFailed = 0;

#define TEST(name) static int name()
#define ASSERT(cond) if (!(cond)) { printf("  FAIL: %s\n", #cond); return 0; }
#define PASS() return 1

#define RUN_TEST(name) do { \
    printf("%-50s ", #name); \
    if (name()) { printf("PASS\n"); gTestsPassed++; } \
    else { gTestsFailed++; } \
} while(0)


//-------------------------------------------------------------------------------------------------
// CxProcess::run tests
//-------------------------------------------------------------------------------------------------

TEST(test_run_simple_command)
{
    CxProcess proc;
    int result = proc.run("echo hello");
    ASSERT(result == 0);
    ASSERT(proc.getExitCode() == 0);

    CxString output = proc.getOutput();
    ASSERT(output.length() > 0);
    ASSERT(output.index("hello") == 0);  // starts with "hello"
    PASS();
}

TEST(test_run_command_with_args)
{
    CxProcess proc;
    int result = proc.run("echo one two three");
    ASSERT(result == 0);
    ASSERT(proc.getExitCode() == 0);

    CxString output = proc.getOutput();
    ASSERT(output.index("one two three") == 0);  // starts with
    PASS();
}

TEST(test_run_failing_command)
{
    CxProcess proc;
    int result = proc.run("false");  // 'false' command returns exit code 1
    ASSERT(result == 0);  // run() succeeded
    ASSERT(proc.getExitCode() == 1);  // but command failed
    PASS();
}

TEST(test_run_multiline_output)
{
    CxProcess proc;
    int result = proc.run("printf 'line1\\nline2\\nline3\\n'");
    ASSERT(result == 0);
    ASSERT(proc.getExitCode() == 0);

    CxString output = proc.getOutput();
    ASSERT(output.index("line1") >= 0);  // contains
    ASSERT(output.index("line2") >= 0);
    ASSERT(output.index("line3") >= 0);
    PASS();
}

TEST(test_run_empty_command)
{
    CxProcess proc;
    int result = proc.run("");
    ASSERT(result == -1);  // should fail for empty command
    PASS();
}

TEST(test_run_captures_stderr)
{
    CxProcess proc;
    // Command that writes to stderr
    int result = proc.run("ls /nonexistent_directory_12345");
    ASSERT(result == 0);  // run() succeeded
    ASSERT(proc.getExitCode() != 0);  // ls should fail

    CxString output = proc.getOutput();
    // stderr should be captured (contains error message)
    ASSERT(output.length() > 0);
    PASS();
}


//-------------------------------------------------------------------------------------------------
// CxProcess::parseBuildError tests
//-------------------------------------------------------------------------------------------------

TEST(test_parse_gcc_error_with_column)
{
    CxString line = "main.cpp:42:15: error: expected ';' after expression";
    CxBuildError err = CxProcess::parseBuildError(line);

    ASSERT(err.valid == 1);
    ASSERT(err.filename == "main.cpp");
    ASSERT(err.line == 42);
    ASSERT(err.column == 15);
    ASSERT(err.message.index("expected") >= 0);  // contains
    PASS();
}

TEST(test_parse_gcc_error_no_column)
{
    CxString line = "foo.c:123: undefined reference to 'bar'";
    CxBuildError err = CxProcess::parseBuildError(line);

    ASSERT(err.valid == 1);
    ASSERT(err.filename == "foo.c");
    ASSERT(err.line == 123);
    ASSERT(err.column == 0);
    PASS();
}

TEST(test_parse_absolute_path)
{
    CxString line = "/home/user/src/file.cpp:99:1: warning: unused variable";
    CxBuildError err = CxProcess::parseBuildError(line);

    ASSERT(err.valid == 1);
    ASSERT(err.filename == "/home/user/src/file.cpp");
    ASSERT(err.line == 99);
    ASSERT(err.column == 1);
    PASS();
}

TEST(test_parse_relative_path)
{
    CxString line = "../lib/utils.cpp:55:10: note: in expansion of macro";
    CxBuildError err = CxProcess::parseBuildError(line);

    ASSERT(err.valid == 1);
    ASSERT(err.filename == "../lib/utils.cpp");
    ASSERT(err.line == 55);
    ASSERT(err.column == 10);
    PASS();
}

TEST(test_parse_no_error_pattern)
{
    CxString line = "make: Entering directory '/home/user/project'";
    CxBuildError err = CxProcess::parseBuildError(line);

    ASSERT(err.valid == 0);
    PASS();
}

TEST(test_parse_empty_line)
{
    CxString line = "";
    CxBuildError err = CxProcess::parseBuildError(line);

    ASSERT(err.valid == 0);
    PASS();
}

TEST(test_parse_plain_text)
{
    CxString line = "Building target...";
    CxBuildError err = CxProcess::parseBuildError(line);

    ASSERT(err.valid == 0);
    PASS();
}

TEST(test_parse_colon_but_no_number)
{
    CxString line = "In file included from header.h:";
    CxBuildError err = CxProcess::parseBuildError(line);

    // This has "header.h:" but no line number after, so should not match
    ASSERT(err.valid == 0);
    PASS();
}

TEST(test_parse_clang_warning)
{
    CxString line = "test.cpp:10:5: warning: comparison of integers [-Wsign-compare]";
    CxBuildError err = CxProcess::parseBuildError(line);

    ASSERT(err.valid == 1);
    ASSERT(err.filename == "test.cpp");
    ASSERT(err.line == 10);
    ASSERT(err.column == 5);
    ASSERT(err.message.index("warning") >= 0);  // contains
    PASS();
}


//-------------------------------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    printf("\n=== CxProcess Tests ===\n\n");

    printf("--- run() tests ---\n");
    RUN_TEST(test_run_simple_command);
    RUN_TEST(test_run_command_with_args);
    RUN_TEST(test_run_failing_command);
    RUN_TEST(test_run_multiline_output);
    RUN_TEST(test_run_empty_command);
    RUN_TEST(test_run_captures_stderr);

    printf("\n--- parseBuildError() tests ---\n");
    RUN_TEST(test_parse_gcc_error_with_column);
    RUN_TEST(test_parse_gcc_error_no_column);
    RUN_TEST(test_parse_absolute_path);
    RUN_TEST(test_parse_relative_path);
    RUN_TEST(test_parse_no_error_pattern);
    RUN_TEST(test_parse_empty_line);
    RUN_TEST(test_parse_plain_text);
    RUN_TEST(test_parse_colon_but_no_number);
    RUN_TEST(test_parse_clang_warning);

    printf("\n=== Results: %d passed, %d failed ===\n\n", gTestsPassed, gTestsFailed);

    return gTestsFailed > 0 ? 1 : 0;
}
