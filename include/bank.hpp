#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <vector>

#include <glog/logging.h>

#include <account_card.hpp>

namespace banking {

  const int ACCOUNTS_CARDS_UL = 1000;

  const int HIDDEN_PASSCODE = 12345;

  using AccountPtr = std::shared_ptr<Account>;
  using CardPtr = std::shared_ptr<Card>;
  using account_card_pair_t = std::pair<CardPtr, std::vector<AccountPtr>>;
  using atm_cb_t = std::function<bool(AtmOperationType, int, std::string&&)>;

  class Bank {
    public:

      /**
       * @brief Generate an account and link to the card number if passed
       *
       * @param holder_name
       * @param amount
       * @param card_no
       */
      void createAndLinkAccount(const std::string &holder_name,
          int amount, long card_no = -1);

      /**
       * @brief verify card pin and generate a transaction token
       *
       * @param card_no
       * @param card_pin
       * @return transaction token
       */
      int verifyAndCreateTransaction(long card_no, int card_pin);

      /**
       * @brief Select a particular account
       *
       * @param transaction_token
       * @param account_no
       */
      void selectAccount(int transaction_token, long account_no);

      /**
       * @brief Generate atm id for new atms
       *
       * @return atm if
       */
      int get_atm_id();

      /**
       * @brief [Needs to deleted] Get account and card number for a specific
       *         holder name
       * @param holder_name
       * @param account_no
       * @param card_no
       * @return true if account with the holder_name was present and details
       *         populated
       */
      bool privilegedOperation(const int &passcode, const std::string &holder_name,
          std::vector<long> &account_no, long &card_no);


      /**
       * @brief does what name implies
       *
       * @param transaction_token
       * @param atm_cb callback function in atm controller that needs to be called
       */
      void acknowledgeTransaction(int transaction_token, atm_cb_t atm_cb);

      /**
       * @brief initialize bank
       *
       */
      void init();

      /**
       * @brief destructor
       *
       */
      ~Bank();

    private:

      /**
       * @brief Constructor
       *
       */
      Bank() {}

      std::vector<long> available_account_ids_, available_card_ids_;
      std::vector<int> available_transaction_tokens_, available_atm_ids_;

      std::mutex account_id_mutex_, card_id_mutex_, t_token_mutex_, atm_id_mutex_;

      std::map<long, account_card_pair_t> account_cards_map_;
      std::map<int, long> transaction_map_;
      std::map<int, atm_cb_t> atm_cb_map_;

      std::mutex account_cards_mutex_, transaction_map_mutex_, atm_cb_mutex_;

      /**
       * @{name} instance of the bank
       */
      static Bank *bank_;

      /**
       * @brief generate a random number from a set of values
       *
       * @tparam T type of the random number to be returned
       * @param mtx
       * @param available_ids
       * @return random number
       */
      template <typename T>
        T getRandomId(std::mutex &mtx,
                      std::vector<T> available_ids) {
          std::lock_guard<std::mutex> lck(mtx);
          std::mt19937 mt(std::chrono::high_resolution_clock::now().time_since_epoch().count());
          std::uniform_int_distribution<int> dst(0, available_ids.size() - 1);
          int index = dst(mt);
          T id = available_ids[index];
          available_ids.erase(available_ids.begin() + index);
          return id;
        }

      /**
       * @brief find an element in map
       *
       * @tparam T typename of key
       * @tparam U typename of value
       * @param map_mutex
       * @param search_map
       * @param key
       * @param mit iterator corresponding to the found element
       * @return true if found in map else false
       */
      template <typename T, typename U>
        bool findInMap(std::mutex &map_mutex,
            const std::map<T, U> &search_map,
            const T &key,
            U &mit) {
          std::lock_guard<std::mutex> lck(map_mutex);
          typename std::map<T, U>::const_iterator it = search_map.find(key);
          if (it != search_map.end()) {
            mit = it->second;
            return true;
          }
          return false;
        }

      /**
       * @brief Change the value of the element specified by the key
       *
       * @tparam T typename of key
       * @tparam U typename of value
       * @param map_mutex
       * @param search_map
       * @param key
       * @param value
       * @return true if key found and value changed, else false
       */
      template <typename T, typename U>
        bool changeValueInMap(std::mutex &map_mutex,
            std::map<T, U> &search_map,
            const T &key,
            U &value) {
          std::lock_guard<std::mutex> lck(map_mutex);
          typename std::map<T, U>::iterator it = search_map.find(key);
          if (it != search_map.end()) {
            it->second = value;
            return true;
          }
          return false;
        }

      /**
       * @brief Add a key-value pair to map
       *
       * @tparam T type name of key
       * @tparam U type name of value
       * @param mtx mutex corresponding the map
       * @param key to be added
       * @param value
       */
      template <typename T, typename U>
        void addToMap(std::mutex &mtx,
            std::map<T, U> &add_map,
            T &key, U &value) {
          std::lock_guard<std::mutex> lck(mtx);
          std::pair<typename std::map<T, U>::iterator, bool> ret;
          ret = add_map.insert(std::pair<T, U>(key, value));
          if (!ret.second)
            throw std::runtime_error("Unable to add to map");
        }

      /**
       * @brief Delete an element from map
       *
       * @tparam T typename of key
       * @tparam U typename of value
       * @param mtx
       * @param delete_map
       * @param key
       */
      template <typename T, typename U>
        void deleteFromMap(std::mutex &mtx,
            std::map<T, U> &delete_map, T &key) {
          std::lock_guard<std::mutex> lck(mtx);
          delete_map.erase(key);
        }

      /**
       * @brief Cleanup after an error condition
       *
       * @param token
       */
      void throwSession(int token);

    public:
      /**
       * @brief Get the single instance of bank
       *
       * @return instance
       */
      static Bank* getBank() {
        if (!bank_) {
          bank_ = new Bank;
          bank_->init();
        }
        return bank_;
      }

      /**
       * @brief calls destructor and reinitializes instance
       *
       */
      void deleteBank() {
        delete bank_;
        bank_ = nullptr;
      }
  };
}
