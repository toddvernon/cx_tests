#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

typedef enum {
    PARSE_OK,
    PARSE_EMPTY,
    PARSE_INVALID_CHAR,
    PARSE_MULTIPLE_DOTS,
    PARSE_MULTIPLE_SIGNS,
    PARSE_NO_DIGITS,
    PARSE_EXP_NO_DIGITS
} ParseError;

typedef struct {
    double value;
    ParseError error;
} ParseResult;

const char* error_to_string(ParseError err) {
    switch (err) {
        case PARSE_OK: return "Success";
        case PARSE_EMPTY: return "Empty string";
        case PARSE_INVALID_CHAR: return "Invalid character";
        case PARSE_MULTIPLE_DOTS: return "Multiple decimal points";
        case PARSE_MULTIPLE_SIGNS: return "Multiple signs";
        case PARSE_NO_DIGITS: return "No digits found";
        case PARSE_EXP_NO_DIGITS: return "No digits in exponent";
        default: return "Unknown error";
    }
}

ParseResult parse_float(const char* str) {
    ParseResult result = {0, PARSE_OK};
    
    enum {
        START,
        SIGN,
        INT_DIGITS,
        DOT,
        FRAC_DIGITS,
        EXP_SIGN,
        EXP_DIGITS
    } state = START;
    
    bool negative = false, exp_negative = false;
    bool has_dot = false, has_exp = false;
    double value = 0.0, fraction = 0.1;
    int exponent = 0;
    
    while (*str) {
        char c = *str++;
        
        switch (state) {
            case START:
                if (isspace(c)) continue;
                if (c == '+' || c == '-') {
                    negative = (c == '-');
                    state = SIGN;
                } else if (isdigit(c)) {
                    value = c - '0';
                    state = INT_DIGITS;
                } else if (c == '.') {
                    state = DOT;
                    has_dot = true;
                } else {
                    result.error = PARSE_INVALID_CHAR;
                    return result;
                }
                break;
                
            case SIGN:
                if (isdigit(c)) {
                    value = c - '0';
                    state = INT_DIGITS;
                } else if (c == '.') {
                    state = DOT;
                    has_dot = true;
                } else {
                    result.error = PARSE_NO_DIGITS;
                    return result;
                }
                break;
                
            case INT_DIGITS:
                if (isdigit(c)) {
                    value = value * 10 + (c - '0');
                } else if (c == '.') {
                    if (has_dot) {
                        result.error = PARSE_MULTIPLE_DOTS;
                        return result;
                    }
                    state = DOT;
                    has_dot = true;
                } else if (c == 'e' || c == 'E') {
                    state = EXP_SIGN;
                    has_exp = true;
                } else {
                    result.error = PARSE_INVALID_CHAR;
                    return result;
                }
                break;
                
            case DOT:
                if (isdigit(c)) {
                    value += (c - '0') * fraction;
                    fraction *= 0.1;
                    state = FRAC_DIGITS;
                } else if (c == 'e' || c == 'E') {
                    if (!isdigit(*(str - 2))) {
                        result.error = PARSE_NO_DIGITS;
                        return result;
                    }
                    state = EXP_SIGN;
                    has_exp = true;
                } else {
                    result.error = PARSE_NO_DIGITS;
                    return result;
                }
                break;
                
            case FRAC_DIGITS:
                if (isdigit(c)) {
                    value += (c - '0') * fraction;
                    fraction *= 0.1;
                } else if (c == 'e' || c == 'E') {
                    state = EXP_SIGN;
                    has_exp = true;
                } else {
                    result.error = PARSE_INVALID_CHAR;
                    return result;
                }
                break;
                
            case EXP_SIGN:
                if (c == '+' || c == '-') {
                    exp_negative = (c == '-');
                    state = EXP_DIGITS;
                } else if (isdigit(c)) {
                    exponent = c - '0';
                    state = EXP_DIGITS;
                } else {
                    result.error = PARSE_EXP_NO_DIGITS;
                    return result;
                }
                break;
                
            case EXP_DIGITS:
                if (isdigit(c)) {
                    exponent = exponent * 10 + (c - '0');
                } else {
                    result.error = PARSE_INVALID_CHAR;
                    return result;
                }
                break;
        }
    }
    
    if (state == START) {
        result.error = PARSE_EMPTY;
    } else if (state == SIGN || state == DOT) {
        result.error = PARSE_NO_DIGITS;
    } else if (state == EXP_SIGN) {
        result.error = PARSE_EXP_NO_DIGITS;
    } else {
        if (has_exp) {
            value *= pow(10, exp_negative ? -exponent : exponent);
        }
        result.value = negative ? -value : value;
    }
    
    return result;
}

void test_parser(const char* input) {
    printf("Testing: '%s'\n", input);
    ParseResult result = parse_float(input);
    
    if (result.error == PARSE_OK) {
        printf("  Success: %f\n", result.value);
    } else {
        printf("  Error: %s\n", error_to_string(result.error));
    }
    printf("--------------------\n");
}

int main() {
    // Valid numbers
    test_parser("123.456");
    test_parser("-42");
    test_parser("+3.14");
    test_parser(".5");
    test_parser("1e3");
    test_parser("-2.5E-4");
    test_parser("0.123e+10");
    test_parser("  42.0  "); // with whitespace
    
    // Invalid numbers
    test_parser("");
    test_parser("abc");
    test_parser("1..2");
    test_parser("--5");
    test_parser("+");
    test_parser(".");
    test_parser("1e");
    test_parser("1e+");
    test_parser("1.2.3");
    test_parser("12e3.4");
    test_parser("1a.5");
    
    return 0;
}

/*

### Test Cases Included:

**Valid Numbers:**
1. Standard decimal (`123.456`)
2. Negative integer (`-42`)
3. Positive with explicit sign (`+3.14`)
4. Decimal without leading digit (`.5`)
5. Scientific notation (`1e3`)
6. Negative with exponent (`-2.5E-4`)
7. Positive exponent (`0.123e+10`)
8. With whitespace (`  42.0  `)

**Error Cases:**
1. Empty string
2. Non-numeric characters (`abc`)
3. Multiple decimal points (`1..2`)
4. Multiple signs (`--5`)
5. Sign without digits (`+`)
6. Decimal point without digits (`.`)
7. Exponent without digits (`1e`)
8. Exponent sign without digits (`1e+`)
9. Multiple decimal points (`1.2.3`)
10. Decimal in exponent (`12e3.4`)
11. Invalid character (`1a.5`)

Each test prints whether the parse was successful (with the parsed value) or the specific error encountered. The `error_to_string()` function provides human-readable error messages.

Sources

*/
