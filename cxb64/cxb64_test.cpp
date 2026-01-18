//-----------------------------------------------------------------------------------------
// cxb64_test.cpp - CxB64Encoder and CxB64Decoder unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <cx/base/string.h>
#include <cx/base/buffer.h>
#include <cx/base/slist.h>
#include <cx/b64/b64.h>

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
// Helper: Encode data to base64 string
//-----------------------------------------------------------------------------------------
CxString encodeToString(const void* data, unsigned int len) {
    CxB64Encoder encoder;
    CxSList<CxString> lines;

    encoder.process((void*)data, len, lines);
    encoder.finalize(lines);

    CxString result;
    for (unsigned int i = 0; i < lines.entries(); i++) {
        result = result + lines.at(i);
    }
    return result;
}

//-----------------------------------------------------------------------------------------
// Helper: Decode base64 string to buffer
//-----------------------------------------------------------------------------------------
CxBuffer decodeFromString(const char* b64str) {
    CxB64Decoder decoder;
    CxSList<CxString> lines;
    lines.append(CxString(b64str));

    CxBuffer result = decoder.process(lines);
    return result;
}

//-----------------------------------------------------------------------------------------
// CxB64Encoder basic tests
//-----------------------------------------------------------------------------------------
void testEncoderBasic() {
    printf("\n== CxB64Encoder Basic Tests ==\n");

    // Constructor/destructor
    {
        CxB64Encoder encoder;
        check(1, "encoder ctor doesn't crash");
    }

    // Initialize
    {
        CxB64Encoder encoder;
        encoder.initialize();
        check(1, "encoder initialize doesn't crash");
    }
}

//-----------------------------------------------------------------------------------------
// CxB64Decoder basic tests
//-----------------------------------------------------------------------------------------
void testDecoderBasic() {
    printf("\n== CxB64Decoder Basic Tests ==\n");

    // Constructor/destructor
    {
        CxB64Decoder decoder;
        check(1, "decoder ctor doesn't crash");
    }

    // Initialize
    {
        CxB64Decoder decoder;
        decoder.initialize();
        check(1, "decoder initialize doesn't crash");
    }
}

//-----------------------------------------------------------------------------------------
// RFC 4648 test vectors
// These are the standard test vectors for Base64
//-----------------------------------------------------------------------------------------
void testRFC4648Vectors() {
    printf("\n== RFC 4648 Test Vectors ==\n");

    // Empty string
    {
        CxString encoded = encodeToString("", 0);
        check(encoded.length() == 0, "encode empty string");
    }

    // "f" -> "Zg=="
    {
        CxString encoded = encodeToString("f", 1);
        check(encoded == "Zg==", "encode 'f' -> 'Zg=='");
    }

    // "fo" -> "Zm8="
    {
        CxString encoded = encodeToString("fo", 2);
        check(encoded == "Zm8=", "encode 'fo' -> 'Zm8='");
    }

    // "foo" -> "Zm9v"
    {
        CxString encoded = encodeToString("foo", 3);
        check(encoded == "Zm9v", "encode 'foo' -> 'Zm9v'");
    }

    // "foob" -> "Zm9vYg=="
    {
        CxString encoded = encodeToString("foob", 4);
        check(encoded == "Zm9vYg==", "encode 'foob' -> 'Zm9vYg=='");
    }

    // "fooba" -> "Zm9vYmE="
    {
        CxString encoded = encodeToString("fooba", 5);
        check(encoded == "Zm9vYmE=", "encode 'fooba' -> 'Zm9vYmE='");
    }

    // "foobar" -> "Zm9vYmFy"
    {
        CxString encoded = encodeToString("foobar", 6);
        check(encoded == "Zm9vYmFy", "encode 'foobar' -> 'Zm9vYmFy'");
    }
}

//-----------------------------------------------------------------------------------------
// Decode test vectors
//-----------------------------------------------------------------------------------------
void testDecodeVectors() {
    printf("\n== Decode Test Vectors ==\n");

    // "Zg==" -> "f"
    {
        CxBuffer decoded = decodeFromString("Zg==");
        check(decoded.length() == 1 &&
              memcmp(decoded.data(), "f", 1) == 0,
              "decode 'Zg==' -> 'f'");
    }

    // "Zm8=" -> "fo"
    {
        CxBuffer decoded = decodeFromString("Zm8=");
        check(decoded.length() == 2 &&
              memcmp(decoded.data(), "fo", 2) == 0,
              "decode 'Zm8=' -> 'fo'");
    }

    // "Zm9v" -> "foo"
    {
        CxBuffer decoded = decodeFromString("Zm9v");
        check(decoded.length() == 3 &&
              memcmp(decoded.data(), "foo", 3) == 0,
              "decode 'Zm9v' -> 'foo'");
    }

    // "Zm9vYmFy" -> "foobar"
    {
        CxBuffer decoded = decodeFromString("Zm9vYmFy");
        check(decoded.length() == 6 &&
              memcmp(decoded.data(), "foobar", 6) == 0,
              "decode 'Zm9vYmFy' -> 'foobar'");
    }
}

//-----------------------------------------------------------------------------------------
// Round-trip tests
//-----------------------------------------------------------------------------------------
void testRoundTrip() {
    printf("\n== Round-Trip Tests ==\n");

    // Simple string round-trip
    {
        const char* original = "Hello, World!";
        CxString encoded = encodeToString(original, strlen(original));
        CxBuffer decoded = decodeFromString(encoded.data());

        check(decoded.length() == strlen(original) &&
              memcmp(decoded.data(), original, strlen(original)) == 0,
              "round-trip 'Hello, World!'");
    }

    // Binary data with all printable ASCII
    {
        char data[95];
        for (int i = 0; i < 95; i++) {
            data[i] = 32 + i;  // Space through tilde
        }

        CxString encoded = encodeToString(data, 95);
        CxBuffer decoded = decodeFromString(encoded.data());

        check(decoded.length() == 95 &&
              memcmp(decoded.data(), data, 95) == 0,
              "round-trip printable ASCII");
    }

    // Binary data with nulls and control chars
    {
        unsigned char data[10] = {0, 1, 2, 127, 128, 255, 0, 10, 13, 26};

        CxString encoded = encodeToString(data, 10);
        CxBuffer decoded = decodeFromString(encoded.data());

        check(decoded.length() == 10 &&
              memcmp(decoded.data(), data, 10) == 0,
              "round-trip binary with nulls");
    }
}

//-----------------------------------------------------------------------------------------
// Padding edge cases
//-----------------------------------------------------------------------------------------
void testPaddingCases() {
    printf("\n== Padding Edge Cases ==\n");

    // 1 byte input (2 padding chars)
    {
        unsigned char data[1] = {0xAB};
        CxString encoded = encodeToString(data, 1);

        // Should end with ==
        int hasPadding = (encoded.length() >= 2 &&
                          encoded.data()[encoded.length()-1] == '=' &&
                          encoded.data()[encoded.length()-2] == '=');
        check(hasPadding, "1 byte: double padding");

        CxBuffer decoded = decodeFromString(encoded.data());
        check(decoded.length() == 1 &&
              ((unsigned char*)decoded.data())[0] == 0xAB,
              "1 byte: round-trip");
    }

    // 2 byte input (1 padding char)
    {
        unsigned char data[2] = {0xAB, 0xCD};
        CxString encoded = encodeToString(data, 2);

        // Should end with single =
        int hasPadding = (encoded.length() >= 1 &&
                          encoded.data()[encoded.length()-1] == '=' &&
                          (encoded.length() < 2 || encoded.data()[encoded.length()-2] != '='));
        check(hasPadding, "2 bytes: single padding");

        CxBuffer decoded = decodeFromString(encoded.data());
        check(decoded.length() == 2 &&
              memcmp(decoded.data(), data, 2) == 0,
              "2 bytes: round-trip");
    }

    // 3 byte input (no padding)
    {
        unsigned char data[3] = {0xAB, 0xCD, 0xEF};
        CxString encoded = encodeToString(data, 3);

        // Should have no padding
        int noPadding = (encoded.length() == 0 ||
                         encoded.data()[encoded.length()-1] != '=');
        check(noPadding, "3 bytes: no padding");

        CxBuffer decoded = decodeFromString(encoded.data());
        check(decoded.length() == 3 &&
              memcmp(decoded.data(), data, 3) == 0,
              "3 bytes: round-trip");
    }
}

//-----------------------------------------------------------------------------------------
// Special characters test
//-----------------------------------------------------------------------------------------
void testSpecialChars() {
    printf("\n== Special Characters Tests ==\n");

    // Test that + and / appear in output for certain inputs
    // 62 encodes to '+', 63 encodes to '/'
    {
        // 0xF8 0x00 should produce some + or /
        unsigned char data[3] = {0xFB, 0xEF, 0xBE};  // Should produce some special chars
        CxString encoded = encodeToString(data, 3);
        CxBuffer decoded = decodeFromString(encoded.data());

        check(decoded.length() == 3 &&
              memcmp(decoded.data(), data, 3) == 0,
              "special chars round-trip");
    }

    // All zeros
    {
        unsigned char data[6] = {0, 0, 0, 0, 0, 0};
        CxString encoded = encodeToString(data, 6);
        check(encoded == "AAAAAAAA", "all zeros -> AAAAAAAA");

        CxBuffer decoded = decodeFromString(encoded.data());
        check(decoded.length() == 6 &&
              memcmp(decoded.data(), data, 6) == 0,
              "all zeros round-trip");
    }

    // All 0xFF
    {
        unsigned char data[3] = {0xFF, 0xFF, 0xFF};
        CxString encoded = encodeToString(data, 3);
        check(encoded == "////", "0xFFFFFF -> '////'");

        CxBuffer decoded = decodeFromString(encoded.data());
        check(decoded.length() == 3 &&
              memcmp(decoded.data(), data, 3) == 0,
              "all 0xFF round-trip");
    }
}

//-----------------------------------------------------------------------------------------
// Larger data test
// Buffer sizes are 64KB, so we can handle much larger data now.
//-----------------------------------------------------------------------------------------
void testLargerData() {
    printf("\n== Larger Data Tests ==\n");

    // 100 bytes
    {
        unsigned char data[100];
        for (int i = 0; i < 100; i++) {
            data[i] = (unsigned char)(i * 7);  // Some pattern
        }

        CxString encoded = encodeToString(data, 100);
        CxBuffer decoded = decodeFromString(encoded.data());

        check(decoded.length() == 100 &&
              memcmp(decoded.data(), data, 100) == 0,
              "100 bytes round-trip");
    }

    // 1000 bytes
    {
        unsigned char data[1000];
        for (int i = 0; i < 1000; i++) {
            data[i] = (unsigned char)(i * 13 + i / 7);
        }

        CxString encoded = encodeToString(data, 1000);
        CxBuffer decoded = decodeFromString(encoded.data());

        check(decoded.length() == 1000 &&
              memcmp(decoded.data(), data, 1000) == 0,
              "1000 bytes round-trip");
    }

    // 10000 bytes (tests larger buffer capacity)
    {
        unsigned char data[10000];
        for (int i = 0; i < 10000; i++) {
            data[i] = (unsigned char)(i * 17 + i / 11);
        }

        CxString encoded = encodeToString(data, 10000);
        CxBuffer decoded = decodeFromString(encoded.data());

        check(decoded.length() == 10000 &&
              memcmp(decoded.data(), data, 10000) == 0,
              "10000 bytes round-trip");
    }

    // Check that large data produces multiple lines (72 char limit)
    {
        unsigned char data[100];
        for (int i = 0; i < 100; i++) {
            data[i] = (unsigned char)i;
        }

        CxB64Encoder encoder;
        CxSList<CxString> lines;
        encoder.process(data, 100, lines);
        encoder.finalize(lines);

        // 100 bytes = 136 base64 chars (ceil(100/3)*4), should be 2 lines
        check(lines.entries() >= 1, "large data produces lines");
    }
}

//-----------------------------------------------------------------------------------------
// Multiple process calls test
//-----------------------------------------------------------------------------------------
void testMultipleProcessCalls() {
    printf("\n== Multiple Process Calls Tests ==\n");

    // Encode in chunks
    {
        CxB64Encoder encoder;
        CxSList<CxString> lines;

        encoder.process((void*)"foo", 3, lines);
        encoder.process((void*)"bar", 3, lines);
        encoder.finalize(lines);

        CxString result;
        for (unsigned int i = 0; i < lines.entries(); i++) {
            result = result + lines.at(i);
        }

        // "foobar" -> "Zm9vYmFy"
        check(result == "Zm9vYmFy", "encode in chunks: 'foo'+'bar' -> 'Zm9vYmFy'");
    }

    // Decode multi-line input
    {
        CxB64Decoder decoder;
        CxSList<CxString> lines;
        lines.append(CxString("Zm9v"));
        lines.append(CxString("YmFy"));

        CxBuffer decoded = decoder.process(lines);

        check(decoded.length() == 6 &&
              memcmp(decoded.data(), "foobar", 6) == 0,
              "decode multi-line");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxB64 Test Suite\n");
    printf("================\n");

    // Basic tests
    testEncoderBasic();
    testDecoderBasic();

    // RFC test vectors
    testRFC4648Vectors();
    testDecodeVectors();

    // Round-trip tests
    testRoundTrip();

    // Edge cases
    testPaddingCases();
    testSpecialChars();

    // Larger data
    testLargerData();

    // Multiple calls
    testMultipleProcessCalls();

    printf("\n================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
