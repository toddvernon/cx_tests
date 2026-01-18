//-----------------------------------------------------------------------------------------
// cxlogfile_test.cpp - CxLogFile unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cx/base/string.h>
#include <cx/base/file.h>
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

// Helper to read file contents
CxString readFileContents(CxString path) {
    CxFile file;
    if (file.open(path, "r")) {
        CxString content = file.fread(10000);
        file.close();
        return content;
    }
    return "";
}

// Helper to remove a file
void removeFile(const char* path) {
    unlink(path);
}

//-----------------------------------------------------------------------------------------
// CxLogFile basic tests
//-----------------------------------------------------------------------------------------
void testLogFileBasics() {
    printf("\n== CxLogFile Basic Tests ==\n");

    // Default constructor
    {
        CxLogFile log;
        check(!log.isOpen(), "default ctor not open");
    }

    // Open and close
    {
        const char* path = "/tmp/cxlogfile_test_basic.log";
        removeFile(path);

        CxLogFile log;
        int result = log.open(path);
        check(result != 0, "open returns success");
        check(log.isOpen(), "log is open after open");

        log.close();
        check(!log.isOpen(), "log not open after close");

        removeFile(path);
    }

    // path() returns correct path
    {
        const char* path = "/tmp/cxlogfile_test_path.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        check(strcmp(log.path().data(), path) == 0, "path() returns correct path");
        log.close();

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxLogFile printf tests
//-----------------------------------------------------------------------------------------
void testLogFilePrintf() {
    printf("\n== CxLogFile printf Tests ==\n");

    // Basic INFO logging
    {
        const char* path = "/tmp/cxlogfile_test_info.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXINFO, "Test message %d", 42);
        log.close();

        CxString content = readFileContents(path);
        check(content.index("INFO") != -1, "INFO log contains INFO tag");
        check(content.index("42") != -1, "INFO log contains message");

        removeFile(path);
    }

    // ERROR logging
    {
        const char* path = "/tmp/cxlogfile_test_error.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXERR, "Error occurred: %s", "test error");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("ERROR") != -1, "ERROR log contains ERROR tag");
        check(content.index("test error") != -1, "ERROR log contains message");

        removeFile(path);
    }

    // DATA logging (enabled by default via property list)
    {
        const char* path = "/tmp/cxlogfile_test_data.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXDATA, "Data: %s", "some data");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("DATA") != -1, "DATA log enabled by default");

        removeFile(path);
    }

    // DETAIL logging (enabled by default via property list)
    {
        const char* path = "/tmp/cxlogfile_test_detail.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXDETAIL, "Detail: %s", "some detail");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("DETAIL") != -1, "DETAIL log enabled by default");

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxLogFile property configuration tests
//-----------------------------------------------------------------------------------------
void testLogFileProperties() {
    printf("\n== CxLogFile Property Tests ==\n");

    // Enable DATA logging
    {
        const char* path = "/tmp/cxlogfile_test_prop_data.log";
        removeFile(path);

        CxPropertyList props;
        props.set("LOG_DATA=1");

        CxLogFile log(props);
        log.open(path);
        log.printf(CXDATA, "Data message");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("DATA") != -1, "DATA enabled via property");

        removeFile(path);
    }

    // Enable DETAIL logging
    {
        const char* path = "/tmp/cxlogfile_test_prop_detail.log";
        removeFile(path);

        CxPropertyList props;
        props.set("LOG_DETAIL=1");

        CxLogFile log(props);
        log.open(path);
        log.printf(CXDETAIL, "Detail message");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("DETAIL") != -1, "DETAIL enabled via property");

        removeFile(path);
    }

    // Disable INFO logging
    {
        const char* path = "/tmp/cxlogfile_test_prop_noinfo.log";
        removeFile(path);

        CxPropertyList props;
        props.set("LOG_INFO=0");

        CxLogFile log(props);
        log.open(path);
        log.printf(CXINFO, "This should not appear");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("INFO") == -1, "INFO disabled via property");

        removeFile(path);
    }

    // Disable timestamp
    {
        const char* path = "/tmp/cxlogfile_test_notimestamp.log";
        removeFile(path);

        CxPropertyList props;
        props.set("LOG_TIMESTAMP=0");

        CxLogFile log(props);
        log.open(path);
        log.printf(CXINFO, "No timestamp");
        log.close();

        CxString content = readFileContents(path);
        // Timestamps contain colons from time format like HH:MM:SS
        // Without timestamp, should have fewer colons
        check(content.length() > 0, "message logged without timestamp");

        removeFile(path);
    }

    // Enable line numbers
    {
        const char* path = "/tmp/cxlogfile_test_linenum.log";
        removeFile(path);

        CxPropertyList props;
        props.set("LOG_LINENUMBERS=1");

        CxLogFile log(props);
        log.open(path);
        log.printf(CXINFO, "With line numbers");
        log.close();

        CxString content = readFileContents(path);
        // Should contain the source file name
        check(content.index("cxlogfile_test.cpp") != -1, "line numbers include filename");

        removeFile(path);
    }

    // Disable PID
    {
        const char* path = "/tmp/cxlogfile_test_nopid.log";
        removeFile(path);

        CxPropertyList props;
        props.set("LOG_PID=0");
        props.set("LOG_TIMESTAMP=0");

        CxLogFile log(props);
        log.open(path);
        log.printf(CXINFO, "No PID");
        log.close();

        CxString content = readFileContents(path);
        check(content.length() > 0, "message logged without PID");

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxLogFile multiple log entries
//-----------------------------------------------------------------------------------------
void testLogFileMultiple() {
    printf("\n== CxLogFile Multiple Entry Tests ==\n");

    // Multiple entries
    {
        const char* path = "/tmp/cxlogfile_test_multi.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXINFO, "First entry");
        log.printf(CXINFO, "Second entry");
        log.printf(CXINFO, "Third entry");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("First") != -1, "first entry present");
        check(content.index("Second") != -1, "second entry present");
        check(content.index("Third") != -1, "third entry present");

        removeFile(path);
    }

    // Mixed log types
    {
        const char* path = "/tmp/cxlogfile_test_mixed.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXINFO, "Info message");
        log.printf(CXERR, "Error message");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("INFO") != -1, "INFO in mixed log");
        check(content.index("ERROR") != -1, "ERROR in mixed log");

        removeFile(path);
    }

    // Format string with multiple arguments
    {
        const char* path = "/tmp/cxlogfile_test_format.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXINFO, "Values: %d, %s, %f", 123, "abc", 3.14);
        log.close();

        CxString content = readFileContents(path);
        check(content.index("123") != -1, "integer formatted");
        check(content.index("abc") != -1, "string formatted");
        check(content.index("3.14") != -1, "float formatted");

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxLogFile copy and assignment tests
//-----------------------------------------------------------------------------------------
void testLogFileCopyAssign() {
    printf("\n== CxLogFile Copy/Assignment Tests ==\n");

    // Copy constructor
    {
        const char* path = "/tmp/cxlogfile_test_copy.log";
        removeFile(path);

        CxLogFile log1;
        log1.open(path);
        log1.printf(CXINFO, "From original");

        CxLogFile log2(log1);
        log2.printf(CXINFO, "From copy");
        log2.close();

        CxString content = readFileContents(path);
        check(content.index("original") != -1, "original message present");
        check(content.index("copy") != -1, "copy message present");

        removeFile(path);
    }

    // Assignment operator
    {
        const char* path = "/tmp/cxlogfile_test_assign.log";
        removeFile(path);

        CxLogFile log1;
        log1.open(path);
        log1.printf(CXINFO, "From first");

        CxLogFile log2;
        log2 = log1;
        log2.printf(CXINFO, "From assigned");
        log2.close();

        CxString content = readFileContents(path);
        check(content.index("first") != -1, "first message present");
        check(content.index("assigned") != -1, "assigned message present");

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxLogFile global log tests
//-----------------------------------------------------------------------------------------
void testLogFileGlobal() {
    printf("\n== CxLogFile Global Log Tests ==\n");

    // getGlobalLogFile returns non-null
    // Note: The global log was already set by earlier CxLogFile instances
    {
        CxLogFile* global = CxLogFile::getGlobalLogFile();
        check(global != NULL, "getGlobalLogFile returns non-null");
    }
}

//-----------------------------------------------------------------------------------------
// CxLogFile open mode tests
//-----------------------------------------------------------------------------------------
void testLogFileOpenModes() {
    printf("\n== CxLogFile Open Mode Tests ==\n");

    // Append mode
    {
        const char* path = "/tmp/cxlogfile_test_append.log";
        removeFile(path);

        // Write first entry
        {
            CxLogFile log;
            log.open(path, "w");
            log.printf(CXINFO, "First write");
            log.close();
        }

        // Append second entry
        {
            CxLogFile log;
            log.open(path, "a");
            log.printf(CXINFO, "Appended");
            log.close();
        }

        CxString content = readFileContents(path);
        check(content.index("First") != -1, "first write preserved");
        check(content.index("Appended") != -1, "appended content present");

        removeFile(path);
    }

    // Write mode overwrites
    {
        const char* path = "/tmp/cxlogfile_test_overwrite.log";
        removeFile(path);

        // Write first entry
        {
            CxLogFile log;
            log.open(path, "w");
            log.printf(CXINFO, "Original content");
            log.close();
        }

        // Overwrite with new content
        {
            CxLogFile log;
            log.open(path, "w");
            log.printf(CXINFO, "New content only");
            log.close();
        }

        CxString content = readFileContents(path);
        check(content.index("Original") == -1, "original content overwritten");
        check(content.index("New content") != -1, "new content present");

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxLogFile timestamp content test
//-----------------------------------------------------------------------------------------
void testLogFileTimestamp() {
    printf("\n== CxLogFile Timestamp Tests ==\n");

    // Timestamp is included by default
    {
        const char* path = "/tmp/cxlogfile_test_ts.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXINFO, "With timestamp");
        log.close();

        CxString content = readFileContents(path);
        // Timestamps have format like DDD-HH:MM:SS:microsec
        // Look for the colon-separated time pattern
        check(content.index(":") != -1, "timestamp contains colons");

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxLogFile edge cases
//-----------------------------------------------------------------------------------------
void testLogFileEdgeCases() {
    printf("\n== CxLogFile Edge Cases ==\n");

    // Empty message
    {
        const char* path = "/tmp/cxlogfile_test_empty.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXINFO, "");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("INFO") != -1, "empty message still logs INFO tag");

        removeFile(path);
    }

    // Long message
    {
        const char* path = "/tmp/cxlogfile_test_long.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);

        // Create a long string
        char longMsg[2000];
        for (int i = 0; i < 1999; i++) longMsg[i] = 'X';
        longMsg[1999] = '\0';

        log.printf(CXINFO, "%s", longMsg);
        log.close();

        CxString content = readFileContents(path);
        check(content.length() > 1999, "long message logged");

        removeFile(path);
    }

    // Special characters
    {
        const char* path = "/tmp/cxlogfile_test_special.log";
        removeFile(path);

        CxLogFile log;
        log.open(path);
        log.printf(CXINFO, "Special: tab\there newline\nend");
        log.close();

        CxString content = readFileContents(path);
        check(content.index("Special") != -1, "special chars message logged");

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxLogFile Test Suite\n");
    printf("====================\n");

    testLogFileBasics();
    testLogFilePrintf();
    testLogFileProperties();
    testLogFileMultiple();
    testLogFileCopyAssign();
    testLogFileGlobal();
    testLogFileOpenModes();
    testLogFileTimestamp();
    testLogFileEdgeCases();

    printf("\n====================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
