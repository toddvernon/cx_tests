//-------------------------------------------------------------------------------------------------
//
//  cxcommandcompleter_example.cpp
//
//  Programmer's guide to the Completer class.
//
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <cx/base/string.h>
#include <cx/commandcompleter/completer.h>


//-------------------------------------------------------------------------------------------------
// Example 1: Basic setup
//
// How to create completers and wire up a two-level hierarchy.
//-------------------------------------------------------------------------------------------------
void example1_setup()
{
    printf("=== Example 1: Setup ===\n\n");

    // userData: your app data returned when candidate is selected
    // Can be anything - command IDs, function pointers, strings, structs...
    int cmdFind = 1;
    int cmdQuit = 2;
    const char *horizSymbol = "─";  // <- this will be returned as userData
    const char *vertSymbol = "│";   // <- this will be returned as userData

    // Level 2: child completer for utf-box symbols
    // addCandidate(name, childCompleter, userData)
    Completer symbols;
    symbols.addCandidate("horizontal", NULL, (void*)horizSymbol);  // userData = "─"
    symbols.addCandidate("vertical", NULL, (void*)vertSymbol);     // userData = "│"

    // Level 1: main commands
    // - "find" and "quit" are leaf commands with userData
    // - "utf-box" has a child completer (symbols) for the next level
    Completer commands;
    commands.addCandidate("find", NULL, &cmdFind);
    commands.addCandidate("quit", NULL, &cmdQuit);
    commands.addCandidate("utf-box", &symbols, NULL);

    printf("Commands: find, quit, utf-box\n");
    printf("utf-box -> child completer with: horizontal, vertical\n\n");
}


//-------------------------------------------------------------------------------------------------
// Example 2: The completion loop
//
// This is how you actually use it - in a loop reacting to user input.
// You don't know what the user will type; you just react to each keypress.
//-------------------------------------------------------------------------------------------------
void example2_completionLoop()
{
    printf("=== Example 2: The completion loop ===\n\n");

    // Setup - same as example 1
    int cmdFind = 1;
    int cmdQuit = 2;
    const char *horizSymbol = "─";
    const char *vertSymbol = "│";

    // addCandidate(name, childCompleter, userData)
    Completer symbols;
    symbols.addCandidate("horizontal", NULL, (void*)horizSymbol);  // userData = "─"
    symbols.addCandidate("vertical", NULL, (void*)vertSymbol);     // userData = "│"

    Completer commands;
    commands.addCandidate("find", NULL, &cmdFind);       // userData = &cmdFind
    commands.addCandidate("quit", NULL, &cmdQuit);       // userData = &cmdQuit
    commands.addCandidate("utf-box", &symbols, NULL);    // has child, no userData

    // --- Session state ---
    Completer *current = &commands;
    CxString input = "";

    // --- Simulate some user input ---
    // In real code this would be: while ((c = getKeypress()) != CANCEL)
    const char *simulatedInput = "u\th\n";  // 'u', TAB, 'h', ENTER

    printf("Simulating input: u <TAB> h <ENTER>\n\n");

    for (int i = 0; simulatedInput[i]; i++) {
        char c = simulatedInput[i];

        // --- Handle ENTER ---
        if (c == '\n') {
            printf("[ENTER]\n");
            CompleterResult r = current->processEnter(input);

            switch (r.getStatus()) {
            case COMPLETER_SELECTED:
                printf("  -> SELECTED: %s\n", r.getSelectedName().data());
                // userData is what we passed as 3rd arg to addCandidate()
                // In this case it's the symbol string "─"
                printf("  -> userData: \"%s\"\n", (const char*)r.getSelectedData());
                // Done! In real code: execute command, exit loop
                break;

            case COMPLETER_NO_MATCH:
                printf("  -> NO_MATCH: unknown command\n");
                // Stay in loop, let user try again
                break;

            case COMPLETER_MULTIPLE:
                printf("  -> MULTIPLE: ambiguous, need more input\n");
                // Stay in loop
                break;

            default:
                break;
            }
            continue;
        }

        // --- Handle TAB ---
        if (c == '\t') {
            printf("[TAB]\n");
            CompleterResult r = current->processTab(input);
            input = r.getInput();
            printf("  -> input: \"%s\"\n", input.data());

            // TAB can also trigger level transition
            if (r.getStatus() == COMPLETER_NEXT_LEVEL) {
                printf("  -> NEXT_LEVEL: transitioning to child\n");
                current = r.getNextLevel();
                input = "";
            }
            continue;
        }

        // --- Handle regular character ---
        printf("[char '%c']\n", c);
        CompleterResult r = current->processChar(input, c);
        input = r.getInput();

        switch (r.getStatus()) {
        case COMPLETER_NO_MATCH:
            printf("  -> NO_MATCH: \"%s\"\n", input.data());
            // Could reject the character, beep, etc.
            break;

        case COMPLETER_MULTIPLE:
            printf("  -> MULTIPLE: \"%s\" (%d matches)\n", input.data(), r.getMatchCount());
            // Show hints to user
            {
                CxString hints[10];
                int n = current->findMatches(input, hints, 10);
                printf("  -> hints: ");
                for (int j = 0; j < n; j++) printf("%s ", hints[j].data());
                printf("\n");
            }
            break;

        case COMPLETER_PARTIAL:
            printf("  -> PARTIAL: \"%s\" (auto-completed, %d matches)\n", input.data(), r.getMatchCount());
            // Auto-completed to common prefix, but still multiple matches
            // Show hints
            break;

        case COMPLETER_UNIQUE:
            printf("  -> UNIQUE: \"%s\" (ready for ENTER)\n", input.data());
            // Single match, auto-completed, waiting for confirmation
            break;

        case COMPLETER_NEXT_LEVEL:
            printf("  -> NEXT_LEVEL: \"%s\"\n", input.data());
            // Single match with child completer - transition immediately
            current = r.getNextLevel();
            input = "";
            printf("  -> transitioned to child completer\n");
            break;

        default:
            break;
        }
    }

    printf("\n");
}


//-------------------------------------------------------------------------------------------------
// Example 3: Pseudocode for real integration
//
// Shows the pattern without simulation - what your real code looks like.
//-------------------------------------------------------------------------------------------------
void example3_pseudocode()
{
    printf("=== Example 3: Real integration pattern ===\n\n");

    printf("// Your event loop looks like this:\n\n");

    printf("Completer *current = &commands;\n");
    printf("CxString input = \"\";\n");
    printf("bool done = false;\n\n");

    printf("while (!done) {\n");
    printf("    char c = getKeypress();  // Your input method\n\n");

    printf("    if (c == ESCAPE) {\n");
    printf("        done = true;  // User cancelled\n");
    printf("    }\n");
    printf("    else if (c == ENTER) {\n");
    printf("        CompleterResult r = current->processEnter(input);\n");
    printf("        if (r.getStatus() == COMPLETER_SELECTED) {\n");
    printf("            executeCommand(r.getSelectedData());\n");
    printf("            done = true;\n");
    printf("        } else {\n");
    printf("            showError(\"Unknown or ambiguous\");\n");
    printf("        }\n");
    printf("    }\n");
    printf("    else if (c == TAB) {\n");
    printf("        CompleterResult r = current->processTab(input);\n");
    printf("        input = r.getInput();\n");
    printf("        if (r.getStatus() == COMPLETER_NEXT_LEVEL) {\n");
    printf("            current = r.getNextLevel();\n");
    printf("            input = \"\";\n");
    printf("        }\n");
    printf("        updateDisplay(input);\n");
    printf("    }\n");
    printf("    else {\n");
    printf("        CompleterResult r = current->processChar(input, c);\n");
    printf("        input = r.getInput();\n\n");

    printf("        if (r.getStatus() == COMPLETER_NEXT_LEVEL) {\n");
    printf("            current = r.getNextLevel();\n");
    printf("            input = \"\";\n");
    printf("        }\n\n");

    printf("        updateDisplay(input);\n");
    printf("        if (r.getMatchCount() > 1) showHints(...);\n");
    printf("    }\n");
    printf("}\n");

    printf("\n");
}


//-------------------------------------------------------------------------------------------------
// main
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    printf("\nCompleter API Examples\n");
    printf("======================\n\n");

    example1_setup();
    example2_completionLoop();
    example3_pseudocode();

    return 0;
}
