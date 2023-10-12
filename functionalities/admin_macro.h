#ifndef ADMIN_CONSTANTS
#define ADMIN_CONSTANTS

// // GET ACCOUNT DETAILS
// #define GET_ACCOUNT_NUMBER "Enter the account number of the account you're searching for"
// #define GET_CUSTOMER_ID "Enter the customer ID of the customer you're searching for"

// #define INVALID_MENU_CHOICE "It seems you've made an invalid menu choice\nYou'll now be redirected to the main menu!^"

// #define CUSTOMER_ID_DOESNT_EXIT "No customer could be found for the given ID"
// #define CUSTOMER_LOGIN_ID_DOESNT_EXIT "No customer could be found for the given login ID$"

// #define ACCOUNT_ID_DOESNT_EXIT "No account could be found for the given account number"

// #define TRANSACTIONS_NOT_FOUND "No transactions were performed on this account by the customer!^"



// ADD ACCOUNT
#define ADMIN_ADD_ACCOUNT_TYPE "What type of account is to be created? Enter 1 for regular account and 2 for joint account"
#define ADMIN_ADD_ACCOUNT_NUMBER "The newly created account's number is :"

// DELETE ACCOUNT
#define ADMIN_DEL_ACCOUNT_NO "What is the account number of the account you want to delete?"
#define ADMIN_DEL_ACCOUNT_SUCCESS "This account has been successfully deleted\nRedirecting you to the main menu ...^"
#define ADMIN_DEL_ACCOUNT_FAILURE "This account cannot be deleted since it still has some money\nRedirecting you to the main menu ...^"

// MODIFY CUSTOMER INFO
#define ADMIN_MOD_CUSTOMER_ID "Enter the ID of the customer who's information you want to edit"
#define ADMIN_MOD_CUSTOMER_MENU "Which information would you like to modify?\n1. Name 2. Age 3. Gender \nPress any other key to cancel"
#define ADMIN_MOD_CUSTOMER_NEW_NAME "What's the updated value for name?"
#define ADMIN_MOD_CUSTOMER_NEW_GENDER "What's the updated value for gender?"
#define ADMIN_MOD_CUSTOMER_NEW_AGE "What's the updated value for age?"

#define ADMIN_MOD_CUSTOMER_SUCCESS "The required modification was successfully made!\nYou'll now be redirected to the main menu!^"

#define ADMIN_LOGOUT "Logging you out now superman! Good bye!$"

// ====================================================

// ========== CUSTOMER SPECIFIC TEXT===================

// LOGIN WELCOME
#define CUSTOMER_LOGIN_WELCOME "Welcome dear customer! Enter your credentials to gain access to your account!"
#define CUSTOMER_LOGIN_SUCCESS "Welcome beloved customer!"

#define CUSTOMER_LOGOUT "Logging you out now dear customer! Good bye!$"

// ADMIN MENU
#define CUSTOMER_MENU "1. Get Customer Details\n2. Deposit Money\n3. Withdraw Money\n4. Get Balance\n5. Get Transaction information\n6. Change Password\nPress any other key to logout"

#define ACCOUNT_DEACTIVATED "It seems your account has been deactivated!^"

#define DEPOSIT_AMOUNT "How much is it that you want to add into your bank?"
#define DEPOSIT_AMOUNT_INVALID "You seem to have passed an invalid amount!^"
#define DEPOSIT_AMOUNT_SUCCESS "The specified amount has been successfully added to your bank account!^"

#define WITHDRAW_AMOUNT "How much is it that you want to withdraw from your bank?"
#define WITHDRAW_AMOUNT_INVALID "You seem to have either passed an invalid amount or you don't have enough money in your bank to withdraw the specified amount^"
#define WITHDRAW_AMOUNT_SUCCESS "The specified amount has been successfully withdrawn from your bank account!^"

#define PASSWORD_CHANGE_OLD_PASS "Enter your old password"
#define PASSWORD_CHANGE_OLD_PASS_INVALID "The entered password doesn't seem to match with the old password"
#define PASSWORD_CHANGE_NEW_PASS "Enter the new password"
#define PASSWORD_CHANGE_NEW_PASS_RE "Reenter the new password"
#define PASSWORD_CHANGE_NEW_PASS_INVALID "The new password and the reentered passwords don't seem to pass!^"
#define PASSWORD_CHANGE_SUCCESS "Password successfully changed!^"

// ====================================================


// #define ACCOUNT_FILE "./records/account.bank"
// #define CUSTOMER_FILE "./records/customer.bank"
// #define TRANSACTION_FILE "./records/transactions.bank"

#endif