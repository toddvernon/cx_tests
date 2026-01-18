//-----------------------------------------------------------------------------------------
// cxprop_test.cpp - CxPropEntry and CxPropertyList unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <cx/base/prop.h>

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
// CxPropEntry constructor tests
//-----------------------------------------------------------------------------------------
void testPropEntryConstructors() {
    printf("\n== CxPropEntry Constructor Tests ==\n");

    // Default constructor
    {
        CxPropEntry pe;
        check(pe.var().length() == 0, "default ctor: var is empty");
        check(pe.val().length() == 0, "default ctor: val is empty");
    }

    // Constructor with var and val
    {
        CxPropEntry pe("name", "value");
        check(pe.var() == "name", "var/val ctor: var is 'name'");
        check(pe.val() == "value", "var/val ctor: val is 'value'");
    }

    // Constructor with empty strings
    {
        CxPropEntry pe("", "");
        check(pe.var().length() == 0, "empty strings ctor: var is empty");
        check(pe.val().length() == 0, "empty strings ctor: val is empty");
    }

    // Constructor with var only (empty val)
    {
        CxPropEntry pe("key", "");
        check(pe.var() == "key", "var only: var is 'key'");
        check(pe.val().length() == 0, "var only: val is empty");
    }

    // Copy constructor
    {
        CxPropEntry pe1("foo", "bar");
        CxPropEntry pe2(pe1);
        check(pe2.var() == "foo", "copy ctor: var copied");
        check(pe2.val() == "bar", "copy ctor: val copied");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropEntry operator tests
//-----------------------------------------------------------------------------------------
void testPropEntryOperators() {
    printf("\n== CxPropEntry Operator Tests ==\n");

    // Assignment operator
    {
        CxPropEntry pe1("key1", "val1");
        CxPropEntry pe2;
        pe2 = pe1;
        check(pe2.var() == "key1", "assignment: var assigned");
        check(pe2.val() == "val1", "assignment: val assigned");
    }

    // Self-assignment
    {
        CxPropEntry pe("self", "assign");
        pe = pe;
        check(pe.var() == "self", "self-assignment: var preserved");
        check(pe.val() == "assign", "self-assignment: val preserved");
    }

    // Equality operator - equal
    {
        CxPropEntry pe1("name", "value");
        CxPropEntry pe2("name", "value");
        check(pe1 == pe2, "equality: same var and val are equal");
    }

    // Equality operator - different var
    {
        CxPropEntry pe1("name1", "value");
        CxPropEntry pe2("name2", "value");
        check(!(pe1 == pe2), "equality: different var not equal");
    }

    // Equality operator - different val
    {
        CxPropEntry pe1("name", "value1");
        CxPropEntry pe2("name", "value2");
        check(!(pe1 == pe2), "equality: different val not equal");
    }

    // Equality operator - both different
    {
        CxPropEntry pe1("name1", "value1");
        CxPropEntry pe2("name2", "value2");
        check(!(pe1 == pe2), "equality: both different not equal");
    }

    // Equality operator - empty entries
    {
        CxPropEntry pe1;
        CxPropEntry pe2;
        check(pe1 == pe2, "equality: empty entries are equal");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropEntry accessor tests
//-----------------------------------------------------------------------------------------
void testPropEntryAccessors() {
    printf("\n== CxPropEntry Accessor Tests ==\n");

    // Direct member access vs accessor
    {
        CxPropEntry pe("direct", "access");
        check(pe._var == pe.var(), "_var equals var()");
        check(pe._val == pe.val(), "_val equals val()");
    }

    // Modify via direct access
    {
        CxPropEntry pe("old", "value");
        pe._var = "new";
        pe._val = "modified";
        check(pe.var() == "new", "direct _var modification works");
        check(pe.val() == "modified", "direct _val modification works");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList constructor tests
//-----------------------------------------------------------------------------------------
void testPropertyListConstructors() {
    printf("\n== CxPropertyList Constructor Tests ==\n");

    // Default constructor
    {
        CxPropertyList pl;
        check(pl.entries() == 0, "default ctor: entries is 0");
    }

    // Copy constructor
    {
        CxPropertyList pl1;
        pl1.set("key", "value");
        CxPropertyList pl2(pl1);
        check(pl2.entries() == 1, "copy ctor: entries copied");
        check(pl2.get("key") == "value", "copy ctor: value accessible");
    }

    // Copy constructor with multiple entries
    {
        CxPropertyList pl1;
        pl1.set("a", "1");
        pl1.set("b", "2");
        pl1.set("c", "3");
        CxPropertyList pl2(pl1);
        check(pl2.entries() == 3, "copy ctor: 3 entries copied");
        check(pl2.get("a") == "1", "copy ctor: entry a");
        check(pl2.get("b") == "2", "copy ctor: entry b");
        check(pl2.get("c") == "3", "copy ctor: entry c");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList set/get tests
//-----------------------------------------------------------------------------------------
void testPropertyListSetGet() {
    printf("\n== CxPropertyList Set/Get Tests ==\n");

    // Basic set and get
    {
        CxPropertyList pl;
        pl.set("name", "Alice");
        check(pl.get("name") == "Alice", "basic set/get");
    }

    // Multiple properties
    {
        CxPropertyList pl;
        pl.set("first", "1");
        pl.set("second", "2");
        pl.set("third", "3");
        check(pl.get("first") == "1", "multiple: first");
        check(pl.get("second") == "2", "multiple: second");
        check(pl.get("third") == "3", "multiple: third");
    }

    // Update existing property
    {
        CxPropertyList pl;
        pl.set("key", "old");
        pl.set("key", "new");
        check(pl.get("key") == "new", "update existing property");
        check(pl.entries() == 1, "update doesn't add duplicate");
    }

    // Get non-existent property returns empty
    {
        CxPropertyList pl;
        pl.set("exists", "yes");
        check(pl.get("notexists").length() == 0, "get non-existent returns empty");
    }

    // Empty value
    {
        CxPropertyList pl;
        pl.set("empty", "");
        check(pl.get("empty").length() == 0, "empty value stored");
        check(pl.has("empty"), "empty value property exists");
    }

    // Set from "var=val" string
    {
        CxPropertyList pl;
        pl.set("PORT=8080");
        check(pl.get("PORT") == "8080", "set from var=val string");
    }

    // Set from "var=val" with spaces
    // Note: library strips leading spaces from var and val, but not trailing from val
    {
        CxPropertyList pl;
        pl.set("  HOST = localhost");
        check(pl.get("HOST") == "localhost", "set from var=val with spaces");
    }

    // Set from "var=val" with empty value
    {
        CxPropertyList pl;
        pl.set("EMPTY=");
        check(pl.has("EMPTY"), "set var= creates property");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList has/remove tests
//-----------------------------------------------------------------------------------------
void testPropertyListHasRemove() {
    printf("\n== CxPropertyList Has/Remove Tests ==\n");

    // has returns true for existing
    {
        CxPropertyList pl;
        pl.set("exists", "yes");
        check(pl.has("exists"), "has returns true for existing");
    }

    // has returns false for non-existing
    {
        CxPropertyList pl;
        pl.set("exists", "yes");
        check(!pl.has("notexists"), "has returns false for non-existing");
    }

    // Remove existing property
    {
        CxPropertyList pl;
        pl.set("toremove", "value");
        int result = pl.remove("toremove");
        check(result == 1, "remove returns true");
        check(!pl.has("toremove"), "removed property no longer exists");
    }

    // Remove non-existing property
    {
        CxPropertyList pl;
        pl.set("keep", "value");
        int result = pl.remove("notexists");
        check(result == 0, "remove non-existing returns false");
        check(pl.has("keep"), "other property still exists");
    }

    // Remove from empty list
    {
        CxPropertyList pl;
        int result = pl.remove("anything");
        check(result == 0, "remove from empty returns false");
    }

    // Remove first of many
    {
        CxPropertyList pl;
        pl.set("a", "1");
        pl.set("b", "2");
        pl.set("c", "3");
        pl.remove("a");
        check(!pl.has("a"), "first removed");
        check(pl.has("b"), "second still exists");
        check(pl.has("c"), "third still exists");
        check(pl.entries() == 2, "entries is 2");
    }

    // Remove middle of many
    {
        CxPropertyList pl;
        pl.set("a", "1");
        pl.set("b", "2");
        pl.set("c", "3");
        pl.remove("b");
        check(pl.has("a"), "first still exists");
        check(!pl.has("b"), "middle removed");
        check(pl.has("c"), "third still exists");
    }

    // Remove last of many
    {
        CxPropertyList pl;
        pl.set("a", "1");
        pl.set("b", "2");
        pl.set("c", "3");
        pl.remove("c");
        check(pl.has("a"), "first still exists");
        check(pl.has("b"), "second still exists");
        check(!pl.has("c"), "last removed");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList entries/at tests
//-----------------------------------------------------------------------------------------
void testPropertyListEntriesAt() {
    printf("\n== CxPropertyList Entries/At Tests ==\n");

    // Empty list has 0 entries
    {
        CxPropertyList pl;
        check(pl.entries() == 0, "empty list has 0 entries");
    }

    // Count after adding
    {
        CxPropertyList pl;
        pl.set("a", "1");
        check(pl.entries() == 1, "1 entry after 1 add");
        pl.set("b", "2");
        check(pl.entries() == 2, "2 entries after 2 adds");
        pl.set("c", "3");
        check(pl.entries() == 3, "3 entries after 3 adds");
    }

    // Count after update (no duplicate)
    {
        CxPropertyList pl;
        pl.set("key", "v1");
        pl.set("key", "v2");
        pl.set("key", "v3");
        check(pl.entries() == 1, "updates don't increase count");
    }

    // at() retrieves correct values
    {
        CxPropertyList pl;
        pl.set("first", "1");
        pl.set("second", "2");
        CxString var, val;
        pl.at(0, &var, &val);
        check(var == "first", "at(0) var is first");
        check(val == "1", "at(0) val is 1");
        pl.at(1, &var, &val);
        check(var == "second", "at(1) var is second");
        check(val == "2", "at(1) val is 2");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList clear tests
//-----------------------------------------------------------------------------------------
void testPropertyListClear() {
    printf("\n== CxPropertyList Clear Tests ==\n");

    // Clear removes all entries
    {
        CxPropertyList pl;
        pl.set("a", "1");
        pl.set("b", "2");
        pl.set("c", "3");
        pl.clear();
        check(pl.entries() == 0, "clear removes all entries");
        check(!pl.has("a"), "a no longer exists");
        check(!pl.has("b"), "b no longer exists");
        check(!pl.has("c"), "c no longer exists");
    }

    // Clear empty list is safe
    {
        CxPropertyList pl;
        pl.clear();
        check(pl.entries() == 0, "clear empty list is safe");
    }

    // Can add after clear
    {
        CxPropertyList pl;
        pl.set("old", "value");
        pl.clear();
        pl.set("new", "value");
        check(pl.entries() == 1, "can add after clear");
        check(pl.get("new") == "value", "new value accessible");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList assignment operator tests
//-----------------------------------------------------------------------------------------
void testPropertyListAssignment() {
    printf("\n== CxPropertyList Assignment Tests ==\n");

    // Basic assignment
    {
        CxPropertyList pl1;
        pl1.set("key", "value");
        CxPropertyList pl2;
        pl2 = pl1;
        check(pl2.get("key") == "value", "basic assignment works");
    }

    // Assignment replaces existing
    {
        CxPropertyList pl1;
        pl1.set("a", "1");
        CxPropertyList pl2;
        pl2.set("b", "2");
        pl2 = pl1;
        check(pl2.has("a"), "assignment has new entry");
        check(pl2.get("a") == "1", "assignment value correct");
    }

    // Self-assignment
    {
        CxPropertyList pl;
        pl.set("self", "test");
        pl = pl;
        check(pl.get("self") == "test", "self-assignment preserves data");
    }

    // Chain assignment
    {
        CxPropertyList pl1;
        pl1.set("chain", "value");
        CxPropertyList pl2, pl3;
        pl3 = pl2 = pl1;
        check(pl2.get("chain") == "value", "chain assignment pl2");
        check(pl3.get("chain") == "value", "chain assignment pl3");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList operator+ tests
//-----------------------------------------------------------------------------------------
void testPropertyListPlus() {
    printf("\n== CxPropertyList operator+ Tests ==\n");

    // Basic append
    {
        CxPropertyList pl1;
        pl1.set("a", "1");
        CxPropertyList pl2;
        pl2.set("b", "2");
        pl1 + pl2;
        check(pl1.has("a"), "original entry preserved");
        check(pl1.has("b"), "appended entry added");
        check(pl1.entries() == 2, "total entries is 2");
    }

    // Append multiple entries
    {
        CxPropertyList pl1;
        pl1.set("x", "1");
        CxPropertyList pl2;
        pl2.set("y", "2");
        pl2.set("z", "3");
        pl1 + pl2;
        check(pl1.entries() == 3, "append multiple: 3 entries");
    }

    // Append to empty
    {
        CxPropertyList pl1;
        CxPropertyList pl2;
        pl2.set("new", "value");
        pl1 + pl2;
        check(pl1.get("new") == "value", "append to empty works");
    }

    // Append empty
    {
        CxPropertyList pl1;
        pl1.set("existing", "value");
        CxPropertyList pl2;
        pl1 + pl2;
        check(pl1.entries() == 1, "append empty doesn't change count");
    }

    // Self-append (should be no-op)
    {
        CxPropertyList pl;
        pl.set("x", "1");
        pl + pl;
        // Self-append returns early, so entries should still be 1
        check(pl.entries() == 1, "self-append is no-op");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList import tests
//-----------------------------------------------------------------------------------------
void testPropertyListImport() {
    printf("\n== CxPropertyList Import Tests ==\n");

    // Import semicolon-delimited
    {
        CxPropertyList pl;
        pl.import("a=1;b=2;c=3", ";");
        check(pl.entries() == 3, "import semicolon: 3 entries");
        check(pl.get("a") == "1", "import semicolon: a=1");
        check(pl.get("b") == "2", "import semicolon: b=2");
        check(pl.get("c") == "3", "import semicolon: c=3");
    }

    // Import newline-delimited
    {
        CxPropertyList pl;
        pl.import("host=localhost\nport=8080\nname=test", "\n");
        check(pl.entries() == 3, "import newline: 3 entries");
        check(pl.get("host") == "localhost", "import newline: host");
        check(pl.get("port") == "8080", "import newline: port");
    }

    // Import comma-delimited
    {
        CxPropertyList pl;
        pl.import("x=10,y=20,z=30", ",");
        check(pl.get("x") == "10", "import comma: x=10");
        check(pl.get("y") == "20", "import comma: y=20");
        check(pl.get("z") == "30", "import comma: z=30");
    }

    // Import single entry
    {
        CxPropertyList pl;
        pl.import("single=value", ";");
        check(pl.entries() == 1, "import single: 1 entry");
        check(pl.get("single") == "value", "import single: value");
    }

    // Import empty string
    {
        CxPropertyList pl;
        pl.import("", ";");
        check(pl.entries() == 0, "import empty: 0 entries");
    }

    // Import adds to existing
    {
        CxPropertyList pl;
        pl.set("existing", "value");
        pl.import("new=imported", ";");
        check(pl.entries() == 2, "import adds to existing");
        check(pl.has("existing"), "existing preserved");
        check(pl.has("new"), "new added");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList parseArgs tests
//-----------------------------------------------------------------------------------------
void testPropertyListParseArgs() {
    printf("\n== CxPropertyList parseArgs Tests ==\n");

    // Simple flag with no argument
    {
        CxPropertyList argDefs;
        argDefs.set("-v", "0");  // 0 = no argument
        CxPropertyList result = CxPropertyList::parseArgs("-v", argDefs);
        check(result.get("-v") == "TRUE", "flag with no arg sets TRUE");
    }

    // Flag with one argument
    {
        CxPropertyList argDefs;
        argDefs.set("-f", "1");  // 1 = one argument
        CxPropertyList result = CxPropertyList::parseArgs("-f myfile.txt", argDefs);
        check(result.get("-f") == "myfile.txt", "flag with arg gets value");
    }

    // Multiple flags
    {
        CxPropertyList argDefs;
        argDefs.set("-a", "0");
        argDefs.set("-b", "1");
        CxPropertyList result = CxPropertyList::parseArgs("-a -b value", argDefs);
        check(result.get("-a") == "TRUE", "multiple: -a is TRUE");
        check(result.get("-b") == "value", "multiple: -b has value");
    }

    // Remaining args go to "*"
    {
        CxPropertyList argDefs;
        argDefs.set("-v", "0");
        CxPropertyList result = CxPropertyList::parseArgs("-v file1 file2", argDefs);
        check(result.get("-v") == "TRUE", "remaining: flag parsed");
        CxString remaining = result.get("*");
        check(remaining.index("file1") != -1, "remaining: contains file1");
        check(remaining.index("file2") != -1, "remaining: contains file2");
    }

    // Flag with attached value (-fvalue instead of -f value)
    {
        CxPropertyList argDefs;
        argDefs.set("-n", "1");
        CxPropertyList result = CxPropertyList::parseArgs("-n100", argDefs);
        check(result.get("-n") == "100", "attached value parsed");
    }

    // Undefined flag returns error
    {
        CxPropertyList argDefs;
        argDefs.set("-known", "0");
        CxPropertyList result = CxPropertyList::parseArgs("-unknown", argDefs);
        check(result.has("$ERROR"), "undefined flag sets $ERROR");
    }

    // Missing argument for flag returns error
    {
        CxPropertyList argDefs;
        argDefs.set("-f", "1");  // expects 1 arg
        CxPropertyList result = CxPropertyList::parseArgs("-f", argDefs);
        check(result.has("$ERROR"), "missing arg sets $ERROR");
    }

    // Empty command line
    {
        CxPropertyList argDefs;
        argDefs.set("-v", "0");
        CxPropertyList result = CxPropertyList::parseArgs("", argDefs);
        check(!result.has("-v"), "empty cmdline: no flags");
        check(result.entries() == 0, "empty cmdline: no entries");
    }

    // Non-flag argument first goes to "*"
    {
        CxPropertyList argDefs;
        argDefs.set("-v", "0");
        CxPropertyList result = CxPropertyList::parseArgs("filename.txt", argDefs);
        check(result.has("*"), "non-flag first goes to *");
        check(result.get("*").index("filename") != -1, "* contains filename");
    }
}

//-----------------------------------------------------------------------------------------
// CxPropertyList edge cases
//-----------------------------------------------------------------------------------------
void testPropertyListEdgeCases() {
    printf("\n== CxPropertyList Edge Cases ==\n");

    // Key with special characters
    {
        CxPropertyList pl;
        pl.set("key.with.dots", "value");
        check(pl.get("key.with.dots") == "value", "key with dots");
    }

    // Value with special characters
    {
        CxPropertyList pl;
        pl.set("url", "http://example.com:8080/path?query=1");
        check(pl.get("url").index("http://") != -1, "value with special chars");
    }

    // Key with spaces (not typical but test behavior)
    {
        CxPropertyList pl;
        pl.set("key with spaces", "value");
        check(pl.get("key with spaces") == "value", "key with spaces");
    }

    // Very long value
    {
        CxPropertyList pl;
        CxString longVal;
        for (int i = 0; i < 100; i++) longVal += "0123456789";
        pl.set("longkey", longVal);
        check(pl.get("longkey").length() == 1000, "very long value");
    }

    // Many properties
    {
        CxPropertyList pl;
        for (int i = 0; i < 100; i++) {
            CxString key, val;
            key.printf("key%d", i);
            val.printf("val%d", i);
            pl.set(key, val);
        }
        check(pl.entries() == 100, "100 properties");
        check(pl.get("key50") == "val50", "property 50 accessible");
        check(pl.get("key99") == "val99", "property 99 accessible");
    }

    // Case sensitivity
    {
        CxPropertyList pl;
        pl.set("Key", "value1");
        pl.set("KEY", "value2");
        pl.set("key", "value3");
        check(pl.entries() == 3, "keys are case-sensitive");
        check(pl.get("Key") == "value1", "Key has value1");
        check(pl.get("KEY") == "value2", "KEY has value2");
        check(pl.get("key") == "value3", "key has value3");
    }

    // Value with equals sign
    {
        CxPropertyList pl;
        pl.set("equation=x=y+z");
        // The first = is the delimiter, rest is value
        check(pl.get("equation") == "x=y+z", "value contains equals sign");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxProp Test Suite\n");
    printf("=================\n");

    testPropEntryConstructors();
    testPropEntryOperators();
    testPropEntryAccessors();
    testPropertyListConstructors();
    testPropertyListSetGet();
    testPropertyListHasRemove();
    testPropertyListEntriesAt();
    testPropertyListClear();
    testPropertyListAssignment();
    testPropertyListPlus();
    testPropertyListImport();
    testPropertyListParseArgs();
    testPropertyListEdgeCases();

    printf("\n=================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
