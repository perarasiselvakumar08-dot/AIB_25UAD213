from flask import Flask, render_template, request
import os

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_FILE = os.path.join(BASE_DIR, "accounts.txt")

app = Flask(__name__)
app.config["TEMPLATES_AUTO_RELOAD"] = True

MIN_BALANCE = 0.0
MAX_TRANSACTION = 10000.0


def load_accounts():
    accounts = []
    if not os.path.exists(DATA_FILE):
        return accounts

    with open(DATA_FILE, "r", encoding="utf-8") as file:
        for raw_line in file:
            line = raw_line.strip()
            if not line:
                continue
            parts = line.split("|")
            if len(parts) != 4:
                continue
            try:
                account_number = int(parts[0])
                name = parts[1].strip()
                balance = float(parts[2])
                status = parts[3].strip()
            except ValueError:
                continue
            if status not in ["Active", "Inactive"]:
                continue
            accounts.append({
                "account_number": account_number,
                "name": name,
                "balance": balance,
                "status": status,
            })
    return accounts


def save_accounts(accounts):
    with open(DATA_FILE, "w", encoding="utf-8") as file:
        for account in accounts:
            file.write(f"{account['account_number']}|{account['name']}|{account['balance']:.2f}|{account['status']}\n")


def get_next_account_number(accounts):
    if not accounts:
        return 1001
    return max(acc["account_number"] for acc in accounts) + 1


def find_account(accounts, account_number):
    return next((acc for acc in accounts if acc["account_number"] == account_number), None)


def validate_amount(amount_text):
    try:
        amount = float(amount_text)
    except (ValueError, TypeError):
        return None, "Please enter a valid numeric amount."
    if amount <= 0:
        return None, "Amount must be greater than zero."
    if amount > MAX_TRANSACTION:
        return None, f"Amount must not exceed ${MAX_TRANSACTION:.2f}."
    return amount, None


@app.route("/", methods=["GET"])
def index():
    accounts = load_accounts()
    total_balance = sum(acc["balance"] for acc in accounts)
    return render_template(
        "index.html",
        accounts=accounts,
        total_balance=total_balance,
        message=None,
        error=None,
    )


@app.route("/add", methods=["POST"])
def add_account():
    accounts = load_accounts()
    name = request.form.get("name", "").strip()
    balance_text = request.form.get("balance", "").strip()

    if not name:
        return render_template(
            "index.html",
            accounts=accounts,
            total_balance=sum(acc["balance"] for acc in accounts),
            message=None,
            error="Account holder name is required.",
        )

    amount, err = validate_amount(balance_text)
    if err:
        return render_template(
            "index.html",
            accounts=accounts,
            total_balance=sum(acc["balance"] for acc in accounts),
            message=None,
            error=err,
        )

    new_account = {
        "account_number": get_next_account_number(accounts),
        "name": name,
        "balance": amount,
        "status": "Active",
    }
    accounts.append(new_account)
    save_accounts(accounts)

    return render_template(
        "index.html",
        accounts=accounts,
        total_balance=sum(acc["balance"] for acc in accounts),
        message=f"Account {new_account['account_number']} created successfully.",
        error=None,
    )


@app.route("/transaction", methods=["POST"])
def transaction():
    accounts = load_accounts()
    account_number_text = request.form.get("account_number", "").strip()
    operation = request.form.get("operation", "deposit")
    amount_text = request.form.get("amount", "").strip()

    try:
        account_number = int(account_number_text)
    except ValueError:
        return render_template(
            "index.html",
            accounts=accounts,
            total_balance=sum(acc["balance"] for acc in accounts),
            message=None,
            error="Please enter a valid account number.",
        )

    account = find_account(accounts, account_number)
    if account is None:
        return render_template(
            "index.html",
            accounts=accounts,
            total_balance=sum(acc["balance"] for acc in accounts),
            message=None,
            error=f"Account {account_number} not found.",
        )

    if account["status"] != "Active":
        return render_template(
            "index.html",
            accounts=accounts,
            total_balance=sum(acc["balance"] for acc in accounts),
            message=None,
            error=f"Account {account_number} is inactive. Transactions are blocked.",
        )

    amount, err = validate_amount(amount_text)
    if err:
        return render_template(
            "index.html",
            accounts=accounts,
            total_balance=sum(acc["balance"] for acc in accounts),
            message=None,
            error=err,
        )

    if operation == "withdraw":
        if account["balance"] - amount < MIN_BALANCE:
            return render_template(
                "index.html",
                accounts=accounts,
                total_balance=sum(acc["balance"] for acc in accounts),
                message=None,
                error=f"Insufficient funds. Current balance: ${account['balance']:.2f}.",
            )
        account["balance"] -= amount
        save_accounts(accounts)
        return render_template(
            "index.html",
            accounts=accounts,
            total_balance=sum(acc["balance"] for acc in accounts),
            message=f"Withdrawal of ${amount:.2f} from account {account_number} completed.",
            error=None,
        )

    account["balance"] += amount
    save_accounts(accounts)
    return render_template(
        "index.html",
        accounts=accounts,
        total_balance=sum(acc["balance"] for acc in accounts),
        message=f"Deposit of ${amount:.2f} to account {account_number} completed.",
        error=None,
    )


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)
