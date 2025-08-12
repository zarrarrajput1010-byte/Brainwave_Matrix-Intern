#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <limits>

using namespace std;

struct Transaction {
	string type;
	double amount;
	string timestamp;
	double balanceAfter;
	
	Transaction(string t, double amt, string ts, double bal)
		: type(t), amount(amt), timestamp(ts), balanceAfter(bal) {}
};

class Account {
private:
	string accountNumber;
	string pin;
	double balance;
	string holderName;
	vector<Transaction> transactions;
	int failedAttempts;
	bool isLocked;
	
public:
	Account(string accNum, string p, double bal, string name)
		: accountNumber(accNum), pin(p), balance(bal), holderName(name),
		  failedAttempts(0), isLocked(false) {}
		  
	//Getters
	string getAccountNumber() const { return accountNumber; }
	string getHolderName() const { return holderName; }
	double getBalance() const { return balance; }
	bool getIsLocked() const { return isLocked; }
	int getFailedAttempts() const { return failedAttempts; }
	
	//pin verification
	bool verifyPin(const string& inputPin) {
		if (isLocked) return false;
		
		if (pin == inputPin) {
			failedAttempts = 0;
			return true;
		}
		else {
			failedAttempts++;
			if (failedAttempts >= 3) {
				isLocked = true;
			}
			return false;
		}
	}
	
	//transaction methods
	bool withdraw(double amount) {
		if (amount <= 0 || amount > balance || amount > 5000) {
			return false;
		}
		balance -= amount;
		addTransaction("WITHDRAWAL", amount);
		return true;
	}
	
	void deposit(double amount) {
		if (amount > 0) {
			balance += amount;
			addTransaction("Deposit", amount);
		}
	}
	
	bool transfer(double amount, Account& toAccount) {
		if (amount <= 0 || amount > balance) {
			return false;
		}
		balance -= amount;
		toAccount.balance += amount;
		addTransaction("TRANSFER OUT", amount);
		toAccount.addTransaction("TRANSFER IN", amount);
		return true;
	}
	
	//transaction history
	void addTransaction(const string& type, double amount) {
		time_t now = time(0);
		char* timeStr = ctime(&now);
		string timestamp(timeStr);
		timestamp.pop_back(); // remove newline
		transactions.push_back(Transaction(type, amount, timestamp, balance));
		
		//keep only last 10 transactions
		if (transactions.size() > 10) {
			transactions.erase(transactions.begin());
		}
	}	
	
	void printTransactionHistory() const {
		if (transactions.empty()) {
			cout << "No transaction history available.\n";
			return;
		}
		
		cout << "\n" << string(80, '=') << "\n";
		cout << "             TRANSACTION HISTORY\n";
		cout << string(80, '=') << "\n";
		cout << left << setw(15) << "TYPE"
			 << setw(12) << "AMOUNT"
			 << setw(15) << "BALANCE"
			 << "TIMESTAMP\n";
		cout << string(80, '-') << "\n";
		
		for (const auto& trans : transactions) {
			cout << left << setw(15) << trans.type
				 << "$" << setw(11) << fixed << setprecision(2) << trans.amount
				 << "$" << setw(14) << fixed << setprecision(2) << trans.balanceAfter
				 << trans.timestamp << "\n";
		}
		cout << string(80, '=') << "\n";
	}
	
	// change pin
	bool changePin(const string& newPin) {
		if (newPin.length() == 4 && newPin.find_first_not_of("0123456789") == string::npos) {
			pin = newPin;
			addTransaction("PIN CHANGE", 0);
			return true;
		}
		return false;
	}
	
	// unlockAccount
	void unlockAccount() {
		isLocked = false;
		failedAttempts = 0;
	}
};

//atm class - main interface
class ATM {
private:
	map<string, Account> accounts;
	Account* currentAccount;
	
	void initializeAccounts() {
		accounts.insert({"1001", Account("1001", "1234", 1000.00, "Dhoni")});
		accounts.insert({"1002", Account("1002", "5678", 4500.00, "Zarri")});
		accounts.insert({"1003", Account("1003", "9988", 11500.00, "Burak")});
		accounts.insert({"1004", Account("1004", "0101", 15000.00, "Virat")});
	}
	
	void clearScreen() {
		#ifdef _WIN32
			system("cls");
		#else
			system("clear");
		#endif
	}
	
	void printHeader() {
		cout << "\n" << string(50, '=') << "\n";
		cout << "          WELCOME TO ATM\n";
		cout << string(50, '=') << "\n";
	}
	
	void printAccountInfo() {
	if (currentAccount) {
		cout << "\nAccount Holder: " << currentAccount->getHolderName() << "\n";
		cout << "Account Number: " << currentAccount->getAccountNumber() << "\n";
		cout << "Current Balance: $" << fixed << setprecision(2)
			 << currentAccount->getBalance() << "\n";
		}
	}
	
	void waitForEnter() {
		cout << "\nPress Enter to continue...";
		cin.ignore();
		cin.get();
	}
	
	double getAmountInput() {
		double amount;
		while (true) {
			cout << "Enter amount: $";
			if (cin >> amount && amount > 0) {
				cin.ignore();
				return amount;
			}
			else {
				cout << "Invalid amount. Please enter a positive number.\n";
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
			}
		}
	}
	
	string getStringInput(const string& prompt) {
		string input;
		cout << prompt;
		cin >> input;
		return input;
	}
	
public:
	ATM() : currentAccount(nullptr) {
		initializeAccounts();
	}
	
	bool login() {
		clearScreen();
		printHeader();
		
		string accNum = getStringInput("Enter Account Number: ");
		
		auto it = accounts.find(accNum);
		if (it == accounts.end()) {
			cout << "Accounts not found!\n";
			waitForEnter();
			return false;
		}
		
		Account& account = it->second;
		
		if (account.getIsLocked()) {
			cout << "Account is locked due to multiple failed attempts!\n";
			cout << "Please contact customer service.\n";
			waitForEnter();
			return false;
		}
		
		string pin = getStringInput("Enter PIN: ");
		
		if (account.verifyPin(pin)) {
			currentAccount = &account;
			cout << "Login successful!\n";
			waitForEnter();
			return true;
		}
		else { 
			cout << "Invalid PIN!\n";
			if (account.getFailedAttempts() >= 3) {
				cout << "Account locked due to 3 failed attempts\n";
			}
		else{
			cout << "Attempts remaining: " << (3 - account.getFailedAttempts()) << "\n";
		}
			waitForEnter();
				return false;
			}	
		}
		
		void showMainMenu() {
			while(currentAccount) {
				clearScreen();
				printHeader();
				printAccountInfo();
				
				cout << "\n" << string(40, '-') << "\n";
				cout << "            MAIN MENU\n";
				cout << string(40, '-') << "\n";
				cout << "1. Balance Inquiry\n";
				cout << "2. Cash Withdrawal\n";
				cout << "3. Cash Deposit\n";
				cout << "4. Fund Transfer\n";
				cout << "5. Transaction History\n";
				cout << "6. Change PIN\n";
				cout << "7. Logout\n";
				cout << string(40, '-') << "\n";
				
				int choice;
				cout << "Select option (1-7): ";
				
				if (!(cin >> choice)) {
					cin.clear();
					cin.ignore(numeric_limits<streamsize>::max(), '\n');
					cout << "Invalid input! Please enter a number.\n";
					waitForEnter();
					continue;
				}
				cin.ignore(); //clear input buffer
				
				switch (choice) {
					case 1: balanceInquiry();
						break;
					case 2: withdrawal(); 
						break;
					case 3: deposit(); 
						break;
					case 4: transfer(); 
						break;
					case 5: transactionHistory(); 
						break;
					case 6: changePin(); 
						break;
					case 7: logout(); 
						break;
					default:
						cout << "Invalid option! Please select 1-7.\n";
						waitForEnter();
				}
			}
		}
		
void balanceInquiry() {
		clearScreen();
		printHeader();
		cout << "\n" << string(40, '=') << "\n";
		cout << "          BALANCE INQUIRY\n";
		cout << string(40, '-') << "\n";
		cout << "Account Holder: " << currentAccount->getHolderName() << "\n";
		cout << "Account Number: " << currentAccount->getAccountNumber() << "\n";
		cout << "AvailableBalance: $" << fixed << setprecision(2)
			 << currentAccount->getBalance() << "\n";
		cout << string(40, '-') << "\n";
		waitForEnter();
	}
	
	void withdrawal() {
		clearScreen();
		printHeader();
		cout << "\n" << string(40, '-') << "\n";
		cout << "         CASH WITHDRAWAL\n";
		cout << string(40, '-') << "\n";
		cout << "Available Balance : $" << fixed << setprecision(2)
			 << currentAccount->getBalance() << "\n";
		cout << "Daily withdrawal limit: $5,000\n";
		cout << string(40, '-') << "\n";
		
		double amount = getAmountInput();
		
		if (amount > currentAccount->getBalance()) {
			cout << "Insufficiant funds!\n";
		}
		else if (amount > 5000) {
			cout << "Amount exceeds daily withdrawal limit of $5,000!\n";
		}
		else if (currentAccount->withdraw(amount)) {
			cout << "Transaction successful!\n";
			cout << "Amount withdrawn: $" << fixed << setprecision(2) << amount << "\n";
			cout << "Remaining balance: $" << fixed << setprecision(2)
				 << currentAccount->getBalance() << "\n";
			}
			else {
				cout << "Transaction failed!\n";
			}
			
			waitForEnter();
		}
		
		void deposit() {
			clearScreen();
			printHeader();
			cout << "\n" << string(40, '-') << "\n";
			cout << "          CASH DEPOSIT\n";
			cout << string(40, '-') << "\n";
			cout << "current Balance: $" << fixed << setprecision(2)
				 << currentAccount->getBalance() << "\n";
			cout << string(40, '-') << "\n";
			
			double amount = getAmountInput();
			
			if (amount > 0) {
				currentAccount->deposit(amount);
				cout << "Transaction successful!\n";
				cout << "Amount deposited: $" << fixed << setprecision(2) << amount <<"\n";
				cout << "New balance: $" << fixed << setprecision(2)
					 << currentAccount->getBalance() << "\n";
			}
		else{
			cout << "Invalid amount!\n";
		}
			
		waitForEnter();
	}
		
	void transfer() {
	clearScreen();
	printHeader();
	cout << "\n" << string(40, '-') << "\n";
	cout << "          FUND TRANSFER\n";
	cout << string(40, '-') << "\n";
	cout << "Available Balance: $" << fixed << setprecision(2)
		 << currentAccount->getBalance() << "\n";
	cout << string(40, '-') << "\n";
	
	string toAccNum = getStringInput("Enter recipent account number: ");
	
	auto it = accounts.find(toAccNum);
	if (it == accounts.end()) {
		cout << "Recipent account not found!\n";
		waitForEnter();
		return;
	}
	
	if (toAccNum == currentAccount->getAccountNumber()) {
		cout << "Cannot transfer to the same account!\n";
		waitForEnter();
		return;
	}
	
	Account& toAccount = it->second;
	cout << "Recipent: " << toAccount.getHolderName() << "\n";
	
	double amount = getAmountInput();
	
	if (currentAccount->transfer(amount, toAccount)) {
		cout << "Transfer successful!\n";
		cout << "Amount transferred: $" << fixed << setprecision(2) << amount << "\n";
		cout << "To: " << toAccount.getHolderName() << "\n";
		cout << "Remaining balance: $" << fixed << setprecision(2)
			 << currentAccount->getBalance() << "\n";
	}
	else {
		cout << "Transfer failed! Insufficient funds or invalid amount.\n";
	}
	
	waitForEnter();
}

	void transactionHistory() {
		clearScreen();
		printHeader();
		currentAccount->printTransactionHistory();
		waitForEnter();
	}
	
	void changePin() {
		clearScreen();
		printHeader();
		cout << "\n" << string(40, '-') << "\n";
		cout << "           CHANGE PIN\n";
		cout << string(40, '-') << "\n";
		
		string currentPin = getStringInput("Enter current PIN: ");
		
		if (!currentAccount->verifyPin(currentPin)) {
			cout << "Invalid current PIN!\n";
			waitForEnter();
			return;
		}
		
		string newPin = getStringInput("Enter new 4-digit PIN: ");
		string confirmPin = getStringInput("Confirm new PIN: ");
		
		if (newPin != confirmPin) {
			cout << "PIN do not match!\n";
		}
		else if (currentAccount->changePin(newPin)) {
			cout << "PIN changed successfully!\n";
		}
		else {
			cout << "Invalid PIN format! PIN must be 4 digits.\n";
		}
		
		waitForEnter();
	}
	
	void logout() {
		currentAccount = nullptr;
		clearScreen();
		cout << "\nThank you for using ATM!\n";
		cout << "Have a great day!\n\n";
	}
	
	void run() {
		while (true) {
			if (login()) {
				showMainMenu();
			}
			
			cout << "Would you like to perform another transaction? (y/n): ";
			char choice;
			cin.ignore();
			
			if (choice != 'y' && choice != 'Y') {
				break;
			}
		}
		
		clearScreen();
		cout << "\n" << string(50, '-') << "\n";
		cout << "         Thank you for using ATM!\n";
		cout << "            Have a great day!\n";
		cout << string(50, '-') << "\n\n";
	}
};

//main function
int main() {
	cout << "==================================================\n";
	cout << "                    ATM SYSTEM\n";
	cout << "==================================================\n";
	cout << "\nDemo Accounts Available:\n";
	cout << "Account: 1001, PIN: 1234, Balance: $1000.00\n";
	cout << "Account: 1002, PIN: 5678, Balance: $4500.00\n";
	cout << "Account: 1003, PIN: 9988, Balance: $11500.00\n";
	cout << "Account: 1004, PIN: 0101, Balance: $15000.00\n";
	cout << "==================================================\n";
	cout << "\nPress Enter to continue...";
	cin.get();
	
	ATM atm;
	atm.run();
	
	return 0;
}
		