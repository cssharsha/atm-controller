#include <exception>
#include <glog/logging.h>

#include <bank.hpp>
#include <stdexcept>
#include <string>

namespace banking {
  Bank *Bank::bank_;

  void Bank::init() {
    LOG(INFO) << "Initializing bank!";
    std::lock_guard<std::mutex> lck1(account_id_mutex_);
    available_account_ids_.resize(ACCOUNTS_CARDS_UL);
    std::iota(available_account_ids_.begin(), available_account_ids_.end(), 0);

    std::lock_guard<std::mutex> lck2(card_id_mutex_);
    available_card_ids_.resize(ACCOUNTS_CARDS_UL);
    std::iota(available_card_ids_.begin(), available_card_ids_.end(), 0);

    std::lock_guard<std::mutex> lck3(t_token_mutex_);
    available_transaction_tokens_.resize(ACCOUNTS_CARDS_UL);
    std::iota(available_transaction_tokens_.begin(), available_transaction_tokens_.end(), 0);

    std::lock_guard<std::mutex> lck4(atm_id_mutex_);
    available_atm_ids_.resize(ACCOUNTS_CARDS_UL);
    std::iota(available_atm_ids_.begin(), available_atm_ids_.end(), 0);
  }

  Bank::~Bank() {
    std::lock_guard<std::mutex> lck1(account_id_mutex_);
    available_account_ids_.clear();

    std::lock_guard<std::mutex> lck2(card_id_mutex_);
    available_card_ids_.clear();

    std::lock_guard<std::mutex> lck3(t_token_mutex_);
    available_transaction_tokens_.clear();

    std::lock_guard<std::mutex> lck4(atm_id_mutex_);
    available_atm_ids_.clear();

    std::lock_guard<std::mutex> lc5(account_cards_mutex_);
    account_cards_map_.clear();

    std::lock_guard<std::mutex> lc6(transaction_map_mutex_);
    transaction_map_.clear();

    std::lock_guard<std::mutex> lc7(atm_cb_mutex_);
    atm_cb_map_.clear();
  }

  void Bank::createAndLinkAccount(const std::string &holder_name, int amount, long card_no) {
    AccountPtr account = std::make_shared<Account>(getRandomId<long>(account_id_mutex_, available_account_ids_),
        holder_name, amount);
    LOG(INFO) << "Creating card and account for " << holder_name << " " << amount;
    account_card_pair_t account_card_pair;
    if (card_no >= 0) {
      if(findInMap<long, account_card_pair_t>(account_cards_mutex_, account_cards_map_,
            card_no, account_card_pair)) {
        LOG(ERROR) << "Card number is not present";
        throw std::runtime_error("Illegal card access");
      }
      account_card_pair.second.push_back(account);
      if (!changeValueInMap<long, account_card_pair_t>(account_cards_mutex_, account_cards_map_,
            card_no, account_card_pair))
      {
        LOG(ERROR) << "Unable to update the link";
        throw std::runtime_error("Corrupted card access");
      }
      return;
    }

    try {
      CardPtr card = std::make_shared<Card>(getRandomId<long>(card_id_mutex_, available_card_ids_));
      std::vector<AccountPtr> cur_card_accounts;
      cur_card_accounts.push_back(account);
      account_card_pair_t account_card_pair = std::pair<CardPtr, std::vector<AccountPtr>>(card, cur_card_accounts);
      addToMap<long, account_card_pair_t>(account_cards_mutex_, account_cards_map_,
          card->get_number(), account_card_pair);
    } catch (std::exception &ex) {
      LOG(ERROR) << "Unable to create an entry for account and card for user: " << holder_name;
      throw ex;
    }
  }

  int Bank::get_atm_id() {
    return getRandomId<int>(atm_id_mutex_, available_atm_ids_);
  }

  int Bank::verifyAndCreateTransaction(long card_no, int card_pin) {
    account_card_pair_t acit;
    DLOG(INFO) << "Current card transaction: " << card_no << " " << card_pin;
    if (!findInMap<long, account_card_pair_t>(account_cards_mutex_, account_cards_map_, card_no, acit)) {
      LOG(ERROR) << "Unable to find card number!";
      throw std::runtime_error("Illegal card access");
    }

    if (!acit.first->verifyCard(card_pin)) {
      LOG(ERROR) << "Incorrect card details!";
      throw std::runtime_error("Illegal card access");
    }

    int transaction_token = getRandomId<int>(t_token_mutex_, available_transaction_tokens_);

    try {
      addToMap<int, long>(transaction_map_mutex_, transaction_map_,
          transaction_token, card_no);
    } catch (std::exception &ex) {
      LOG(ERROR) << "Unable to initialize a transaction";
      throw std::runtime_error("Transaction initialization error");
    }
    return transaction_token;
  }

  void Bank::acknowledgeTransaction(int transaction_token, atm_cb_t atm_cb) {
    long card_no;
    if (findInMap<int, long>(transaction_map_mutex_, transaction_map_, transaction_token, card_no)) {
      try{
        addToMap<int, atm_cb_t>(atm_cb_mutex_, atm_cb_map_, transaction_token, atm_cb);
      } catch (std::exception &ex) {
        throw ex;
      }
    } else {
      throw std::runtime_error("Unable to find transaction");
    }

    std::string accounts = "";
    account_card_pair_t account_card_pair;
    if (findInMap<long, account_card_pair_t>(account_cards_mutex_, account_cards_map_, card_no, account_card_pair)) {
      for (const auto &acc : account_card_pair.second) {
        accounts = accounts + std::to_string(acc->get_id()) + std::string(" ");
      }
      atm_cb(SHOW_INPUT, (int)account_card_pair.second.size(), std::string(accounts));
    } else {
      atm_cb(SHOW_ERROR, -1, "Unable to find accounts");
    }
  }

  void Bank::selectAccount(int transaction_token, long account_no) {
    LOG(INFO) << "Selected account: " << transaction_token << " " << account_no;
    long card_no;
    atm_cb_t atm_cb;
    if (!findInMap<int, long>(transaction_map_mutex_, transaction_map_, transaction_token, card_no)) {
      LOG(ERROR) << "Not found!!!!";
      throwSession(transaction_token);
      return;
    }

    account_card_pair_t account_card_pair;

    if (!findInMap<long, account_card_pair_t>(account_cards_mutex_,
          account_cards_map_, card_no,
          account_card_pair)) {
      LOG(ERROR) << "Not found!!!!";
      throwSession(transaction_token);
      return;
    }

    AccountPtr account;
    bool found = false;
    for (auto &acc : account_card_pair.second) {
      if (acc->get_id() == account_no) {
        found = true;
        account = acc;
        break;
      }
    }
    if (!found) {
      LOG(ERROR) << "Not found!!!!";
      throwSession(transaction_token);
      return;
    }

    acc_cb_t f = std::bind(&Account::performTransaction, account,
        std::placeholders::_1,
        std::placeholders::_2);
    account_card_pair.first->set_account_callback(f);
    LOG(INFO) << "Calling input";

    if (findInMap<int, atm_cb_t>(atm_cb_mutex_, atm_cb_map_, transaction_token, atm_cb))
      atm_cb(SHOW_INPUT, 1, "Select transaction type");
    else
      throwSession(transaction_token);
  }

  void Bank::throwSession(int token) {
    long card_no;
    if (findInMap<int, long>(transaction_map_mutex_, transaction_map_, token, card_no)) {
      account_card_pair_t account_card_pair;
      if (findInMap<long, account_card_pair_t>(account_cards_mutex_, account_cards_map_,
            card_no, account_card_pair)) {
        account_card_pair.first->reset_account_callback();
      }
    }
    deleteFromMap<int, long>(transaction_map_mutex_, transaction_map_, token);

    atm_cb_t atm_cb;
    bool found = findInMap<int, atm_cb_t>(atm_cb_mutex_, atm_cb_map_, token, atm_cb);

    deleteFromMap<int, atm_cb_t>(atm_cb_mutex_, atm_cb_map_, token);

    if (found)
      atm_cb(SHOW_ERROR, -1, "Corrupt transaction");
  }

  bool Bank::privilegedOperation(const int &passcode, const std::string &holder_name,
      std::vector<long> &account_no, long &card_no) {
    if (passcode != HIDDEN_PASSCODE) {
      LOG(ERROR) << "Unauthorized access";
      return false;
    }

    std::lock_guard<std::mutex> lck(account_cards_mutex_);
    std::map<long, account_card_pair_t>::iterator it = account_cards_map_.begin();
    bool found = false;
    for (;it != account_cards_map_.end();it++) {
      for (const auto &account:it->second.second) {
        if (account->get_name() == holder_name) {
          LOG(INFO) << "Found " << holder_name;
          found = true;
          break;
        }
      }
      if (found)
        break;
    }

    if (found) {
      LOG(INFO) << "Number of accounts: " << it->second.second.size();
      for (const auto &account:it->second.second) {
        LOG(INFO) << account->get_id();
        account_no.push_back(account->get_id());
      }
      card_no = it->first;
    }
    return found;
  }
}
