/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <memory>
#include <string>

#include "bat/ledger/internal/uphold/uphold_util.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/common/brave_paths.h"
#include "brave/components/brave_rewards/browser/rewards_service_impl.h"
#include "brave/components/brave_rewards/browser/test/common/rewards_browsertest_context_helper.h"
#include "brave/components/brave_rewards/browser/test/common/rewards_browsertest_context_util.h"
#include "brave/components/brave_rewards/browser/test/common/rewards_browsertest_network_util.h"
#include "brave/components/brave_rewards/browser/test/common/rewards_browsertest_observer.h"
#include "brave/components/brave_rewards/browser/test/common/rewards_browsertest_response.h"
#include "brave/components/brave_rewards/browser/test/common/rewards_browsertest_util.h"
#include "chrome/browser/ui/views/frame/browser_view.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/testing_profile.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/network_session_configurator/common/network_switches.h"
#include "net/dns/mock_host_resolver.h"

// npm run test -- brave_browser_tests --filter=RewardsBrowserTest.*

namespace rewards_browsertest {

class RewardsBrowserTest : public InProcessBrowserTest {
 public:
  RewardsBrowserTest() {
    observer_ = std::make_unique<RewardsBrowserTestObserver>();
    response_ = std::make_unique<RewardsBrowserTestResponse>();
  }

  void SetUpOnMainThread() override {
    InProcessBrowserTest::SetUpOnMainThread();

    // HTTP resolver
    host_resolver()->AddRule("*", "127.0.0.1");
    https_server_.reset(new net::EmbeddedTestServer(
        net::test_server::EmbeddedTestServer::TYPE_HTTPS));
    https_server_->SetSSLConfig(net::EmbeddedTestServer::CERT_OK);
    https_server_->RegisterRequestHandler(
        base::BindRepeating(&rewards_browsertest_util::HandleRequest));
    ASSERT_TRUE(https_server_->Start());

    // Rewards service
    brave::RegisterPathProvider();
    auto* profile = browser()->profile();
    rewards_service_ = static_cast<brave_rewards::RewardsServiceImpl*>(
        brave_rewards::RewardsServiceFactory::GetForProfile(profile));

    // Response mock
    base::ScopedAllowBlockingForTesting allow_blocking;
    response_->LoadMocks();
    rewards_service_->ForTestingSetTestResponseCallback(
        base::BindRepeating(
            &RewardsBrowserTest::GetTestResponse,
            base::Unretained(this)));

    // Observer
    observer_->Initialize(rewards_service_);
    if (!rewards_service_->IsWalletInitialized()) {
      observer_->WaitForWalletInitialization();
    }
    rewards_service_->SetLedgerEnvForTesting();
  }

  void TearDown() override {
    InProcessBrowserTest::TearDown();
  }

  void SetUpCommandLine(base::CommandLine* command_line) override {
    // HTTPS server only serves a valid cert for localhost, so this is needed
    // to load pages from other hosts without an error
    command_line->AppendSwitch(switches::kIgnoreCertificateErrors);
  }

  void GetTestResponse(
      const std::string& url,
      int32_t method,
      int* response_status_code,
      std::string* response,
      std::map<std::string, std::string>* headers) {
    response_->Get(
        url,
        method,
        response_status_code,
        response);
  }

  content::WebContents* contents() const {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  GURL uphold_auth_url() {
    GURL url("chrome://rewards/uphold/authorization?"
             "code=0c42b34121f624593ee3b04cbe4cc6ddcd72d&state=123456789");
    return url;
  }

  brave_rewards::RewardsServiceImpl* rewards_service_;
  std::unique_ptr<net::EmbeddedTestServer> https_server_;
  std::unique_ptr<RewardsBrowserTestObserver> observer_;
  std::unique_ptr<RewardsBrowserTestResponse> response_;
};

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, RenderWelcome) {
  rewards_browsertest_helper::EnableRewards(browser());

  EXPECT_STREQ(
      contents()->GetLastCommittedURL().spec().c_str(),
      "chrome://rewards/");
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, ToggleRewards) {
  rewards_browsertest_helper::EnableRewards(browser());

  // Toggle rewards off
  rewards_browsertest_util::WaitForElementThenClick(
      contents(),
      "[data-test-id2='enableMain']");
  std::string value = rewards_browsertest_util::WaitForElementThenGetAttribute(
      contents(),
      "[data-test-id2='enableMain']",
      "data-toggled");
  ASSERT_STREQ(value.c_str(), "false");

  // Toggle rewards back on
  rewards_browsertest_util::WaitForElementThenClick(
      contents(),
      "[data-test-id2='enableMain']");
  value = rewards_browsertest_util::WaitForElementThenGetAttribute(
      contents(),
      "[data-test-id2='enableMain']",
      "data-toggled");
  ASSERT_STREQ(value.c_str(), "true");
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, ActivateSettingsModal) {
  rewards_browsertest_helper::EnableRewards(browser());

  rewards_browsertest_util::WaitForElementThenClick(
      contents(),
      "[data-test-id='settingsButton']");
  rewards_browsertest_util::WaitForElementToAppear(
      contents(),
      "#modal");
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, ToggleAutoContribute) {
  rewards_browsertest_helper::EnableRewards(browser());

  // once rewards has loaded, reload page to activate auto-contribute
  contents()->GetController().Reload(content::ReloadType::NORMAL, true);
  EXPECT_TRUE(WaitForLoadStop(contents()));

  // toggle auto contribute off
  rewards_browsertest_util::WaitForElementThenClick(
      contents(),
      "[data-test-id2='autoContribution']");
  std::string value =
      rewards_browsertest_util::WaitForElementThenGetAttribute(
        contents(),
        "[data-test-id2='autoContribution']",
        "data-toggled");
  ASSERT_STREQ(value.c_str(), "false");

  // toggle auto contribute back on
  rewards_browsertest_util::WaitForElementThenClick(
      contents(),
      "[data-test-id2='autoContribution']");
  value = rewards_browsertest_util::WaitForElementThenGetAttribute(
      contents(),
      "[data-test-id2='autoContribution']",
      "data-toggled");
  ASSERT_STREQ(value.c_str(), "true");
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, PrefsTestInPrivateWindow) {
  rewards_browsertest_helper::EnableRewards(browser());
  EXPECT_TRUE(rewards_browsertest_util::IsRewardsEnabled(browser()));
  EXPECT_FALSE(rewards_browsertest_util::IsRewardsEnabled(browser(), true));
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, SiteBannerDefaultTipChoices) {
  rewards_browsertest_helper::EnableRewards(browser());

  rewards_browsertest_util::NavigateToPublisherPage(
      browser(),
      https_server_.get(),
      "3zsistemi.si");

  content::WebContents* site_banner =
      rewards_browsertest_helper::OpenSiteBanner(
          browser(),
          rewards_browsertest_util::ContributionType::OneTimeTip);
  auto tip_options = rewards_browsertest_util::GetSiteBannerTipOptions(
      site_banner);
  ASSERT_EQ(tip_options, std::vector<double>({ 1, 5, 50 }));

  site_banner = rewards_browsertest_helper::OpenSiteBanner(
      browser(),
      rewards_browsertest_util::ContributionType::MonthlyTip);
  tip_options = rewards_browsertest_util::GetSiteBannerTipOptions(
      site_banner);
  ASSERT_EQ(tip_options, std::vector<double>({ 1, 10, 100 }));
}

IN_PROC_BROWSER_TEST_F(
    RewardsBrowserTest,
    SiteBannerDefaultPublisherAmounts) {
  rewards_browsertest_helper::EnableRewards(browser());

  rewards_browsertest_util::NavigateToPublisherPage(
      browser(),
      https_server_.get(),
      "laurenwags.github.io");

  content::WebContents* site_banner =
      rewards_browsertest_helper::OpenSiteBanner(
          browser(),
          rewards_browsertest_util::ContributionType::OneTimeTip);
  const auto tip_options = rewards_browsertest_util::GetSiteBannerTipOptions(
      site_banner);
  ASSERT_EQ(tip_options, std::vector<double>({ 5, 10, 20 }));
}

IN_PROC_BROWSER_TEST_F(
  RewardsBrowserTest,
  NewTabPageWidgetEnableRewards) {
  rewards_browsertest_helper::EnableRewards(browser(), true);
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, NotVerifiedWallet) {
  rewards_browsertest_helper::EnableRewards(browser());

  // Click on verify button
  rewards_browsertest_util::WaitForElementThenClick(
      contents(),
      "#verify-wallet-button");

  // Click on verify button in on boarding
  rewards_browsertest_util::WaitForElementThenClick(
      contents(),
      "#on-boarding-verify-button");

  // Check if we are redirected to uphold
  {
    const GURL current_url = contents()->GetURL();
    ASSERT_TRUE(base::StartsWith(
        current_url.spec(),
        braveledger_uphold::GetUrl() + "/authorize/",
        base::CompareCase::INSENSITIVE_ASCII));
  }

  // Fake successful authentication
  ui_test_utils::NavigateToURLBlockUntilNavigationsComplete(
        browser(),
        uphold_auth_url(), 1);

  // Check if we are redirected to KYC page
  {
    const GURL current_url = contents()->GetURL();
    ASSERT_TRUE(base::StartsWith(
        current_url.spec(),
        braveledger_uphold::GetUrl() + "/signup/step2",
        base::CompareCase::INSENSITIVE_ASCII));
  }
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, PanelDontDoRequests) {
  // Open the Rewards popup
  content::WebContents* popup_contents =
      rewards_browsertest_helper::OpenRewardsPopup(browser());
  ASSERT_TRUE(popup_contents);

  // Make sure that no request was made
  ASSERT_FALSE(response_->WasRequestMade());
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, ShowMonthlyIfACOff) {
  rewards_browsertest_util::EnableRewardsViaCode(browser(), rewards_service_);
  rewards_service_->SetAutoContributeEnabled(false);

  rewards_browsertest_util::NavigateToPublisherPage(
      browser(),
      https_server_.get(),
      "3zsistemi.si");

  // Open the Rewards popup
  content::WebContents* popup_contents =
      rewards_browsertest_helper::OpenRewardsPopup(browser());
  ASSERT_TRUE(popup_contents);

  rewards_browsertest_util::WaitForElementToAppear(
      popup_contents,
      "#panel-donate-monthly");
}

IN_PROC_BROWSER_TEST_F(RewardsBrowserTest, ShowACPercentInThePanel) {
  rewards_browsertest_helper::EnableRewards(browser());

  rewards_browsertest_helper::VisitPublisher(
      browser(),
      rewards_browsertest_util::GetUrl(https_server_.get(), "3zsistemi.si"),
      true);

  rewards_browsertest_util::NavigateToPublisherPage(
      browser(),
      https_server_.get(),
      "3zsistemi.si");

  // Open the Rewards popup
  content::WebContents* popup_contents =
      rewards_browsertest_helper::OpenRewardsPopup(browser());
  ASSERT_TRUE(popup_contents);

  const std::string score =
      rewards_browsertest_util::WaitForElementThenGetContent(
          popup_contents,
          "[data-test-id='attention-score']");
  EXPECT_NE(score.find("100%"), std::string::npos);
}

}  // namespace rewards_browsertest
