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

  Card::Card(long card_no) : number_(card_no), pin_(8888) {
    DLOG(INFO) << "Creating card " << number_ << " " << pin_;
  }

  bool Card::verifyCard(int card_pin) {
    if (pin_ != card_pin) {
      return false;
    }
    return true;
  }
}
