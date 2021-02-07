#pragma once

#include <functional>

namespace banking {

  enum AtmOperationType {
    SHOW,
    SHOW_INPUT,
    SHOW_ERROR,
    TAKE,
    GIVE
  };

  enum TransactionType {
    DEPOSIT,
    WITHDRAW,
    CHECK_BALANCE
  };

  using acc_cb_t = std::function<bool(const TransactionType&, int&)>;

  class Account {
    public:
      /**
       * @brief Constructor
       *
       * @param account_id
       * @param holder_name
       * @param amount
       */
      Account (long account_id, const std::string &holder_name, int amount);

      /**
       * @brief Getter function for account name
       *
       */
      std::string get_name() { return name_; }

      /**
       * @brief Getter function for id
       *
       */
      long get_id() { return id_; }

      /**
       * @brief Perfrom the secified transaction type
       *
       * @param trans_type
       * @param amount
       * @return true/false
       */
      bool performTransaction(const TransactionType &trans_type, int &amount);

    private:
      long id_;
      std::string name_;
      int money_;
  };

  class Card {
    public:
      /**
       * @brief Constructor
       *
       * @param card_no
       */
      Card(long card_no);

      /**
       * @brief Verify if card pin is correct
       *
       * @param card_pin
       * @return true/false
       */
      bool verifyCard(int card_pin);

      /**
       * @brief Getter function for card number
       *
       */
      long& get_number() { return number_; }

      bool set_account_callback(acc_cb_t &acc_b);

      void reset_account_callback() { acc_cb_ = nullptr; }

    private:
      long number_;
      int pin_;
      acc_cb_t acc_cb_;
  };
}
