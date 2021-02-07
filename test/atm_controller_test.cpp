#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <glog/logging.h>

#include <banking_fixture.hpp>

using namespace banking;
using namespace testing;

TEST_F(BankingFixture, CorrectCardTest) {
  AtmControllerMock atm_mock;
  EXPECT_CALL(atm_mock, controllerDisplay(SHOW_INPUT,_,_));
  atm_mock.callInsertCard(card_no, 8888);
}

TEST_F(BankingFixture, InorrectCardTest) {
  AtmControllerMock atm_mock;
  EXPECT_CALL(atm_mock, controllerDisplay(SHOW_ERROR,_,_));
  atm_mock.callInsertCard(card_no, 8899);
}

TEST_F(BankingFixture, SelectAccountTest) {
  AtmControllerMock atm_mock;
  EXPECT_CALL(atm_mock, controllerDisplay(SHOW_INPUT,_,_));
  atm_mock.callInsertCard(card_no, 8888);
  EXPECT_CALL(atm_mock, controllerDisplay(SHOW_INPUT,_,_));
  atm_mock.callSelectAccount(selectAccount());
}

TEST_F(BankingFixture, SelectIncorrectAccountTest) {
  AtmControllerMock atm_mock;
  EXPECT_CALL(atm_mock, controllerDisplay(SHOW_INPUT,_,_));
  atm_mock.callInsertCard(card_no, 8888);
  EXPECT_CALL(atm_mock, controllerDisplay(SHOW_ERROR,_,_));
  atm_mock.callSelectAccount(1000000);
}

int main(int argc, char **argv) {
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(argv[0]);
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
