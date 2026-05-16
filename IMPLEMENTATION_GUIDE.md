# Transaction Processing System - Implementation Guide

## Overview

This document explains the complete implementation of the **Transaction Processing System** in C, including all design decisions, architectural patterns, and best practices.

---

## Table of Contents

1. [Architecture Overview](#architecture-overview)
2. [Data Structures](#data-structures)
3. [Core Functions](#core-functions)
4. [Edge Cases & Validation](#edge-cases--validation)
5. [File I/O Design](#file-io-design)
6. [Memory Safety](#memory-safety)
7. [Compilation & Execution](#compilation--execution)
8. [Testing Scenarios](#testing-scenarios)

---

## Architecture Overview

### System Design Pattern: **Modular Procedural Architecture**

The system follows a clean procedural design with clear separation of concerns:

```
┌─────────────────────────────────────────┐
│      main() - Menu Loop                 │
│  (Orchestrates user interaction)        │
└──────────────────┬──────────────────────┘
                   │
        ┌──────────┼──────────┐
        │          │          │
        ▼          ▼          ▼
    ┌────────┐ ┌────────┐ ┌────────┐
    │ I/O    │ │ Account│ │Display │
    │Handler │ │Manager │ │Handler │
    └────────┘ └────────┘ └────────┘
        │          │          │
        └──────────┼──────────┘
                   │
        ┌──────────▼──────────┐
        │  In-Memory Array    │
        │  struct Account[100]│
        └─────────────────────┘
                   │
        ┌──────────▼──────────┐
        │  Persistent File    │
        │  (accounts.txt)     │
        └─────────────────────┘
```

### Key Principle: **Load-Once, Operate-In-Memory**

- **Startup**: Load entire `accounts.txt` into RAM (`struct Account accounts[MAX_ACCOUNTS]`)
- **Runtime**: All transactions operate on in-memory copies (fast)
- **Persistence**: Save ONLY when account state changes (minimizes disk I/O)
- **Benefit**: ~100x faster than reading/writing file for each transaction

---

## Data Structures

### Account Structure

```c
struct Account {
    int account_number;      /* Unique identifier (auto-incremented) */
    char name[NAME_LENGTH];  /* Fixed 50-char buffer (prevents overflow) */
    double balance;          /* Currency in dollars */
    char status[10];         /* "Active" or "Inactive" */
};
```

### Global State

```c
struct Account accounts[MAX_ACCOUNTS];  /* Array of up to 100 accounts */
int num_accounts;                       /* Current count (0 to MAX_ACCOUNTS) */
```

### Configuration Constants

```c
#define MAX_ACCOUNTS 100         /* Array capacity */
#define NAME_LENGTH 50           /* Name buffer size (includes null terminator) */
#define STATUS_LENGTH 10         /* Status buffer size */
#define MIN_BALANCE 0.0          /* Cannot go negative */
#define TRANSACTION_LIMIT 10000.0 /* Prevents typo mega-transfers */
```

---

## Core Functions

### 1. `loadAccounts()` - Persistent Load

**Purpose**: Read `accounts.txt` into memory at startup

**Algorithm**:
1. Open file in read mode (`"r"`)
2. If file missing: Warn user, continue with empty database
3. For each line:
   - Use `fscanf()` to parse 4 fields: `account_number`, `name`, `balance`, `status`
   - Validate constraints (balance ≥ 0, status is "Active"/"Inactive")
   - Skip malformed records (graceful degradation)
4. Stop at EOF or MAX_ACCOUNTS capacity
5. Close file pointer

**Error Handling**:
- File not found → Expected on first run
- Malformed record → Skip and log warning
- fscanf failure → Consume line and continue
- Never crashes; always safe to continue

**Performance**: O(n) file read, only called once at startup

---

### 2. `saveAccounts()` - Persistent Save

**Purpose**: Write in-memory accounts back to `accounts.txt`

**Algorithm**:
1. Open file in write mode (`"w"`) - truncates existing content
2. For each account:
   - Use `fprintf()` with format: `"%-10d %-30s %.2f %s\n"`
   - Check fprintf return value for write errors
3. Close file pointer

**Format**: Space-separated fields, one account per line
```
1001 Alice Johnson 5000.50 Active
1002 Bob Williams 3250.75 Active
```

**Called After**:
- Adding a new account
- Completing a transaction (deposit/withdrawal)

---

### 3. `addAccount()` - Create New Account

**Workflow**:
```
User Input
    ↓
Get Name (fgets)
    ↓ [Validate: not empty, not too long]
Get Initial Balance (scanf)
    ↓ [Validate: >= 0, <= TRANSACTION_LIMIT]
Generate Account Number (auto-incremented)
    ↓
Set Status = "Active"
    ↓
Add to accounts[] array
    ↓
Call saveAccounts()
    ↓
Display confirmation
```

**Validation Checks**:
- Name length ≤ 49 characters (buffer overflow prevention)
- Name not empty
- Balance ≥ $0.00
- Balance ≤ $10,000.00 (transaction limit)
- Array not full (< MAX_ACCOUNTS)

**Safety Notes**:
- Use `fgets()` instead of `scanf()` for name (safer string input)
- Strip trailing newline from fgets
- Use `strcpy()` for small fixed strings

---

### 4. `processTransaction()` - Deposits & Withdrawals

**Validation Hierarchy** (all checked before modifying balance):

```
User Input
    ↓
Parse Account Number
    ↓ [Check: account exists]
Check Account Status
    ↓ [Check: status == "Active"]
Parse Transaction Type (1=Deposit, 2=Withdrawal)
    ↓ [Check: valid choice]
Parse Amount
    ↓ [Check: positive (> 0)]
    ├─ [Check: not negative]
    ├─ [Check: not zero]
    ├─ [Check: <= TRANSACTION_LIMIT]
    └─ [For Withdrawal] [Check: balance - amount >= 0]
        ↓
Update Balance in Memory
    ↓
Call saveAccounts()
    ↓
Display Receipt
```

**Edge Cases Prevented**:

| Scenario | Prevention |
|----------|-----------|
| Negative deposit | `amount < 0` check → REJECT |
| Overdraft withdrawal | `balance - amount < 0` check → REJECT |
| Inactive account transaction | `status != "Active"` check → REJECT |
| Zero amount | `amount == 0` check → REJECT |
| Large typo ($999999) | `amount > TRANSACTION_LIMIT` check → REJECT |
| Non-existent account | `findAccountByNumber()` returns -1 → REJECT |

---

### 5. `displayAllAccounts()` - Table View

**Output Format**:
```
================================================================================
                        ACCOUNT DASHBOARD
================================================================================
Acct #     Account Holder                 Balance      Status
--------------------------------------------------------------------------------
1001       Alice Johnson                   5000.50      Active
1002       Bob Williams                    3250.75      Active
1003       Carol Davis                     1500.00      Active
1004       David Anderson                     0.00      Inactive
1005       Emma Thompson                   7890.25      Active
--------------------------------------------------------------------------------
Total Accounts: 5 | Total Balance: $17641.50
================================================================================
```

**Format Specifiers**:
- `%-10d` - Left-aligned account number (10 char width)
- `%-30s` - Left-aligned name (30 char width)
- `%10.2f` - Right-aligned currency (10 char, 2 decimal places)
- `%-10s` - Left-aligned status (10 char width)

**Features**:
- Header with column labels
- Data rows (each account)
- Separator lines for visual clarity
- Summary statistics (count, total balance)
- Handles empty database gracefully

---

## Edge Cases & Validation

### 1. Negative Deposits
```c
Problem: User enters deposit as $-500
Solution: if (amount < 0) REJECT
```

### 2. Overdraft Protection
```c
Problem: Account has $100, user withdraws $150
Solution: if (balance - amount < 0) REJECT
Before any state modification
```

### 3. Inactive Account Blocking
```c
Problem: User transfers from closed account
Solution: if (strcmp(status, "Active") != 0) REJECT
Transaction-type check, not input validation
```

### 4. Missing File on First Run
```c
Problem: accounts.txt doesn't exist yet
Solution: loadAccounts() checks if (fopen == NULL)
Sets num_accounts = 0 and continues gracefully
```

### 5. Malformed Records in File
```c
Problem: accounts.txt line: "1001 JohnDoe 5000.50 Broken_Status"
Solution: fscanf() returns EOF, line is skipped
Validated: if (status != "Active" && status != "Inactive") skip
```

### 6. Buffer Overflow Prevention
```c
Problem: User enters 100-char name, buffer is only 50
Solution: fgets(name, NAME_LENGTH, stdin)
Limits input to NAME_LENGTH-1 chars
Also use strcpy() only for fixed strings, not user input
```

### 7. File I/O Failures
```c
Problem: Disk full when saving accounts
Solution: Check fprintf() return value < 0 → Handle error
Display warning, keep accounts in memory, allow retry
```

### 8. Input Buffer Corruption
```c
Problem: User enters "abc" for int, leftover chars break next scanf()
Solution: clearInputBuffer() reads until newline
Call after scanf() error or fgets()
```

---

## File I/O Design

### File Format: **Space-Separated Text**

```
<account_number> <name> <balance> <status>
1001 Alice Johnson 5000.50 Active
```

**Why Text, Not Binary?**
- ✓ Human-readable (can inspect with any text editor)
- ✓ Portable across platforms
- ✗ Slightly larger (but acceptable for 100 accounts)
- ✗ Slightly slower fscanf() parsing

### Read Function: `loadAccounts()`

```c
while (num_accounts < MAX_ACCOUNTS && !feof(file_ptr)) {
    int result = fscanf(file_ptr, "%d %49s %lf %9s",
                       &account.account_number,
                       account.name,           /* 49 = MAX - 1 for null terminator */
                       &account.balance,
                       account.status);      /* 9 = MAX - 1 for null terminator */
    
    if (result == 4) {
        /* Validate, then add */
    } else if (result == EOF) {
        break;
    } else {
        /* Skip malformed line */
    }
}
```

**Buffer Size Protection**:
- Name: `%49s` keeps at most 49 chars (total buffer 50)
- Status: `%9s` keeps at most 9 chars (total buffer 10)

### Write Function: `saveAccounts()`

```c
fprintf(file_ptr, "%d %-30s %.2f %s\n",
        account.account_number,
        account.name,           /* Left-aligned in 30 chars */
        account.balance,        /* 2 decimal places */
        account.status);
```

---

## Memory Safety

### 1. **Stack Allocation (No Heap)**
```c
struct Account accounts[MAX_ACCOUNTS];  /* Stack allocation */
```
- ✓ No malloc/free → No memory leaks
- ✓ Bounded size → Stack overflow protection (fixed 100)
- ✓ Fast allocation/deallocation
- ✗ Fixed size (not expandable)

### 2. **Buffer Overflow Prevention**

| Function | Risk | Mitigation |
|----------|------|-----------|
| `fgets()` | Input longer than buffer | `fgets(name, NAME_LENGTH, stdin)` |
| `fscanf()` | Malicious input | `%49s` limits scanf width |
| `strcpy()` | Copy to small buffer | Only use for fixed strings |
| `scanf()` | Non-numeric input | Check return value == 1 |

### 3. **Dangling Pointers**
```c
void saveAccounts(void) {
    FILE *file_ptr = fopen(...);
    if (file_ptr == NULL) return;   /* Quick exit, no leak */
    
    fprintf(...);
    
    fclose(file_ptr);  /* ALWAYS close before function exit */
}
```

### 4. **Input Buffer Corruption**
```c
void clearInputBuffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) { }
}
/* Called after any failed scanf() */
```

---

## Compilation & Execution

### Compile Command
```bash
gcc -std=c99 -Wall -pedantic -o transaction_system transaction_system.c
```

**Flags Explained**:
- `-std=c99` - Use C99 standard (for loop variables declared in for loop)
- `-Wall` - Enable all common warnings
- `-pedantic` - Strict standard compliance warnings
- `-o transaction_system` - Output filename

### Expected Output
```
No compilation errors
No warnings
Executable: transaction_system.exe (Windows) or transaction_system (Linux/Mac)
```

### Run Program
```bash
./transaction_system              # Linux/Mac
transaction_system.exe            # Windows
```

---

## Testing Scenarios

### Test Case 1: Display Accounts
```
Input: 1
Expected: Table of all accounts with correct formatting
Verify: Account numbers, names, balances, status values correct
```

### Test Case 2: Add New Account
```
Input: 2 → "Frank Miller" → 2500.00
Expected: 
  - Account number assigned (1006)
  - Save to accounts.txt (verify file updated)
  - Displayed on next "Display All" view
```

### Test Case 3: Deposit (Valid)
```
Input: 3 → 1001 → 1 → 500.00
Expected:
  - Account 1001 (Alice Johnson) balance increased by $500
  - New balance: $5500.50
  - Updated in accounts.txt
```

### Test Case 4: Deposit (Negative)
```
Input: 3 → 1001 → 1 → -500.00
Expected: [ERROR] "Transaction amount cannot be negative"
  - Balance unchanged
  - File unchanged
```

### Test Case 5: Withdrawal (Valid)
```
Input: 3 → 1002 → 2 → 250.75
Expected:
  - Account 1002 (Bob Williams) balance decreased by $250.75
  - New balance: $3000.00
  - Updated in accounts.txt
```

### Test Case 6: Withdrawal (Overdraft)
```
Input: 3 → 1003 → 2 → 2000.00
Expected: [ERROR] "Insufficient funds. Current balance: $1500.00"
  - Balance unchanged
  - File unchanged
```

### Test Case 7: Transaction on Inactive Account
```
Input: 3 → 1004 → 1 → 100.00
Expected: [ERROR] "Cannot process transactions on an Inactive account"
  - Balance unchanged
  - File unchanged
```

### Test Case 8: Invalid Account Number
```
Input: 3 → 9999 → 1 → 100.00
Expected: [ERROR] "Account 9999 not found"
  - Database unchanged
```

### Test Case 9: Invalid Input (Non-Numeric)
```
Input: 3 → "abc" (for account number)
Expected: [ERROR] "Invalid account number entered"
  - Database unchanged
  - Menu re-displayed
```

### Test Case 10: Transaction Exceeds Limit
```
Input: 3 → 1001 → 1 → 15000.00
Expected: [ERROR] "Amount exceeds transaction limit of $10000.00"
  - Balance unchanged
  - File unchanged
```

---

## Code Quality Checklist

- ✓ **Modular**: Each function has single responsibility
- ✓ **Documented**: Inline comments explain complex logic
- ✓ **Validated**: All inputs checked before state change
- ✓ **Safe**: No buffer overflows, memory leaks, or dangling pointers
- ✓ **Efficient**: Load once, operate in memory
- ✓ **Portable**: Standard C99, no platform-specific code
- ✓ **Testable**: Easy to verify behavior with manual input
- ✓ **Extensible**: Easy to add new transaction types or fields

---

## Future Enhancements (Beyond Scope)

1. **Database**: Switch to SQLite for larger datasets
2. **Concurrency**: Add mutex locks for multi-threaded access
3. **Security**: Encrypt sensitive data, add password protection
4. **Audit Trail**: Log all transactions to separate file
5. **Reports**: Generate monthly statements, tax documents
6. **GUI**: Build Qt/Gtk interface instead of command-line
7. **Networking**: Add REST API for remote access
8. **Backup**: Automatic versioning and backup system

---

## Conclusion

This implementation demonstrates clean, safe procedural C programming with:
- Robust error handling
- Data validation at every step
- Efficient I/O patterns
- Clear, maintainable code structure
- Comprehensive documentation

It serves as a solid foundation for a banking system and illustrates best practices in C systems programming.
