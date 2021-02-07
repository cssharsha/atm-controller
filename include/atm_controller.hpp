#pragma once

#include <bank.hpp>

namespace banking {

  /**
   * struct Transaction - Holds current transaction details
   */
  struct Transaction {
    Transaction() : token_(-1) {  }
    Transaction(int token) : token_(token) {}

    int token_;
  };

  class AtmController {
    public:
      /**
       * @brief Constructor
       *
       */
      AtmController();

      /**
       * @brief API to call when card is inserted
       *
       * @param card_no
       * @param card_pin
       */
      void insertCard(long card_no, int card_pin);

      /**
       * @brief Select account
       *
       * @param account_no
       */
      void selectAccount(long account_no);

      /**
       * @brief Callback function that bank calls
       *
       * @param atm_operation
       * @param info
       * @param display_msg
       * @return true/false
       */
      virtual bool controllerDisplay(AtmOperationType atm_op,
          int info,
          std::string&& display_msg);

    private:
      int atm_id_;
      Transaction transaction_;
  };
}
