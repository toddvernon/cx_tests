//-------------------------------------------------------------------------------------------------
//
//  cxcommandcompleter_test.cpp
//
//  Test suite for the Completer class.
//
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <cx/base/string.h>
#include <cx/commandcompleter/completer.h>

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
// Matching tests
//-------------------------------------------------------------------------------------------------

TEST(test_prefix_match_single_char)
{
    Completer c;
    c.addCandidate("quit");

    CompleterResult r = c.processChar("", 'q');
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);
    ASSERT(r.getInput() == "quit");
    PASS();
}

TEST(test_prefix_match_multiple_chars)
{
    // Need multiple candidates so first char doesn't auto-complete
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("binary");

    CompleterResult r = c.processChar("", 'b');
    ASSERT(r.getStatus() == COMPLETER_MULTIPLE);  // 'b' matches both

    r = c.processChar(r.getInput(), 'u');
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);  // 'bu' only matches buffer-list
    ASSERT(r.getInput() == "buffer-list");
    PASS();
}

TEST(test_dehyphenated_match)
{
    // "bufferl" should match "buffer-list" (hyphens ignored in matching)
    // Need multiple candidates so typing doesn't auto-complete too early
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");

    // Type "buffer" - gets to common prefix
    CompleterResult r = c.processChar("", 'b');
    ASSERT(r.getInput() == "buffer-");  // auto-completed to common prefix

    // Type 'l' (skipping typing the hyphen) - should match buffer-list
    r = c.processChar(r.getInput(), 'l');
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);
    ASSERT(r.getInput() == "buffer-list");
    PASS();
}

TEST(test_no_match)
{
    Completer c;
    c.addCandidate("quit");
    c.addCandidate("find");

    CompleterResult r = c.processChar("", 'x');
    ASSERT(r.getStatus() == COMPLETER_NO_MATCH);
    ASSERT(r.getMatchCount() == 0);
    PASS();
}


//-------------------------------------------------------------------------------------------------
// processChar status tests
//-------------------------------------------------------------------------------------------------

TEST(test_status_unique)
{
    // Single match, no child
    Completer c;
    c.addCandidate("quit");
    c.addCandidate("find");

    CompleterResult r = c.processChar("", 'q');
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);
    ASSERT(r.getMatchCount() == 1);
    PASS();
}

TEST(test_status_multiple)
{
    // Multiple matches, no common prefix beyond what's typed
    Completer c;
    c.addCandidate("save");
    c.addCandidate("search");

    CompleterResult r = c.processChar("", 's');
    ASSERT(r.getStatus() == COMPLETER_MULTIPLE);
    ASSERT(r.getMatchCount() == 2);
    ASSERT(r.getInput() == "s");  // no auto-completion
    PASS();
}

TEST(test_status_partial)
{
    // Multiple matches with common prefix - auto-completes to prefix
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");

    CompleterResult r = c.processChar("", 'b');
    ASSERT(r.getStatus() == COMPLETER_PARTIAL);
    ASSERT(r.getMatchCount() == 2);
    ASSERT(r.getInput() == "buffer-");  // auto-completed to common prefix
    PASS();
}

TEST(test_status_next_level)
{
    // Single match with child completer
    Completer child;
    child.addCandidate("option1");

    Completer c;
    c.addCandidate("parent", &child, NULL);

    CompleterResult r = c.processChar("", 'p');
    ASSERT(r.getStatus() == COMPLETER_NEXT_LEVEL);
    ASSERT(r.getNextLevel() == &child);
    PASS();
}

TEST(test_status_no_match_processchar)
{
    Completer c;
    c.addCandidate("quit");

    CompleterResult r = c.processChar("", 'z');
    ASSERT(r.getStatus() == COMPLETER_NO_MATCH);
    PASS();
}


//-------------------------------------------------------------------------------------------------
// processEnter status tests
//-------------------------------------------------------------------------------------------------

TEST(test_enter_selected_exact)
{
    // Exact match
    Completer c;
    c.addCandidate("quit");

    CompleterResult r = c.processEnter("quit");
    ASSERT(r.getStatus() == COMPLETER_SELECTED);
    ASSERT(r.getSelectedName() == "quit");
    PASS();
}

TEST(test_enter_selected_unique_prefix)
{
    // Unique prefix match
    Completer c;
    c.addCandidate("quit");
    c.addCandidate("find");

    CompleterResult r = c.processEnter("q");
    ASSERT(r.getStatus() == COMPLETER_SELECTED);
    ASSERT(r.getSelectedName() == "quit");
    PASS();
}

TEST(test_enter_no_match)
{
    Completer c;
    c.addCandidate("quit");

    CompleterResult r = c.processEnter("xyz");
    ASSERT(r.getStatus() == COMPLETER_NO_MATCH);
    PASS();
}

TEST(test_enter_multiple_ambiguous)
{
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");

    CompleterResult r = c.processEnter("buffer-");
    ASSERT(r.getStatus() == COMPLETER_MULTIPLE);
    ASSERT(r.getMatchCount() == 2);
    PASS();
}


//-------------------------------------------------------------------------------------------------
// Auto-completion tests
//-------------------------------------------------------------------------------------------------

TEST(test_autocomplete_single_match)
{
    Completer c;
    c.addCandidate("quit");

    CompleterResult r = c.processChar("", 'q');
    ASSERT(r.getInput() == "quit");  // fully completed
    PASS();
}

TEST(test_autocomplete_common_prefix)
{
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");
    c.addCandidate("buffer-prev");

    CompleterResult r = c.processChar("", 'b');
    ASSERT(r.getInput() == "buffer-");  // common prefix
    PASS();
}

TEST(test_autocomplete_no_common_prefix)
{
    Completer c;
    c.addCandidate("save");
    c.addCandidate("search");

    CompleterResult r = c.processChar("", 's');
    ASSERT(r.getInput() == "s");  // no common prefix beyond 's'
    PASS();
}

TEST(test_autocomplete_resolves_ambiguity)
{
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");

    CompleterResult r = c.processChar("", 'b');
    ASSERT(r.getStatus() == COMPLETER_PARTIAL);
    ASSERT(r.getInput() == "buffer-");

    // Type 'l' to resolve
    r = c.processChar(r.getInput(), 'l');
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);
    ASSERT(r.getInput() == "buffer-list");
    PASS();
}


//-------------------------------------------------------------------------------------------------
// userData tests
//-------------------------------------------------------------------------------------------------

TEST(test_userdata_returned_on_unique)
{
    int myData = 42;

    Completer c;
    c.addCandidate("quit", NULL, &myData);

    CompleterResult r = c.processChar("", 'q');
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);
    ASSERT(r.getSelectedData() == &myData);
    PASS();
}

TEST(test_userdata_returned_on_enter)
{
    int myData = 99;

    Completer c;
    c.addCandidate("quit", NULL, &myData);

    CompleterResult r = c.processEnter("quit");
    ASSERT(r.getStatus() == COMPLETER_SELECTED);
    ASSERT(r.getSelectedData() == &myData);
    ASSERT(*(int*)r.getSelectedData() == 99);
    PASS();
}

TEST(test_userdata_null_when_ambiguous)
{
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");

    CompleterResult r = c.processChar("", 'b');
    ASSERT(r.getStatus() == COMPLETER_PARTIAL);
    ASSERT(r.getSelectedData() == NULL);
    PASS();
}


//-------------------------------------------------------------------------------------------------
// Child completer tests
//-------------------------------------------------------------------------------------------------

TEST(test_child_completer_transition)
{
    Completer child;
    child.addCandidate("horizontal");
    child.addCandidate("vertical");

    Completer parent;
    parent.addCandidate("utf-box", &child, NULL);

    CompleterResult r = parent.processChar("", 'u');
    ASSERT(r.getStatus() == COMPLETER_NEXT_LEVEL);
    ASSERT(r.getNextLevel() == &child);
    ASSERT(r.getInput() == "utf-box");
    PASS();
}

TEST(test_child_completer_full_flow)
{
    const char *symbol = "─";

    Completer child;
    child.addCandidate("horizontal", NULL, (void*)symbol);
    child.addCandidate("vertical", NULL, (void*)"│");

    Completer parent;
    parent.addCandidate("utf-box", &child, NULL);

    // Select parent
    CompleterResult r = parent.processChar("", 'u');
    ASSERT(r.getStatus() == COMPLETER_NEXT_LEVEL);

    // Transition to child
    Completer *current = r.getNextLevel();

    // Select from child
    r = current->processChar("", 'h');
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);
    ASSERT(r.getInput() == "horizontal");

    // Confirm
    r = current->processEnter(r.getInput());
    ASSERT(r.getStatus() == COMPLETER_SELECTED);
    ASSERT(r.getSelectedData() == symbol);
    PASS();
}


//-------------------------------------------------------------------------------------------------
// Edge case tests
//-------------------------------------------------------------------------------------------------

TEST(test_empty_completer)
{
    Completer c;
    // No candidates added

    CompleterResult r = c.processChar("", 'a');
    ASSERT(r.getStatus() == COMPLETER_NO_MATCH);
    ASSERT(r.getMatchCount() == 0);
    PASS();
}

TEST(test_single_candidate)
{
    Completer c;
    c.addCandidate("only-one");

    // Any matching char completes immediately
    CompleterResult r = c.processChar("", 'o');
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);
    ASSERT(r.getInput() == "only-one");
    PASS();
}

TEST(test_empty_input_enter)
{
    Completer c;
    c.addCandidate("quit");

    CompleterResult r = c.processEnter("");
    // Empty input matches everything, so it's ambiguous if multiple
    // With single candidate, it should match
    ASSERT(r.getStatus() == COMPLETER_SELECTED);
    PASS();
}

TEST(test_empty_input_enter_multiple)
{
    Completer c;
    c.addCandidate("quit");
    c.addCandidate("find");

    CompleterResult r = c.processEnter("");
    // Empty matches both - ambiguous
    ASSERT(r.getStatus() == COMPLETER_MULTIPLE);
    PASS();
}


//-------------------------------------------------------------------------------------------------
// Incomplete state tests (not done yet)
//-------------------------------------------------------------------------------------------------

TEST(test_incomplete_needs_more_chars)
{
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");

    CompleterResult r = c.processChar("", 'b');
    ASSERT(r.getStatus() == COMPLETER_PARTIAL);  // NOT done

    // Still need more input
    r = c.processEnter(r.getInput());
    ASSERT(r.getStatus() == COMPLETER_MULTIPLE);  // Can't select yet
    PASS();
}

TEST(test_tab_at_common_prefix_still_stuck)
{
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");

    // Get to common prefix
    CompleterResult r = c.processChar("", 'b');
    ASSERT(r.getInput() == "buffer-");

    // TAB doesn't help - already at common prefix
    r = c.processTab(r.getInput());
    ASSERT(r.getStatus() == COMPLETER_MULTIPLE);
    ASSERT(r.getInput() == "buffer-");  // unchanged
    PASS();
}

TEST(test_tab_completes_when_possible)
{
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("quit");

    CompleterResult r = c.processTab("b");
    ASSERT(r.getStatus() == COMPLETER_UNIQUE);
    ASSERT(r.getInput() == "buffer-list");
    PASS();
}


//-------------------------------------------------------------------------------------------------
// findMatches tests
//-------------------------------------------------------------------------------------------------

TEST(test_find_matches_returns_names)
{
    Completer c;
    c.addCandidate("buffer-list");
    c.addCandidate("buffer-next");
    c.addCandidate("quit");

    CxString names[10];
    int count = c.findMatches("buffer", names, 10);

    ASSERT(count == 2);
    ASSERT(names[0] == "buffer-list");
    ASSERT(names[1] == "buffer-next");
    PASS();
}

TEST(test_find_matches_respects_limit)
{
    Completer c;
    c.addCandidate("a1");
    c.addCandidate("a2");
    c.addCandidate("a3");

    CxString names[2];
    int count = c.findMatches("a", names, 2);

    ASSERT(count == 2);  // limited to 2
    PASS();
}


//-------------------------------------------------------------------------------------------------
// main
//-------------------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    printf("\nCompleter Test Suite\n");
    printf("====================\n\n");

    // Matching tests
    printf("--- Matching ---\n");
    RUN_TEST(test_prefix_match_single_char);
    RUN_TEST(test_prefix_match_multiple_chars);
    RUN_TEST(test_dehyphenated_match);
    RUN_TEST(test_no_match);

    // processChar status tests
    printf("\n--- processChar status ---\n");
    RUN_TEST(test_status_unique);
    RUN_TEST(test_status_multiple);
    RUN_TEST(test_status_partial);
    RUN_TEST(test_status_next_level);
    RUN_TEST(test_status_no_match_processchar);

    // processEnter status tests
    printf("\n--- processEnter status ---\n");
    RUN_TEST(test_enter_selected_exact);
    RUN_TEST(test_enter_selected_unique_prefix);
    RUN_TEST(test_enter_no_match);
    RUN_TEST(test_enter_multiple_ambiguous);

    // Auto-completion tests
    printf("\n--- Auto-completion ---\n");
    RUN_TEST(test_autocomplete_single_match);
    RUN_TEST(test_autocomplete_common_prefix);
    RUN_TEST(test_autocomplete_no_common_prefix);
    RUN_TEST(test_autocomplete_resolves_ambiguity);

    // userData tests
    printf("\n--- userData ---\n");
    RUN_TEST(test_userdata_returned_on_unique);
    RUN_TEST(test_userdata_returned_on_enter);
    RUN_TEST(test_userdata_null_when_ambiguous);

    // Child completer tests
    printf("\n--- Child completers ---\n");
    RUN_TEST(test_child_completer_transition);
    RUN_TEST(test_child_completer_full_flow);

    // Edge cases
    printf("\n--- Edge cases ---\n");
    RUN_TEST(test_empty_completer);
    RUN_TEST(test_single_candidate);
    RUN_TEST(test_empty_input_enter);
    RUN_TEST(test_empty_input_enter_multiple);

    // Incomplete states
    printf("\n--- Incomplete states ---\n");
    RUN_TEST(test_incomplete_needs_more_chars);
    RUN_TEST(test_tab_at_common_prefix_still_stuck);
    RUN_TEST(test_tab_completes_when_possible);

    // findMatches tests
    printf("\n--- findMatches ---\n");
    RUN_TEST(test_find_matches_returns_names);
    RUN_TEST(test_find_matches_respects_limit);

    // Summary
    printf("\n====================\n");
    printf("Passed: %d\n", gTestsPassed);
    printf("Failed: %d\n", gTestsFailed);
    printf("\n");

    return gTestsFailed > 0 ? 1 : 0;
}
