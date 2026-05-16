# Testing Guide - Transaction Processing System

## Overview

This comprehensive testing guide ensures all features work correctly and edge cases are properly handled. Each test includes:
- **Input**: Exact values to enter
- **Expected Output**: What should happen
- **Validation**: How to verify success
- **Edge Case**: The scenario being tested

---

## Test Suite Statistics

- **Total Tests**: 15
- **Core Features**: 3 (Display, Add, Transact)
- **Edge Cases**: 8 (Negative, Overdraft, Inactive, etc.)
- **Error Cases**: 4 (Invalid input, missing account, etc.)
- **Estimated Duration**: 30-45 minutes

---

## Pre-Test Setup

1. **Compile**: `gcc -std=c99 -Wall -pedantic -o transaction_system transaction_system.c`
2. **Verify**: Check that `accounts.txt` exists with sample data
3. **Clean State**: Delete `accounts.txt` and recompile to test fresh start

---

## TEST GROUP 1: Basic Display & Navigation

### Test 1.1: Startup and Menu Display
**Purpose**: Verify program initializes correctly

**Steps**:
1. Run: `./transaction_system`

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

**Validation**: ✓ Banner displays, ✓ Accounts loaded, ✓ Menu shows all 4 options

---

### Test 1.2: Display All Accounts
**Purpose**: Verify table formatting and account data

**Steps**:
1. From menu: Enter `1`

**Expected Output**:
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

**Validation**: 
- ✓ All 5 accounts display
- ✓ Account numbers correct
- ✓ Names correctly aligned
- ✓ Balances showing 2 decimal places
- ✓ Status "Active" or "Inactive"
- ✓ Total balance calculated correctly ($5000.50 + $3250.75 + $1500.00 + $0.00 + $7890.25 = $17641.50)

---

### Test 1.3: Menu Loops Back
**Purpose**: Verify menu repeats after operations

**Steps**:
1. Enter `1` (Display)
2. Observe menu reappears

**Expected**: Menu displays again immediately

---

## TEST GROUP 2: Account Management (Add Account)

### Test 2.1: Add Valid Account
**Purpose**: Verify new account creation and persistence

**Steps**:
```
Choose: 2
Name: Grace Lee
Balance: 3500.00
```

**Expected Output**:
```
--- ADD NEW ACCOUNT ---
Enter account holder's name (max 49 chars): Grace Lee
Enter initial balance ($): 3500.00

[SUCCESS] Account created:
  Account Number: 1006
  Name: Grace Lee
  Balance: $3500.00
  Status: Active

[INFO] Saved 6 account(s) to accounts.txt
```

**Validation**:
- ✓ Account number auto-generated (1006)
- ✓ Status set to "Active"
- ✓ File saved (verify `accounts.txt` updated)
- ✓ Account appears in next "Display All"

---

### Test 2.2: Add Account with Boundary Name
**Purpose**: Test max name length (49 chars)

**Steps**:
```
Choose: 2
Name: [Enter 49 character name]
Balance: 1000.00
```

**Expected**:
```
[SUCCESS] Account created:
  Account Number: 1007
  Name: [Your 49-char name]
  ...
```

**Validation**: ✓ Name stored correctly without truncation

---

### Test 2.3: Add Account - Empty Name
**Purpose**: Verify empty name is rejected

**Steps**:
```
Choose: 2
Name: [Just press Enter]
```

**Expected Output**:
```
Enter account holder's name (max 49 chars):
[ERROR] Account name cannot be empty.
```

**Validation**: ✓ Error shown, ✓ No account created, ✓ Menu returns

---

### Test 2.4: Add Account - Negative Balance
**Purpose**: Verify negative initial balance is rejected

**Steps**:
```
Choose: 2
Name: Henry Jones
Balance: -500.00
```

**Expected Output**:
```
Enter account holder's name (max 49 chars): Henry Jones
Enter initial balance ($): -500.00
[ERROR] Initial balance must be >= $0 and <= $10000.00
```

**Validation**: ✓ No account created, ✓ Menu returns

---

### Test 2.5: Add Account - Excessive Balance
**Purpose**: Verify initial balance exceeds transaction limit

**Steps**:
```
Choose: 2
Name: Isabella Clark
Balance: 15000.00
```

**Expected Output**:
```
Enter initial balance ($): 15000.00
[ERROR] Initial balance must be >= $0 and <= $10000.00
```

**Validation**: ✓ No account created, ✓ Menu returns

---

## TEST GROUP 3: Transactions (Deposits)

### Test 3.1: Valid Deposit
**Purpose**: Verify successful deposit and persistence

**Steps**:
```
Choose: 3
Account: 1001
Type: 1 (Deposit)
Amount: 500.00
```

**Expected Output**:
```
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

**Validation**:
- ✓ Balance increased ($5000.50 → $5500.50)
- ✓ File updated (check `accounts.txt`)
- ✓ Next "Display All" shows new balance

---

### Test 3.2: Multiple Deposits
**Purpose**: Verify accumulating deposits

**Steps**:
```
First:  3 → 1002 → 1 → 250.00
Second: 3 → 1002 → 1 → 100.00
Third:  3 → 1002 → 1 → 150.00
Then:   1 (Display)
```

**Expected**: Bob Williams balance should be:
- Start: $3250.75
- After 1st: $3500.75
- After 2nd: $3600.75
- After 3rd: $3750.75 ✓

---

### Test 3.3: Deposit - Negative Amount
**Purpose**: Verify negative deposits rejected

**Steps**:
```
Choose: 3
Account: 1001
Type: 1
Amount: -100.00
```

**Expected Output**:
```
Enter amount ($): -100.00
[ERROR] Transaction amount cannot be negative.
```

**Validation**: ✓ Balance unchanged, ✓ File unchanged

---

### Test 3.4: Deposit - Excessive Amount
**Purpose**: Verify deposit exceeding limit rejected

**Steps**:
```
Choose: 3
Account: 1001
Type: 1
Amount: 15000.00
```

**Expected Output**:
```
Enter amount ($): 15000.00
[ERROR] Amount exceeds transaction limit of $10000.00
```

**Validation**: ✓ Balance unchanged

---

## TEST GROUP 4: Transactions (Withdrawals)

### Test 4.1: Valid Withdrawal
**Purpose**: Verify successful withdrawal

**Steps**:
```
Choose: 3
Account: 1002
Type: 2 (Withdrawal)
Amount: 250.00
```

**Expected Output**:
```
--- PROCESS TRANSACTION ---
Enter account number: 1002
...
Enter choice (1 or 2): 2
Enter amount ($): 250.00

[SUCCESS] Withdrawal completed:
  Amount withdrawn: $250.00
  Account: 1002 (Bob Williams)
  New balance: $3000.75
[INFO] Saved 5 account(s) to accounts.txt
```

**Validation**:
- ✓ Balance decreased ($3250.75 → $3000.75)
- ✓ File updated

---

### Test 4.2: Withdrawal - Overdraft Protection
**Purpose**: Verify withdrawal exceeding balance rejected

**Steps**:
```
Choose: 3
Account: 1003
Type: 2
Amount: 2000.00
```

**Note**: Account 1003 (Carol Davis) has balance $1500.00

**Expected Output**:
```
Enter amount ($): 2000.00
[ERROR] Insufficient funds. Current balance: $1500.00
        Withdrawal request: $2000.00
        Would result in: -$500.00 (not allowed)
```

**Validation**: 
- ✓ Withdrawal REJECTED
- ✓ Balance unchanged ($1500.00)
- ✓ Account protected from overdraft

**CRITICAL**: This demonstrates the overdraft prevention edge case!

---

### Test 4.3: Withdrawal - Exact Balance
**Purpose**: Verify withdrawal of entire balance allowed

**Steps**:
```
Choose: 3
Account: 1003
Type: 2
Amount: 1500.00
```

**Expected**:
```
[SUCCESS] Withdrawal completed:
  Amount withdrawn: $1500.00
  Account: 1003 (Carol Davis)
  New balance: $0.00
```

**Validation**: ✓ Balance becomes $0.00 (minimum allowed)

---

### Test 4.4: Withdrawal from Zero Balance
**Purpose**: Verify zero balance prevents withdrawal

**Steps**:
```
Choose: 3
Account: 1003 (after previous test, now at $0.00)
Type: 2
Amount: 100.00
```

**Expected Output**:
```
[ERROR] Insufficient funds. Current balance: $0.00
        Withdrawal request: $100.00
        Would result in: -$100.00 (not allowed)
```

**Validation**: ✓ Transaction rejected

---

## TEST GROUP 5: Inactive Account Blocking

### Test 5.1: Deposit to Inactive Account
**Purpose**: Verify deposit blocked on inactive account

**Steps**:
```
Choose: 3
Account: 1004 (David Anderson - Inactive)
Type: 1
Amount: 100.00
```

**Expected Output**:
```
--- PROCESS TRANSACTION ---
Enter account number: 1004
...
[ERROR] Cannot process transactions on an Inactive account.
```

**Validation**: 
- ✓ Transaction blocked
- ✓ Balance unchanged ($0.00)
- ✓ Account status respected

---

### Test 5.2: Withdrawal from Inactive Account
**Purpose**: Verify withdrawal blocked on inactive account

**Steps**:
```
Choose: 3
Account: 1004
Type: 2
Amount: 100.00
```

**Expected Output**:
```
[ERROR] Cannot process transactions on an Inactive account.
```

**Validation**: ✓ Transaction blocked

**CRITICAL**: This demonstrates the account status policy enforcement!

---

## TEST GROUP 6: Error Handling & Validation

### Test 6.1: Non-Existent Account
**Purpose**: Verify transaction rejected for missing account

**Steps**:
```
Choose: 3
Account: 9999
Type: 1
Amount: 100.00
```

**Expected Output**:
```
--- PROCESS TRANSACTION ---
Enter account number: 9999
[ERROR] Account 9999 not found.
```

**Validation**: ✓ Transaction rejected immediately

---

### Test 6.2: Invalid Input - Non-Numeric Account
**Purpose**: Verify invalid account input handled

**Steps**:
```
Choose: 3
Account: abc [type "abc" instead of number]
```

**Expected Output**:
```
Enter account number: abc
[ERROR] Invalid account number entered.
```

**Validation**: ✓ Program doesn't crash

---

### Test 6.3: Invalid Input - Non-Numeric Amount
**Purpose**: Verify invalid amount input handled

**Steps**:
```
Choose: 3
Account: 1001
Type: 1
Amount: $500 [or "five hundred"]
```

**Expected Output**:
```
Enter amount ($): $500
[ERROR] Invalid amount entered.
```

**Validation**: ✓ Program recovers gracefully

---

### Test 6.4: Invalid Menu Choice
**Purpose**: Verify invalid menu selection handled

**Steps**:
```
Main Menu:
Enter your choice (1-4): 9
```

**Expected Output**:
```
[ERROR] Please enter a number between 1 and 4.

================================================================================
                   TRANSACTION PROCESSING SYSTEM
================================================================================
1. Display All Accounts
...
```

**Validation**: ✓ Menu retries, ✓ No crash

---

### Test 6.5: Zero Amount Transaction
**Purpose**: Verify zero amount rejected

**Steps**:
```
Choose: 3
Account: 1001
Type: 1
Amount: 0.00
```

**Expected Output**:
```
[ERROR] Transaction amount must be greater than zero.
```

**Validation**: ✓ Transaction rejected

---

## TEST GROUP 7: File Persistence

### Test 7.1: Changes Saved to File
**Purpose**: Verify accounts.txt is updated

**Steps**:
1. Add new account (Test 2.1)
2. Make deposit (Test 3.1)
3. Check `accounts.txt` file content

**Validation**:
```bash
cat accounts.txt
```
Should show:
- Grace Lee account (added in Test 2.1)
- Alice Johnson with updated balance

---

### Test 7.2: Fresh Start - Data Persists
**Purpose**: Verify accounts loaded on restart

**Steps**:
1. Run program with changes saved
2. Exit program: Choose option 4
3. Rerun program

**Expected**: All previous changes still present

**Validation**:
- ✓ Choose option 1 (Display All)
- ✓ Grace Lee account visible
- ✓ Alice Johnson balance shows updated amount

---

### Test 7.3: Fresh Database
**Purpose**: Verify program works with no accounts.txt

**Steps**:
1. Delete `accounts.txt` file
2. Recompile and run

**Expected Output**:
```
[INFO] accounts.txt not found. Starting with empty database.

================================================================================
                   TRANSACTION PROCESSING SYSTEM
================================================================================
1. Display All Accounts
...
```

**Optional Tests**:
- Display All → "No accounts found in database"
- Add Account → Successfully creates first account
- Verify new account saved

---

## TEST GROUP 8: Boundary Conditions

### Test 8.1: Maximum Accounts (100)
**Purpose**: Verify array capacity protection

**Steps** (Optional - time-consuming):
1. Create first 98 accounts programmatically (or manually if patient)
2. Try to add 101st account

**Expected**:
```
[ERROR] Account database is full (max: 100 accounts).
```

---

### Test 8.2: Maximum Name Length
**Purpose**: Verify 49-char name limit

**Steps**:
```
Choose: 2
Name: ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRST (49 chars)
Balance: 1000.00
```

**Expected**: ✓ Account created, name stored correctly

---

### Test 8.3: Minimum Balance
**Purpose**: Verify $0.00 is minimum balance

**Steps**:
```
Choose: 2
Name: John Smith
Balance: 0.00
```

**Expected**: ✓ Account created with $0.00 balance

---

### Test 8.4: Maximum Transaction
**Purpose**: Verify $10,000 transaction limit

**Steps**:
```
Choose: 3
Account: 1001
Type: 1
Amount: 10000.00
```

**Expected**: ✓ Deposit accepted (at limit)

---

## Summary Report Template

After running all tests, complete this checklist:

```
═══════════════════════════════════════════════════════════════════════
                    TEST RESULTS SUMMARY
═══════════════════════════════════════════════════════════════════════

FUNCTIONALITY TESTS:
[ ] Display All Accounts ........................ PASS / FAIL
[ ] Add New Account ............................. PASS / FAIL
[ ] Process Deposit ............................. PASS / FAIL
[ ] Process Withdrawal .......................... PASS / FAIL
[ ] Exit Program ............................... PASS / FAIL

VALIDATION TESTS:
[ ] Negative Deposits Rejected ................. PASS / FAIL
[ ] Overdraft Protection Active ............... PASS / FAIL
[ ] Inactive Accounts Blocked ................. PASS / FAIL
[ ] Invalid Account Rejected .................. PASS / FAIL
[ ] Zero Amount Rejected ....................... PASS / FAIL
[ ] Excessive Amount Rejected ................. PASS / FAIL
[ ] Negative Balance Rejected ................. PASS / FAIL
[ ] Excessive Balance Rejected ................ PASS / FAIL

PERSISTENCE TESTS:
[ ] New Accounts Saved ......................... PASS / FAIL
[ ] Transactions Saved ......................... PASS / FAIL
[ ] Data Loads on Restart ..................... PASS / FAIL
[ ] Fresh Database Handled .................... PASS / FAIL

ERROR HANDLING TESTS:
[ ] Invalid Menu Input Handled ................ PASS / FAIL
[ ] Invalid Account Input Handled ............ PASS / FAIL
[ ] Invalid Amount Input Handled ............. PASS / FAIL
[ ] Non-Existent Account Handled ............ PASS / FAIL

PERFORMANCE TESTS:
[ ] Program Starts Quickly .................... PASS / FAIL
[ ] Menu Responsive ........................... PASS / FAIL
[ ] No Lag in Operations ...................... PASS / FAIL

═══════════════════════════════════════════════════════════════════════
TOTAL: ___/28 TESTS PASSED

NOTES:
[Your observations here]

═══════════════════════════════════════════════════════════════════════
```

---

## Debugging Tips

### If Test Fails

1. **Check compilation**: Recompile with `-Wall -pedantic` to catch warnings
2. **Print state**: Add `printf()` statements to trace execution
3. **Review output**: Compare with expected output exactly
4. **Check file**: View `accounts.txt` after operations
5. **Test in isolation**: Test one feature at a time
6. **Read error messages**: They explain what went wrong

### Useful Debug Commands

```bash
# View file contents
cat accounts.txt

# Check file size
wc -l accounts.txt

# Clear for fresh test
rm accounts.txt

# Recompile with debugging
gcc -g -std=c99 -Wall -pedantic -o transaction_system transaction_system.c
```

---

## Conclusion

This test suite comprehensively validates:
- ✓ All core functionality
- ✓ Edge case handling
- ✓ Error conditions
- ✓ Data persistence
- ✓ Boundary conditions
- ✓ User input validation

A passing test suite indicates production-ready code!
