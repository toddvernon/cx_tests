//-----------------------------------------------------------------------------------------
// cxdirectory_test.cpp - CxDirectory unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cx/base/string.h>
#include <cx/base/file.h>
#include <cx/base/directory.h>

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

// Helper to create a directory
int createDir(const char* path) {
    return mkdir(path, 0755);
}

// Helper to remove a directory
void removeDir(const char* path) {
    rmdir(path);
}

// Helper to create a file with content
void createFile(const char* path, const char* content) {
    CxFile file;
    file.open(path, "w");
    file.printf("%s", content);
    file.close();
}

// Helper to remove a file
void removeFile(const char* path) {
    unlink(path);
}

//-----------------------------------------------------------------------------------------
// Setup/Teardown test directory structure
//-----------------------------------------------------------------------------------------
static const char* TEST_BASE = "/tmp/cxdir_test";

void setupTestStructure() {
    // Create test directory structure:
    // /tmp/cxdir_test/
    //   file1.txt
    //   file2.txt
    //   data.csv
    //   readme.md
    //   subdir/
    //     nested.txt

    createDir(TEST_BASE);

    CxString path;

    path = TEST_BASE; path += "/file1.txt";
    createFile(path.data(), "content1");

    path = TEST_BASE; path += "/file2.txt";
    createFile(path.data(), "content2");

    path = TEST_BASE; path += "/data.csv";
    createFile(path.data(), "a,b,c");

    path = TEST_BASE; path += "/readme.md";
    createFile(path.data(), "# README");

    path = TEST_BASE; path += "/subdir";
    createDir(path.data());

    path = TEST_BASE; path += "/subdir/nested.txt";
    createFile(path.data(), "nested content");
}

void teardownTestStructure() {
    CxString path;

    path = TEST_BASE; path += "/subdir/nested.txt";
    removeFile(path.data());

    path = TEST_BASE; path += "/subdir";
    removeDir(path.data());

    path = TEST_BASE; path += "/file1.txt";
    removeFile(path.data());

    path = TEST_BASE; path += "/file2.txt";
    removeFile(path.data());

    path = TEST_BASE; path += "/data.csv";
    removeFile(path.data());

    path = TEST_BASE; path += "/readme.md";
    removeFile(path.data());

    removeDir(TEST_BASE);
}

//-----------------------------------------------------------------------------------------
// CxDirectory basic constructor tests
//-----------------------------------------------------------------------------------------
void testDirectoryBasics() {
    printf("\n== CxDirectory Basic Tests ==\n");

    // Default constructor
    {
        CxDirectory dir;
        check(dir.pathIsRelative(), "default ctor is relative path");
        check(dir.directoryPath().length() == 0, "default ctor empty directory path");
    }

    // Constructor with existing directory
    {
        CxDirectory dir(TEST_BASE);
        check(dir.pathIsDirectory(), "directory path recognized as directory");
        check(!dir.pathIsFile(), "directory path not a file");
        check(!dir.pathIsInvalid(), "existing directory not invalid");
    }

    // Constructor with existing file
    {
        CxString filePath = TEST_BASE;
        filePath += "/file1.txt";
        CxDirectory dir(filePath);
        check(dir.pathIsFile(), "file path recognized as file");
        check(!dir.pathIsDirectory(), "file path not a directory");
        check(!dir.pathIsInvalid(), "existing file not invalid");
    }

    // Constructor with non-existent path
    {
        CxDirectory dir("/nonexistent/path/somewhere");
        check(dir.pathIsInvalid(), "nonexistent path is invalid");
        check(!dir.pathIsFile(), "invalid path not a file");
        check(!dir.pathIsDirectory(), "invalid path not a directory");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory path type tests
//-----------------------------------------------------------------------------------------
void testDirectoryPathType() {
    printf("\n== CxDirectory Path Type Tests ==\n");

    // Absolute path
    {
        CxDirectory dir(TEST_BASE);
        check(dir.pathIsAbsolute(), "absolute path recognized");
        check(!dir.pathIsRelative(), "absolute path not relative");
    }

    // Relative path (current directory)
    {
        CxDirectory dir(".");
        check(dir.pathIsRelative(), "relative path recognized");
        check(!dir.pathIsAbsolute(), "relative path not absolute");
    }

    // Relative path with subdirectory notation
    {
        CxDirectory dir("subdir/file.txt");
        check(dir.pathIsRelative(), "subdir path is relative");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory directoryPath tests
//-----------------------------------------------------------------------------------------
void testDirectoryPath() {
    printf("\n== CxDirectory directoryPath Tests ==\n");

    // Directory path for a directory
    {
        CxDirectory dir(TEST_BASE);
        CxString dpath = dir.directoryPath();
        check(dpath.index("cxdir_test") != -1, "directoryPath contains dir name");
        // Should end with /
        check(dpath.data()[dpath.length()-1] == '/', "directoryPath ends with /");
    }

    // Directory path for a file
    {
        CxString filePath = TEST_BASE;
        filePath += "/file1.txt";
        CxDirectory dir(filePath);
        CxString dpath = dir.directoryPath();
        check(dpath.index("cxdir_test") != -1, "file's directoryPath contains parent dir");
        check(dpath.index("file1.txt") == -1, "file's directoryPath excludes filename");
    }

    // Root directory
    {
        CxDirectory dir("/tmp");
        CxString dpath = dir.directoryPath();
        check(dpath.index("tmp") != -1, "root subdir in path");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory fileNameList tests
//-----------------------------------------------------------------------------------------
void testFileNameList() {
    printf("\n== CxDirectory fileNameList Tests ==\n");

    // List files in directory
    {
        CxDirectory dir(TEST_BASE);
        CxSList<CxFileName> files = dir.fileNameList();

        // Should have at least 4 files + subdir + . and ..
        check(files.entries() >= 4, "fileNameList has expected entries");

        // Check for specific files
        int foundTxt = 0;
        int foundCsv = 0;
        int foundMd = 0;

        for (int i = 0; i < files.entries(); i++) {
            CxFileName fn = files.at(i);
            if (strcmp(fn.extension().data(), "txt") == 0) foundTxt++;
            if (strcmp(fn.extension().data(), "csv") == 0) foundCsv++;
            if (strcmp(fn.extension().data(), "md") == 0) foundMd++;
        }

        check(foundTxt >= 2, "found txt files");
        check(foundCsv >= 1, "found csv file");
        check(foundMd >= 1, "found md file");
    }

    // File list for a file path contains just that file
    {
        CxString filePath = TEST_BASE;
        filePath += "/file1.txt";
        CxDirectory dir(filePath);
        CxSList<CxFileName> files = dir.fileNameList();
        check(files.entries() == 1, "file path has one entry in list");
        if (files.entries() > 0) {
            check(strcmp(files.at(0).fullName().data(), "file1.txt") == 0,
                  "file path contains correct filename");
        }
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory filePathList tests
//-----------------------------------------------------------------------------------------
void testFilePathList() {
    printf("\n== CxDirectory filePathList Tests ==\n");

    // List files with full paths
    {
        CxDirectory dir(TEST_BASE);
        CxSList<CxString> paths = dir.filePathList();

        check(paths.entries() >= 4, "filePathList has expected entries");

        // Check that paths are absolute
        int allAbsolute = 1;
        for (int i = 0; i < paths.entries(); i++) {
            if (paths.at(i).data()[0] != '/') {
                allAbsolute = 0;
            }
        }
        check(allAbsolute, "all file paths are absolute");

        // Check paths contain directory
        int pathsContainDir = 1;
        for (int i = 0; i < paths.entries(); i++) {
            if (paths.at(i).index("cxdir_test") == -1) {
                pathsContainDir = 0;
            }
        }
        check(pathsContainDir, "file paths contain directory name");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory wildcard template tests
//-----------------------------------------------------------------------------------------
void testDirectoryWildcard() {
    printf("\n== CxDirectory Wildcard Tests ==\n");

    // Filter by *.txt at construction
    {
        CxDirectory dir(TEST_BASE, "*.txt");
        CxSList<CxFileName> files = dir.fileNameList();

        check(files.entries() == 2, "*.txt filter returns 2 files");

        int allTxt = 1;
        for (int i = 0; i < files.entries(); i++) {
            if (strcmp(files.at(i).extension().data(), "txt") != 0) {
                allTxt = 0;
            }
        }
        check(allTxt, "all filtered files are txt");
    }

    // Filter by specific pattern
    {
        CxDirectory dir(TEST_BASE, "file*.txt");
        CxSList<CxFileName> files = dir.fileNameList();

        check(files.entries() == 2, "file*.txt returns 2 files");
    }

    // Filter that matches nothing
    {
        CxDirectory dir(TEST_BASE, "*.xyz");
        CxSList<CxFileName> files = dir.fileNameList();

        check(files.entries() == 0, "*.xyz filter returns 0 files");
    }

    // Filter with * (match all)
    {
        CxDirectory dir(TEST_BASE, "*");
        CxSList<CxFileName> files = dir.fileNameList();

        // Should match everything including . and .. and subdir
        check(files.entries() >= 4, "* filter matches files");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory filterBy tests
//-----------------------------------------------------------------------------------------
void testDirectoryFilterBy() {
    printf("\n== CxDirectory filterBy Tests ==\n");

    // filterBy after construction
    {
        CxDirectory dir(TEST_BASE);
        CxSList<CxFileName> allFiles = dir.fileNameList();
        int allCount = allFiles.entries();

        int filteredCount = dir.filterBy("*.txt");
        check(filteredCount == 2, "filterBy *.txt returns 2");

        CxSList<CxFileName> filtered = dir.fileNameList();
        check(filtered.entries() == 2, "fileNameList after filter has 2 entries");
        check(filtered.entries() < allCount, "filter reduced count");
    }

    // Multiple filterBy calls (progressive filtering)
    {
        CxDirectory dir(TEST_BASE);

        int count1 = dir.filterBy("file*");
        check(count1 >= 2, "first filter matches file* files");

        int count2 = dir.filterBy("*.txt");
        check(count2 <= count1, "second filter further reduces");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory copy and assignment tests
//-----------------------------------------------------------------------------------------
void testDirectoryCopyAssign() {
    printf("\n== CxDirectory Copy/Assignment Tests ==\n");

    // Copy constructor
    {
        CxDirectory dir1(TEST_BASE);
        CxDirectory dir2(dir1);

        check(dir2.pathIsDirectory(), "copied dir is directory");
        check(dir2.pathIsAbsolute(), "copied dir is absolute");
        check(strcmp(dir2.directoryPath().data(), dir1.directoryPath().data()) == 0,
              "copied dir has same path");
        check(dir2.fileNameList().entries() == dir1.fileNameList().entries(),
              "copied dir has same file count");
    }

    // Assignment operator
    {
        CxDirectory dir1(TEST_BASE);
        CxDirectory dir2;
        dir2 = dir1;

        check(dir2.pathIsDirectory(), "assigned dir is directory");
        check(strcmp(dir2.directoryPath().data(), dir1.directoryPath().data()) == 0,
              "assigned dir has same path");
    }

    // Self-assignment
    {
        CxDirectory dir(TEST_BASE);
        CxString origPath = dir.directoryPath();
        dir = dir;
        check(strcmp(dir.directoryPath().data(), origPath.data()) == 0,
              "self-assignment preserves state");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory::getFileType static method tests
//-----------------------------------------------------------------------------------------
void testGetFileType() {
    printf("\n== CxDirectory::getFileType Tests ==\n");

    // File type for existing file
    {
        CxString filePath = TEST_BASE;
        filePath += "/file1.txt";
        CxDirectory::fileType ft = CxDirectory::getFileType(filePath);
        check(ft == CxDirectory::CxFILE, "getFileType returns CxFILE for file");
    }

    // File type for existing directory
    {
        CxDirectory::fileType ft = CxDirectory::getFileType(TEST_BASE);
        check(ft == CxDirectory::CxDIRECTORY, "getFileType returns CxDIRECTORY for dir");
    }

    // File type for non-existent path
    {
        CxDirectory::fileType ft = CxDirectory::getFileType("/nonexistent/path");
        check(ft == CxDirectory::CxINVALID, "getFileType returns CxINVALID for nonexistent");
    }

    // File type for /tmp
    {
        CxDirectory::fileType ft = CxDirectory::getFileType("/tmp");
        check(ft == CxDirectory::CxDIRECTORY, "getFileType returns CxDIRECTORY for /tmp");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory nested directory tests
//-----------------------------------------------------------------------------------------
void testNestedDirectory() {
    printf("\n== CxDirectory Nested Directory Tests ==\n");

    // Access nested directory
    {
        CxString subPath = TEST_BASE;
        subPath += "/subdir";
        CxDirectory dir(subPath);

        check(dir.pathIsDirectory(), "subdir is directory");

        CxSList<CxFileName> files = dir.fileNameList();
        // Should have at least nested.txt, . and ..
        check(files.entries() >= 1, "subdir has files");

        int foundNested = 0;
        for (int i = 0; i < files.entries(); i++) {
            if (strcmp(files.at(i).fullName().data(), "nested.txt") == 0) {
                foundNested = 1;
            }
        }
        check(foundNested, "found nested.txt in subdir");
    }

    // Access nested file
    {
        CxString nestedPath = TEST_BASE;
        nestedPath += "/subdir/nested.txt";
        CxDirectory dir(nestedPath);

        check(dir.pathIsFile(), "nested file is file");
        check(!dir.pathIsDirectory(), "nested file not directory");
    }
}

//-----------------------------------------------------------------------------------------
// CxDirectory edge cases
//-----------------------------------------------------------------------------------------
void testDirectoryEdgeCases() {
    printf("\n== CxDirectory Edge Cases ==\n");

    // Path with trailing slash
    {
        CxString path = TEST_BASE;
        path += "/";
        CxDirectory dir(path);
        check(dir.pathIsDirectory() || dir.pathIsInvalid(),
              "trailing slash handled");
    }

    // Path with spaces (if supported)
    {
        CxString spacePath = "/tmp/cxdir test space";
        createDir(spacePath.data());

        CxDirectory dir(spacePath);
        check(dir.pathIsDirectory(), "path with space handled");

        removeDir(spacePath.data());
    }

    // Empty string path
    {
        CxDirectory dir("");
        // Should handle gracefully
        check(dir.directoryPath().length() == 0 || dir.pathIsInvalid(),
              "empty path handled gracefully");
    }

    // Whitespace in path gets stripped before stat() (bug fix)
    {
        CxString path = "  ";
        path += TEST_BASE;
        path += "  \n";
        CxDirectory dir(path);
        check(dir.pathIsDirectory(), "leading/trailing whitespace stripped from path");
    }

    // Trailing whitespace only
    {
        CxString path = TEST_BASE;
        path += "\t\n  ";
        CxDirectory dir(path);
        check(dir.pathIsDirectory(), "trailing whitespace stripped");
    }

    // Leading whitespace only
    {
        CxString path = "   \t";
        path += TEST_BASE;
        CxDirectory dir(path);
        check(dir.pathIsDirectory(), "leading whitespace stripped");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxDirectory Test Suite\n");
    printf("======================\n");

    setupTestStructure();

    testDirectoryBasics();
    testDirectoryPathType();
    testDirectoryPath();
    testFileNameList();
    testFilePathList();
    testDirectoryWildcard();
    testDirectoryFilterBy();
    testDirectoryCopyAssign();
    testGetFileType();
    testNestedDirectory();
    testDirectoryEdgeCases();

    teardownTestStructure();

    printf("\n======================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
