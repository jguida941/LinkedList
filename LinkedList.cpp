//============================================================================
// Name        : LinkedList.cpp
// Author      : Justin Guida
// Version     : 1.0
//============================================================================

#include <algorithm>
#include <iostream>
#include <time.h>
#include <iomanip> // for fixed and setprecision
#include <chrono> // for high_resolution_clock
#include <string>
#include <limits>
#include <cstdlib>    // getenv
#ifdef __unix__
#include <unistd.h>   // isatty, STDOUT_FILENO
#include <sys/ioctl.h> // winsize, ioctl
#endif
using namespace std::chrono;


#include "CSVparser.hpp"

using namespace std;

// ANSI color codes and borders (theme-adjustable via COLOR_THEME)
static string RESET   = "\033[0m";
static string RED     = "\033[31m";
static string GREEN   = "\033[38;5;22m";   // darker green (bold later in theme)
static string YELLOW  = "\033[33m";
static string BLUE    = "\033[38;5;18m";   // very dark navy
static string MAGENTA = "\033[35m";
static string CYAN    = "\033[38;5;30m";   // deep teal (instead of light cyan)
static string WHITE   = "\033[37m";
static string BORDER  = ""; // initialized in setColorTheme()

static void setColorTheme() {
    string theme = "light";
    if (const char* t = std::getenv("COLOR_THEME")) theme = t;

    if (theme == "mono" || theme == "none") {
        RESET = RED = GREEN = YELLOW = BLUE = MAGENTA = CYAN = WHITE = "";
    } else if (theme == "dark") {
        // Dark background: slightly brighter labels
        GREEN   = "\033[1;32m";
        BLUE    = "\033[1;34m";
        CYAN    = "\033[1;36m";
        YELLOW  = "\033[1;33m";
        RED     = "\033[1;31m";
        MAGENTA = "\033[1;35m";
        RESET   = "\033[0m";
    } else {
        // Light background (default): bold dark 256-color tones
        GREEN   = "\033[1;38;5;22m";
        BLUE    = "\033[1;38;5;18m";
        CYAN    = "\033[1;38;5;30m";
        YELLOW  = "\033[1;38;5;94m";  // dark goldenrod for better contrast
        RED     = "\033[1;31m";
        MAGENTA = "\033[1;35m";
        RESET   = "\033[0m";
    }
    BORDER = CYAN + "========================================" + RESET;
}

//============================================================================
// Global definitions visible to all methods and classes
//============================================================================

// Forward declarations
double strToDouble(string str, char ch);


// Define a structure to hold bid information with unique identifier, title, fund, and amount
struct Bid {
    string bidId;
    string title;
    string fund;
    double amount;

    Bid() {
        amount = 0.0;
    }
};

// Helpers for printing, pausing, and cleaning input
void displayBid(const Bid& bid);        // full-width row output
void displayBidCompact(const Bid& bid); // compact one-line output
void waitForEnter();                    // pause until Enter pressed

// Detect terminal width (columns) with sensible fallbacks.
static int getTerminalWidth() {
    int cols = 0;
    if (const char* c = std::getenv("COLUMNS")) {
        cols = std::atoi(c);
    }
#ifdef __unix__
    if (cols <= 0 && isatty(STDOUT_FILENO)) {
        struct winsize ws{};
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
            cols = ws.ws_col;
        }
    }
#endif
    if (cols <= 0) cols = 100; // generic default when unknown
    if (cols < 50) cols = 50;  // enforce a minimal reasonable width
    return cols;
}




//============================================================================
// Linked-List class definition
//============================================================================

/**
 * Define a class containing data members and methods to
 * implement a linked-list.
 */
class LinkedList {
private:
    // Internal structure for list entries, housekeeping variables
    struct Node {
        Bid bid;
        Node *next;

        // Default; no successor
        Node() : next(nullptr) {}

        // Use initializer list to construct bid + next directly so there's no extra assignment
        Node(const Bid& aBid) : bid(aBid), next(nullptr) {}
    };


    Node *head;
    Node *tail;
    int   size;

// Declared as const to avoid modifying the list
public:
    LinkedList();
    virtual ~LinkedList();
    void Append(const Bid& bid);
    void Prepend(const Bid& bid);
    void PrintList() const; // const only outputs list no modifications
    void Remove(const string& bidId);
    Bid Search(const string& bidId) const; // const search does not change the list
    int Size() const; //const just returns the current size
};

/**
 * Default constructor
 **/
// set head, tail, size to nullptr, 0 using initializer list
LinkedList::LinkedList() : head(nullptr), tail(nullptr), size(0) {}

/**
 * Destructor:
 * Start from the head of the list.
 * For each node:
 * Save a pointer to the next node.
 * Delete the current node to free memory.
 * Advance to the saved next node.
 * Continue until all nodes are deleted.
 */

LinkedList::~LinkedList() {
    Node* current = head;
    while (current != nullptr) {
        Node* nextNode = current->next; // store next before delete
        delete current;                 // free current node
        current = nextNode;             // move forward
    }
}
/**
 * Append:
 * Append a new bid to the end of the list
 * Allocate a new node containing the bid.
 * If the list is empty, set both head and tail to this node.
 * Otherwise, link the new node to the current tail and update tail.
 * Increment the size counter.
 */
void LinkedList::Append(const Bid& bid) {
    Node *newNode = new Node(bid);
    if (head == nullptr) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
    size++;
}

/**
  * Prepend:
  * Prepend a new bid to the start of the list.
  * Allocate a new node containing the bid.
  * If the list is empty, set both head and tail to this node.
  * Otherwise, link the new node so its next points to the current head,
  * Update head to the new node
  * Increment the size counter.
**/
void LinkedList::Prepend(const Bid& bid) {
    Node *newNode = new Node(bid);
    if (head == nullptr) {
        head = tail = newNode;
    } else {
        newNode->next = head;
        head = newNode;
    }
    size++;
}

/**
 * Simple output of all bids in the list
 * PrintList walks through the linked list from head to tail.
 * For each node, it calls displayBid to show the bid data,
 * then moves on to the next node until the list ends.
**/
void LinkedList::PrintList() const {
    Node *current = head;
    while (current != nullptr) {
        displayBid(current->bid);
        current = current->next;
    }
}

/**
 * Remove a specified bid
 * @param bidId The bid id to remove from the list
 * If list is empty, do nothing.
 * If head matches, hold onto head, move head to next, delete old head,
 * Then shrink size, and if head is now null then tail is null too.
 * Otherwise, walk list until a match, relink previous node to skip the match,
 * delete the match node, and shrink size.
**/
void LinkedList::Remove(const string& bidId) {
    if (head == nullptr) {
        return;
    }
    if (head->bid.bidId == bidId) {
        Node *temp = head;
        head = head->next;
        delete temp; // free node memory
        size--;

        if (head == nullptr) {
            tail = nullptr;
        }
        return;
    }

    Node *current = head;
    while (current ->next != nullptr) {
        if (current->next->bid.bidId == bidId) {
            Node* temp = current->next;
            current->next = temp->next; //bypass the node being removed

            if (temp == tail) {
                tail = current;
            }
            delete temp; // free node memory
            size--;
            return;
        }
        current = current->next;
    }
}

/**
 * Search for the specified bidId
 * @param bidId The bid id to search for
 * if head is not null and matches bidId, return head bid
 * otherwise start at node after head and walk list
 * if a node’s bidId matches, return that bid
 * if end of list reached with no match, return empty Bid
**/
Bid LinkedList::Search(const string& bidId) const {
    if (head != nullptr && head->bid.bidId == bidId) {
        return head->bid;
    }
    Node* current = head ? head->next : nullptr;
    while (current != nullptr) {
        if (current->bid.bidId == bidId) {
            return current->bid;
        }
        current = current->next;
    }
    return Bid{}; // returns empty bid if no match found

}

/**
 * Returns the current size (number of elements) in the list
 **/
int LinkedList::Size() const {
    return size;
}

//============================================================================
// Static methods used for testing
//============================================================================


/**
 * @param bid struct containing the bid info
 * Display the bid information with colors and aligned columns.
 *
 * Bid display helpers:
*
 * displayBid:
 * Full table-style output used for printing all bids (case 3, searches).
 * Uses a wide Title column (90 chars) so long names align with Fund/Amount.
 *
 * displayBidCompact:
 * Lightweight confirmation print used only after adding a new bid (case 1).
 * Truncates Title to ~40 chars and adds "..." if too long.
 * Prevents formatting issues where the full table would shove output far right.
 *
 * waitForEnter:
 * Pauses program until user presses Enter.
 * Ensures the user has time to read confirmation messages before the menu returns.
 **/

void displayBid(const Bid& bid) {
    // Fixed widths for non-title fields
    const int idWidth    = 8;   // bidId field width
    int       fundWidth  = 20;  // preferred fund width (shrinkable)
    const int fundMin    = 12;  // minimal fund width when space is tight
    const int amtWidth   = 10;  // amount numeric width

    // Constant label/separator lengths (visible chars only)
    const int len_id_lbl    = 4;  // "ID: "
    const int len_title_lbl = 7;  // "Title: "
    const int len_fund_lbl  = 6;  // "Fund: "
    const int len_amt_lbl   = 9;  // "Amount: $"
    const int sep           = 3;  // " | "
    const int margin        = 3;  // safety margin to avoid last-column wrap

    // Determine terminal width
    const int term = getTerminalWidth();

    // Compute title width with preferred fund width
    auto reservedWithFund = [&](int fw) {
        return len_id_lbl + idWidth + sep + len_title_lbl + sep +
               len_fund_lbl + fw + sep + len_amt_lbl + amtWidth + margin;
    };

    int titleWidth = term - reservedWithFund(fundWidth);

    // If not enough space, try shrinking Fund width down to fundMin
    if (titleWidth < 5 && fundWidth > fundMin) {
        fundWidth = max(fundMin, fundWidth - (5 - titleWidth));
        titleWidth = term - reservedWithFund(fundWidth);
    }

    // Fallback: very narrow terminals -> 2-line compact layout
    const int minSingleLine = 90; // threshold where one line is comfortable
    if (term < minSingleLine || titleWidth < 5) {
        // Line 1: ID | Title
        const int reserved1 = len_id_lbl + idWidth + sep + len_title_lbl + margin;
        int titleWidth1 = max(5, term - reserved1);
        string title1 = bid.title;
        if ((int)title1.size() > titleWidth1) {
            title1 = (titleWidth1 >= 3) ? title1.substr(0, titleWidth1 - 3) + "..."
                                        : title1.substr(0, titleWidth1);
        }

        cout << CYAN << "ID: " << RESET << left << setw(idWidth) << bid.bidId
             << " | " << GREEN << "Title: " << RESET
             << left << setw(titleWidth1) << title1 << '\n';

        // Line 2: Fund | Amount
        const int reserved2 = len_fund_lbl + sep + len_amt_lbl + amtWidth + margin;
        int fundWidth2 = max(fundMin, term - reserved2);

        cout << YELLOW << "Fund: " << RESET << left << setw(fundWidth2) << bid.fund
             << " | " << MAGENTA << "Amount: $" << RESET
             << right << fixed << setprecision(2) << setw(amtWidth) << bid.amount
             << '\n';
        return;
    }

    // Prepare possibly truncated title so the line doesn't wrap
    string title = bid.title;
    if ((int)title.size() > titleWidth) {
        if (titleWidth >= 3) {
            title = title.substr(0, titleWidth - 3) + "...";
        } else {
            title = title.substr(0, titleWidth);
        }
    }

    cout << CYAN << "ID: " << RESET << left << setw(idWidth) << bid.bidId
         << " | " << GREEN << "Title: " << RESET
         << left << setw(titleWidth) << title
         << " | " << YELLOW << "Fund: " << RESET << left << setw(fundWidth) << bid.fund
         << " | " << MAGENTA << "Amount: $" << RESET
         << right << fixed << setprecision(2) << setw(amtWidth) << bid.amount
         << endl;
}

void displayBidCompact(const Bid& bid) {
    const int titlePreview = 40;          // short preview so the line stays compact
    string t = bid.title;
    if ((int)t.size() > titlePreview) {
        t = t.substr(0, titlePreview - 3) + "...";
    }

    cout << CYAN << "ID: " << RESET << bid.bidId
         << " | " << GREEN << "Title: " << RESET << t
         << " | " << YELLOW << "Fund: " << RESET << bid.fund
         << " | " << MAGENTA << "Amount: $" << RESET
         << fixed << setprecision(2) << bid.amount << '\n';
}

// Pause helper so the user sees a prompt before the menu returns.
void waitForEnter() {
    cout << CYAN << "Press Enter to continue..." << RESET << flush;
    cin.get();  // buffer is already clean; just wait for one Enter
}


/**
 * Prompt user for bid information
 *
 * @return Bid struct containing the bid info
 **/
Bid getBid() {
    Bid bid;

    cout << "Enter Id: ";
    getline(cin, bid.bidId); // no pre-ignore needed

    cout << "Enter title: ";
    getline(cin, bid.title);

    cout << "Enter fund: ";
    cin >> bid.fund;

    cout << "Enter amount: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // deal with leftover newline from >> fund
    string strAmount;
    getline(cin, strAmount);
    bid.amount = strToDouble(strAmount, '$');

    return bid;
}

/**
 * Load a CSV file containing bids into a LinkedList
 *
 * @return a LinkedList containing all the bids read
 **/
void loadBids(string csvPath, LinkedList *list) {
    cout << "Loading CSV file " << csvPath << endl;

    try {
        // Initialize the CSV Parser inside the try so constructor errors are caught
        csv::Parser file(csvPath);

        // loop to read rows of a CSV file
        for (int i = 0; i < file.rowCount(); i++) {
            // initialize a bid using data from current row (i)
            Bid bid;
            bid.bidId = file[i][1];
            bid.title = file[i][0];
            bid.fund = file[i][8];
            bid.amount = strToDouble(file[i][4], '$');

            // add this bid to the end
            list->Append(bid);
        }
    } catch (const csv::Error &e) {
        std::cerr << "Error loading CSV '" << csvPath << "': " << e.what() << std::endl;
    }
}

/**
 * Simple C function to convert a string to a double
 * after stripping out unwanted char
 *
 * credit: http://stackoverflow.com/a/24875936
 *
 * @param ch The character to strip out
 **/
double strToDouble(string str, char ch) {
    str.erase(remove(str.begin(), str.end(), ch), str.end());
    return atof(str.c_str());
}

/**
 * The one and only main() method
 *
 * @param arg[1] path to CSV file to load from (optional)
 * @param arg[2] the bid Id to use when searching the list (optional)
 */
int main(int argc, char *argv[]) {
    setColorTheme();
    // process command line arguments
    string csvPath, bidKey;
    switch (argc) {
        case 2:
            csvPath = argv[1];
            bidKey = "98109";
            break;
        case 3:
            csvPath = argv[1];
            bidKey = argv[2];
            break;
        default:
            csvPath = "eBid_Monthly_Sales.csv";
            bidKey = "98109";
    }

    LinkedList bidList;

    Bid bid;

    int choice = 0;
while (choice != 9) {
    cout << BORDER << '\n';
    cout << YELLOW << "                MENU" << RESET << '\n';
    cout << BORDER << '\n';
    cout << GREEN  << "  1. Enter a Bid"         << RESET << '\n';
    cout << GREEN  << "  2. Load Bids"            << RESET << '\n';
    cout << GREEN  << "  3. Display All Bids"     << RESET << '\n';
    cout << GREEN  << "  4. Find Bid"             << RESET << '\n';
    cout << GREEN  << "  5. Remove Bid"           << RESET << '\n';
    cout << RED    << "  9. Exit"                 << RESET << '\n';
    cout << BORDER << '\n';
    cout << CYAN   << "Enter choice: "            << RESET;

    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << RED << "Invalid input. Please enter a number.\n" << RESET;
        choice = 0;
        continue;
    }
    // success path: eat the trailing newline once
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    switch (choice) {
        case 1: {
            Bid b = getBid();
            bidList.Append(b);
            cout << BORDER << '\n';
            cout << GREEN << "Bid added." << RESET << '\n';
            displayBidCompact(b);      // compact confirmation line only
            cout << BORDER << '\n';
            waitForEnter();            // prompt user to press Enter
            break;
        }
        case 2: {
            clock_t ticks = clock(); // clock works for CSV load
            loadBids(csvPath, &bidList);
            ticks = clock() - ticks;
            cout << BORDER << '\n';
            cout << GREEN << bidList.Size() << " bids read\n" << RESET;
            cout << "time: " << (ticks * 1000.0 / CLOCKS_PER_SEC)  << " ms\n";
            cout << "time: " << (ticks * 1.0   / CLOCKS_PER_SEC)   << " s\n";
            cout << BORDER << '\n';
            break;
        }

        // Added error handling:
        // Prevents trying to display bids when none have been loaded yet.
        // If the list is empty, show an error instead of printing nothing.
        case 3: {
            cout << BORDER << '\n';
            if (bidList.Size() == 0) {
                cout << RED << "Error: No bids loaded yet." << RESET << '\n';
                cout << RED << "Please select option 2 first." << RESET << '\n';

            } else {
                bidList.PrintList();
            }
            cout << BORDER << '\n';
            break;
        }
        case 4: {
            auto start = high_resolution_clock::now(); // // use high-res; clock() was 0 ms on my machine
            Bid result = bidList.Search(bidKey);
            auto end   = high_resolution_clock::now();
            auto us    = duration_cast<microseconds>(end - start).count();

            cout << BORDER << '\n';
            if (!result.bidId.empty()) {
                displayBid(result);
            } else {
                cout << RED << "Bid Id " << bidKey << " not found.\n" << RESET;
            }
            cout << fixed << setprecision(6) // set precision to 6 decimal places for time display
                 << "time: " << us << " microseconds\n"
                 << "time: " << (us / 1'000'000.0) << " seconds\n";
            cout << BORDER << '\n';
            break;
        }
        case 5: {
            bidList.Remove(bidKey);
            cout << GREEN << "Removed bid id " << bidKey << " (if present).\n" << RESET;
            break;
        }
        case 9:
            cout << YELLOW << "Good bye.\n" << RESET;
            break;
        default:
            cout << RED << "Invalid choice.\n" << RESET;
    }
}

    return 0;
}

