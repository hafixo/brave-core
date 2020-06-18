/**
 * Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package org.chromium.chrome.browser.onboarding;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import androidx.appcompat.app.AppCompatActivity;

import org.chromium.chrome.R;
import org.chromium.base.ApplicationStatus;
import org.chromium.chrome.browser.ChromeTabbedActivity;
import org.chromium.chrome.browser.onboarding.NonSwipeableViewPager;
import org.chromium.chrome.browser.onboarding.OnboardingPrefManager;
import org.chromium.chrome.browser.onboarding.OnboardingViewPagerAdapter;
import org.chromium.chrome.browser.BraveActivity;

import java.util.Calendar;
import java.util.Date;

public class OnboardingActivity extends AppCompatActivity implements OnViewPagerAction {
    private NonSwipeableViewPager viewPager;
    private static final int DAYS_4 = 4;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_onboarding);

        OnboardingPrefManager.getInstance().setOnboardingShown(true);

        OnboardingViewPagerAdapter onboardingViewPagerAdapter = new OnboardingViewPagerAdapter(
            this, getSupportFragmentManager(), this);
        viewPager = findViewById(R.id.view_pager);
        viewPager.setAdapter(onboardingViewPagerAdapter);
    }

    @Override
    public void onSkip() {
        if (!OnboardingPrefManager.getInstance().hasOnboardingShownForSkip()) {
            Calendar calender = Calendar.getInstance();
            calender.setTime(new Date());
            calender.add(Calendar.DATE, DAYS_4);

            OnboardingPrefManager.getInstance().setNextOnboardingDate(
                calender.getTimeInMillis());
        }
        finish();
    }

    @Override
    public void onNext() {
        viewPager.setCurrentItem(viewPager.getCurrentItem() + 1);
    }

    @Override
    public void onContinueToWallet() {
        if (BraveActivity.getBraveActivity() != null ) {
            BraveActivity.getBraveActivity().openRewardsPanel();
        }
        finish();
    }

    @Override
    public void onBackPressed() {

    }

    static public OnboardingActivity getOnboardingActivity() {
        for (Activity ref : ApplicationStatus.getRunningActivities()) {
            if (!(ref instanceof OnboardingActivity)) continue;

            return (OnboardingActivity)ref;
        }

        return null;
    }
}
