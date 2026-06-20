//-----------------------------------------------------------------------------------------
// cxprocess_test.cpp - CxProcess unit tests
//
// Covers the run() variants: capture, exit codes, combined stdout/stderr,
// cwd, timeout/kill, signal exit mapping, and state reset between runs.
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <cx/base/string.h>
#include <cx/process/process.h>

static int testsPassed = 0;
static int testsFailed = 0;

static void
check( int condition, const char* testName )
{
    if ( condition ) {
        testsPassed++;
        printf( "  PASS: %s\n", testName );
    } else {
        testsFailed++;
        printf( "  FAIL: %s\n", testName );
    }
}

static int
contains( CxString h, const char* n )
{
    return h.index( CxString( n ) ) != -1;
}

static CxString
trimNewline( CxString s )
{
    while ( s.length() > 0 ) {
        int c = s.charAt( s.length() - 1 );
        if ( c == '\n' || c == '\r' ) {
            s = s.subString( 0, s.length() - 1 );
        } else {
            break;
        }
    }
    return s;
}


int
main( int argc, char **argv )
{
    (void) argc;
    (void) argv;

    printf( "CxProcess Test Suite\n" );
    printf( "====================\n" );

    // 1. simple run
    {
        CxProcess p;
        int rc = p.run( "echo hello" );
        check( rc == 0, "echo: rc 0" );
        check( p.getExitCode() == 0, "echo: exit 0" );
        check( contains( p.getOutput(), "hello" ), "echo: output has hello" );
        check( p.wasTimedOut() == 0, "echo: not timed out" );
    }

    // 2. nonzero exit
    {
        CxProcess p;
        p.run( "exit 3" );
        check( p.getExitCode() == 3, "exit 3: exit code 3" );
    }

    // 3. empty / NULL command
    {
        CxProcess p;
        check( p.run( "" ) == -1, "empty command -> -1" );
        check( p.run( (const char*)0 ) == -1, "NULL command -> -1" );
    }

    // 4. combined stdout + stderr
    {
        CxProcess p;
        p.run( "echo OUT; echo ERR 1>&2" );
        check( contains( p.getOutput(), "OUT" ), "combined: has stdout" );
        check( contains( p.getOutput(), "ERR" ), "combined: has stderr" );
    }

    // 5. cwd honored (use /bin/pwd so we read getcwd, not a stale $PWD)
    {
        CxProcess p;
        p.run( "/bin/pwd", "/", 0 );
        check( trimNewline( p.getOutput() ) == CxString( "/" ), "cwd=/: pwd prints /" );
    }

    // 6. bad cwd -> child chdir fails -> exit 127
    {
        CxProcess p;
        p.run( "/bin/pwd", "/no_such_dir_xyz_12345", 0 );
        check( p.getExitCode() == 127, "bad cwd -> exit 127" );
    }

    // 7. cwd NULL inherits the caller's dir
    {
        CxProcess p;
        p.run( "/bin/pwd", (const char*)0, 0 );
        check( p.getOutput().length() > 0, "cwd NULL: pwd non-empty" );
    }

    // 8. timeout kills a slow command
    {
        CxProcess p;
        int rc = p.run( "sleep 5", (const char*)0, 400 );
        check( rc == -1, "timeout: rc -1" );
        check( p.wasTimedOut() == 1, "timeout: wasTimedOut 1" );
        check( p.getExitCode() == -1, "timeout: exit -1" );
    }

    // 9. fast command under a generous timeout: no false timeout
    {
        CxProcess p;
        p.run( "echo quick", (const char*)0, 5000 );
        check( p.getExitCode() == 0, "no-false-timeout: exit 0" );
        check( p.wasTimedOut() == 0, "no-false-timeout: not timed out" );
        check( contains( p.getOutput(), "quick" ), "no-false-timeout: output" );
    }

    // 10. command killed by a signal -> 128 + signal
    {
        CxProcess p;
        p.run( "kill -TERM $$" );
        check( p.getExitCode() == 128 + 15, "self SIGTERM -> exit 143" );
    }

    // 11. multi-line output
    {
        CxProcess p;
        p.run( "for i in 1 2 3; do echo line$i; done" );
        check( contains( p.getOutput(), "line1" )
            && contains( p.getOutput(), "line2" )
            && contains( p.getOutput(), "line3" ), "multi-line output captured" );
    }

    // 12. state reset between runs
    {
        CxProcess p;
        p.run( "sleep 5", (const char*)0, 300 );      // time out, exit -1
        check( p.wasTimedOut() == 1, "reset setup: timed out" );
        p.run( "echo ok" );                            // must reset cleanly
        check( p.wasTimedOut() == 0, "reset: wasTimedOut cleared" );
        check( p.getExitCode() == 0, "reset: exit 0" );
        check( contains( p.getOutput(), "ok" ), "reset: fresh output" );
        check( !contains( p.getOutput(), "line" ), "reset: no stale output" );
    }

    printf( "\n====================\n" );
    printf( "Results: %d passed, %d failed\n", testsPassed, testsFailed );

    return testsFailed > 0 ? 1 : 0;
}
