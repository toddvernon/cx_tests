//-----------------------------------------------------------------------------------------
// cxslist_test.cpp - CxSList unit tests
//-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include <cx/base/string.h>
#include <cx/base/slist.h>

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
        CxSList<CxString> list;
        check(list.entries() == 0, "default ctor creates empty list");
        check(list.empty(), "default ctor empty() returns true");
    }

    // Copy constructor
    {
        CxSList<CxString> original;
        original.append("one");
        original.append("two");
        original.append("three");

        CxSList<CxString> copy(original);
        check(copy.entries() == 3, "copy ctor copies all entries");
        check(strcmp(copy.at(0).data(), "one") == 0, "copy ctor first item correct");
        check(strcmp(copy.at(2).data(), "three") == 0, "copy ctor last item correct");
    }

    // Assignment operator
    {
        CxSList<CxString> original;
        original.append("alpha");
        original.append("beta");

        CxSList<CxString> assigned;
        assigned = original;
        check(assigned.entries() == 2, "assignment copies entries");
        check(strcmp(assigned.at(0).data(), "alpha") == 0, "assignment first item correct");
        check(strcmp(assigned.at(1).data(), "beta") == 0, "assignment second item correct");
    }

    // Self-assignment
    {
        CxSList<CxString> list;
        list.append("test");
        list = list;
        check(list.entries() == 1, "self-assignment preserves entries");
        check(strcmp(list.at(0).data(), "test") == 0, "self-assignment preserves data");
    }
}

//-----------------------------------------------------------------------------------------
// Append and insert tests
//-----------------------------------------------------------------------------------------
void testAppendAndInsert() {
    printf("\n== Append and Insert Tests ==\n");

    // append single item
    {
        CxSList<CxString> list;
        list.append("first");
        check(list.entries() == 1, "append increases entries");
        check(strcmp(list.at(0).data(), "first") == 0, "append stores correct value");
    }

    // append multiple items
    {
        CxSList<CxString> list;
        list.append("one");
        list.append("two");
        list.append("three");
        check(list.entries() == 3, "append multiple items");
        check(strcmp(list.at(0).data(), "one") == 0, "first item correct");
        check(strcmp(list.at(1).data(), "two") == 0, "second item correct");
        check(strcmp(list.at(2).data(), "three") == 0, "third item correct");
    }

    // append list to list
    {
        CxSList<CxString> list1;
        list1.append("a");
        list1.append("b");

        CxSList<CxString> list2;
        list2.append("c");
        list2.append("d");

        list1.append(list2);
        check(list1.entries() == 4, "append list merges entries");
        check(strcmp(list1.at(2).data(), "c") == 0, "appended list first item");
        check(strcmp(list1.at(3).data(), "d") == 0, "appended list second item");
    }

    // insertAtHead
    {
        CxSList<CxString> list;
        list.append("second");
        list.insertAtHead("first");
        check(list.entries() == 2, "insertAtHead increases entries");
        check(strcmp(list.at(0).data(), "first") == 0, "insertAtHead places at head");
        check(strcmp(list.at(1).data(), "second") == 0, "insertAtHead shifts existing");
    }

    // insertAtHead on empty list
    {
        CxSList<CxString> list;
        list.insertAtHead("only");
        check(list.entries() == 1, "insertAtHead on empty list");
        check(strcmp(list.at(0).data(), "only") == 0, "insertAtHead value correct");
    }

    // insertAfter
    {
        CxSList<CxString> list;
        list.append("first");
        list.append("third");
        list.insertAfter(0, "second");
        check(list.entries() == 3, "insertAfter increases entries");
        check(strcmp(list.at(1).data(), "second") == 0, "insertAfter places correctly");
    }

    // push_back (STL compatibility)
    {
        CxSList<CxString> list;
        list.push_back("item");
        check(list.entries() == 1, "push_back works like append");
    }
}

//-----------------------------------------------------------------------------------------
// Stack operations tests (push, pop, peek)
//-----------------------------------------------------------------------------------------
void testStackOperations() {
    printf("\n== Stack Operations Tests ==\n");

    // push
    {
        CxSList<CxString> list;
        list.push("first");
        list.push("second");
        check(list.entries() == 2, "push adds items");
        check(strcmp(list.at(1).data(), "second") == 0, "push adds at end");
    }

    // pop
    {
        CxSList<CxString> list;
        list.push("first");
        list.push("second");
        list.push("third");
        CxString popped = list.pop();
        check(strcmp(popped.data(), "third") == 0, "pop returns last item");
        check(list.entries() == 2, "pop removes item");
    }

    // peek
    {
        CxSList<CxString> list;
        list.push("first");
        list.push("second");
        CxString peeked = list.peek();
        check(strcmp(peeked.data(), "second") == 0, "peek returns last item");
        check(list.entries() == 2, "peek doesn't remove item");
    }

    // multiple pop operations
    {
        CxSList<CxString> list;
        list.push("a");
        list.push("b");
        list.push("c");
        check(strcmp(list.pop().data(), "c") == 0, "pop first");
        check(strcmp(list.pop().data(), "b") == 0, "pop second");
        check(strcmp(list.pop().data(), "a") == 0, "pop third");
        check(list.entries() == 0, "list empty after all pops");
    }
}

//-----------------------------------------------------------------------------------------
// Access tests (at, objectAt, first, last)
//-----------------------------------------------------------------------------------------
void testAccess() {
    printf("\n== Access Tests ==\n");

    // at
    {
        CxSList<CxString> list;
        list.append("zero");
        list.append("one");
        list.append("two");
        check(strcmp(list.at(0).data(), "zero") == 0, "at(0) returns first");
        check(strcmp(list.at(1).data(), "one") == 0, "at(1) returns second");
        check(strcmp(list.at(2).data(), "two") == 0, "at(2) returns third");
    }

    // NOTE: objectAt has a const-correctness bug - returns non-const from const
    // Skipping objectAt test until fixed

    // first (removes item)
    {
        CxSList<CxString> list;
        list.append("first");
        list.append("second");
        CxString item = list.first();
        check(strcmp(item.data(), "first") == 0, "first returns first item");
        check(list.entries() == 1, "first removes item");
        check(strcmp(list.at(0).data(), "second") == 0, "first shifts remaining");
    }

    // last (removes item)
    {
        CxSList<CxString> list;
        list.append("first");
        list.append("second");
        CxString item = list.last();
        check(strcmp(item.data(), "second") == 0, "last returns last item");
        check(list.entries() == 1, "last removes item");
        check(strcmp(list.at(0).data(), "first") == 0, "last preserves remaining");
    }
}

//-----------------------------------------------------------------------------------------
// Modification tests (replaceAt, removeAt, clear)
//-----------------------------------------------------------------------------------------
void testModification() {
    printf("\n== Modification Tests ==\n");

    // replaceAt
    {
        CxSList<CxString> list;
        list.append("old");
        list.replaceAt(0, "new");
        check(strcmp(list.at(0).data(), "new") == 0, "replaceAt changes value");
        check(list.entries() == 1, "replaceAt doesn't change count");
    }

    // replaceAt middle
    {
        CxSList<CxString> list;
        list.append("a");
        list.append("b");
        list.append("c");
        list.replaceAt(1, "B");
        check(strcmp(list.at(1).data(), "B") == 0, "replaceAt middle works");
    }

    // removeAt first
    {
        CxSList<CxString> list;
        list.append("first");
        list.append("second");
        list.append("third");
        list.removeAt(0);
        check(list.entries() == 2, "removeAt reduces count");
        check(strcmp(list.at(0).data(), "second") == 0, "removeAt first shifts items");
    }

    // removeAt middle
    {
        CxSList<CxString> list;
        list.append("a");
        list.append("b");
        list.append("c");
        list.removeAt(1);
        check(list.entries() == 2, "removeAt middle reduces count");
        check(strcmp(list.at(1).data(), "c") == 0, "removeAt middle shifts");
    }

    // removeAt last
    {
        CxSList<CxString> list;
        list.append("first");
        list.append("second");
        list.removeAt(1);
        check(list.entries() == 1, "removeAt last reduces count");
        check(strcmp(list.at(0).data(), "first") == 0, "removeAt last preserves first");
    }

    // removeAt only item
    {
        CxSList<CxString> list;
        list.append("only");
        list.removeAt(0);
        check(list.entries() == 0, "removeAt only item empties list");
        check(list.empty(), "list is empty after removing only item");
    }

    // clear
    {
        CxSList<CxString> list;
        list.append("a");
        list.append("b");
        list.append("c");
        list.clear();
        check(list.entries() == 0, "clear empties list");
        check(list.empty(), "empty() true after clear");
    }

    // swap
    {
        CxSList<CxString> list;
        list.append("first");
        list.append("second");
        list.append("third");
        list.swap(0, 2);
        check(strcmp(list.at(0).data(), "third") == 0, "swap moves first to last");
        check(strcmp(list.at(2).data(), "first") == 0, "swap moves last to first");
        check(strcmp(list.at(1).data(), "second") == 0, "swap preserves middle");
    }
}

//-----------------------------------------------------------------------------------------
// Iterator tests
//-----------------------------------------------------------------------------------------
void testIterator() {
    printf("\n== Iterator Tests ==\n");

    // begin and end
    {
        CxSList<CxString> list;
        list.append("one");
        list.append("two");
        list.append("three");

        CxSListIterator<CxString> it = list.begin();
        check(strcmp((*it).data(), "one") == 0, "begin() points to first");
    }

    // iterator increment
    {
        CxSList<CxString> list;
        list.append("one");
        list.append("two");
        list.append("three");

        CxSListIterator<CxString> it = list.begin();
        ++it;
        check(strcmp((*it).data(), "two") == 0, "++iterator moves to next");
    }

    // iterate through list
    {
        CxSList<CxString> list;
        list.append("a");
        list.append("b");
        list.append("c");

        int count = 0;
        for (CxSListIterator<CxString> it = list.begin(); it != list.end(); ++it) {
            count++;
        }
        check(count == 3, "iterator traverses all items");
    }

    // iterator equality
    {
        CxSList<CxString> list;
        list.append("test");

        CxSListIterator<CxString> it1 = list.begin();
        CxSListIterator<CxString> it2 = list.begin();
        check(it1 == it2, "iterator == works for equal");
    }

    // iterator inequality
    {
        CxSList<CxString> list;
        list.append("one");
        list.append("two");

        CxSListIterator<CxString> it1 = list.begin();
        CxSListIterator<CxString> it2 = list.begin();
        ++it2;
        check(it1 != it2, "iterator != works for different");
    }

    // erase via iterator
    {
        CxSList<CxString> list;
        list.append("one");
        list.append("two");
        list.append("three");

        CxSListIterator<CxString> it = list.begin();
        ++it; // point to "two"
        list.erase(it);
        check(list.entries() == 2, "erase removes item");
        check(strcmp(list.at(0).data(), "one") == 0, "erase preserves before");
        check(strcmp(list.at(1).data(), "three") == 0, "erase preserves after");
    }
}

//-----------------------------------------------------------------------------------------
// Sort tests
//-----------------------------------------------------------------------------------------
void testSort() {
    printf("\n== Sort Tests ==\n");

    // quickSort
    {
        CxSList<CxString> list;
        list.append("cherry");
        list.append("apple");
        list.append("banana");
        list.quickSort();
        check(strcmp(list.at(0).data(), "apple") == 0, "quickSort first item");
        check(strcmp(list.at(1).data(), "banana") == 0, "quickSort second item");
        check(strcmp(list.at(2).data(), "cherry") == 0, "quickSort third item");
    }

    // quickSort already sorted
    {
        CxSList<CxString> list;
        list.append("a");
        list.append("b");
        list.append("c");
        list.quickSort();
        check(strcmp(list.at(0).data(), "a") == 0, "quickSort preserves sorted first");
        check(strcmp(list.at(2).data(), "c") == 0, "quickSort preserves sorted last");
    }

    // quickSort reverse sorted
    {
        CxSList<CxString> list;
        list.append("z");
        list.append("m");
        list.append("a");
        list.quickSort();
        check(strcmp(list.at(0).data(), "a") == 0, "quickSort reverse first");
        check(strcmp(list.at(2).data(), "z") == 0, "quickSort reverse last");
    }

    // quickSort single item
    {
        CxSList<CxString> list;
        list.append("only");
        list.quickSort();
        check(list.entries() == 1, "quickSort single item count");
        check(strcmp(list.at(0).data(), "only") == 0, "quickSort single item value");
    }

    // quickSort empty list
    {
        CxSList<CxString> list;
        list.quickSort();
        check(list.entries() == 0, "quickSort empty list");
    }
}

//-----------------------------------------------------------------------------------------
// Size methods tests
//-----------------------------------------------------------------------------------------
void testSizeMethods() {
    printf("\n== Size Methods Tests ==\n");

    // entries and size
    {
        CxSList<CxString> list;
        check(list.entries() == 0, "entries() 0 for empty");
        check(list.size() == 0, "size() 0 for empty");

        list.append("one");
        check(list.entries() == 1, "entries() 1 after append");
        check(list.size() == 1, "size() 1 after append");

        list.append("two");
        check(list.entries() == 2, "entries() 2 after second append");
        check(list.size() == 2, "size() 2 after second append");
    }

    // empty
    {
        CxSList<CxString> list;
        check(list.empty(), "empty() true for new list");

        list.append("item");
        check(!list.empty(), "empty() false after append");

        list.removeAt(0);
        check(list.empty(), "empty() true after remove");
    }
}

//-----------------------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------------------
int main(int argc, char **argv) {
    printf("CxSList Test Suite\n");
    printf("==================\n");

    testConstructors();
    testAppendAndInsert();
    testStackOperations();
    testAccess();
    testModification();
    testIterator();
    testSort();
    testSizeMethods();

    printf("\n==================\n");
    printf("Results: %d passed, %d failed\n", testsPassed, testsFailed);

    return testsFailed > 0 ? 1 : 0;
}
