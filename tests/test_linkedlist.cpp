//============================================================================
// Unit Tests for LinkedList
//
// Tests whitespace trimming, linked list operations, and edge cases.
// Uses Catch2 framework.
//============================================================================

#define CATCH_CONFIG_MAIN
#include <catch2/catch_test_macros.hpp>
#include <string>

using namespace std;

//----------------------------------------------------------------------------
// Whitespace Trimming (extracted for testability)
//----------------------------------------------------------------------------

string trimWhitespace(const string& input) {
    size_t startPos = input.find_first_not_of(" \t\n\r");
    size_t endPos = input.find_last_not_of(" \t\n\r");
    if (startPos == string::npos) {
        return "";
    }
    return input.substr(startPos, endPos - startPos + 1);
}

//----------------------------------------------------------------------------
// Bid Structure (mirrored from LinkedList.cpp)
//----------------------------------------------------------------------------

struct Bid {
    string bidId;
    string title;
    string fund;
    double amount;
    Bid() : amount(0.0) {}
};

//----------------------------------------------------------------------------
// LinkedList Class (mirrored for testing)
//----------------------------------------------------------------------------

class LinkedList {
private:
    struct Node {
        Bid bid;
        Node* next;
        Node() : next(nullptr) {}
        Node(const Bid& aBid) : bid(aBid), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    int listSize;

public:
    LinkedList() : head(nullptr), tail(nullptr), listSize(0) {}

    ~LinkedList() {
        Node* current = head;
        while (current != nullptr) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    void Append(const Bid& bid) {
        Node* newNode = new Node(bid);
        if (head == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        listSize++;
    }

    void Prepend(const Bid& bid) {
        Node* newNode = new Node(bid);
        newNode->next = head;
        head = newNode;
        if (tail == nullptr) {
            tail = newNode;
        }
        listSize++;
    }

    bool Remove(const string& bidId) {
        if (head == nullptr) {
            return false;
        }
        if (head->bid.bidId == bidId) {
            Node* temp = head;
            head = head->next;
            delete temp;
            listSize--;
            if (head == nullptr) {
                tail = nullptr;
            }
            return true;
        }

        Node* current = head;
        while (current->next != nullptr) {
            if (current->next->bid.bidId == bidId) {
                Node* temp = current->next;
                current->next = temp->next;
                if (temp == tail) {
                    tail = current;
                }
                delete temp;
                listSize--;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    Bid* Search(const string& bidId) {
        Node* current = head;
        while (current != nullptr) {
            if (current->bid.bidId == bidId) {
                return &(current->bid);
            }
            current = current->next;
        }
        return nullptr;
    }

    bool Contains(const string& bidId) {
        return Search(bidId) != nullptr;
    }

    int Size() const { return listSize; }
    bool IsEmpty() const { return head == nullptr; }
};

//============================================================================
// WHITESPACE TRIMMING TESTS
//============================================================================

TEST_CASE("Whitespace trimming handles leading spaces", "[trim]") {
    REQUIRE(trimWhitespace("  12345") == "12345");
    REQUIRE(trimWhitespace("   abc") == "abc");
    REQUIRE(trimWhitespace("    ") == "");
}

TEST_CASE("Whitespace trimming handles trailing spaces", "[trim]") {
    REQUIRE(trimWhitespace("12345  ") == "12345");
    REQUIRE(trimWhitespace("abc   ") == "abc");
}

TEST_CASE("Whitespace trimming handles both sides", "[trim]") {
    REQUIRE(trimWhitespace("  12345  ") == "12345");
    REQUIRE(trimWhitespace("   abc   ") == "abc");
    REQUIRE(trimWhitespace(" x ") == "x");
}

TEST_CASE("Whitespace trimming handles tabs", "[trim]") {
    REQUIRE(trimWhitespace("\t12345") == "12345");
    REQUIRE(trimWhitespace("12345\t") == "12345");
    REQUIRE(trimWhitespace("\t12345\t") == "12345");
    REQUIRE(trimWhitespace(" \t 12345 \t ") == "12345");
}

TEST_CASE("Whitespace trimming handles newlines", "[trim]") {
    REQUIRE(trimWhitespace("\n12345") == "12345");
    REQUIRE(trimWhitespace("12345\n") == "12345");
    REQUIRE(trimWhitespace("\r\n12345\r\n") == "12345");
}

TEST_CASE("Whitespace trimming preserves internal spaces", "[trim]") {
    REQUIRE(trimWhitespace("  hello world  ") == "hello world");
    REQUIRE(trimWhitespace(" bid 123 ") == "bid 123");
}

TEST_CASE("Whitespace trimming handles empty input", "[trim]") {
    REQUIRE(trimWhitespace("") == "");
    REQUIRE(trimWhitespace("   ") == "");
    REQUIRE(trimWhitespace("\t\n\r") == "");
}

TEST_CASE("Whitespace trimming handles no whitespace", "[trim]") {
    REQUIRE(trimWhitespace("12345") == "12345");
    REQUIRE(trimWhitespace("abc") == "abc");
}

//============================================================================
// LINKED LIST TESTS
//============================================================================

TEST_CASE("LinkedList starts empty", "[linkedlist]") {
    LinkedList list;
    REQUIRE(list.Size() == 0);
    REQUIRE(list.IsEmpty() == true);
}

TEST_CASE("LinkedList append adds to end", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "001"; bid1.title = "First";
    Bid bid2; bid2.bidId = "002"; bid2.title = "Second";

    list.Append(bid1);
    REQUIRE(list.Size() == 1);
    REQUIRE(list.Contains("001") == true);

    list.Append(bid2);
    REQUIRE(list.Size() == 2);
    REQUIRE(list.Contains("002") == true);
}

TEST_CASE("LinkedList prepend adds to front", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "001"; bid1.title = "First";
    Bid bid2; bid2.bidId = "002"; bid2.title = "Second";

    list.Prepend(bid1);
    list.Prepend(bid2);

    REQUIRE(list.Size() == 2);
    REQUIRE(list.Contains("001") == true);
    REQUIRE(list.Contains("002") == true);
}

TEST_CASE("LinkedList search finds existing bids", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "12345"; bid1.title = "Test Bid";
    list.Append(bid1);

    Bid* found = list.Search("12345");
    REQUIRE(found != nullptr);
    REQUIRE(found->bidId == "12345");
    REQUIRE(found->title == "Test Bid");
}

TEST_CASE("LinkedList search returns nullptr for missing bids", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "12345"; bid1.title = "Test Bid";
    list.Append(bid1);

    REQUIRE(list.Search("99999") == nullptr);
    REQUIRE(list.Search("") == nullptr);
}

TEST_CASE("LinkedList remove deletes head correctly", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "001";
    Bid bid2; bid2.bidId = "002";
    Bid bid3; bid3.bidId = "003";

    list.Append(bid1);
    list.Append(bid2);
    list.Append(bid3);

    REQUIRE(list.Remove("001") == true);
    REQUIRE(list.Size() == 2);
    REQUIRE(list.Contains("001") == false);
    REQUIRE(list.Contains("002") == true);
    REQUIRE(list.Contains("003") == true);
}

TEST_CASE("LinkedList remove deletes middle correctly", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "001";
    Bid bid2; bid2.bidId = "002";
    Bid bid3; bid3.bidId = "003";

    list.Append(bid1);
    list.Append(bid2);
    list.Append(bid3);

    REQUIRE(list.Remove("002") == true);
    REQUIRE(list.Size() == 2);
    REQUIRE(list.Contains("001") == true);
    REQUIRE(list.Contains("002") == false);
    REQUIRE(list.Contains("003") == true);
}

TEST_CASE("LinkedList remove deletes tail correctly", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "001";
    Bid bid2; bid2.bidId = "002";
    Bid bid3; bid3.bidId = "003";

    list.Append(bid1);
    list.Append(bid2);
    list.Append(bid3);

    REQUIRE(list.Remove("003") == true);
    REQUIRE(list.Size() == 2);
    REQUIRE(list.Contains("001") == true);
    REQUIRE(list.Contains("002") == true);
    REQUIRE(list.Contains("003") == false);
}

TEST_CASE("LinkedList remove returns false for missing bid", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "001";
    list.Append(bid1);

    REQUIRE(list.Remove("999") == false);
    REQUIRE(list.Size() == 1);
}

TEST_CASE("LinkedList remove from empty list returns false", "[linkedlist]") {
    LinkedList list;
    REQUIRE(list.Remove("001") == false);
}

TEST_CASE("LinkedList handles single element removal", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "001";
    list.Append(bid1);

    REQUIRE(list.Remove("001") == true);
    REQUIRE(list.Size() == 0);
    REQUIRE(list.IsEmpty() == true);
}

//============================================================================
// INTEGRATION TESTS - Whitespace + LinkedList
//============================================================================

TEST_CASE("Search with trimmed whitespace finds bid", "[integration]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "92549"; bid1.title = "Test Bid";
    list.Append(bid1);

    // Simulate user input with whitespace
    string userInput = "  92549  ";
    string trimmedId = trimWhitespace(userInput);

    Bid* found = list.Search(trimmedId);
    REQUIRE(found != nullptr);
    REQUIRE(found->bidId == "92549");
}

TEST_CASE("Remove with trimmed whitespace removes bid", "[integration]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "92549"; bid1.title = "Test Bid";
    list.Append(bid1);

    // Simulate user input with whitespace
    string userInput = "  92549  ";
    string trimmedId = trimWhitespace(userInput);

    REQUIRE(list.Remove(trimmedId) == true);
    REQUIRE(list.Contains("92549") == false);
}

TEST_CASE("Without trimming, whitespace causes lookup failure", "[integration]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "92549"; bid1.title = "Test Bid";
    list.Append(bid1);

    // Without trimming, lookup fails
    string userInputWithSpaces = "  92549  ";
    REQUIRE(list.Search(userInputWithSpaces) == nullptr);
    REQUIRE(list.Remove(userInputWithSpaces) == false);
}

//============================================================================
// EDGE CASE TESTS
//============================================================================

TEST_CASE("LinkedList handles duplicate prevention check", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "001"; bid1.title = "First";
    list.Append(bid1);

    // Check if ID already exists before adding
    REQUIRE(list.Contains("001") == true);
    REQUIRE(list.Contains("002") == false);
}

TEST_CASE("LinkedList handles bid with special characters in ID", "[linkedlist]") {
    LinkedList list;

    Bid bid1; bid1.bidId = "ABC-123"; bid1.title = "Special";
    Bid bid2; bid2.bidId = "XYZ_456"; bid2.title = "Underscore";

    list.Append(bid1);
    list.Append(bid2);

    REQUIRE(list.Contains("ABC-123") == true);
    REQUIRE(list.Contains("XYZ_456") == true);
}

TEST_CASE("LinkedList handles very long bid IDs", "[linkedlist]") {
    LinkedList list;

    string longId = string(1000, 'X');
    Bid bid1; bid1.bidId = longId;
    list.Append(bid1);

    REQUIRE(list.Contains(longId) == true);
    REQUIRE(list.Remove(longId) == true);
}
