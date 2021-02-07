#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <bank.hpp>
#include <atm_controller.hpp>

using namespace ::testing;
using namespace banking;

class AtmControllerMock : AtmController {
  public:
  AtmControllerMock() {
    AtmController();
  }

  void callInsertCard(long card_no, int card_pin) {
    this->insertCard(card_no, card_pin);
  }

  void callSelectAccount(long account_no) {
    this->selectAccount(account_no);
  }

  MOCK_METHOD(bool, controllerDisplay, (AtmOperationType, int, std::string&&));
};

class BankingFixture : public Test {
  public:
  std::string holder_name;
  std::vector<long> account_no;
  long card_no;
  int card_pin;
  int money;

  BankingFixture() : holder_name("someone"), money(1000) {
    Bank::getBank()->createAndLinkAccount(holder_name, money);
    Bank::getBank()->privilegedOperation(HIDDEN_PASSCODE, holder_name, account_no, card_no);
    for (const auto &acc:account_no)
      LOG(INFO) << acc;
  }

  long selectAccount() {
    return account_no[0];
  }

  void SetUp() override {
  }

  void TearDown() override {
    Bank::getBank()->deleteBank();
  }
};
