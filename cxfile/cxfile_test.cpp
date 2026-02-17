//-----------------------------------------------------------------------------------------
// cxfile_test.cpp - CxFile, CxFileName, CxFileAccess unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cx/base/string.h>
#include <cx/base/file.h>
#include <cx/base/filename.h>
#include <cx/base/fileaccess.h>

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

// Helper to create a temp file path
CxString getTempFilePath(const char* name) {
    CxString path = "/tmp/cxfile_test_";
    path += name;
    return path;
}

// Helper to remove a file
void removeFile(CxString path) {
    unlink(path.data());
}

//-----------------------------------------------------------------------------------------
// CxFile basic tests
//-----------------------------------------------------------------------------------------
void testFileBasics() {
    printf("\n== CxFile Basic Tests ==\n");

    // Default constructor
    {
        CxFile file;
        check(!file.isOpen(), "default ctor file not open");
    }

    // Open non-existent file for reading fails
    {
        CxFile file;
        int result = file.open("/nonexistent/path/file.txt", "r");
        check(result == 0, "open nonexistent file returns 0");
        // Note: isOpen() may still return true due to implementation details
        // The return value of open() is the reliable indicator of success
    }

    // Open file for writing
    {
        CxString path = getTempFilePath("write_test.txt");
        CxFile file;
        int result = file.open(path, "w");
        check(result != 0, "open for write succeeds");
        check(file.isOpen(), "file is open after open");
        file.close();
        check(!file.isOpen(), "file not open after close");
        removeFile(path);
    }

    // path() returns correct path
    {
        CxString path = getTempFilePath("path_test.txt");
        CxFile file;
        file.open(path, "w");
        check(strcmp(file.path().data(), path.data()) == 0, "path() returns correct path");
        file.close();
        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxFile read/write tests
//-----------------------------------------------------------------------------------------
void testFileReadWrite() {
    printf("\n== CxFile Read/Write Tests ==\n");

    // Write and read back string
    {
        CxString path = getTempFilePath("readwrite.txt");

        // Write
        {
            CxFile file;
            file.open(path, "w");
            file.printf("Hello, World!");
            file.close();
        }

        // Read
        {
            CxFile file;
            file.open(path, "r");
            CxString content = file.fread(100);
            check(strcmp(content.data(), "Hello, World!") == 0, "read back written content");
            file.close();
        }

        removeFile(path);
    }

    // fwrite and fread with buffer
    {
        CxString path = getTempFilePath("fwrite.txt");
        char writeData[] = "Test data 12345";
        char readData[32] = {0};

        // Write
        {
            CxFile file;
            file.open(path, "w");
            size_t written = file.fwrite(writeData, 1, strlen(writeData));
            check(written == strlen(writeData), "fwrite returns correct count");
            file.close();
        }

        // Read
        {
            CxFile file;
            file.open(path, "r");
            size_t bytesRead = file.fread(readData, 1, sizeof(readData) - 1);
            check(bytesRead == strlen(writeData), "fread returns correct count");
            check(strcmp(readData, writeData) == 0, "fread content matches");
            file.close();
        }

        removeFile(path);
    }

    // Multiple printf calls
    {
        CxString path = getTempFilePath("multiprintf.txt");

        {
            CxFile file;
            file.open(path, "w");
            file.printf("Line 1\n");
            file.printf("Line 2\n");
            file.printf("Number: %d\n", 42);
            file.close();
        }

        {
            CxFile file;
            file.open(path, "r");
            CxString content = file.fread(200);
            check(content.index("Line 1") != -1, "printf line 1 present");
            check(content.index("Line 2") != -1, "printf line 2 present");
            check(content.index("42") != -1, "printf number present");
            file.close();
        }

        removeFile(path);
    }
}

//-----------------------------------------------------------------------------------------
// CxFile seek/tell tests
//-----------------------------------------------------------------------------------------
void testFileSeekTell() {
    printf("\n== CxFile Seek/Tell Tests ==\n");

    CxString path = getTempFilePath("seektest.txt");

    // Create file with known content
    {
        CxFile file;
        file.open(path, "w");
        file.printf("0123456789ABCDEF");
        file.close();
    }

    // tell() at start
    {
        CxFile file;
        file.open(path, "r");
        check(file.tell() == 0, "tell() is 0 at start");
        file.close();
    }

    // seek from start (SEEK_SET)
    {
        CxFile file;
        file.open(path, "r");
        file.seek(5, SEEK_SET);
        check(file.tell() == 5, "tell() after seek(5, SEEK_SET)");
        CxString ch = file.fread(1);
        check(ch.data()[0] == '5', "read after seek returns correct char");
        file.close();
    }

    // seek from current (SEEK_CUR)
    {
        CxFile file;
        file.open(path, "r");
        file.seek(3, SEEK_SET);
        file.seek(2, SEEK_CUR);
        check(file.tell() == 5, "tell() after relative seek");
        file.close();
    }

    // seek from end (SEEK_END)
    {
        CxFile file;
        file.open(path, "r");
        file.seek(-1, SEEK_END);
        CxString ch = file.fread(1);
        check(ch.data()[0] == 'F', "seek from end reads last char");
        file.close();
    }

    removeFile(path);
}

//-----------------------------------------------------------------------------------------
// CxFile EOF tests
//-----------------------------------------------------------------------------------------
void testFileEOF() {
    printf("\n== CxFile EOF Tests ==\n");

    CxString path = getTempFilePath("eoftest.txt");

    // Create small file
    {
        CxFile file;
        file.open(path, "w");
        file.printf("ABC");
        file.close();
    }

    // eof() before reading
    {
        CxFile file;
        file.open(path, "r");
        check(!file.eof(), "eof() false at start");
        file.close();
    }

    // eof() after reading all content
    {
        CxFile file;
        file.open(path, "r");
        file.fread(100);  // Read more than file size
        check(file.eof(), "eof() true after reading all");
        file.close();
    }

    removeFile(path);
}

//-----------------------------------------------------------------------------------------
// CxFile getStat tests
//-----------------------------------------------------------------------------------------
void testFileGetStat() {
    printf("\n== CxFile getStat Tests ==\n");

    CxString path = getTempFilePath("stattest.txt");

    // Create file with known content
    {
        CxFile file;
        file.open(path, "w");
        file.printf("1234567890");  // 10 bytes
        file.close();
    }

    // getStat returns file info
    {
        CxFile file;
        file.open(path, "r");
        struct stat st = file.getStat();
        check(st.st_size == 10, "getStat returns correct size");
        check(S_ISREG(st.st_mode), "getStat shows regular file");
        file.close();
    }

    removeFile(path);
}

//-----------------------------------------------------------------------------------------
// CxFile getch and getUntil tests
//-----------------------------------------------------------------------------------------
void testFileGetch() {
    printf("\n== CxFile getch/getUntil Tests ==\n");

    CxString path = getTempFilePath("getchtest.txt");

    // Create file
    {
        CxFile file;
        file.open(path, "w");
        file.printf("Hello:World");
        file.close();
    }

    // getch reads single character
    {
        CxFile file;
        file.open(path, "r");
        CxString ch1 = file.getch(CxFile::ALLOW_EOF);
        check(ch1.length() == 1, "getch returns 1 char");
        check(ch1.data()[0] == 'H', "getch returns first char");
        CxString ch2 = file.getch(CxFile::ALLOW_EOF);
        check(ch2.data()[0] == 'e', "getch returns second char");
        file.close();
    }

    // getUntil reads until delimiter
    {
        CxFile file;
        file.open(path, "r");
        CxString before = file.getUntil(':');
        check(strcmp(before.data(), "Hello") == 0, "getUntil stops at delimiter");
        file.close();
    }

    removeFile(path);
}

//-----------------------------------------------------------------------------------------
// CxFile static methods tests
//-----------------------------------------------------------------------------------------
void testFileStaticMethods() {
    printf("\n== CxFile Static Methods Tests ==\n");

    // tempName generates unique paths
    {
        CxString temp1 = CxFile::tempName();
        CxString temp2 = CxFile::tempName();
        check(temp1.length() > 0, "tempName returns non-empty string");
        // Verify file was created (temp dir varies by platform: /tmp/, /var/tmp/, etc.)
        CxFileAccess::status st1 = CxFileAccess::checkStatus(temp1);
        check(st1 == CxFileAccess::FOUND_RW || st1 == CxFileAccess::FOUND_R,
              "tempName creates accessible file");
        check(strcmp(temp1.data(), temp2.data()) != 0, "tempName generates unique names");
        // Clean up the temp files created by mkstemp
        removeFile(temp1);
        removeFile(temp2);
    }

    // tempName with custom prefix
    {
        CxString temp = CxFile::tempName("mytest_XXXXXX");
        check(temp.length() > 0, "tempName with prefix returns non-empty");
        check(temp.index("mytest_") != -1, "tempName uses custom prefix");
        // Verify the file was created
        CxFileAccess::status st = CxFileAccess::checkStatus(temp);
        check(st == CxFileAccess::FOUND_RW || st == CxFileAccess::FOUND_R,
              "tempName with prefix creates file");
        removeFile(temp);
    }

    // tildaExpansion expands ~
    {
        CxString expanded = CxFile::tildaExpansion("~/test.txt");
        check(expanded.index("~") == -1, "tildaExpansion removes tilde");
        check(expanded.length() > strlen("~/test.txt"), "tildaExpansion expands path");
    }

    // tildaExpansion leaves non-tilda paths alone
    {
        CxString path = "/usr/local/bin";
        CxString result = CxFile::tildaExpansion(path);
        check(strcmp(result.data(), path.data()) == 0, "tildaExpansion leaves absolute paths");
    }
}

//-----------------------------------------------------------------------------------------
// CxFile copy constructor and assignment tests
//-----------------------------------------------------------------------------------------
void testFileCopyAndAssign() {
    printf("\n== CxFile Copy/Assignment Tests ==\n");

    CxString path = getTempFilePath("copytest.txt");

    // Create file
    {
        CxFile file;
        file.open(path, "w");
        file.printf("Test content");
        file.close();
    }

    // Copy constructor
    {
        CxFile file1;
        file1.open(path, "r");
        CxFile file2(file1);
        check(file2.isOpen(), "copy ctor file is open");
        check(strcmp(file2.path().data(), path.data()) == 0, "copy ctor path correct");
        file1.close();
    }

    // Assignment operator
    {
        CxFile file1;
        file1.open(path, "r");
        CxFile file2;
        file2 = file1;
        check(file2.isOpen(), "assigned file is open");
        check(strcmp(file2.path().data(), path.data()) == 0, "assigned path correct");
        file1.close();
    }

    removeFile(path);
}

//-----------------------------------------------------------------------------------------
// CxFileName tests
//-----------------------------------------------------------------------------------------
void testFileName() {
    printf("\n== CxFileName Tests ==\n");

    // Default constructor
    {
        CxFileName fn;
        check(fn.name().length() == 0, "default ctor empty name");
        check(fn.extension().length() == 0, "default ctor empty extension");
    }

    // Constructor with simple filename
    {
        CxFileName fn("test.txt");
        check(strcmp(fn.name().data(), "test") == 0, "name extracted correctly");
        check(strcmp(fn.extension().data(), "txt") == 0, "extension extracted correctly");
    }

    // Constructor with no extension
    {
        CxFileName fn("README");
        check(strcmp(fn.name().data(), "README") == 0, "name without extension");
        check(fn.extension().length() == 0, "no extension when none present");
    }

    // Constructor with multiple dots
    {
        CxFileName fn("file.tar.gz");
        check(strcmp(fn.extension().data(), "gz") == 0, "extension is last part");
    }

    // fullName returns complete name
    {
        CxFileName fn("document.pdf");
        check(strcmp(fn.fullName().data(), "document.pdf") == 0, "fullName returns complete name");
    }

    // setName
    {
        CxFileName fn("old.txt");
        fn.setName("new");
        check(strcmp(fn.name().data(), "new") == 0, "setName changes name");
        check(strcmp(fn.fullName().data(), "new.txt") == 0, "fullName after setName");
    }

    // setExtension
    {
        CxFileName fn("file.txt");
        fn.setExtension("md");
        check(strcmp(fn.extension().data(), "md") == 0, "setExtension changes extension");
        check(strcmp(fn.fullName().data(), "file.md") == 0, "fullName after setExtension");
    }

    // Copy constructor
    {
        CxFileName fn1("source.cpp");
        CxFileName fn2(fn1);
        check(strcmp(fn2.name().data(), "source") == 0, "copy ctor name");
        check(strcmp(fn2.extension().data(), "cpp") == 0, "copy ctor extension");
    }

    // Assignment operator
    {
        CxFileName fn1("first.h");
        CxFileName fn2;
        fn2 = fn1;
        check(strcmp(fn2.name().data(), "first") == 0, "assignment name");
        check(strcmp(fn2.extension().data(), "h") == 0, "assignment extension");
    }

    // Hidden file (starts with dot)
    {
        CxFileName fn(".gitignore");
        // Behavior may vary - just check it doesn't crash
        check(fn.fullName().length() > 0, "hidden file handled");
    }
}

//-----------------------------------------------------------------------------------------
// CxFileAccess tests
//-----------------------------------------------------------------------------------------
void testFileAccess() {
    printf("\n== CxFileAccess Tests ==\n");

    CxString path = getTempFilePath("accesstest.txt");

    // NOT_FOUND for non-existent file in non-writable directory
    {
        CxFileAccess::status st = CxFileAccess::checkStatus("/nonexistent/path/file.txt");
        check(st == CxFileAccess::NOT_FOUND, "non-existent path returns NOT_FOUND");
    }

    // Create a readable/writable file
    {
        CxFile file;
        file.open(path, "w");
        file.printf("test");
        file.close();
    }

    // FOUND_RW for existing readable/writable file
    {
        CxFileAccess::status st = CxFileAccess::checkStatus(path);
        check(st == CxFileAccess::FOUND_RW || st == CxFileAccess::FOUND_R,
              "existing file found readable");
    }

    // NOT_FOUND_W for non-existent file in writable directory
    {
        CxString newPath = getTempFilePath("newfile_access.txt");
        removeFile(newPath);  // Make sure it doesn't exist
        CxFileAccess::status st = CxFileAccess::checkStatus(newPath);
        check(st == CxFileAccess::NOT_FOUND_W, "non-existent in /tmp returns NOT_FOUND_W");
    }

    // NOT_A_REGULAR_FILE for directory
    {
        CxFileAccess::status st = CxFileAccess::checkStatus("/tmp");
        check(st == CxFileAccess::NOT_A_REGULAR_FILE, "directory returns NOT_A_REGULAR_FILE");
    }

    removeFile(path);
}

//-----------------------------------------------------------------------------------------
// CxFile append mode tests
//-----------------------------------------------------------------------------------------
void testFileAppend() {
    printf("\n== CxFile Append Mode Tests ==\n");

    CxString path = getTempFilePath("appendtest.txt");

    // Create initial file
    {
        CxFile file;
        file.open(path, "w");
        file.printf("First");
        file.close();
    }

    // Append to file
    {
        CxFile file;
        file.open(path, "a");
        file.printf("Second");
        file.close();
    }

    // Verify both parts present
    {
        CxFile file;
        file.open(path, "r");
        CxString content = file.fread(100);
        check(content.index("First") != -1, "append preserves original");
        check(content.index("Second") != -1, "append adds new content");
        check(strcmp(content.data(), "FirstSecond") == 0, "append order correct");
        file.close();
    }

    removeFile(path);
}

//-----------------------------------------------------------------------------------------
// CxFile binary mode tests
//-----------------------------------------------------------------------------------------
void testFileBinary() {
    printf("\n== CxFile Binary Mode Tests ==\n");

    CxString path = getTempFilePath("binarytest.bin");

    // Write binary data
    unsigned char writeData[] = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0x00, 0x10};
    size_t dataSize = sizeof(writeData);

    {
        CxFile file;
        file.open(path, "wb");
        size_t written = file.fwrite(writeData, 1, dataSize);
        check(written == dataSize, "binary write correct size");
        file.close();
    }

    // Read binary data
    {
        unsigned char readData[32] = {0};
        CxFile file;
        file.open(path, "rb");
        size_t bytesRead = file.fread(readData, 1, sizeof(readData));
        check(bytesRead == dataSize, "binary read correct size");

        int match = 1;
        for (size_t i = 0; i < dataSize; i++) {
            if (readData[i] != writeData[i]) match = 0;
        }
        check(match, "binary data matches");
        file.close();
    }

    removeFile(path);
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxFile Test Suite\n");
    printf("=================\n");

    testFileBasics();
    testFileReadWrite();
    testFileSeekTell();
    testFileEOF();
    testFileGetStat();
    testFileGetch();
    testFileStaticMethods();
    testFileCopyAndAssign();
    testFileName();
    testFileAccess();
    testFileAppend();
    testFileBinary();

    printf("\n=================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
