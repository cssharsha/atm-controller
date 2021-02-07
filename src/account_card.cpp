#include <glog/logging.h>

#include <account_card.hpp>

namespace banking {

  Account::Account(long account_id,
      const std::string &holder_name,
      int amount) :
    id_(account_id),
    name_(holder_name),
    money_(amount) {
      DLOG(INFO) << "Created account: " << id_ << " " << name_ << " " << money_;
    }

  bool Account::performTransaction(const TransactionType &trans_type, int &amount) {
    switch (trans_type) {
      case DEPOSIT: money_ += amount;
                    return true;
      case WITHDRAW: if (money_ >= amount) {
                       money_ -= amount;
                       return true;
                     }
                     return false;
      case CHECK_BALANCE: amount = money_;
                          return false;
    }
  }

  Card::Card(long card_no) : number_(card_no), pin_(8888) {
    DLOG(INFO) << "Creating card " << number_ << " " << pin_;
  }

  bool Card::verifyCard(int card_pin) {
    if (pin_ != card_pin) {
      return false;
    }
    return true;
  }

  bool Card::set_account_callback(acc_cb_t &acc_cb) {
    if (acc_cb_ != nullptr) {
      acc_cb_ = nullptr;
      return false;
    }
    acc_cb_ = acc_cb;
    return true;
  }
}
