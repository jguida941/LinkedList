//============================================================================
// Name        : LinkedList.cpp
// Author      : Justin Guida
// Version     : 1.1.0
//
// Bid management system using a singly linked list.
//
// Why a linked list instead of vector?
// - Educational: demonstrates manual memory management and pointer operations
// - O(1) append/prepend without reallocations
// - Trade-off: O(n) search, but acceptable for ~12k records
//
// Why custom CSV parser?
// - Handles quoted fields with embedded commas (standard in bid data)
// - Strips $ from amounts automatically
//============================================================================

#include <algorithm>
#include <iostream>
#include <fstream>
#include <time.h>
#include <iomanip>
#include <chrono>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <cstdlib>

// Unix-only: for detecting terminal width so output adjusts to fit
#ifdef __unix__
#include <unistd.h>
#include <sys/ioctl.h>
#endif

using namespace std::chrono;
#include "CSVparser.hpp"
using namespace std;

//============================================================================
// Terminal Colors
//
// Why mutable globals? We detect the terminal theme at startup and swap
// color codes accordingly. Using 256-color (38;5;XXX) instead of basic ANSI
// gives us consistent colors across different terminals.
//
// Colors are set to empty strings when NO_COLOR is set or mono mode is
// requested - this way we can concatenate them without conditionals.
//============================================================================

static string RESET   = "\033[0m";
static string RED     = "\033[31m";
static string GREEN   = "\033[32m";
static string YELLOW  = "\033[33m";
static string BLUE    = "\033[34m";
static string MAGENTA = "\033[35m";
static string CYAN    = "\033[36m";
static string WHITE   = "\033[37m";
static string BOLD    = "\033[1m";
static string DIM     = "\033[2m";

// Unicode box-drawing characters. We fall back to ASCII (+, -, |) when colors
// are disabled because terminals that can't do ANSI often can't do Unicode.
static string BOX_TL  = "\u250C";
static string BOX_TR  = "\u2510";
static string BOX_BL  = "\u2514";
static string BOX_BR  = "\u2518";
static string BOX_H   = "\u2500";
static string BOX_V   = "\u2502";
static string BOX_LT  = "\u251C";
static string BOX_RT  = "\u2524";

static bool isDarkMode = false;

/**
 * Tries to detect if the terminal has a dark or light background.
 *
 * Why do we need this? Bright green on white is unreadable. Dark blue on
 * black disappears. There's no standard way to query this, so we check
 * several env vars that terminals sometimes set.
 *
 * Returns true for dark, false for light. Defaults to dark because most
 * developers use dark terminals.
 */
static bool detectDarkMode() {
    // User can override with COLOR_THEME=dark or COLOR_THEME=light
    if (const char* t = std::getenv("COLOR_THEME")) {
        string theme = t;
        if (theme == "dark") return true;
        if (theme == "light") return false;
    }

    // Check COLORFGBG (format: "fg;bg" - bg > 7 usually means dark)
    if (const char* cfg = std::getenv("COLORFGBG")) {
        string s = cfg;
        size_t pos = s.rfind(';');
        if (pos != string::npos) {
            int bg = std::atoi(s.substr(pos + 1).c_str());
            if (bg >= 0 && bg <= 6) return true;   // dark background colors
            if (bg >= 7 && bg <= 15) return false; // light background colors
        }
    }

    // Check common dark mode indicators
    if (const char* term = std::getenv("TERM_PROGRAM")) {
        string tp = term;
        // iTerm2 often defaults to dark
        if (tp == "iTerm.app") {
            if (const char* profile = std::getenv("ITERM_PROFILE")) {
                string p = profile;
                // Common dark profile names
                if (p.find("Dark") != string::npos || p.find("dark") != string::npos) return true;
                if (p.find("Light") != string::npos || p.find("light") != string::npos) return false;
            }
        }
    }

    // macOS: check system appearance
    if (const char* appearance = std::getenv("TERM_PROGRAM")) {
        // If running in Apple Terminal, check for dark mode hints
        #ifdef __APPLE__
        // Could use system call but keep it simple - default to checking env
        #endif
    }

    // Default: assume dark mode (more common for developers)
    return true;
}

static void setColorTheme() {
    // Check for mono/no-color mode
    if (const char* t = std::getenv("COLOR_THEME")) {
        string theme = t;
        if (theme == "mono" || theme == "none") {
            RESET = RED = GREEN = YELLOW = BLUE = MAGENTA = CYAN = WHITE = BOLD = DIM = "";
            BOX_TL = BOX_TR = BOX_BL = BOX_BR = "+";
            BOX_H = "-";
            BOX_V = "|";
            BOX_LT = BOX_RT = "+";
            return;
        }
    }

    // Check for NO_COLOR standard
    if (std::getenv("NO_COLOR")) {
        RESET = RED = GREEN = YELLOW = BLUE = MAGENTA = CYAN = WHITE = BOLD = DIM = "";
        BOX_TL = BOX_TR = BOX_BL = BOX_BR = "+";
        BOX_H = "-";
        BOX_V = "|";
        BOX_LT = BOX_RT = "+";
        return;
    }

    isDarkMode = detectDarkMode();

    if (isDarkMode) {
        // Dark background: bright, vibrant colors
        GREEN   = "\033[38;5;114m";   // soft green
        BLUE    = "\033[38;5;111m";   // soft blue
        CYAN    = "\033[38;5;80m";    // bright cyan
        YELLOW  = "\033[38;5;221m";   // gold
        RED     = "\033[38;5;203m";   // soft red
        MAGENTA = "\033[38;5;177m";   // soft magenta
        WHITE   = "\033[38;5;255m";   // bright white
        BOLD    = "\033[1m";
        DIM     = "\033[2m";
        RESET   = "\033[0m";
    } else {
        // Light background: darker, more saturated colors
        GREEN   = "\033[38;5;28m";    // forest green
        BLUE    = "\033[38;5;25m";    // dark blue
        CYAN    = "\033[38;5;30m";    // teal
        YELLOW  = "\033[38;5;130m";   // dark orange/brown
        RED     = "\033[38;5;160m";   // dark red
        MAGENTA = "\033[38;5;127m";   // dark magenta
        WHITE   = "\033[38;5;235m";   // dark gray (for contrast)
        BOLD    = "\033[1m";
        DIM     = "\033[2m";
        RESET   = "\033[0m";
    }
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

// Box drawing helpers for nice UI
static void drawBoxTop(int width) {
    cout << CYAN << BOX_TL;
    for (int i = 0; i < width - 2; i++) cout << BOX_H;
    cout << BOX_TR << RESET << '\n';
}

static void drawBoxBottom(int width) {
    cout << CYAN << BOX_BL;
    for (int i = 0; i < width - 2; i++) cout << BOX_H;
    cout << BOX_BR << RESET << '\n';
}

static void drawBoxMiddle(int width) {
    cout << CYAN << BOX_LT;
    for (int i = 0; i < width - 2; i++) cout << BOX_H;
    cout << BOX_RT << RESET << '\n';
}

static void drawBoxLine(const string& text, int width, const string& color = "") {
    // Calculate visible length (without ANSI codes)
    int visLen = 0;
    bool inEscape = false;
    for (char c : text) {
        if (c == '\033') inEscape = true;
        else if (inEscape && c == 'm') inEscape = false;
        else if (!inEscape) visLen++;
    }

    int padding = width - 4 - visLen;  // 4 = "│ " + " │"
    if (padding < 0) padding = 0;

    cout << CYAN << BOX_V << RESET << " " << color << text << RESET;
    for (int i = 0; i < padding; i++) cout << ' ';
    cout << " " << CYAN << BOX_V << RESET << '\n';
}

static void drawBoxLineCenter(const string& text, int width, const string& color = "") {
    int visLen = 0;
    bool inEscape = false;
    for (char c : text) {
        if (c == '\033') inEscape = true;
        else if (inEscape && c == 'm') inEscape = false;
        else if (!inEscape) visLen++;
    }

    int totalPad = width - 4 - visLen;
    int leftPad = totalPad / 2;
    int rightPad = totalPad - leftPad;

    cout << CYAN << BOX_V << RESET << " ";
    for (int i = 0; i < leftPad; i++) cout << ' ';
    cout << color << text << RESET;
    for (int i = 0; i < rightPad; i++) cout << ' ';
    cout << " " << CYAN << BOX_V << RESET << '\n';
}

static void displayMenu() {
    const int boxWidth = 26;

    cout << '\n';
    drawBoxTop(boxWidth);
    drawBoxLineCenter("BID SYSTEM", boxWidth, BOLD + YELLOW);
    drawBoxMiddle(boxWidth);
    drawBoxLine("[1] Enter Bid", boxWidth, GREEN);
    drawBoxLine("[2] Load Bids", boxWidth, GREEN);
    drawBoxLine("[3] Show All", boxWidth, GREEN);
    drawBoxLine("[4] Find Bid", boxWidth, GREEN);
    drawBoxLine("[5] Remove Bid", boxWidth, GREEN);
    drawBoxMiddle(boxWidth);
    drawBoxLine("[9] Exit", boxWidth, RED);
    drawBoxBottom(boxWidth);
    cout << '\n';
}

static void displayResult(const string& title, const vector<string>& lines, const string& titleColor = "") {
    int maxLen = title.length();
    for (const auto& line : lines) {
        int visLen = 0;
        bool inEscape = false;
        for (char c : line) {
            if (c == '\033') inEscape = true;
            else if (inEscape && c == 'm') inEscape = false;
            else if (!inEscape) visLen++;
        }
        if (visLen > maxLen) maxLen = visLen;
    }

    int boxWidth = max(32, maxLen + 6);

    cout << '\n';
    drawBoxTop(boxWidth);
    drawBoxLineCenter(title, boxWidth, titleColor.empty() ? (BOLD + CYAN) : titleColor);
    drawBoxMiddle(boxWidth);
    for (const auto& line : lines) {
        drawBoxLine(line, boxWidth);
    }
    drawBoxBottom(boxWidth);
}

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
// LinkedList Class
//
// Why a singly linked list with both head AND tail pointers?
// - Head pointer: required for traversal from the start
// - Tail pointer: makes Append O(1) instead of O(n)
// - Without tail, we'd have to walk the whole list to add to the end
//
// Why track size separately?
// - Avoids O(n) traversal just to count elements
// - Used to show "12023 bids loaded" without re-counting
//
// Trade-offs:
// - Extra 8 bytes per list for tail pointer
// - Must keep tail in sync during Remove (edge case when removing last node)
//============================================================================

class LinkedList {
private:
    struct Node {
        Bid bid;
        Node *next;
        Node() : next(nullptr) {}
        Node(const Bid& aBid) : bid(aBid), next(nullptr) {}
    };

    Node *head;
    Node *tail;
    int   size;

public:
    LinkedList();
    virtual ~LinkedList();
    void Append(const Bid& bid);
    void Prepend(const Bid& bid);
    void PrintList() const;
    void Remove(const string& bidId);
    Bid Search(const string& bidId) const;
    int Size() const;
};

LinkedList::LinkedList() : head(nullptr), tail(nullptr), size(0) {}

/**
 * Destructor - must manually free all nodes we allocated with 'new'.
 * Save next pointer BEFORE deleting current node, or we lose it.
 */
LinkedList::~LinkedList() {
    Node* current = head;
    while (current != nullptr) {
        Node* nextNode = current->next;
        delete current;
        current = nextNode;
    }
}

/**
 * Append - O(1) thanks to tail pointer.
 * This is why we maintain tail - CSV loading adds 12k bids sequentially.
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

    cout << CYAN << "Enter ID: " << RESET;
    getline(cin, bid.bidId); // no pre-ignore needed

    cout << GREEN << "Enter Title: " << RESET;
    getline(cin, bid.title);

    cout << YELLOW << "Enter Fund: " << RESET;
    cin >> bid.fund;

    cout << MAGENTA << "Enter Amount: " << RESET << "$";
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
// Helper to check if a file exists
static bool fileExists(const string& path) {
    ifstream f(path);
    return f.good();
}

// Find the CSV file in common locations
static string findCsvFile(const string& filename) {
    // Try these paths in order
    vector<string> searchPaths = {
        filename,                           // as given
        "data/" + filename,                 // from project root
        "../data/" + filename,              // from build/ directory
        "../../data/" + filename,           // from build/Release/ directory
    };

    for (const auto& path : searchPaths) {
        if (fileExists(path)) {
            return path;
        }
    }
    return filename; // return original if not found (will error later)
}

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
            csvPath = findCsvFile("eBid_Monthly_Sales.csv");
            bidKey = "98109";
    }

    LinkedList bidList;

    Bid bid;

    int choice = 0;
    while (choice != 9) {
        displayMenu();
        cout << CYAN << "Enter choice: " << RESET;

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayResult("ERROR", {RED + "Invalid input. Please enter a number." + RESET}, BOLD + RED);
            waitForEnter();
            choice = 0;
            continue;
        }
        // success path: eat the trailing newline once
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                // Get bid info from user
                cout << '\n';
                Bid b = getBid();

                // Check if bid ID already exists (prevent duplicates)
                Bid existing = bidList.Search(b.bidId);
                if (!existing.bidId.empty()) {
                    displayResult("ERROR", {
                        RED + "Bid ID " + b.bidId + " already exists." + RESET,
                        DIM + "Use a different ID or remove the existing bid first." + RESET
                    }, BOLD + RED);
                    cout << '\n';
                    waitForEnter();
                    break;
                }

                // Add the new bid
                bidList.Append(b);

                stringstream ss;
                ss << fixed << setprecision(2) << b.amount;

                displayResult("BID ADDED", {
                    CYAN + "ID:      " + RESET + b.bidId,
                    GREEN + "Title:   " + RESET + b.title,
                    YELLOW + "Fund:    " + RESET + b.fund,
                    MAGENTA + "Amount:  " + RESET + "$" + ss.str()
                }, BOLD + GREEN);
                cout << '\n';
                waitForEnter();
                break;
            }
            case 2: {
                clock_t ticks = clock();
                loadBids(csvPath, &bidList);
                ticks = clock() - ticks;

                stringstream ms, sec;
                ms << fixed << setprecision(2) << (ticks * 1000.0 / CLOCKS_PER_SEC);
                sec << fixed << setprecision(4) << (ticks * 1.0 / CLOCKS_PER_SEC);

                displayResult("BIDS LOADED", {
                    GREEN + to_string(bidList.Size()) + " bids read" + RESET,
                    DIM + "Time: " + ms.str() + " ms (" + sec.str() + " s)" + RESET
                }, BOLD + GREEN);
                cout << '\n';
                waitForEnter();
                break;
            }
            case 3: {
                if (bidList.Size() == 0) {
                    displayResult("ERROR", {
                        RED + "No bids loaded yet." + RESET,
                        DIM + "Please select option 2 first." + RESET
                    }, BOLD + RED);
                } else {
                    cout << '\n';
                    drawBoxTop(getTerminalWidth() - 2);
                    drawBoxLineCenter("ALL BIDS (" + to_string(bidList.Size()) + " total)", getTerminalWidth() - 2, BOLD + CYAN);
                    drawBoxBottom(getTerminalWidth() - 2);
                    cout << '\n';
                    bidList.PrintList();
                    cout << '\n';
                }
                waitForEnter();
                break;
            }
            case 4: {
                cout << '\n' << CYAN << "Enter Bid ID to find: " << RESET;
                string searchId;
                getline(cin, searchId);

                // Trim whitespace
                size_t startPos = searchId.find_first_not_of(" \t");
                size_t endPos = searchId.find_last_not_of(" \t");
                if (startPos != string::npos) {
                    searchId = searchId.substr(startPos, endPos - startPos + 1);
                } else {
                    searchId = "";
                }

                if (searchId.empty()) {
                    displayResult("ERROR", {RED + "No ID entered." + RESET}, BOLD + RED);
                    cout << '\n';
                    waitForEnter();
                    break;
                }

                auto startTime = high_resolution_clock::now();
                Bid result = bidList.Search(searchId);
                auto endTime = high_resolution_clock::now();
                auto us = duration_cast<microseconds>(endTime - startTime).count();

                if (!result.bidId.empty()) {
                    stringstream ss;
                    ss << fixed << setprecision(2) << result.amount;

                    stringstream timeStr;
                    timeStr << us << " us (" << fixed << setprecision(6) << (us / 1'000'000.0) << " s)";

                    // Don't truncate - let the box grow to fit content
                    displayResult("BID FOUND", {
                        CYAN + "ID:      " + RESET + result.bidId,
                        GREEN + "Title:   " + RESET + result.title,
                        YELLOW + "Fund:    " + RESET + result.fund,
                        MAGENTA + "Amount:  " + RESET + "$" + ss.str(),
                        "",
                        DIM + "Search time: " + timeStr.str() + RESET
                    }, BOLD + GREEN);
                } else {
                    displayResult("NOT FOUND", {
                        RED + "Bid ID " + searchId + " not found." + RESET
                    }, BOLD + RED);
                }
                cout << '\n';
                waitForEnter();
                break;
            }
            case 5: {
                cout << '\n' << CYAN << "Enter Bid ID to remove: " << RESET;
                string removeId;
                getline(cin, removeId);

                // Trim whitespace
                size_t startPos = removeId.find_first_not_of(" \t");
                size_t endPos = removeId.find_last_not_of(" \t");
                if (startPos != string::npos) {
                    removeId = removeId.substr(startPos, endPos - startPos + 1);
                } else {
                    removeId = "";
                }

                if (removeId.empty()) {
                    displayResult("ERROR", {RED + "No ID entered." + RESET}, BOLD + RED);
                    cout << '\n';
                    waitForEnter();
                    break;
                }

                Bid check = bidList.Search(removeId);
                if (!check.bidId.empty()) {
                    bidList.Remove(removeId);
                    displayResult("BID REMOVED", {
                        GREEN + "Successfully removed bid ID: " + removeId + RESET
                    }, BOLD + GREEN);
                } else {
                    displayResult("NOT FOUND", {
                        RED + "Bid ID " + removeId + " was not in the list." + RESET
                    }, BOLD + RED);
                }
                cout << '\n';
                waitForEnter();
                break;
            }
            case 9: {
                cout << '\n';
                drawBoxTop(20);
                drawBoxLineCenter("Goodbye!", 20, BOLD + YELLOW);
                drawBoxBottom(20);
                cout << '\n';
                break;
            }
            default: {
                displayResult("ERROR", {RED + "Invalid choice. Please try again." + RESET}, BOLD + RED);
                waitForEnter();
            }
        }
    }

    return 0;
}

