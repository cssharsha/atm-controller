#include <glog/logging.h>

#include <atm_controller.hpp>

namespace banking {

  AtmController::AtmController() {
    atm_id_ = Bank::getBank()->get_atm_id();
    transaction_ = std::make_unique<Transaction>();
  }

  void AtmController::insertCard(long card_no, int card_pin) {
    if (transaction_->token_ >= 0) {
      LOG(ERROR) << "A previous transaction already present";
      controllerDisplay(SHOW_ERROR, -1, "Transaction duplication");
      return;
    }

    int transaction_token;
    try {
      transaction_token = Bank::getBank()->verifyAndCreateTransaction(card_no,
          card_pin);
    } catch (std::exception &ex) {
      controllerDisplay(SHOW_ERROR, -1, ex.what());
      return;
    }

    auto f = std::bind(&AtmController::controllerDisplay, this,
        std::placeholders::_1,
        std::placeholders::_2,
        std::placeholders::_3);
    try {
      Bank::getBank()->acknowledgeTransaction(transaction_token, f);
    } catch (std::exception &ex) {
      LOG(ERROR) << "Error acknowledging transaction: " << ex.what();
      controllerDisplay(SHOW_ERROR, -1, ex.what());
    }
  }

  bool AtmController::controllerDisplay(AtmOperationType atm_op,
      int info, std::string &&display_msg) {
    LOG(INFO) << "Nothing doing right now: " << atm_op << ": " << info << " " << display_msg;
    return true;
  }

}
