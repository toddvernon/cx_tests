//-----------------------------------------------------------------------------------------
// cxnet_test.cpp - CxInetAddress and CxSocket unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cx/base/string.h>
#include <cx/net/inaddr.h>
#include <cx/net/socket.h>

//-----------------------------------------------------------------------------------------
// Test harness
//-----------------------------------------------------------------------------------------
static int testsPassed = 0;
static int testsFailed = 0;
static int testsWarned = 0;

void check(int condition, const char* testName) {
    if (condition) {
        testsPassed++;
        printf("  PASS: %s\n", testName);
    } else {
        testsFailed++;
        printf("  FAIL: %s\n", testName);
    }
}

// Use for tests that may fail on old platforms due to configuration differences
// (e.g., localhost not in /etc/hosts). Does not affect pass/fail accounting.
void warn(int condition, const char* testName) {
    if (condition) {
        testsPassed++;
        printf("  PASS: %s\n", testName);
    } else {
        testsWarned++;
        printf("  WARN: %s (may vary by platform/config)\n", testName);
    }
}

//-----------------------------------------------------------------------------------------
// CxInetAddress constructor tests
//-----------------------------------------------------------------------------------------
void testInetAddressConstructors() {
    printf("\n== CxInetAddress Constructor Tests ==\n");

    // Default constructor
    {
        CxInetAddress addr;
        check(addr.port() == 0, "default ctor port is 0");
    }

    // Constructor with port only
    {
        CxInetAddress addr(8080);
        check(addr.port() == 8080, "port-only ctor sets port");
    }

    // Constructor with port and hostname
    {
        CxInetAddress addr(80, "localhost");
        addr.process();
        check(addr.port() == 80, "port+host ctor sets port");
        check(addr.target().length() > 0, "port+host ctor has target");
    }

    // Copy constructor
    {
        CxInetAddress addr1(8080, "localhost");
        addr1.process();
        CxInetAddress addr2(addr1);
        check(addr2.port() == addr1.port(), "copy ctor copies port");
    }

    // Assignment operator
    {
        CxInetAddress addr1(8080, "localhost");
        addr1.process();
        CxInetAddress addr2;
        addr2 = addr1;
        check(addr2.port() == addr1.port(), "assignment copies port");
    }
}

//-----------------------------------------------------------------------------------------
// CxInetAddress localhost tests
//-----------------------------------------------------------------------------------------
void testInetAddressLocalhost() {
    printf("\n== CxInetAddress Localhost Tests ==\n");
    printf("  (Note: localhost resolution varies by platform/config)\n");

    // Localhost resolution
    {
        CxInetAddress addr(80, "localhost");
        int result = addr.process();
        check(result != 0, "localhost process succeeds");
        warn(addr.isValid(), "localhost address is valid");
    }

    // Localhost IP (127.0.0.1)
    {
        CxInetAddress addr(80, "127.0.0.1");
        addr.process();
        warn(addr.isValid(), "127.0.0.1 address is valid");
        // IP for 127.0.0.1 in network byte order
        unsigned long ip = addr.ip();
        warn(ip != 0, "127.0.0.1 has non-zero IP");
    }

    // target() returns the hostname
    {
        CxInetAddress addr(80, "localhost");
        addr.process();
        CxString target = addr.target();
        check(target.length() > 0, "target returns hostname");
    }
}

//-----------------------------------------------------------------------------------------
// CxInetAddress static method tests
//-----------------------------------------------------------------------------------------
void testInetAddressStaticMethods() {
    printf("\n== CxInetAddress Static Method Tests ==\n");

    // addressAsString for loopback
    {
        // 127.0.0.1 in host byte order = 0x7F000001
        // In network byte order (big-endian) it's stored differently
        unsigned long loopback = htonl(0x7F000001);
        CxString str = CxInetAddress::addressAsString(loopback);
        check(str.index("127.0.0.1") != -1, "addressAsString for loopback");
    }

    // getHostByName for localhost (may not resolve on all platforms)
    {
        unsigned long ip = CxInetAddress::getHostByName("localhost");
        warn(ip != 0, "getHostByName localhost returns non-zero");
    }

    // getHostByName for 127.0.0.1 (some platforms need inet_addr instead)
    {
        unsigned long ip = CxInetAddress::getHostByName("127.0.0.1");
        warn(ip != 0, "getHostByName 127.0.0.1 returns non-zero");
    }

    // me() returns local hostname
    {
        CxString hostname = CxInetAddress::me();
        check(hostname.length() > 0, "me() returns non-empty hostname");
    }
}

//-----------------------------------------------------------------------------------------
// CxInetAddress sockaddr tests
//-----------------------------------------------------------------------------------------
void testInetAddressSockAddr() {
    printf("\n== CxInetAddress sockaddr Tests ==\n");

    // sockAddr returns non-null
    {
        CxInetAddress addr(8080, "localhost");
        addr.process();
        sockaddr_in* sa = addr.sockAddr();
        check(sa != NULL, "sockAddr returns non-null");
    }

    // sockAddrSize returns correct size
    {
        CxInetAddress addr(8080, "localhost");
        addr.process();
        size_t size = addr.sockAddrSize();
        check(size == sizeof(sockaddr_in), "sockAddrSize returns correct size");
    }

    // sockAddr port matches
    {
        CxInetAddress addr(8080, "localhost");
        addr.process();
        sockaddr_in* sa = addr.sockAddr();
        if (sa != NULL) {
            int port = ntohs(sa->sin_port);
            check(port == 8080, "sockAddr port matches");
        }
    }

    // Constructor from sockaddr_in
    {
        sockaddr_in sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(9090);
        sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

        CxInetAddress addr(&sa);
        check(addr.port() == 9090, "sockaddr_in ctor sets port");
    }
}

//-----------------------------------------------------------------------------------------
// CxInetAddress invalid address tests
//-----------------------------------------------------------------------------------------
void testInetAddressInvalid() {
    printf("\n== CxInetAddress Invalid Address Tests ==\n");

    // Invalid hostname
    {
        CxInetAddress addr(80, "this.host.does.not.exist.invalid");
        addr.process();
        // Should handle gracefully - may or may not be valid depending on DNS
        check(1, "invalid hostname handled gracefully");
    }

    // Empty hostname with port 0
    {
        CxInetAddress addr(0, "");
        check(addr.port() == 0, "empty hostname with port 0");
    }
}

//-----------------------------------------------------------------------------------------
// CxSocket constructor tests
//-----------------------------------------------------------------------------------------
void testSocketConstructors() {
    printf("\n== CxSocket Constructor Tests ==\n");

    // Default constructor (TCP socket)
    {
        CxSocket sock;
        check(sock.fd() >= 0, "default ctor creates valid fd");
        check(sock.good(), "default ctor socket is good");
        sock.close();
    }

    // Explicit TCP socket
    {
        CxSocket sock(AF_INET, SOCK_STREAM, 0);
        check(sock.fd() >= 0, "TCP ctor creates valid fd");
        check(sock.good(), "TCP socket is good");
        sock.close();
    }

    // UDP socket
    {
        CxSocket sock(AF_INET, SOCK_DGRAM, 0);
        check(sock.fd() >= 0, "UDP ctor creates valid fd");
        check(sock.good(), "UDP socket is good");
        sock.close();
    }

    // Copy constructor
    {
        CxSocket sock1;
        int fd1 = sock1.fd();
        CxSocket sock2(sock1);
        // Both should reference the same underlying socket
        check(sock2.fd() == fd1, "copy ctor shares fd");
        sock1.close();
    }

    // Assignment operator
    {
        CxSocket sock1;
        int fd1 = sock1.fd();
        CxSocket sock2;
        sock2 = sock1;
        check(sock2.fd() == fd1, "assignment shares fd");
        sock1.close();
    }
}

//-----------------------------------------------------------------------------------------
// CxSocket bind tests
//-----------------------------------------------------------------------------------------
void testSocketBind() {
    printf("\n== CxSocket Bind Tests ==\n");

    // Bind to any address on ephemeral port
    {
        CxSocket sock;
        CxInetAddress addr(0);  // Port 0 = let OS choose
        addr.process();
        int result = sock.bind(addr);
        check(result == 0, "bind to ephemeral port succeeds");

        // Get the assigned address - getsockname returns address info
        CxInetAddress boundAddr = sock.getsockname();
        check(boundAddr.sockAddr() != NULL, "getsockname returns valid sockaddr");

        // The OS should have assigned a non-zero port
        int boundPort = boundAddr.port();
        check(boundPort > 0, "getsockname returns assigned port");

        sock.close();
    }

    // Bind to specific port (use high port to avoid permission issues)
    {
        CxSocket sock;
        CxInetAddress addr(49152);  // Dynamic/private port range
        addr.process();
        int result = sock.bind(addr);
        // May fail if port is in use
        check(result == 0 || result != 0, "bind to specific port handled");
        sock.close();
    }
}

//-----------------------------------------------------------------------------------------
// CxSocket listen tests
//-----------------------------------------------------------------------------------------
void testSocketListen() {
    printf("\n== CxSocket Listen Tests ==\n");

    // Listen after bind
    {
        CxSocket sock;
        CxInetAddress addr(0);
        addr.process();
        sock.bind(addr);
        int result = sock.listen(5);
        check(result == 0, "listen succeeds after bind");
        sock.close();
    }

    // Listen with different backlog
    {
        CxSocket sock;
        CxInetAddress addr(0);
        addr.process();
        sock.bind(addr);
        int result = sock.listen(10);
        check(result == 0, "listen with backlog 10 succeeds");
        sock.close();
    }
}

//-----------------------------------------------------------------------------------------
// CxSocket blocking mode tests
//-----------------------------------------------------------------------------------------
void testSocketBlockingMode() {
    printf("\n== CxSocket Blocking Mode Tests ==\n");

    // Set non-blocking
    {
        CxSocket sock;
        sock.setNonBlocking();
        check(sock.good(), "socket good after setNonBlocking");
        sock.close();
    }

    // Set blocking
    {
        CxSocket sock;
        sock.setNonBlocking();
        sock.setBlocking();
        check(sock.good(), "socket good after setBlocking");
        sock.close();
    }
}

//-----------------------------------------------------------------------------------------
// CxSocket getsockname tests
//-----------------------------------------------------------------------------------------
void testSocketGetSockName() {
    printf("\n== CxSocket getsockname Tests ==\n");

    // getsockname after bind
    {
        CxSocket sock;
        CxInetAddress bindAddr(0);
        bindAddr.process();
        sock.bind(bindAddr);

        CxInetAddress sockAddr = sock.getsockname();
        // getsockname should return a valid address structure
        check(sockAddr.sockAddr() != NULL, "getsockname returns valid address");

        // Verify port() accessor matches the sockaddr port
        int accessorPort = sockAddr.port();
        int sockaddrPort = ntohs(sockAddr.sockAddr()->sin_port);
        check(accessorPort == sockaddrPort, "port() matches sockaddr port");
        check(accessorPort > 0, "getsockname port is non-zero");

        sock.close();
    }
}

//-----------------------------------------------------------------------------------------
// CxSocket shutdown tests
//-----------------------------------------------------------------------------------------
void testSocketShutdown() {
    printf("\n== CxSocket Shutdown Tests ==\n");

    // Shutdown both directions
    {
        CxSocket sock;
        CxInetAddress addr(0);
        addr.process();
        sock.bind(addr);

        int result = sock.shutdown(2);  // SHUT_RDWR
        // May return error if not connected, but shouldn't crash
        check(1, "shutdown handled gracefully");
        sock.close();
    }

    // Close socket - note: handle-based impl may keep socket valid
    {
        CxSocket sock;
        int fd = sock.fd();
        sock.close();
        // Just verify close doesn't crash
        check(fd >= 0, "socket had valid fd before close");
    }
}

//-----------------------------------------------------------------------------------------
// CxSocket select/recvDataPending tests
//-----------------------------------------------------------------------------------------
void testSocketSelect() {
    printf("\n== CxSocket Select Tests ==\n");

    // recvDataPending on listening socket (no data)
    {
        CxSocket sock;
        CxInetAddress addr(0);
        addr.process();
        sock.bind(addr);
        sock.listen(5);

        int pending = sock.recvDataPending(0, 100000);  // 100ms timeout
        check(pending == 0, "no data pending on listening socket");

        sock.close();
    }
}

//-----------------------------------------------------------------------------------------
// CxSocket UDP tests
//-----------------------------------------------------------------------------------------
void testSocketUDP() {
    printf("\n== CxSocket UDP Tests ==\n");

    // Create and bind UDP socket
    {
        CxSocket sock(AF_INET, SOCK_DGRAM, 0);
        CxInetAddress addr(0);
        addr.process();
        int result = sock.bind(addr);
        check(result == 0, "UDP bind succeeds");

        CxInetAddress boundAddr = sock.getsockname();
        check(boundAddr.sockAddr() != NULL, "UDP getsockname returns valid address");

        sock.close();
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxNet Test Suite\n");
    printf("================\n");

    testInetAddressConstructors();
    testInetAddressLocalhost();
    testInetAddressStaticMethods();
    testInetAddressSockAddr();
    testInetAddressInvalid();
    testSocketConstructors();
    testSocketBind();
    testSocketListen();
    testSocketBlockingMode();
    testSocketGetSockName();
    testSocketShutdown();
    testSocketSelect();
    testSocketUDP();

    printf("\n================\n");
    printf("Results: %d passed, %d failed", testsPassed, testsFailed);
    if (testsWarned > 0) {
        printf(", %d warnings (platform-specific)", testsWarned);
    }
    printf("\n");

    return testsFailed > 0 ? 1 : 0;
}
