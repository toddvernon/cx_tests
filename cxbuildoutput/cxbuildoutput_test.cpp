//-------------------------------------------------------------------------------------------------
//
//  cxbuildoutput_test.cpp
//
//  Test suite for the BuildOutput and CxProcess classes.
//  Tests both the low-level process execution (CxProcess) and the higher-level
//  non-blocking build output system (BuildOutput).
//
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <unistd.h>
#include <cx/base/string.h>
#include <cx/process/process.h>
#include <cx/buildoutput/buildoutput.h>

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


//=================================================================================================
// CxProcess::run tests (basic process execution)
//=================================================================================================

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
    int result = proc.run("echo line1; echo line2; echo line3");
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
    // Command that writes to stderr (cat reliably fails on missing files on all platforms)
    int result = proc.run("cat /nonexistent_file_12345");
    ASSERT(result == 0);  // run() succeeded
    ASSERT(proc.getExitCode() != 0);  // cat should fail

    CxString output = proc.getOutput();
    // stderr should be captured (contains error message)
    ASSERT(output.length() > 0);
    PASS();
}


//=================================================================================================
// CxProcess::parseBuildError tests (error line parsing)
//=================================================================================================

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


//=================================================================================================
// BuildOutput tests - initial state
//=================================================================================================

TEST(test_buildoutput_initial_state)
{
    BuildOutput build;

    ASSERT(build.getState() == BUILD_IDLE);
    ASSERT(build.isRunning() == 0);
    ASSERT(build.isComplete() == 0);
    ASSERT(build.lineCount() == 0);
    ASSERT(build.errorCount() == 0);
    ASSERT(build.warningCount() == 0);
    ASSERT(build.exitCode() == -1);
    PASS();
}

TEST(test_buildoutput_lineAt_empty)
{
    BuildOutput build;

    ASSERT(build.lineAt(0) == NULL);
    ASSERT(build.lineAt(-1) == NULL);
    ASSERT(build.lineAt(100) == NULL);
    PASS();
}


//=================================================================================================
// BuildOutput tests - start and poll
//=================================================================================================

TEST(test_buildoutput_start_simple)
{
    BuildOutput build;

    int result = build.start("echo hello");
    ASSERT(result == 0);
    ASSERT(build.getState() == BUILD_RUNNING);
    ASSERT(build.isRunning() == 1);

    // Poll until complete
    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);  // 10ms
        maxPolls--;
    }

    ASSERT(build.isComplete() == 1);
    ASSERT(build.isRunning() == 0);
    ASSERT(build.exitCode() == 0);
    ASSERT(build.lineCount() >= 1);

    // Check that we captured "hello"
    BuildOutputLine *line = build.lineAt(0);
    ASSERT(line != NULL);
    ASSERT(line->text.index("hello") >= 0);
    PASS();
}

TEST(test_buildoutput_start_empty_command)
{
    BuildOutput build;

    int result = build.start("");
    ASSERT(result == -1);
    ASSERT(build.getState() == BUILD_IDLE);
    PASS();
}

TEST(test_buildoutput_start_failing_command)
{
    BuildOutput build;

    int result = build.start("false");
    ASSERT(result == 0);

    // Poll until complete
    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.isComplete() == 1);
    ASSERT(build.getState() == BUILD_ERROR);  // non-zero exit
    ASSERT(build.exitCode() == 1);
    PASS();
}

TEST(test_buildoutput_multiline)
{
    BuildOutput build;

    int result = build.start("echo line1; echo line2; echo line3");
    ASSERT(result == 0);

    // Poll until complete
    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.isComplete() == 1);
    ASSERT(build.lineCount() == 4);  // 3 lines + "Build Done"

    BuildOutputLine *line1 = build.lineAt(0);
    BuildOutputLine *line2 = build.lineAt(1);
    BuildOutputLine *line3 = build.lineAt(2);

    ASSERT(line1 != NULL);
    ASSERT(line2 != NULL);
    ASSERT(line3 != NULL);

    ASSERT(line1->text == "line1");
    ASSERT(line2->text == "line2");
    ASSERT(line3->text == "line3");
    PASS();
}


//=================================================================================================
// BuildOutput tests - line classification
//=================================================================================================

TEST(test_buildoutput_classify_error)
{
    BuildOutput build;

    // Simulate compiler error output
    int result = build.start("echo 'test.cpp:10:5: error: undefined reference'");
    ASSERT(result == 0);

    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.lineCount() == 2);  // 1 line + "Build Done"
    ASSERT(build.errorCount() == 1);
    ASSERT(build.warningCount() == 0);

    BuildOutputLine *line = build.lineAt(0);
    ASSERT(line != NULL);
    ASSERT(line->type == BUILD_LINE_ERROR);
    ASSERT(line->filename == "test.cpp");
    ASSERT(line->line == 10);
    ASSERT(line->column == 5);
    PASS();
}

TEST(test_buildoutput_classify_warning)
{
    BuildOutput build;

    int result = build.start("echo 'foo.c:20:1: warning: unused variable'");
    ASSERT(result == 0);

    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.lineCount() == 2);  // 1 line + "Build Done"
    ASSERT(build.errorCount() == 0);
    ASSERT(build.warningCount() == 1);

    BuildOutputLine *line = build.lineAt(0);
    ASSERT(line != NULL);
    ASSERT(line->type == BUILD_LINE_WARNING);
    ASSERT(line->filename == "foo.c");
    ASSERT(line->line == 20);
    PASS();
}

TEST(test_buildoutput_classify_note)
{
    BuildOutput build;

    int result = build.start("echo 'bar.h:5:1: note: candidate function'");
    ASSERT(result == 0);

    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.lineCount() == 2);  // 1 line + "Build Done"

    BuildOutputLine *line = build.lineAt(0);
    ASSERT(line != NULL);
    ASSERT(line->type == BUILD_LINE_NOTE);
    PASS();
}

TEST(test_buildoutput_classify_command)
{
    BuildOutput build;

    int result = build.start("echo 'g++ -c main.cpp -o main.o'");
    ASSERT(result == 0);

    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.lineCount() == 2);  // 1 line + "Build Done"

    BuildOutputLine *line = build.lineAt(0);
    ASSERT(line != NULL);
    ASSERT(line->type == BUILD_LINE_COMMAND);
    PASS();
}

TEST(test_buildoutput_classify_plain)
{
    BuildOutput build;

    int result = build.start("echo 'Building target...'");
    ASSERT(result == 0);

    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.lineCount() == 2);  // 1 line + "Build Done"

    BuildOutputLine *line = build.lineAt(0);
    ASSERT(line != NULL);
    ASSERT(line->type == BUILD_LINE_PLAIN);
    PASS();
}

TEST(test_buildoutput_mixed_output)
{
    BuildOutput build;

    // Simulate mixed build output
    CxString cmd = "echo 'g++ -c test.cpp'; ";
    cmd += "echo 'test.cpp:10:5: error: undefined'; ";
    cmd += "echo 'test.cpp:15:1: warning: unused'; ";
    cmd += "echo 'Build failed'";

    int result = build.start(cmd);
    ASSERT(result == 0);

    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.lineCount() == 5);  // 4 lines + "Build Done"
    ASSERT(build.errorCount() == 1);
    ASSERT(build.warningCount() == 1);

    ASSERT(build.lineAt(0)->type == BUILD_LINE_COMMAND);
    ASSERT(build.lineAt(1)->type == BUILD_LINE_ERROR);
    ASSERT(build.lineAt(2)->type == BUILD_LINE_WARNING);
    ASSERT(build.lineAt(3)->type == BUILD_LINE_PLAIN);
    PASS();
}


//=================================================================================================
// BuildOutput tests - clear and reuse
//=================================================================================================

TEST(test_buildoutput_clear)
{
    BuildOutput build;

    // Run a command
    build.start("echo test");
    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.lineCount() > 0);
    ASSERT(build.isComplete() == 1);

    // Clear and verify state reset
    build.clear();

    ASSERT(build.getState() == BUILD_IDLE);
    ASSERT(build.lineCount() == 0);
    ASSERT(build.errorCount() == 0);
    ASSERT(build.warningCount() == 0);
    ASSERT(build.exitCode() == -1);
    PASS();
}

TEST(test_buildoutput_reuse_after_clear)
{
    BuildOutput build;

    // First run
    build.start("echo first");
    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }
    ASSERT(build.lineAt(0)->text.index("first") >= 0);

    // Second run (start() calls clear internally)
    build.start("echo second");
    maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    ASSERT(build.lineCount() == 2);  // 1 line + "Build Done"
    ASSERT(build.lineAt(0)->text.index("second") >= 0);
    PASS();
}


//=================================================================================================
// BuildOutput tests - getCommand
//=================================================================================================

TEST(test_buildoutput_get_command)
{
    BuildOutput build;

    build.start("echo hello world");

    CxString cmd = build.getCommand();
    ASSERT(cmd == "echo hello world");

    // Poll to complete
    int maxPolls = 100;
    while (build.isRunning() && maxPolls > 0) {
        build.poll();
        usleep(10000);
        maxPolls--;
    }

    // Command should still be available after completion
    ASSERT(build.getCommand() == "echo hello world");
    PASS();
}


//-------------------------------------------------------------------------------------------------
// Main
//-------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    printf("\n=== BuildOutput & CxProcess Tests ===\n\n");

    printf("--- CxProcess::run() tests ---\n");
    RUN_TEST(test_run_simple_command);
    RUN_TEST(test_run_command_with_args);
    RUN_TEST(test_run_failing_command);
    RUN_TEST(test_run_multiline_output);
    RUN_TEST(test_run_empty_command);
    RUN_TEST(test_run_captures_stderr);

    printf("\n--- CxProcess::parseBuildError() tests ---\n");
    RUN_TEST(test_parse_gcc_error_with_column);
    RUN_TEST(test_parse_gcc_error_no_column);
    RUN_TEST(test_parse_absolute_path);
    RUN_TEST(test_parse_relative_path);
    RUN_TEST(test_parse_no_error_pattern);
    RUN_TEST(test_parse_empty_line);
    RUN_TEST(test_parse_plain_text);
    RUN_TEST(test_parse_colon_but_no_number);
    RUN_TEST(test_parse_clang_warning);

    printf("\n--- BuildOutput initial state tests ---\n");
    RUN_TEST(test_buildoutput_initial_state);
    RUN_TEST(test_buildoutput_lineAt_empty);

    printf("\n--- BuildOutput start/poll tests ---\n");
    RUN_TEST(test_buildoutput_start_simple);
    RUN_TEST(test_buildoutput_start_empty_command);
    RUN_TEST(test_buildoutput_start_failing_command);
    RUN_TEST(test_buildoutput_multiline);

    printf("\n--- BuildOutput line classification tests ---\n");
    RUN_TEST(test_buildoutput_classify_error);
    RUN_TEST(test_buildoutput_classify_warning);
    RUN_TEST(test_buildoutput_classify_note);
    RUN_TEST(test_buildoutput_classify_command);
    RUN_TEST(test_buildoutput_classify_plain);
    RUN_TEST(test_buildoutput_mixed_output);

    printf("\n--- BuildOutput clear/reuse tests ---\n");
    RUN_TEST(test_buildoutput_clear);
    RUN_TEST(test_buildoutput_reuse_after_clear);
    RUN_TEST(test_buildoutput_get_command);

    printf("\n=== Results: %d passed, %d failed ===\n\n", gTestsPassed, gTestsFailed);

    return gTestsFailed > 0 ? 1 : 0;
}
