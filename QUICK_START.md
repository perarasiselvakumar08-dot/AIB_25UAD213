# Transaction Processing System - Quick Start Guide

## Prerequisites

You need a C compiler installed on your system:

### Windows
- **MinGW**: Download from https://www.mingw-w64.org/
- **MSVC**: Visual Studio Community with C++ tools
- **WSL**: Windows Subsystem for Linux with GCC

### Linux/Mac
- **GCC**: `sudo apt-get install build-essential` (Ubuntu/Debian)
- **Clang**: `xcode-select --install` (Mac)

---

## Step 1: Verify Compiler Installation

### Windows (PowerShell)
```powershell
gcc --version
```

### Linux/Mac (Terminal)
```bash
gcc --version
```

**Expected Output**:
```
gcc (GCC) 11.4.0
Copyright (C) 2021 Free Software Foundation, Inc.
...
```

If command not found, install GCC first.

---

## Step 2: Compile the Program

### Option A: Manual Compilation
```bash
gcc -std=c99 -Wall -pedantic -o transaction_system transaction_system.c
```

### Option B: Using Makefile (Linux/Mac)
```bash
make
```

### Option C: Windows Batch File
```batch
compile.bat
```

**Expected Output**:
```
[No output if successful - clean compilation]
[If errors, check syntax and compiler messages]
```

---

## Step 3: Verify Compilation

List directory contents to confirm executable was created:

### Linux/Mac
```bash
ls -la transaction_system
```

### Windows PowerShell
```powershell
Get-Item transaction_system.exe
```

---

## Step 4: Run the Program

### Linux/Mac
```bash
./transaction_system
```

### Windows Command Prompt
```cmd
transaction_system.exe
```

### Windows PowerShell
```powershell
.\transaction_system.exe
```

**Expected Output**:
```
╔════════════════════════════════════════════════════════════════════════════╗
║                  TRANSACTION PROCESSING SYSTEM (v1.0)                      ║
║                      Mini Project - 24UCS271                               ║
╚════════════════════════════════════════════════════════════════════════════╝

[INFO] Loaded 5 account(s) from accounts.txt

================================================================================
                   TRANSACTION PROCESSING SYSTEM
================================================================================
1. Display All Accounts
2. Add New Account
3. Process Transaction (Deposit/Withdrawal)
4. Exit
================================================================================
Enter your choice (1-4):
```

---

## Menu Options Explained

### Option 1: Display All Accounts
Displays a formatted table of all accounts with:
- Account number
- Account holder name
- Current balance
- Account status

### Option 2: Add New Account
Prompts you to:
1. Enter account holder's name
2. Enter initial balance
3. System automatically generates account number
4. Saves to `accounts.txt`

### Option 3: Process Transaction
Allows you to:
1. Select an account by number
2. Choose Deposit or Withdrawal
3. Enter the transaction amount
4. System validates and applies the transaction
5. Saves to `accounts.txt`

### Option 4: Exit
Closes the program. All changes are already saved to `accounts.txt`.

---

## Sample Workflow

### 1. View Accounts
```
Enter your choice (1-4): 1

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

### 2. Make a Deposit
```
Enter your choice (1-4): 3

--- PROCESS TRANSACTION ---
Enter account number: 1001
Transaction Type:
  1 = Deposit
  2 = Withdrawal
Enter choice (1 or 2): 1
Enter amount ($): 500.00

[SUCCESS] Deposit completed:
  Amount deposited: $500.00
  Account: 1001 (Alice Johnson)
  New balance: $5500.50
[INFO] Saved 5 account(s) to accounts.txt
```

### 3. Check the Updated File
```bash
cat accounts.txt
```
Output will show the updated balance for Alice Johnson.

---

## Troubleshooting

### Compilation Errors

#### "gcc: command not found"
**Solution**: Install GCC compiler for your OS

#### "No such file or directory"
**Check**:
1. Verify you're in the directory containing `transaction_system.c`
2. Use `ls` (Linux/Mac) or `dir` (Windows) to list files

#### Syntax Error on Line X
**Solution**:
1. Check that you're using the provided source code
2. Modern versions of GCC should accept the code (C99 standard)
3. Recompile: `gcc -std=c99 -o transaction_system transaction_system.c`

### Runtime Errors

#### File Not Found: accounts.txt
**Why**: Program creates this file on first run when adding an account
**Fix**: Don't worry, just add a new account (Option 2)

#### "Cannot open accounts.txt for writing"
**Why**: File permissions or disk space issue
**Fix**: 
1. Check disk space: `df -h` (Linux/Mac) or `dir` (Windows)
2. Check permissions: Make sure file isn't read-only
3. Try running from a different directory with write permissions

#### "Transaction amount cannot be negative"
**Why**: You entered a negative amount (e.g., -500)
**Fix**: Enter positive amounts only

#### "Insufficient funds"
**Why**: Trying to withdraw more than account balance
**Fix**: The transaction is rejected to protect the account

### Features Not Working

#### Accounts saved, but don't appear on restart
**Why**: Maybe running from different directory?
**Check**: `accounts.txt` should be in same directory as executable

#### Input ignored or skipped
**Why**: Invalid input in previous operation corrupted the buffer
**Fix**: The program's `clearInputBuffer()` should handle this automatically

---

## File Structure

After compilation and first run, your directory should contain:

```
transaction_system.c          (Source code)
transaction_system.exe        (Compiled program)
accounts.txt                  (Data file - created after first save)
IMPLEMENTATION_GUIDE.md       (Detailed documentation)
QUICK_START.md               (This file)
Makefile                     (Optional - for make command)
```

---

## Verification Checklist

- [ ] GCC installed and working
- [ ] Successfully compiled without errors
- [ ] Executable runs (see startup banner)
- [ ] Menu displays correctly
- [ ] Can view accounts (Option 1)
- [ ] Can add an account (Option 2)
- [ ] Can make a deposit (Option 3)
- [ ] Can make a withdrawal (Option 3)
- [ ] File `accounts.txt` is created/updated
- [ ] Exiting and restarting shows saved accounts
- [ ] Negative deposits are rejected
- [ ] Overdraft attempts are rejected
- [ ] Inactive accounts block transactions

---

## Common Modifications (For Students)

### Change Transaction Limit
In `transaction_system.c`, line ~30:
```c
#define TRANSACTION_LIMIT 10000.0  /* Change this value */
```

### Change Maximum Accounts
In `transaction_system.c`, line ~26:
```c
#define MAX_ACCOUNTS 100  /* Change this value */
```

### Change Name Length
In `transaction_system.c`, line ~28:
```c
#define NAME_LENGTH 50  /* Change this value */
```

### Modify Output Format
Each `printf()` call can be customized. For example, in `displayAllAccounts()`:
```c
printf("%-10d %-30s %10.2f %10s\n",  /* Adjust column widths */
```

---

## Performance Notes

- **Fast**: All operations in memory (no disk I/O per transaction)
- **Scalable**: Handles 100 accounts instantly
- **Responsive**: Menu navigation is immediate
- **Persistent**: File I/O ensures no data loss

---

## Security Notes

- ✓ Buffer overflow protection (fixed-size strings)
- ✓ Input validation (range checks, type checks)
- ✓ Transaction validation (prevents overdrafts)
- ⚠️ No password protection (learn from this for production)
- ⚠️ No encryption (learn from this for production)

---

## Next Steps for Learning

1. **Read** `IMPLEMENTATION_GUIDE.md` for detailed design explanation
2. **Modify** the source code to add new features
3. **Test** edge cases to understand error handling
4. **Debug** by adding `printf()` statements to trace execution
5. **Extend** with new transaction types or account features

---

## Support & Questions

For issues or questions:
1. Check error messages carefully
2. Review `IMPLEMENTATION_GUIDE.md` comments
3. Examine the test cases section in the guide
4. Verify your input matches expected formats

---

**Happy Banking! 🏦**
