//-----------------------------------------------------------------------------------------
// cxlog_test.cpp - CxLogFile unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <cx/base/string.h>
#include <cx/base/prop.h>
#include <cx/log/logfile.h>

//-----------------------------------------------------------------------------------------
// Test harness
//-----------------------------------------------------------------------------------------
static int testsPassed = 0;
static int testsFailed = 0;

void check(int condition, const char* testName) {
    if (condition) {
        testsPassed++;
        printf("  PASS: %s\n", testName);
    } else {
        testsFailed++;
        printf("  FAIL: %s\n", testName);
    }
}

//-----------------------------------------------------------------------------------------
// Helper: check if string contains substring
//-----------------------------------------------------------------------------------------
int contains(const CxString& haystack, const char* needle) {
    return strstr(haystack.data(), needle) != NULL;
}

//-----------------------------------------------------------------------------------------
// Helper: read file contents
//-----------------------------------------------------------------------------------------
CxString readFileContents(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) return CxString("");

    CxString contents;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), f)) {
        contents += buffer;
    }
    fclose(f);
    return contents;
}

//-----------------------------------------------------------------------------------------
// Helper: create temp file path
//-----------------------------------------------------------------------------------------
CxString getTempFilePath(const char* suffix) {
    char path[256];
    sprintf(path, "/tmp/cxlog_test_%d_%s.log", getpid(), suffix);
    return CxString(path);
}

//-----------------------------------------------------------------------------------------
// Helper: cleanup temp file
//-----------------------------------------------------------------------------------------
void removeTempFile(CxString path) {
    unlink(path.data());
}

//-----------------------------------------------------------------------------------------
// Constructor tests
//-----------------------------------------------------------------------------------------
void testConstructor() {
    printf("\n== CxLogFile Constructor Tests ==\n");

    // Default constructor
    {
        CxLogFile log;
        check(1, "default constructor creates log object");
    }

    // Constructor with properties
    {
        CxPropertyList props;
        props.set("LOG_INFO=1");
        props.set("LOG_ERROR=1");
        CxLogFile log(props);
        check(1, "constructor with properties works");
    }
}

//-----------------------------------------------------------------------------------------
// Open/Close tests
//-----------------------------------------------------------------------------------------
void testOpenClose() {
    printf("\n== CxLogFile Open/Close Tests ==\n");

    CxString tempPath = getTempFilePath("open");

    // Open file
    {
        CxLogFile log;
        int result = log.open(tempPath);
        check(result == 1, "open returns success");
        check(log.isOpen() == 1, "isOpen returns true after open");
        log.close();
    }

    // File was created
    {
        FILE* f = fopen(tempPath.data(), "r");
        check(f != NULL, "log file was created");
        if (f) fclose(f);
    }

    // Close
    {
        CxLogFile log;
        log.open(tempPath);
        log.close();
        check(log.isOpen() == 0, "isOpen returns false after close");
    }

    // Open with mode
    {
        CxLogFile log;
        int result = log.open(tempPath, "a");
        check(result == 1, "open with mode 'a' succeeds");
        log.close();
    }

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Path tests
//-----------------------------------------------------------------------------------------
void testPath() {
    printf("\n== CxLogFile Path Tests ==\n");

    CxString tempPath = getTempFilePath("path");

    CxLogFile log;
    log.open(tempPath);
    CxString retrievedPath = log.path();
    check(retrievedPath == tempPath, "path() returns correct path");
    log.close();

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Printf tests - basic log types
//-----------------------------------------------------------------------------------------
void testPrintfBasic() {
    printf("\n== CxLogFile Printf Basic Tests ==\n");

    CxString tempPath = getTempFilePath("printf");

    // Test INFO logging
    {
        CxPropertyList props;
        props.set("LOG_INFO=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");
        props.set("LOG_LINENUMBERS=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "test message");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "[  INFO"), "INFO message contains [  INFO tag");
        check(contains(contents, "test message"), "INFO message contains user text");
    }

    // Test ERROR logging
    {
        CxPropertyList props;
        props.set("LOG_ERROR=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");
        props.set("LOG_LINENUMBERS=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXERR, "error message");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "[ ERROR"), "ERROR message contains [ ERROR tag");
        check(contains(contents, "error message"), "ERROR message contains user text");
    }

    // Test DATA logging
    {
        CxPropertyList props;
        props.set("LOG_DATA=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");
        props.set("LOG_LINENUMBERS=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXDATA, "data message");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "[  DATA"), "DATA message contains [  DATA tag");
    }

    // Test DETAIL logging
    {
        CxPropertyList props;
        props.set("LOG_DETAIL=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");
        props.set("LOG_LINENUMBERS=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXDETAIL, "detail message");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "[DETAIL"), "DETAIL message contains [DETAIL tag");
    }

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Printf with format arguments
//-----------------------------------------------------------------------------------------
void testPrintfFormat() {
    printf("\n== CxLogFile Printf Format Tests ==\n");

    CxString tempPath = getTempFilePath("format");

    CxPropertyList props;
    props.set("LOG_INFO=1");
    props.set("LOG_TIMESTAMP=0");
    props.set("LOG_PID=0");
    props.set("LOG_THREADID=0");
    props.set("LOG_LINENUMBERS=0");

    // Integer format
    {
        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "value=%d", 42);
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "value=42"), "printf formats integer correctly");
    }

    // String format
    {
        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "name=%s", "test");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "name=test"), "printf formats string correctly");
    }

    // Multiple arguments
    {
        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "a=%d b=%d c=%d", 1, 2, 3);
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "a=1 b=2 c=3"), "printf formats multiple args");
    }

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Log flag tests (enable/disable)
//-----------------------------------------------------------------------------------------
void testLogFlags() {
    printf("\n== CxLogFile Log Flag Tests ==\n");

    CxString tempPath = getTempFilePath("flags");

    // Disable INFO logging
    {
        CxPropertyList props;
        props.set("LOG_INFO=0");  // Disable
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "should not appear");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(!contains(contents, "should not appear"), "disabled INFO not logged");
    }

    // Enable ERROR, disable others
    {
        CxPropertyList props;
        props.set("LOG_INFO=0");
        props.set("LOG_ERROR=1");
        props.set("LOG_DATA=0");
        props.set("LOG_DETAIL=0");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "info hidden");
        log.printf(CXERR, "error visible");
        log.printf(CXDATA, "data hidden");
        log.printf(CXDETAIL, "detail hidden");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        check(!contains(contents, "info hidden"), "INFO disabled correctly");
        check(contains(contents, "error visible"), "ERROR enabled correctly");
        check(!contains(contents, "data hidden"), "DATA disabled correctly");
        check(!contains(contents, "detail hidden"), "DETAIL disabled correctly");
    }

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Timestamp tests
//-----------------------------------------------------------------------------------------
void testTimestamp() {
    printf("\n== CxLogFile Timestamp Tests ==\n");

    CxString tempPath = getTempFilePath("timestamp");

    // With timestamp
    {
        CxPropertyList props;
        props.set("LOG_INFO=1");
        props.set("LOG_TIMESTAMP=1");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");
        props.set("LOG_LINENUMBERS=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "with timestamp");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        // Timestamp format includes day-hour:minute:second
        check(contains(contents, ":"), "timestamp includes time separators");
    }

    // Without timestamp
    {
        CxPropertyList props;
        props.set("LOG_INFO=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");
        props.set("LOG_LINENUMBERS=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "no timestamp");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        // Should be shorter without timestamp
        check(contents.length() > 0, "message logged without timestamp");
    }

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// PID tests
//-----------------------------------------------------------------------------------------
void testPID() {
    printf("\n== CxLogFile PID Tests ==\n");

    CxString tempPath = getTempFilePath("pid");

    // With PID
    {
        CxPropertyList props;
        props.set("LOG_INFO=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=1");
        props.set("LOG_THREADID=0");
        props.set("LOG_LINENUMBERS=0");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "with pid");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        // PID should be a number in the output
        char pidStr[32];
        sprintf(pidStr, "%d", getpid());
        check(contains(contents, pidStr), "PID appears in log output");
    }

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Line number tests
//-----------------------------------------------------------------------------------------
void testLineNumbers() {
    printf("\n== CxLogFile Line Number Tests ==\n");

    CxString tempPath = getTempFilePath("lineno");

    // With line numbers
    {
        CxPropertyList props;
        props.set("LOG_INFO=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");
        props.set("LOG_LINENUMBERS=1");

        CxLogFile log(props);
        log.open(tempPath);
        log.printf(CXINFO, "with line numbers");
        log.close();

        CxString contents = readFileContents(tempPath.data());
        // Should contain the filename
        check(contains(contents, "cxlog_test.cpp"), "filename appears with line numbers");
    }

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Copy constructor and assignment tests
//-----------------------------------------------------------------------------------------
void testCopyAndAssignment() {
    printf("\n== CxLogFile Copy/Assignment Tests ==\n");

    CxString tempPath = getTempFilePath("copy");

    // Copy constructor
    {
        CxPropertyList props;
        props.set("LOG_INFO=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");

        CxLogFile log1(props);
        log1.open(tempPath);

        CxLogFile log2(log1);  // Copy constructor
        log2.printf(CXINFO, "from copy");
        log2.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "from copy"), "copy constructor shares file handle");
    }

    // Assignment operator
    {
        CxPropertyList props;
        props.set("LOG_INFO=1");
        props.set("LOG_TIMESTAMP=0");
        props.set("LOG_PID=0");
        props.set("LOG_THREADID=0");

        CxLogFile log1(props);
        log1.open(tempPath);

        CxLogFile log2;
        log2 = log1;  // Assignment
        log2.printf(CXINFO, "from assignment");
        log2.close();

        CxString contents = readFileContents(tempPath.data());
        check(contains(contents, "from assignment"), "assignment operator shares file handle");
    }

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Global log file tests
//-----------------------------------------------------------------------------------------
void testGlobalLogFile() {
    printf("\n== CxLogFile Global Log Tests ==\n");

    // Get global log file
    {
        CxLogFile* pGlobal = CxLogFile::getGlobalLogFile();
        check(pGlobal != NULL, "getGlobalLogFile returns non-null");
    }

    // Global log file is same on multiple calls
    {
        CxLogFile* pGlobal1 = CxLogFile::getGlobalLogFile();
        CxLogFile* pGlobal2 = CxLogFile::getGlobalLogFile();
        check(pGlobal1 == pGlobal2, "getGlobalLogFile returns same instance");
    }
}

//-----------------------------------------------------------------------------------------
// Multiple messages test
//-----------------------------------------------------------------------------------------
void testMultipleMessages() {
    printf("\n== CxLogFile Multiple Messages Tests ==\n");

    CxString tempPath = getTempFilePath("multi");

    CxPropertyList props;
    props.set("LOG_INFO=1");
    props.set("LOG_ERROR=1");
    props.set("LOG_TIMESTAMP=0");
    props.set("LOG_PID=0");
    props.set("LOG_THREADID=0");
    props.set("LOG_LINENUMBERS=0");

    CxLogFile log(props);
    log.open(tempPath);

    log.printf(CXINFO, "message one");
    log.printf(CXINFO, "message two");
    log.printf(CXERR, "error one");
    log.printf(CXINFO, "message three");

    log.close();

    CxString contents = readFileContents(tempPath.data());
    check(contains(contents, "message one"), "first message logged");
    check(contains(contents, "message two"), "second message logged");
    check(contains(contents, "error one"), "error message logged");
    check(contains(contents, "message three"), "third message logged");

    // Count occurrences of INFO tag
    int infoCount = 0;
    const char* p = contents.data();
    while ((p = strstr(p, "[  INFO")) != NULL) {
        infoCount++;
        p++;
    }
    check(infoCount == 3, "correct number of INFO entries");

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Append mode test
//-----------------------------------------------------------------------------------------
void testAppendMode() {
    printf("\n== CxLogFile Append Mode Tests ==\n");

    CxString tempPath = getTempFilePath("append");

    CxPropertyList props;
    props.set("LOG_INFO=1");
    props.set("LOG_TIMESTAMP=0");
    props.set("LOG_PID=0");
    props.set("LOG_THREADID=0");
    props.set("LOG_LINENUMBERS=0");

    // Write first message
    {
        CxLogFile log(props);
        log.open(tempPath, "w");
        log.printf(CXINFO, "first");
        log.close();
    }

    // Append second message
    {
        CxLogFile log(props);
        log.open(tempPath, "a");
        log.printf(CXINFO, "second");
        log.close();
    }

    CxString contents = readFileContents(tempPath.data());
    check(contains(contents, "first"), "first message preserved in append");
    check(contains(contents, "second"), "second message appended");

    removeTempFile(tempPath);
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxLogFile Test Suite\n");
    printf("====================\n");

    testConstructor();
    testOpenClose();
    testPath();
    testPrintfBasic();
    testPrintfFormat();
    testLogFlags();
    testTimestamp();
    testPID();
    testLineNumbers();
    testCopyAndAssignment();
    testGlobalLogFile();
    testMultipleMessages();
    testAppendMode();

    printf("\n====================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
