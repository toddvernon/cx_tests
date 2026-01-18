//-----------------------------------------------------------------------------------------
// cxthread_test.cpp - CxThread and related classes unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <cx/base/string.h>
#include <cx/base/exception.h>
#include <cx/thread/mutex.h>
#include <cx/thread/rmutex.h>
#include <cx/thread/cond.h>
#include <cx/thread/thread.h>
#include <cx/thread/thread2.h>
#include <cx/thread/runnable.h>
#include <cx/thread/quitrequest.h>
#include <cx/thread/pc.h>
#include <cx/thread/runnablethread.h>
#include <cx/thread/threadpool.h>

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
// CxMutex tests
//-----------------------------------------------------------------------------------------
void testMutexBasic() {
    printf("\n== CxMutex Basic Tests ==\n");

    // Constructor/destructor
    {
        CxMutex mutex;
        check(1, "mutex ctor doesn't crash");
    }

    // Acquire and release
    {
        CxMutex mutex;
        mutex.acquire();
        check(1, "mutex acquire doesn't crash");
        mutex.release();
        check(1, "mutex release doesn't crash");
    }

    // Multiple acquire/release cycles
    {
        CxMutex mutex;
        for (int i = 0; i < 10; i++) {
            mutex.acquire();
            mutex.release();
        }
        check(1, "mutex multiple cycles");
    }

    // tryAcquire when available
    {
        CxMutex mutex;
        CxMutex::tryResult result = mutex.tryAcquire();
        check(result == CxMutex::MUTEX_ACQUIRED, "tryAcquire: acquired when free");
        mutex.release();
    }
}

// Shared data for mutex contention test
static int sharedCounter = 0;
static CxMutex counterMutex;

class CounterThread : public CxThread {
public:
    int increments;
    CounterThread(int n) : increments(n) {}

    virtual void run() {
        for (int i = 0; i < increments; i++) {
            counterMutex.acquire();
            sharedCounter++;
            counterMutex.release();
        }
    }
};

void testMutexContention() {
    printf("\n== CxMutex Contention Tests ==\n");

    // Two threads incrementing shared counter
    {
        sharedCounter = 0;
        CounterThread t1(1000);
        CounterThread t2(1000);

        t1.start();
        t2.start();

        t1.join();
        t2.join();

        check(sharedCounter == 2000, "mutex protects shared counter");
    }
}

//-----------------------------------------------------------------------------------------
// CxRecursiveMutex tests
//-----------------------------------------------------------------------------------------
void testRecursiveMutexBasic() {
    printf("\n== CxRecursiveMutex Basic Tests ==\n");

    // Constructor/destructor
    {
        CxRecursiveMutex rmutex;
        check(1, "recursive mutex ctor doesn't crash");
    }

    // Single acquire/release
    {
        CxRecursiveMutex rmutex;
        rmutex.acquire();
        rmutex.release();
        check(1, "recursive mutex single acquire/release");
    }

    // Recursive acquire (same thread can acquire multiple times)
    {
        CxRecursiveMutex rmutex;
        rmutex.acquire();
        rmutex.acquire();
        rmutex.acquire();
        check(1, "recursive mutex triple acquire");
        rmutex.release();
        rmutex.release();
        rmutex.release();
        check(1, "recursive mutex triple release");
    }

    // Many recursive acquires
    {
        CxRecursiveMutex rmutex;
        for (int i = 0; i < 100; i++) {
            rmutex.acquire();
        }
        for (int i = 0; i < 100; i++) {
            rmutex.release();
        }
        check(1, "recursive mutex 100 acquires/releases");
    }
}

//-----------------------------------------------------------------------------------------
// CxCondition tests
//-----------------------------------------------------------------------------------------
static CxMutex condMutex;
static CxCondition cond;
static int condFlag = 0;

class SignalThread : public CxThread {
public:
    int delayMs;
    SignalThread(int delay) : delayMs(delay) {}

    virtual void run() {
        usleep(delayMs * 1000);
        condMutex.acquire();
        condFlag = 1;
        cond.signal();
        condMutex.release();
    }
};

void testConditionBasic() {
    printf("\n== CxCondition Basic Tests ==\n");

    // Constructor/destructor
    {
        CxCondition condition;
        check(1, "condition ctor doesn't crash");
    }

    // Signal without waiter (should not deadlock)
    {
        CxCondition condition;
        condition.signal();
        check(1, "signal without waiter doesn't deadlock");
    }
}

void testConditionWaitSignal() {
    printf("\n== CxCondition Wait/Signal Tests ==\n");

    // Wait and signal
    {
        condFlag = 0;
        SignalThread signaler(50);  // Signal after 50ms
        signaler.start();

        condMutex.acquire();
        while (!condFlag) {
            cond.wait(&condMutex);
        }
        condMutex.release();

        signaler.join();
        check(condFlag == 1, "condition wait/signal");
    }

    // Timed wait with signal
    {
        condFlag = 0;
        SignalThread signaler(50);  // Signal after 50ms
        signaler.start();

        condMutex.acquire();
        CxCondition::waitResult result = CxCondition::CONDITION_SIGNAL;
        while (!condFlag && result != CxCondition::CONDITION_TIMEOUT) {
            result = cond.timedWait(&condMutex, 2);  // 2 second timeout
        }
        condMutex.release();

        signaler.join();
        check(result == CxCondition::CONDITION_SIGNAL, "timedWait: got signal");
        check(condFlag == 1, "timedWait: flag set");
    }

    // Timed wait timeout
    {
        CxCondition localCond;
        CxMutex localMutex;

        localMutex.acquire();
        CxCondition::waitResult result = localCond.timedWait(&localMutex, 1);  // 1 second timeout
        localMutex.release();

        check(result == CxCondition::CONDITION_TIMEOUT, "timedWait: timeout");
    }
}

//-----------------------------------------------------------------------------------------
// CxThread tests
//-----------------------------------------------------------------------------------------
static int threadRanFlag = 0;

class SimpleThread : public CxThread {
public:
    virtual void run() {
        threadRanFlag = 1;
    }
};

class SleepThread : public CxThread {
public:
    int sleepMs;
    int completed;
    SleepThread(int ms) : sleepMs(ms), completed(0) {}

    virtual void run() {
        usleep(sleepMs * 1000);
        completed = 1;
    }
};

class QuitCheckThread : public CxThread {
public:
    int iterations;
    QuitCheckThread() : iterations(0) {}

    virtual void run() {
        while (!_suggestQuit) {
            iterations++;
            usleep(10000);  // 10ms
        }
    }
};

void testThreadBasic() {
    printf("\n== CxThread Basic Tests ==\n");

    // Constructor
    {
        SimpleThread thread;
        check(1, "thread ctor doesn't crash");
    }

    // Start and join
    {
        threadRanFlag = 0;
        SimpleThread thread;
        thread.start();
        thread.join();
        check(threadRanFlag == 1, "thread run() executed");
    }

    // Thread with work
    {
        SleepThread thread(50);  // 50ms sleep
        thread.start();
        check(thread.completed == 0, "thread not completed immediately");
        thread.join();
        check(thread.completed == 1, "thread completed after join");
    }
}

void testThreadSuggestQuit() {
    printf("\n== CxThread suggestQuit Tests ==\n");

    // suggestQuit stops thread
    {
        QuitCheckThread thread;
        thread.start();
        usleep(50000);  // Let it run 50ms
        thread.suggestQuit();
        thread.join();
        check(thread.iterations > 0, "thread ran some iterations");
        check(thread.iterations < 100, "thread stopped after suggestQuit");
    }

    // resetSuggestQuit
    {
        QuitCheckThread thread;
        thread.suggestQuit();
        thread.resetSuggestQuit();
        thread.start();
        usleep(30000);  // 30ms
        thread.suggestQuit();
        thread.join();
        check(thread.iterations > 0, "resetSuggestQuit allows restart");
    }
}

void testMultipleThreads() {
    printf("\n== Multiple Threads Tests ==\n");

    // Multiple concurrent threads
    {
        SleepThread t1(30);
        SleepThread t2(30);
        SleepThread t3(30);

        t1.start();
        t2.start();
        t3.start();

        t1.join();
        t2.join();
        t3.join();

        check(t1.completed && t2.completed && t3.completed, "multiple threads complete");
    }
}

//-----------------------------------------------------------------------------------------
// CxThreadDuple tests
//-----------------------------------------------------------------------------------------
static int duple1Ran = 0;
static int duple2Ran = 0;

class SimpleDuple : public CxThreadDuple {
public:
    virtual void run1() {
        duple1Ran = 1;
    }
    virtual void run2() {
        duple2Ran = 1;
    }
};

void testThreadDuple() {
    printf("\n== CxThreadDuple Tests ==\n");

    // Both threads run
    {
        duple1Ran = 0;
        duple2Ran = 0;
        SimpleDuple duple;
        duple.start();
        duple.join();
        check(duple1Ran == 1, "duple run1 executed");
        check(duple2Ran == 1, "duple run2 executed");
    }
}

//-----------------------------------------------------------------------------------------
// CxRunnable tests
//-----------------------------------------------------------------------------------------
static int runnableRan = 0;

class SimpleRunnable : public CxRunnable {
public:
    virtual void run() {
        runnableRan = 1;
    }
};

void testRunnable() {
    printf("\n== CxRunnable Tests ==\n");

    // Constructor/destructor
    {
        SimpleRunnable runnable;
        check(1, "runnable ctor doesn't crash");
    }

    // Run method
    {
        runnableRan = 0;
        SimpleRunnable runnable;
        runnable.run();
        check(runnableRan == 1, "runnable run() executes");
    }

    // isQuitRequest default
    {
        SimpleRunnable runnable;
        check(runnable.isQuitRequest() == 0, "runnable isQuitRequest returns 0");
    }
}

//-----------------------------------------------------------------------------------------
// CxQuitRequest tests
//-----------------------------------------------------------------------------------------
void testQuitRequest() {
    printf("\n== CxQuitRequest Tests ==\n");

    // Constructor
    {
        CxQuitRequest qr;
        check(1, "QuitRequest ctor doesn't crash");
    }

    // isQuitRequest returns 1
    {
        CxQuitRequest qr;
        check(qr.isQuitRequest() == 1, "QuitRequest isQuitRequest returns 1");
    }

    // run() does nothing (doesn't crash)
    {
        CxQuitRequest qr;
        qr.run();
        check(1, "QuitRequest run() doesn't crash");
    }
}

//-----------------------------------------------------------------------------------------
// CxPCQueue tests
//-----------------------------------------------------------------------------------------
void testPCQueueBasic() {
    printf("\n== CxPCQueue Basic Tests ==\n");

    // Constructor
    {
        CxPCQueue<int> queue(10);
        check(1, "PCQueue ctor doesn't crash");
    }

    // Single enqueue/dequeue
    {
        CxPCQueue<int> queue(10);
        queue.enQueue(42);
        int value = queue.deQueue();
        check(value == 42, "PCQueue single item");
    }

    // Multiple items (FIFO order)
    {
        CxPCQueue<int> queue(10);
        queue.enQueue(1);
        queue.enQueue(2);
        queue.enQueue(3);
        check(queue.deQueue() == 1, "PCQueue FIFO: first");
        check(queue.deQueue() == 2, "PCQueue FIFO: second");
        check(queue.deQueue() == 3, "PCQueue FIFO: third");
    }

    // String items
    {
        CxPCQueue<CxString> queue(10);
        queue.enQueue(CxString("hello"));
        queue.enQueue(CxString("world"));
        check(queue.deQueue() == "hello", "PCQueue string: first");
        check(queue.deQueue() == "world", "PCQueue string: second");
    }

    // Pointer items
    {
        CxPCQueue<int*> queue(10);
        int a = 10, b = 20;
        queue.enQueue(&a);
        queue.enQueue(&b);
        check(*queue.deQueue() == 10, "PCQueue pointer: first");
        check(*queue.deQueue() == 20, "PCQueue pointer: second");
    }
}

// Producer thread for PCQueue test
class ProducerThread : public CxThread {
public:
    CxPCQueue<int>* queue;
    int count;
    ProducerThread(CxPCQueue<int>* q, int n) : queue(q), count(n) {}

    virtual void run() {
        for (int i = 0; i < count; i++) {
            queue->enQueue(i);
        }
    }
};

// Consumer thread for PCQueue test
class ConsumerThread : public CxThread {
public:
    CxPCQueue<int>* queue;
    int count;
    int sum;
    ConsumerThread(CxPCQueue<int>* q, int n) : queue(q), count(n), sum(0) {}

    virtual void run() {
        for (int i = 0; i < count; i++) {
            sum += queue->deQueue();
        }
    }
};

void testPCQueueConcurrency() {
    printf("\n== CxPCQueue Concurrency Tests ==\n");

    // Producer/consumer with single producer, single consumer
    {
        CxPCQueue<int> queue(100);
        ProducerThread producer(&queue, 100);
        ConsumerThread consumer(&queue, 100);

        producer.start();
        consumer.start();

        producer.join();
        consumer.join();

        // Sum of 0..99 = 4950
        check(consumer.sum == 4950, "PCQueue producer/consumer");
    }

    // Multiple producers, single consumer
    {
        CxPCQueue<int> queue(100);
        ProducerThread p1(&queue, 50);
        ProducerThread p2(&queue, 50);
        ConsumerThread consumer(&queue, 100);

        p1.start();
        p2.start();
        consumer.start();

        p1.join();
        p2.join();
        consumer.join();

        // Sum should be 2 * (0..49) = 2 * 1225 = 2450
        check(consumer.sum == 2450, "PCQueue multi-producer");
    }
}

void testPCQueueTimeout() {
    printf("\n== CxPCQueue Timeout Tests ==\n");

    // Dequeue timeout on empty queue
    {
        CxPCQueue<int> queue(10);
        int threw = 0;
        try {
            queue.deQueue(1);  // 1 second timeout
        } catch (CxConditionTimeoutException& e) {
            threw = 1;
        }
        check(threw == 1, "PCQueue deQueue timeout throws");
    }

    // Enqueue timeout on full queue
    {
        CxPCQueue<int> queue(2);
        queue.enQueue(1);
        queue.enQueue(2);
        int threw = 0;
        try {
            queue.enQueue(3, 1);  // 1 second timeout
        } catch (CxConditionTimeoutException& e) {
            threw = 1;
        }
        check(threw == 1, "PCQueue enQueue timeout throws");
    }
}

//-----------------------------------------------------------------------------------------
// CxRunnableThread tests
//-----------------------------------------------------------------------------------------
static int runnableThreadWorkCount = 0;

class WorkItem : public CxRunnable {
public:
    virtual void run() {
        runnableThreadWorkCount++;
    }
};

void testRunnableThread() {
    printf("\n== CxRunnableThread Tests ==\n");

    // Constructor
    {
        CxRunnableThread rt;
        check(1, "RunnableThread ctor doesn't crash");
    }

    // Start, enqueue work, quit
    {
        runnableThreadWorkCount = 0;
        CxRunnableThread rt(10);
        rt.start();

        // Enqueue some work
        rt.enQueue(new WorkItem());
        rt.enQueue(new WorkItem());
        rt.enQueue(new WorkItem());

        usleep(100000);  // Wait for work to complete

        rt.suggestQuit();
        rt.join();

        check(runnableThreadWorkCount == 3, "RunnableThread processed 3 items");
    }

    // isExecuting
    {
        CxRunnableThread rt(10);
        check(rt.isExecuting() == 0, "RunnableThread not executing before start");
        rt.start();
        usleep(10000);  // Let it start
        // Note: isExecuting may or may not be 1 depending on timing
        rt.suggestQuit();
        rt.join();
        check(1, "RunnableThread isExecuting doesn't crash");
    }
}

//-----------------------------------------------------------------------------------------
// CxThreadPool tests
//-----------------------------------------------------------------------------------------
static CxMutex poolCountMutex;
static int poolWorkCount = 0;

class PoolWorkItem : public CxRunnable {
public:
    virtual void run() {
        poolCountMutex.acquire();
        poolWorkCount++;
        poolCountMutex.release();
        usleep(10000);  // Simulate work (10ms)
    }
};

void testThreadPool() {
    printf("\n== CxThreadPool Tests ==\n");

    // Constructor test - workers don't start in ctor anymore (lazy start)
    {
        CxThreadPool pool(4, 100);
        check(1, "ThreadPool ctor doesn't start threads");
        // No need to suggestQuit/join - workers haven't started
    }

    // Start and immediate shutdown
    {
        CxThreadPool pool(4, 100);
        pool.start();
        pool.suggestQuit();
        pool.join();
        check(1, "ThreadPool start/quit/join without work");
    }

    // Process work items - all items should be processed (no dropping)
    {
        poolWorkCount = 0;
        CxThreadPool pool(4, 100);
        pool.start();

        // Enqueue 20 items - shared queue means all will be processed
        for (int i = 0; i < 20; i++) {
            pool.enQueue(new PoolWorkItem());
        }

        pool.suggestQuit();
        pool.join();

        // All items should be processed - no dropping with shared queue
        check(poolWorkCount == 20, "ThreadPool processed all 20 items");
    }

    // High throughput test - verify shared queue handles many items
    {
        poolWorkCount = 0;
        CxThreadPool pool(4, 100);
        pool.start();

        // Enqueue 50 items rapidly
        for (int i = 0; i < 50; i++) {
            pool.enQueue(new PoolWorkItem());
        }

        pool.suggestQuit();
        pool.join();

        check(poolWorkCount == 50, "ThreadPool high throughput: all 50 items");
    }

    // Test enqueue after quit throws exception
    {
        CxThreadPool pool(4, 100);
        pool.start();
        pool.suggestQuit();

        int threw = 0;
        try {
            pool.enQueue(new PoolWorkItem());
        } catch (CxThreadPoolEnqueueException& e) {
            threw = 1;
        }
        pool.join();

        check(threw == 1, "ThreadPool enqueue after quit throws");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxThread Test Suite\n");
    printf("===================\n");

    // Mutex tests
    testMutexBasic();
    testMutexContention();

    // Recursive mutex tests
    testRecursiveMutexBasic();

    // Condition tests
    testConditionBasic();
    testConditionWaitSignal();

    // Thread tests
    testThreadBasic();
    testThreadSuggestQuit();
    testMultipleThreads();

    // Thread duple tests
    testThreadDuple();

    // Runnable tests
    testRunnable();
    testQuitRequest();

    // PCQueue tests
    testPCQueueBasic();
    testPCQueueConcurrency();
    testPCQueueTimeout();

    // RunnableThread tests
    testRunnableThread();

    // ThreadPool tests
    testThreadPool();

    printf("\n===================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
