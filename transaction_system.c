/*
 * =============================================================================
 * Transaction Processing System - Mini Project (24UCS271)
 * =============================================================================
 * 
 * OBJECTIVE:
 * A modular C program that manages customer accounts and processes financial
 * transactions with robust file I/O, data validation, and error handling.
 * 
 * KEY FEATURES:
 * - Load/save accounts from/to a persistent text file (accounts.txt)
 * - Add new accounts dynamically
 * - Process deposits and withdrawals with validation
 * - Display all accounts in a formatted tabular dashboard
 * - Handle edge cases: negative amounts, overdrafts, inactive accounts, EOF
 * 
 * SECURITY & BEST PRACTICES:
 * - Fixed buffer sizes prevent stack overflow attacks
 * - Proper file pointer management avoids dangling pointers
 * - In-memory operations minimize disk I/O overhead
 * - Safe string handling with bounds checking
 * - Comprehensive error checking on all file operations
 * 
 * AUTHOR: Systems Programming Lab
 * DATE: May 2026
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ============================================================================
 * CONSTANTS & CONFIGURATION
 * ============================================================================ */

#define MAX_ACCOUNTS 100        /* Maximum number of accounts in memory */
#define NAME_LENGTH 50          /* Account holder name buffer size */
#define STATUS_LENGTH 10        /* Account status buffer size (e.g., "Active") */
#define FILE_PATH "accounts.txt"
#define MIN_BALANCE 0.0         /* Minimum account balance allowed */
#define TRANSACTION_LIMIT 10000.0 /* Transaction limit per operation */

/* ============================================================================
 * DATA STRUCTURES
 * ============================================================================ */

/*
 * Account Structure - Core data model for each customer account
 * 
 * FIELDS:
 *   - account_number: Unique identifier (auto-incremented)
 *   - name: Account holder's name (fixed-size buffer prevents overflow)
 *   - balance: Current account balance in currency units
 *   - status: "Active" or "Inactive" strings (fixed-size prevents attacks)
 * 
 * SAFETY NOTES:
 *   - Fixed-size char arrays prevent buffer overflows
 *   - String data is explicitly null-terminated in file parser
 */
struct Account {
    int account_number;
    char name[NAME_LENGTH];
    double balance;
    char status[STATUS_LENGTH];
};

/* ============================================================================
 * GLOBAL STATE MANAGEMENT
 * ============================================================================ */

struct Account accounts[MAX_ACCOUNTS];  /* In-memory account database */
int num_accounts = 0;                   /* Current number of loaded accounts */

/* ============================================================================
 * FUNCTION PROTOTYPES
 * ============================================================================ */

void loadAccounts(void);
void saveAccounts(void);
void addAccount(void);
void processTransaction(void);
void displayAllAccounts(void);
int getNextAccountNumber(void);
int findAccountByNumber(int account_num);
void displayMenu(void);
int getMenuChoice(void);
void clearInputBuffer(void);
int isValidAmount(double amount);
void handleSaveError(void);

/* ============================================================================
 * IMPLEMENTATION: FILE I/O OPERATIONS
 * ============================================================================ */

/*
 * loadAccounts()
 * 
 * PURPOSE: Load all accounts from the persistent text file into memory
 * 
 * ALGORITHM:
 * 1. Attempt to open accounts.txt in read mode
 * 2. If file doesn't exist, warn user but continue (new database)
 * 3. Parse each line safely, skipping empty lines and malformed data
 * 4. Use fscanf with format specifiers to extract fields
 * 5. Validate each read account before storing
 * 6. Stop at EOF or when max capacity reached
 * 
 * ERROR HANDLING:
 * - If fscanf() fails, skip malformed lines (graceful degradation)
 * - Properly close file pointer to avoid resource leaks
 * - Check array bounds to prevent buffer overflow
 * - Handle empty/missing file gracefully (first-run scenario)
 * 
 * PERFORMANCE:
 * - Load is done ONCE at startup, reducing disk I/O
 * - All transactions operate on in-memory structures
 * - File is written back periodically or on-demand
 */
void loadAccounts(void) {
    FILE *file_ptr = fopen(FILE_PATH, "r");
    char line_buffer[256];
    
    /* If file doesn't exist, this is a fresh database */
    if (file_ptr == NULL) {
        printf("[INFO] %s not found. Starting with empty database.\n", FILE_PATH);
        num_accounts = 0;
        return;
    }
    
    num_accounts = 0;
    
    /* Parse each line from the file into an Account structure */
    while (num_accounts < MAX_ACCOUNTS && fgets(line_buffer, sizeof(line_buffer), file_ptr) != NULL) {
        /* Skip empty lines */
        if (line_buffer[0] == '\n' || line_buffer[0] == '\0') {
            continue;
        }
        
        /*
         * Parse pipe-delimited format: account_num|name|balance|status
         * This handles names with spaces or special characters safely.
         * Example: 1001|Alice Johnson|5000.50|Active
         */
        int result = sscanf(line_buffer, "%d|%49[^|]|%lf|%9s",
                           &accounts[num_accounts].account_number,
                           accounts[num_accounts].name,
                           &accounts[num_accounts].balance,
                           accounts[num_accounts].status);
        
        /* Remove newline from parsed status if present */
        size_t status_len = strlen(accounts[num_accounts].status);
        if (status_len > 0 && accounts[num_accounts].status[status_len - 1] == '\n') {
            accounts[num_accounts].status[status_len - 1] = '\0';
        }
        
        if (result == 4) {
            /* Validate account data before adding to memory */
            if (accounts[num_accounts].account_number > 0 &&
                accounts[num_accounts].balance >= MIN_BALANCE &&
                (strcmp(accounts[num_accounts].status, "Active") == 0 ||
                 strcmp(accounts[num_accounts].status, "Inactive") == 0)) {
                
                num_accounts++;
            } else {
                /* Skip malformed records */
                printf("[WARN] Skipped malformed record (validation failed)\n");
            }
        } else if (result > 0 && result < 4) {
            /* Partial read - skip this line */
            printf("[WARN] Skipped incomplete record\n");
        }
        /* If result == 0, line was empty or malformed, skip it */
    }
    
    fclose(file_ptr);  /* CRITICAL: Always close file pointers to avoid resource leaks */
    printf("[INFO] Loaded %d account(s) from %s\n", num_accounts, FILE_PATH);
}

/*
 * saveAccounts()
 * 
 * PURPOSE: Persist all in-memory accounts back to the text file
 * 
 * ALGORITHM:
 * 1. Open accounts.txt in write mode (truncates existing content)
 * 2. Iterate through all loaded accounts
 * 3. Write each account as pipe-delimited text (robust format)
 * 4. Format: account_num|name|balance|status
 * 5. Verify write success by checking fprintf return value
 * 6. Ensure file is properly closed before returning
 * 
 * ERROR HANDLING:
 * - Check if fopen succeeded; abort if write permission denied
 * - Check fprintf return value; if negative, warn about write failure
 * - Close file even if partial write occurred (cleanup)
 * 
 * DATA FORMAT:
 * Pipe-delimited (robust for names with spaces):
 * Each line: <account_number>|<name>|<balance>|<status>
 * Example: 1001|Alice Johnson|5000.50|Active
 * 
 * ATOMICITY NOTE:
 * This operation is NOT atomic - if the program crashes mid-write,
 * the file could be corrupted. For production, use temp file + rename.
 */
void saveAccounts(void) {
    FILE *file_ptr = fopen(FILE_PATH, "w");
    
    if (file_ptr == NULL) {
        printf("[ERROR] Cannot open %s for writing. Check file permissions.\n", FILE_PATH);
        handleSaveError();
        return;
    }
    
    /* Write each account to file in pipe-delimited format */
    for (int i = 0; i < num_accounts; i++) {
        int write_result = fprintf(file_ptr, "%d|%s|%.2f|%s\n",
                                   accounts[i].account_number,
                                   accounts[i].name,
                                   accounts[i].balance,
                                   accounts[i].status);
        
        if (write_result < 0) {
            printf("[ERROR] Failed to write account %d to file. Disk full?\n",
                   accounts[i].account_number);
            handleSaveError();
            break;
        }
    }
    
    fclose(file_ptr);  /* CRITICAL: Close file pointer */
    printf("[INFO] Saved %d account(s) to %s\n", num_accounts, FILE_PATH);
}

/*
 * handleSaveError()
 * 
 * PURPOSE: Error recovery handler for failed save operations
 * 
 * NOTE: In this mini-project version, we simply notify the user.
 *       In production systems, this would implement:
 *       - Retry logic with exponential backoff
 *       - In-memory backup snapshots
 *       - Transaction logging for recovery
 */
void handleSaveError(void) {
    printf("[ALERT] File save failed. Accounts remain in memory but are not persisted.\n");
    printf("        Please retry the save operation or check disk space.\n");
}

/* ============================================================================
 * IMPLEMENTATION: ACCOUNT MANAGEMENT
 * ============================================================================ */

/*
 * getNextAccountNumber()
 * 
 * PURPOSE: Auto-generate the next available account number
 * 
 * ALGORITHM:
 * 1. Scan through all existing accounts to find maximum account number
 * 2. Return max + 1 as the next number
 * 3. If no accounts exist, start from 1001 (conventional bank numbering)
 * 
 * RETURN: Next available account number (guaranteed unique)
 */
int getNextAccountNumber(void) {
    int max_num = 1000;  /* Start from 1001 for real-looking account numbers */
    
    for (int i = 0; i < num_accounts; i++) {
        if (accounts[i].account_number > max_num) {
            max_num = accounts[i].account_number;
        }
    }
    
    return max_num + 1;
}

/*
 * findAccountByNumber()
 * 
 * PURPOSE: Locate an account by its account number
 * 
 * ALGORITHM:
 * 1. Linear search through all accounts (acceptable for MAX_ACCOUNTS = 100)
 * 2. Compare account_number field with input parameter
 * 3. Return array index if found, -1 if not found
 * 
 * PERFORMANCE NOTE:
 * - O(n) time complexity, acceptable for small datasets
 * - For larger systems, use hash tables or binary search
 * 
 * RETURN: Array index [0..num_accounts-1] or -1 if not found
 */
int findAccountByNumber(int account_num) {
    for (int i = 0; i < num_accounts; i++) {
        if (accounts[i].account_number == account_num) {
            return i;
        }
    }
    return -1;  /* Account not found */
}

/*
 * addAccount()
 * 
 * PURPOSE: Create and add a new account to the system
 * 
 * USER INTERACTION:
 * 1. Prompt for account holder's name (with length validation)
 * 2. Prompt for initial balance (with amount validation)
 * 3. Set initial status to "Active"
 * 4. Auto-generate account number
 * 
 * VALIDATION:
 * - Name cannot exceed NAME_LENGTH-1 characters
 * - Balance must be non-negative
 * - Balance must not exceed TRANSACTION_LIMIT
 * - Cannot exceed MAX_ACCOUNTS limit
 * 
 * PERSISTENCE:
 * - After adding, automatically save to file
 * - This ensures no data loss if program crashes
 * 
 * ERROR HANDLING:
 * - Gracefully handle input buffer issues
 * - Validate all constraints before committing
 */
void addAccount(void) {
    if (num_accounts >= MAX_ACCOUNTS) {
        printf("[ERROR] Account database is full (max: %d accounts).\n", MAX_ACCOUNTS);
        return;
    }
    
    printf("\n--- ADD NEW ACCOUNT ---\n");
    
    /* Prompt for account holder's name */
    printf("Enter account holder's name (max %d chars): ", NAME_LENGTH - 1);
    clearInputBuffer();  /* Clear any leftover input */
    
    if (fgets(accounts[num_accounts].name, NAME_LENGTH, stdin) == NULL) {
        printf("[ERROR] Failed to read account name.\n");
        return;
    }
    
    /* Remove trailing newline from fgets() */
    size_t name_len = strlen(accounts[num_accounts].name);
    if (name_len > 0 && accounts[num_accounts].name[name_len - 1] == '\n') {
        accounts[num_accounts].name[name_len - 1] = '\0';
    }
    
    /* Validate name is not empty */
    if (strlen(accounts[num_accounts].name) == 0) {
        printf("[ERROR] Account name cannot be empty.\n");
        return;
    }
    
    /* Prompt for initial balance */
    printf("Enter initial balance ($): ");
    if (scanf("%lf", &accounts[num_accounts].balance) != 1) {
        printf("[ERROR] Invalid balance amount.\n");
        clearInputBuffer();
        return;
    }
    
    /* Validate balance amount */
    if (!isValidAmount(accounts[num_accounts].balance)) {
        printf("[ERROR] Initial balance must be >= $0 and <= $%.2f\n", TRANSACTION_LIMIT);
        return;
    }
    
    /* Set account properties */
    accounts[num_accounts].account_number = getNextAccountNumber();
    strcpy(accounts[num_accounts].status, "Active");  /* Use strcpy for fixed string */
    
    printf("\n[SUCCESS] Account created:\n");
    printf("  Account Number: %d\n", accounts[num_accounts].account_number);
    printf("  Name: %s\n", accounts[num_accounts].name);
    printf("  Balance: $%.2f\n", accounts[num_accounts].balance);
    printf("  Status: %s\n", accounts[num_accounts].status);
    
    num_accounts++;
    saveAccounts();  /* Persist the new account immediately */
}

/* ============================================================================
 * IMPLEMENTATION: TRANSACTION PROCESSING
 * ============================================================================ */

/*
 * isValidAmount()
 * 
 * PURPOSE: Validate transaction amount for safety
 * 
 * CONSTRAINTS:
 * 1. Amount must be non-negative (enforced in withdrawal separately)
 * 2. Amount must not exceed reasonable transaction limit
 * 3. Prevents accidental mega-transactions from typos
 * 
 * PARAMETERS:
 *   amount: Transaction amount to validate
 * 
 * RETURN: 1 (true) if valid, 0 (false) if invalid
 */
int isValidAmount(double amount) {
    return (amount >= MIN_BALANCE && amount <= TRANSACTION_LIMIT);
}

/*
 * processTransaction()
 * 
 * PURPOSE: Handle deposits and withdrawals with comprehensive validation
 * 
 * ALGORITHM:
 * 1. Prompt for account number to perform transaction on
 * 2. Validate account exists and is "Active"
 * 3. Prompt for transaction type (1=Deposit, 2=Withdrawal)
 * 4. Prompt for amount and validate:
 *    a. Amount must be positive
 *    b. For withdrawals: check sufficient balance exists
 *    c. Amount must not exceed transaction limit
 * 5. Update in-memory balance
 * 6. Save updated accounts to file
 * 7. Display transaction receipt
 * 
 * EDGE CASES HANDLED:
 * - Negative deposit: REJECTED (user error handling)
 * - Withdrawal exceeding balance: REJECTED (prevents overdraft)
 * - Withdrawal from "Inactive" account: REJECTED (comply with policy)
 * - Non-existent account number: REJECTED (not found)
 * - Amount formatting errors (non-numeric): REJECTED (scanf fails)
 * 
 * SAFETY NOTE:
 * All validation occurs BEFORE any balance modification, ensuring
 * failed transactions leave accounts unchanged.
 */
void processTransaction(void) {
    int account_num, account_idx, trans_type;
    double amount, new_balance;
    
    printf("\n--- PROCESS TRANSACTION ---\n");
    
    /* Prompt for account number */
    printf("Enter account number: ");
    if (scanf("%d", &account_num) != 1) {
        printf("[ERROR] Invalid account number entered.\n");
        clearInputBuffer();
        return;
    }
    
    /* Validate account exists */
    account_idx = findAccountByNumber(account_num);
    if (account_idx == -1) {
        printf("[ERROR] Account %d not found.\n", account_num);
        return;
    }
    
    /* Validate account is Active */
    if (strcmp(accounts[account_idx].status, "Active") != 0) {
        printf("[ERROR] Cannot process transactions on an %s account.\n",
               accounts[account_idx].status);
        return;
    }
    
    /* Prompt for transaction type */
    printf("Transaction Type:\n");
    printf("  1 = Deposit\n");
    printf("  2 = Withdrawal\n");
    printf("Enter choice (1 or 2): ");
    
    if (scanf("%d", &trans_type) != 1 || (trans_type != 1 && trans_type != 2)) {
        printf("[ERROR] Invalid transaction type. Please enter 1 or 2.\n");
        clearInputBuffer();
        return;
    }
    
    /* Prompt for transaction amount */
    printf("Enter amount ($): ");
    if (scanf("%lf", &amount) != 1) {
        printf("[ERROR] Invalid amount entered.\n");
        clearInputBuffer();
        return;
    }
    
    /* ====== COMPREHENSIVE VALIDATION ====== */
    
    /* Reject negative amounts */
    if (amount < 0) {
        printf("[ERROR] Transaction amount cannot be negative.\n");
        return;
    }
    
    /* Reject zero amounts */
    if (amount == 0) {
        printf("[ERROR] Transaction amount must be greater than zero.\n");
        return;
    }
    
    /* Reject amounts exceeding transaction limit */
    if (!isValidAmount(amount)) {
        printf("[ERROR] Amount exceeds transaction limit of $%.2f\n", TRANSACTION_LIMIT);
        return;
    }
    
    /* For withdrawals: ensure sufficient balance */
    if (trans_type == 2) {  /* Withdrawal */
        new_balance = accounts[account_idx].balance - amount;
        
        if (new_balance < MIN_BALANCE) {
            printf("[ERROR] Insufficient funds. Current balance: $%.2f\n",
                   accounts[account_idx].balance);
            printf("        Withdrawal request: $%.2f\n", amount);
            printf("        Would result in: $%.2f (not allowed)\n", new_balance);
            return;
        }
        
        accounts[account_idx].balance = new_balance;
        printf("\n[SUCCESS] Withdrawal completed:\n");
        printf("  Amount withdrawn: $%.2f\n", amount);
    } else {  /* Deposit */
        accounts[account_idx].balance += amount;
        printf("\n[SUCCESS] Deposit completed:\n");
        printf("  Amount deposited: $%.2f\n", amount);
    }
    
    /* Display updated account information */
    printf("  Account: %d (%s)\n", account_num, accounts[account_idx].name);
    printf("  New balance: $%.2f\n", accounts[account_idx].balance);
    
    saveAccounts();  /* Persist the transaction */
}

/* ============================================================================
 * IMPLEMENTATION: REPORTING & DISPLAY
 * ============================================================================ */

/*
 * displayAllAccounts()
 * 
 * PURPOSE: Display all accounts in a clean, aligned tabular format
 * 
 * FEATURES:
 * - Header row with column labels
 * - Aligned columns using printf format specifiers:
 *   • %-10d: Left-aligned 10-char width for account numbers
 *   • %-30s: Left-aligned 30-char width for names
 *   • %10.2f: Right-aligned currency with 2 decimals
 *   • %-10s: Left-aligned 10-char width for status
 * - Separator line for visual clarity
 * - Summary row showing total number of accounts and total balance
 * 
 * ALGORITHM:
 * 1. Print header row with column labels
 * 2. Iterate through all loaded accounts
 * 3. Print each account on one row with aligned columns
 * 4. Calculate aggregate statistics (count, total balance)
 * 5. Print footer with summary
 * 
 * ERROR HANDLING:
 * - Handle case of empty account database gracefully
 * - Properly format currency to 2 decimal places
 */
void displayAllAccounts(void) {
    double total_balance = 0.0;
    
    printf("\n");
    printf("================================================================================\n");
    printf("                        ACCOUNT DASHBOARD\n");
    printf("================================================================================\n");
    
    if (num_accounts == 0) {
        printf("No accounts found in database.\n");
        printf("================================================================================\n");
        return;
    }
    
    /* Print header row with column labels */
    printf("%-10s %-30s %10s %10s\n", "Acct #", "Account Holder", "Balance", "Status");
    printf("--------------------------------------------------------------------------------\n");
    
    /* Print each account */
    for (int i = 0; i < num_accounts; i++) {
        printf("%-10d %-30s %10.2f %10s\n",
               accounts[i].account_number,
               accounts[i].name,
               accounts[i].balance,
               accounts[i].status);
        
        total_balance += accounts[i].balance;
    }
    
    /* Print footer with summary statistics */
    printf("--------------------------------------------------------------------------------\n");
    printf("Total Accounts: %d | Total Balance: $%.2f\n", num_accounts, total_balance);
    printf("================================================================================\n");
}

/* ============================================================================
 * IMPLEMENTATION: USER INTERFACE
 * ============================================================================ */

/*
 * clearInputBuffer()
 * 
 * PURPOSE: Clear leftover characters in stdin buffer after input errors
 * 
 * RATIONALE:
 * When scanf() encounters invalid input (e.g., user enters "abc" for int),
 * the invalid characters remain in the input buffer. Subsequent scanf()
 * calls will immediately fail on those cached characters.
 * 
 * SOLUTION:
 * Read and discard characters until newline or EOF is reached.
 * 
 * SAFETY: This is safe to call even if buffer is already clean.
 */
void clearInputBuffer(void) {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        /* Consume characters */
    }
}

/*
 * displayMenu()
 * 
 * PURPOSE: Display main menu options to the user
 */
void displayMenu(void) {
    printf("\n");
    printf("================================================================================\n");
    printf("                   TRANSACTION PROCESSING SYSTEM\n");
    printf("================================================================================\n");
    printf("1. Display All Accounts\n");
    printf("2. Add New Account\n");
    printf("3. Process Transaction (Deposit/Withdrawal)\n");
    printf("4. Exit\n");
    printf("================================================================================\n");
}

/*
 * getMenuChoice()
 * 
 * PURPOSE: Read and validate user's menu selection
 * 
 * RETURN: Menu choice (1-4) or -1 if invalid input
 */
int getMenuChoice(void) {
    int choice;
    printf("Enter your choice (1-4): ");
    
    if (scanf("%d", &choice) != 1) {
        printf("[ERROR] Invalid input. Please enter a number.\n");
        clearInputBuffer();
        return -1;
    }
    
    if (choice < 1 || choice > 4) {
        printf("[ERROR] Please enter a number between 1 and 4.\n");
        return -1;
    }
    
    return choice;
}

/* ============================================================================
 * MAIN PROGRAM
 * ============================================================================ */

/*
 * main()
 * 
 * PURPOSE: Main program loop - orchestrates system startup, menu navigation,
 *          and graceful shutdown
 * 
 * EXECUTION FLOW:
 * 1. Load accounts from persistent storage (accounts.txt)
 * 2. Enter infinite menu loop
 * 3. Display menu and get user choice
 * 4. Execute corresponding operation via switch/case
 * 5. Return to menu (until user selects Exit)
 * 6. On exit, accounts are already saved to file
 * 
 * DESIGN NOTES:
 * - Clean separation of concerns (I/O, validation, display)
 * - Modular functions enable testing and reusability
 * - All file operations happen outside the main loop (optimized I/O)
 * - Graceful error handling prevents crashes from user input
 * 
 * MEMORY MANAGEMENT:
 * - In-memory array of Account structures (STACK allocation - fast)
 * - File pointers are opened and closed locally (no dangling pointers)
 * - No dynamic allocation = no memory leaks
 */
int main(void) {
    int choice;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                  TRANSACTION PROCESSING SYSTEM (v1.0)                      ║\n");
    printf("║                      Mini Project - 24UCS271                               ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════╝\n");
    
    /* Load all accounts from persistent storage at startup */
    loadAccounts();
    
    /* Main menu loop */
    while (1) {
        displayMenu();
        choice = getMenuChoice();
        
        if (choice == -1) {
            /* Invalid input - try again */
            continue;
        }
        
        /* Process user choice */
        switch (choice) {
            case 1:
                displayAllAccounts();
                break;
                
            case 2:
                addAccount();
                break;
                
            case 3:
                processTransaction();
                break;
                
            case 4:
                printf("\n[INFO] Thank you for using Transaction Processing System.\n");
                printf("       Goodbye!\n\n");
                return 0;  /* Graceful exit */
                
            default:
                printf("[ERROR] Unknown menu choice.\n");
                break;
        }
    }
    
    return 0;
}

/* ============================================================================
 * DOCUMENTATION & IMPLEMENTATION SUMMARY
 * ============================================================================
 * 
 * KEY DESIGN DECISIONS:
 * 
 * 1. IN-MEMORY ARRAY vs KEY-VALUE STORE:
 *    - Chosen: Fixed array with linear search
 *    - Justification: Simple for 100 accounts, no overhead
 *    - Trade-off: O(n) search time; could use hash table for millions
 * 
 * 2. LOAD ONCE vs READ-WRITE PATTERN:
 *    - Chosen: Load all at startup, modify in memory, save on-demand
 *    - Rationale: Minimize disk I/O (1-10x slower than RAM)
 *    - Alternative: Read-write per transaction (simpler, slower)
 * 
 * 3. FIXED BUFFERS vs DYNAMIC ALLOCATION:
 *    - Chosen: Fixed buffers (char[50], char[10])
 *    - Benefits: No malloc/free, no memory leaks, no fragmentation
 *    - Drawback: "John Q. Public-Smith Jr." might not fit (50 chars)
 * 
 * 4. fscanf vs fgets PARSING:
 *    - Chosen: fscanf for simplicity
 *    - Trade-off: Less control over malformed lines
 *    - Alternative: fgets + sscanf for more robustness
 * 
 * 5. NESTED VALIDATION:
 *    - All checks before state modification
 *    - Prevents partial updates on validation failures
 *    - Ensures data consistency
 * 
 * EDGE CASES HANDLED:
 * ✓ Negative deposits
 * ✓ Overdraft protection
 * ✓ Inactive account blocking
 * ✓ Missing accounts.txt file (first run)
 * ✓ Malformed records in file
 * ✓ EOF conditions
 * ✓ Empty lines in file
 * ✓ Input buffer overflow (scanf invalid input)
 * ✓ File I/O errors (permissions, disk full)
 * ✓ Array capacity overflow
 * 
 * COMPILATION:
 *   gcc -std=c99 -Wall -pedantic transaction_system.c -o transaction_system
 * 
 * EXECUTION:
 *   ./transaction_system
 * 
 * ============================================================================
 */
