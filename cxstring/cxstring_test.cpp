//-----------------------------------------------------------------------------------------
// cxstring_test.cpp - CxString unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <cx/base/string.h>

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
// Constructor tests
//-----------------------------------------------------------------------------------------
void testConstructors() {
    printf("\n== Constructor Tests ==\n");

    // Default constructor
    {
        CxString s;
        check(s.isNull(), "default ctor creates null string");
        check(s.length() == 0, "default ctor length is 0");
    }

    // From const char*
    {
        CxString s("hello");
        check(s.length() == 5, "char* ctor sets correct length");
        check(strcmp(s.data(), "hello") == 0, "char* ctor sets correct data");
    }

    // From const char* with length
    {
        CxString s("hello world", 5);
        check(s.length() == 5, "char* ctor with len truncates");
        check(strcmp(s.data(), "hello") == 0, "char* ctor with len has correct data");
    }

    // From single char
    {
        CxString s('X');
        check(s.length() == 1, "char ctor length is 1");
        check(strcmp(s.data(), "X") == 0, "char ctor has correct data");
    }

    // From int
    {
        CxString s(42);
        check(strcmp(s.data(), "42") == 0, "int ctor converts correctly");

        CxString neg(-123);
        check(strcmp(neg.data(), "-123") == 0, "int ctor handles negative");
    }

    // From long
    {
        CxString s((long)123456789);
        check(strcmp(s.data(), "123456789") == 0, "long ctor converts correctly");
    }

    // From double
    {
        CxString s(3.14);
        check(s.length() > 0, "double ctor creates non-empty string");
        // Note: exact format may vary, just check it's reasonable
    }

    // Copy constructor
    {
        CxString original("test");
        CxString copy(original);
        check(strcmp(copy.data(), "test") == 0, "copy ctor copies data");
        check(copy.length() == original.length(), "copy ctor copies length");
    }

    // From CxString pointer
    {
        CxString original("pointer test");
        CxString copy(&original);
        check(strcmp(copy.data(), "pointer test") == 0, "pointer ctor copies data");
    }

    // From NULL CxString pointer
    {
        CxString copy((const CxString*)NULL);
        check(copy.isNull(), "pointer ctor with NULL creates null string");
    }
}

//-----------------------------------------------------------------------------------------
// Operator tests
//-----------------------------------------------------------------------------------------
void testOperators() {
    printf("\n== Operator Tests ==\n");

    // Assignment operator
    {
        CxString a("original");
        CxString b;
        b = a;
        check(strcmp(b.data(), "original") == 0, "assignment copies data");
        check(b.length() == a.length(), "assignment copies length");
    }

    // Self-assignment
    {
        CxString a("self");
        a = a;
        check(strcmp(a.data(), "self") == 0, "self-assignment preserves data");
    }

    // Concatenation operator +
    {
        CxString a("hello");
        CxString b(" world");
        CxString c = a + b;
        check(strcmp(c.data(), "hello world") == 0, "operator+ concatenates");
        check(c.length() == 11, "operator+ correct length");
        check(strcmp(a.data(), "hello") == 0, "operator+ doesn't modify left operand");
    }

    // Append operator +=
    {
        CxString a("hello");
        CxString b(" world");
        a += b;
        check(strcmp(a.data(), "hello world") == 0, "operator+= appends");
        check(a.length() == 11, "operator+= correct length");
    }

    // Equality operator ==
    {
        CxString a("test");
        CxString b("test");
        CxString c("other");
        check(a == b, "operator== returns true for equal strings");
        check(!(a == c), "operator== returns false for different strings");
    }

    // Inequality operator !=
    {
        CxString a("test");
        CxString b("test");
        CxString c("other");
        check(!(a != b), "operator!= returns false for equal strings");
        check(a != c, "operator!= returns true for different strings");
    }

    // Less-than-or-equal operator <=
    {
        CxString a("apple");
        CxString b("banana");
        CxString c("apple");
        check(a <= b, "operator<= returns true for lesser string");
        check(a <= c, "operator<= returns true for equal strings");
        check(!(b <= a), "operator<= returns false for greater string");
    }

    // Empty string operations
    {
        CxString empty;
        CxString text("hello");
        CxString result = empty + text;
        check(strcmp(result.data(), "hello") == 0, "empty + string works");

        result = text + empty;
        check(strcmp(result.data(), "hello") == 0, "string + empty works");
    }
}

//-----------------------------------------------------------------------------------------
// String operations tests
//-----------------------------------------------------------------------------------------
void testStringOperations() {
    printf("\n== String Operations Tests ==\n");

    // append(CxString)
    {
        CxString s("hello");
        CxString suffix(" world");
        s.append(suffix);
        check(strcmp(s.data(), "hello world") == 0, "append(CxString) works");
        check(s.length() == 11, "append(CxString) correct length");
    }

    // append(char)
    {
        CxString s("abc");
        s.append('d');
        check(strcmp(s.data(), "abcd") == 0, "append(char) works");
        check(s.length() == 4, "append(char) correct length");
    }

    // append to empty string
    {
        CxString s;
        s.append("test");
        check(strcmp(s.data(), "test") == 0, "append to empty string works");
    }

    // insert(CxString, pos)
    {
        CxString s("helloworld");
        CxString space(" ");
        s.insert(space, 5);
        check(strcmp(s.data(), "hello world") == 0, "insert(CxString, pos) works");
    }

    // insert at beginning
    {
        CxString s("world");
        CxString prefix("hello ");
        s.insert(prefix, 0);
        check(strcmp(s.data(), "hello world") == 0, "insert at beginning works");
    }

    // insert(char, pos)
    {
        CxString s("helloworld");
        s.insert(' ', 5);
        check(strcmp(s.data(), "hello world") == 0, "insert(char, pos) works");
    }

    // remove(start, len)
    {
        CxString s("hello world");
        s.remove(5, 6);
        check(strcmp(s.data(), "hello") == 0, "remove(start, len) works");
        check(s.length() == 5, "remove correct length");
    }

    // remove from beginning
    {
        CxString s("hello world");
        s.remove(0, 6);
        check(strcmp(s.data(), "world") == 0, "remove from beginning works");
    }

    // subString
    {
        CxString s("hello world");
        CxString sub = s.subString(0, 5);
        check(strcmp(sub.data(), "hello") == 0, "subString from start works");
        check(sub.length() == 5, "subString correct length");
    }

    // subString middle
    {
        CxString s("hello world");
        CxString sub = s.subString(6, 5);
        check(strcmp(sub.data(), "world") == 0, "subString from middle works");
    }

    // subString - original unchanged
    {
        CxString s("hello world");
        CxString sub = s.subString(0, 5);
        check(strcmp(s.data(), "hello world") == 0, "subString doesn't modify original");
    }

    // replaceAll
    {
        CxString s("hello hello hello");
        int count = s.replaceAll("hello", "hi");
        check(strcmp(s.data(), "hi hi hi") == 0, "replaceAll replaces all occurrences");
        check(count == 3, "replaceAll returns correct count");
    }

    // replaceAll - no match
    {
        CxString s("hello world");
        int count = s.replaceAll("xyz", "abc");
        check(strcmp(s.data(), "hello world") == 0, "replaceAll with no match unchanged");
        check(count == 0, "replaceAll returns 0 for no matches");
    }

    // replaceAll - longer replacement
    {
        CxString s("a b c");
        s.replaceAll(" ", "---");
        check(strcmp(s.data(), "a---b---c") == 0, "replaceAll with longer replacement");
    }

    // replaceAll - shorter replacement
    {
        CxString s("aaa bbb ccc");
        s.replaceAll("aaa", "x");
        check(strcmp(s.data(), "x bbb ccc") == 0, "replaceAll with shorter replacement");
    }

    // numberOfOccurances
    {
        CxString s("hello hello hello");
        int count = s.numberOfOccurances("hello");
        check(count == 3, "numberOfOccurances counts correctly");
    }

    // numberOfOccurances - no match
    {
        CxString s("hello world");
        int count = s.numberOfOccurances("xyz");
        check(count == 0, "numberOfOccurances returns 0 for no matches");
    }
}

//-----------------------------------------------------------------------------------------
// Searching tests
//-----------------------------------------------------------------------------------------
void testSearching() {
    printf("\n== Searching Tests ==\n");

    // firstChar(char)
    {
        CxString s("hello world");
        check(s.firstChar('o') == 4, "firstChar finds first occurrence");
        check(s.firstChar('h') == 0, "firstChar finds at beginning");
        check(s.firstChar('d') == 10, "firstChar finds at end");
        check(s.firstChar('x') == -1, "firstChar returns -1 for not found");
    }

    // firstChar(charSet)
    {
        CxString s("hello world");
        check(s.firstChar(" ") == 5, "firstChar(charSet) finds delimiter");
        check(s.firstChar("aeiou") == 1, "firstChar(charSet) finds first vowel");
        check(s.firstChar("xyz") == -1, "firstChar(charSet) returns -1 for not found");
    }

    // firstChar(charSet) with theDelim output
    {
        CxString s("hello, world");
        char delim = 0;
        int pos = s.firstChar(",;:", &delim);
        check(pos == 5, "firstChar(charSet, delim) finds position");
        check(delim == ',', "firstChar(charSet, delim) returns delimiter");
    }

    // lastChar
    {
        CxString s("hello world");
        check(s.lastChar('o') == 7, "lastChar finds last occurrence");
        check(s.lastChar('h') == 0, "lastChar finds at beginning");
        check(s.lastChar('d') == 10, "lastChar finds at end");
        check(s.lastChar('x') == -1, "lastChar returns -1 for not found");
    }

    // index(CxString)
    {
        CxString s("hello world hello");
        check(s.index("world") == 6, "index finds substring");
        check(s.index("hello") == 0, "index finds at beginning");
        check(s.index("xyz") == -1, "index returns -1 for not found");
    }

    // index with start position
    {
        CxString s("hello world hello");
        check(s.index("hello", 0) == 0, "index with start=0 finds first");
        check(s.index("hello", 1) == 12, "index with start=1 finds second");
        check(s.index("hello", 13) == -1, "index with start past last returns -1");
    }

    // charAt
    {
        CxString s("hello");
        check(s.charAt(0) == 'h', "charAt returns first char");
        check(s.charAt(4) == 'o', "charAt returns last char");
        check(s.charAt(2) == 'l', "charAt returns middle char");
    }

    // charInString - returns TRUE/FALSE, not position
    // Note: header says "return -1 if not found" but implementation returns FALSE(0)
    {
        CxString s("hello");
        check(s.charInString('e') == TRUE, "charInString returns TRUE when found");
        check(s.charInString('x') == FALSE, "charInString returns FALSE when not found");
    }
}

//-----------------------------------------------------------------------------------------
// Tokenizing tests
//-----------------------------------------------------------------------------------------
void testTokenizing() {
    printf("\n== Tokenizing Tests ==\n");

    // nextToken basic - note: leaves delimiter in remaining string
    {
        CxString s("hello world test");
        CxString token = s.nextToken(" ");
        check(strcmp(token.data(), "hello") == 0, "nextToken returns first token");
        check(strcmp(s.data(), " world test") == 0, "nextToken leaves delimiter in string");
    }

    // nextToken multiple calls - stripLeading removes delims before each token
    {
        CxString s("one,two,three");
        CxString t1 = s.nextToken(",");
        CxString t2 = s.nextToken(",");
        CxString t3 = s.nextToken(",");
        check(strcmp(t1.data(), "one") == 0, "nextToken first of three");
        check(strcmp(t2.data(), "two") == 0, "nextToken second of three");
        check(strcmp(t3.data(), "three") == 0, "nextToken third of three");
    }

    // nextToken with delim output
    {
        CxString s("key=value;other");
        char delim = 0;
        CxString token = s.nextToken("=;", &delim);
        check(strcmp(token.data(), "key") == 0, "nextToken with delim returns token");
        check(delim == '=', "nextToken reports which delimiter was found");
    }

    // nextToken no delimiter found
    {
        CxString s("nodelimiter");
        CxString token = s.nextToken(",");
        check(strcmp(token.data(), "nodelimiter") == 0, "nextToken returns whole string if no delim");
        check(s.length() == 0 || s.isNull(), "nextToken empties string when no delim");
    }

    // nextToken with leading delimiters - stripLeading removes them first
    {
        CxString s("  hello  world  ");
        CxString token = s.nextToken(" ");
        check(strcmp(token.data(), "hello") == 0, "nextToken strips leading delims first");
    }

    // stripLeading
    {
        CxString s("   hello");
        s.stripLeading(" ");
        check(strcmp(s.data(), "hello") == 0, "stripLeading removes leading spaces");
    }

    // stripLeading multiple chars
    {
        CxString s("...###hello");
        s.stripLeading(".#");
        check(strcmp(s.data(), "hello") == 0, "stripLeading removes multiple char types");
    }

    // stripLeading nothing to strip
    {
        CxString s("hello");
        s.stripLeading(" ");
        check(strcmp(s.data(), "hello") == 0, "stripLeading unchanged when nothing to strip");
    }

    // stripLeading all chars
    {
        CxString s("   ");
        s.stripLeading(" ");
        check(s.length() == 0, "stripLeading can strip entire string");
    }

    // stripTrailing
    {
        CxString s("hello   ");
        s.stripTrailing(" ");
        check(strcmp(s.data(), "hello") == 0, "stripTrailing removes trailing spaces");
    }

    // stripTrailing multiple chars
    {
        CxString s("hello...###");
        s.stripTrailing(".#");
        check(strcmp(s.data(), "hello") == 0, "stripTrailing removes multiple char types");
    }

    // stripTrailing nothing to strip
    {
        CxString s("hello");
        s.stripTrailing(" ");
        check(strcmp(s.data(), "hello") == 0, "stripTrailing unchanged when nothing to strip");
    }

    // stripTrailing all chars
    {
        CxString s("   ");
        s.stripTrailing(" ");
        check(s.length() == 0, "stripTrailing can strip entire string");
    }

    // stripLeading and stripTrailing chained
    {
        CxString s("  hello world  ");
        s.stripLeading(" ").stripTrailing(" ");
        check(strcmp(s.data(), "hello world") == 0, "strip methods can be chained");
    }
}

//-----------------------------------------------------------------------------------------
// Conversion tests
//-----------------------------------------------------------------------------------------
void testConversions() {
    printf("\n== Conversion Tests ==\n");

    // toInt
    {
        CxString s("42");
        check(s.toInt() == 42, "toInt converts positive");

        CxString neg("-123");
        check(neg.toInt() == -123, "toInt converts negative");

        CxString zero("0");
        check(zero.toInt() == 0, "toInt converts zero");
    }

    // toLong
    {
        CxString s("123456789");
        check(s.toLong() == 123456789L, "toLong converts positive");

        CxString neg("-987654321");
        check(neg.toLong() == -987654321L, "toLong converts negative");
    }

    // toUnsignedLong
    {
        CxString s("4000000000");
        check(s.toUnsignedLong() == 4000000000UL, "toUnsignedLong converts large value");
    }

    // toFloat
    {
        CxString s("3.14");
        float f = s.toFloat();
        check(f > 3.13 && f < 3.15, "toFloat converts decimal");

        CxString neg("-2.5");
        float fn = neg.toFloat();
        check(fn > -2.51 && fn < -2.49, "toFloat converts negative");
    }

    // toDouble
    {
        CxString s("3.14159265");
        double d = s.toDouble();
        check(d > 3.141 && d < 3.142, "toDouble converts decimal");

        CxString neg("-2.71828");
        double dn = neg.toDouble();
        check(dn > -2.719 && dn < -2.717, "toDouble converts negative");
    }

    // isInt
    {
        CxString valid("12345");
        check(valid.isInt(), "isInt returns true for valid int");

        CxString negative("-42");
        check(negative.isInt(), "isInt returns true for negative");

        CxString invalid("12.34");
        check(!invalid.isInt(), "isInt returns false for float");

        CxString text("hello");
        check(!text.isInt(), "isInt returns false for text");

        CxString mixed("123abc");
        check(!mixed.isInt(), "isInt returns false for mixed");
    }

    // isFloat
    {
        CxString valid("3.14");
        check(valid.isFloat(), "isFloat returns true for decimal");

        CxString integer("42");
        check(integer.isFloat(), "isFloat returns true for integer");

        CxString negative("-2.5");
        check(negative.isFloat(), "isFloat returns true for negative");

        CxString text("hello");
        check(!text.isFloat(), "isFloat returns false for text");
    }

    // setInt
    {
        CxString s;
        s.setInt(999);
        check(strcmp(s.data(), "999") == 0, "setInt sets value");

        s.setInt(-42);
        check(strcmp(s.data(), "-42") == 0, "setInt overwrites with negative");
    }

    // setLong
    {
        CxString s;
        s.setLong(123456789L);
        check(strcmp(s.data(), "123456789") == 0, "setLong sets value");
    }

    // setUnsignedLong
    {
        CxString s;
        s.setUnsignedLong(4000000000UL);
        check(strcmp(s.data(), "4000000000") == 0, "setUnsignedLong sets large value");
    }

    // setDouble
    {
        CxString s;
        s.setDouble(3.14);
        check(s.toDouble() > 3.13 && s.toDouble() < 3.15, "setDouble sets value");
    }

    // setString
    {
        CxString s;
        CxString src("hello");
        s.setString(src);
        check(strcmp(s.data(), "hello") == 0, "setString copies value");
    }
}

//-----------------------------------------------------------------------------------------
// Static method tests
//-----------------------------------------------------------------------------------------
void testStaticMethods() {
    printf("\n== Static Method Tests ==\n");

    // toUpper
    {
        CxString result = CxString::toUpper("hello");
        check(strcmp(result.data(), "HELLO") == 0, "toUpper converts lowercase");

        result = CxString::toUpper("Hello World");
        check(strcmp(result.data(), "HELLO WORLD") == 0, "toUpper converts mixed case");

        result = CxString::toUpper("ALREADY");
        check(strcmp(result.data(), "ALREADY") == 0, "toUpper preserves uppercase");

        result = CxString::toUpper("test123");
        check(strcmp(result.data(), "TEST123") == 0, "toUpper preserves digits");
    }

    // toLower
    {
        CxString result = CxString::toLower("HELLO");
        check(strcmp(result.data(), "hello") == 0, "toLower converts uppercase");

        result = CxString::toLower("Hello World");
        check(strcmp(result.data(), "hello world") == 0, "toLower converts mixed case");

        result = CxString::toLower("already");
        check(strcmp(result.data(), "already") == 0, "toLower preserves lowercase");

        result = CxString::toLower("TEST123");
        check(strcmp(result.data(), "test123") == 0, "toLower preserves digits");
    }

    // netNormalize - removes trailing CR and/or LF
    {
        CxString result = CxString::netNormalize("hello\r\n");
        check(strcmp(result.data(), "hello") == 0, "netNormalize removes CRLF");

        result = CxString::netNormalize("hello\n");
        check(strcmp(result.data(), "hello") == 0, "netNormalize removes LF");

        result = CxString::netNormalize("hello\r");
        check(strcmp(result.data(), "hello") == 0, "netNormalize removes CR");

        result = CxString::netNormalize("hello");
        check(strcmp(result.data(), "hello") == 0, "netNormalize unchanged without newlines");
    }

    // removeCarriageReturns - removes all CR from string
    {
        CxString result = CxString::removeCarriageReturns("hello\r\nworld\r\n");
        check(strcmp(result.data(), "hello\nworld\n") == 0, "removeCarriageReturns removes all CR");

        result = CxString::removeCarriageReturns("no carriage returns");
        check(strcmp(result.data(), "no carriage returns") == 0, "removeCarriageReturns unchanged without CR");
    }

    // urlDecode
    {
        CxString result = CxString::urlDecode("hello%20world");
        check(strcmp(result.data(), "hello world") == 0, "urlDecode decodes %20 to space");

        result = CxString::urlDecode("hello+world");
        check(strcmp(result.data(), "hello world") == 0, "urlDecode decodes + to space");

        result = CxString::urlDecode("test%3Dvalue");
        check(strcmp(result.data(), "test=value") == 0, "urlDecode decodes %3D to =");

        result = CxString::urlDecode("plain");
        check(strcmp(result.data(), "plain") == 0, "urlDecode unchanged without encoding");
    }

    // hashValue
    {
        CxString s1("test");
        CxString s2("test");
        CxString s3("different");
        check(s1.hashValue() == s2.hashValue(), "hashValue same for equal strings");
        check(s1.hashValue() != s3.hashValue(), "hashValue different for different strings");
    }

    // compare
    {
        CxString a("apple");
        CxString b("banana");
        CxString c("apple");
        check(a.compare(c) == 0, "compare returns 0 for equal");
        check(a.compare(b) < 0, "compare returns negative for lesser");
        check(b.compare(a) > 0, "compare returns positive for greater");
    }

    // printf
    {
        CxString s;
        s.printf("Value: %d", 42);
        check(strcmp(s.data(), "Value: 42") == 0, "printf formats int");

        s.printf("%s %s", "hello", "world");
        check(strcmp(s.data(), "hello world") == 0, "printf formats strings");
    }

    // findAndReplaceFirst
    {
        CxString s("hello hello hello");
        int pos = s.findAndReplaceFirst("hello", "hi");
        check(pos == 0, "findAndReplaceFirst returns position");
        check(strcmp(s.data(), "hi hello hello") == 0, "findAndReplaceFirst replaces first only");

        pos = s.findAndReplaceFirst("xyz", "abc");
        check(pos == -1, "findAndReplaceFirst returns -1 for no match");
    }

    // findAndReplaceFirst with startPos
    {
        CxString s("hello hello hello");
        int pos = s.findAndReplaceFirst("hello", "hi", 1);
        check(pos == 6, "findAndReplaceFirst with startPos finds second");
        check(strcmp(s.data(), "hello hi hello") == 0, "findAndReplaceFirst with startPos replaces correctly");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxString Test Suite\n");
    printf("===================\n");

    testConstructors();
    testOperators();
    testStringOperations();
    testSearching();
    testTokenizing();
    testConversions();
    testStaticMethods();

    printf("\n===================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
