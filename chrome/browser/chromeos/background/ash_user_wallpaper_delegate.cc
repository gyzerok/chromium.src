// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/background/ash_user_wallpaper_delegate.h"

#include "ash/shell.h"
#include "ash/desktop_background/user_wallpaper_delegate.h"
#include "ash/wm/window_animations.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/chromeos/extensions/wallpaper_manager_util.h"
#include "chrome/browser/chromeos/login/startup_utils.h"
#include "chrome/browser/chromeos/login/user.h"
#include "chrome/browser/chromeos/login/user_manager.h"
#include "chrome/browser/chromeos/login/wallpaper_manager.h"
#include "chrome/browser/chromeos/login/wizard_controller.h"
#include "chromeos/chromeos_switches.h"
#include "chromeos/login/login_state.h"
#include "content/public/browser/notification_service.h"

namespace chromeos {

namespace {

bool IsNormalWallpaperChange() {
  if (chromeos::LoginState::Get()->IsUserLoggedIn() ||
      !CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kFirstExecAfterBoot) ||
      WizardController::IsZeroDelayEnabled() ||
      !CommandLine::ForCurrentProcess()->HasSwitch(switches::kLoginManager)) {
    return true;
  }

  return false;
}

class UserWallpaperDelegate : public ash::UserWallpaperDelegate {
 public:
  UserWallpaperDelegate()
      : boot_animation_finished_(false),
        animation_duration_override_in_ms_(0) {
  }

  virtual ~UserWallpaperDelegate() {
  }

  virtual int GetAnimationType() OVERRIDE {
    return ShouldShowInitialAnimation() ?
        ash::WINDOW_VISIBILITY_ANIMATION_TYPE_BRIGHTNESS_GRAYSCALE :
        static_cast<int>(wm::WINDOW_VISIBILITY_ANIMATION_TYPE_FADE);
  }

  virtual int GetAnimationDurationOverride() OVERRIDE {
    return animation_duration_override_in_ms_;
  }

  virtual void SetAnimationDurationOverride(
      int animation_duration_in_ms) OVERRIDE {
    animation_duration_override_in_ms_ = animation_duration_in_ms;
  }

  virtual bool ShouldShowInitialAnimation() OVERRIDE {
    if (IsNormalWallpaperChange() || boot_animation_finished_)
      return false;

    // It is a first boot case now. If kDisableBootAnimation flag
    // is passed, it only disables any transition after OOBE.
    bool is_registered = StartupUtils::IsDeviceRegistered();
    const CommandLine* command_line = CommandLine::ForCurrentProcess();
    bool disable_boot_animation = command_line->
        HasSwitch(switches::kDisableBootAnimation);
    if (is_registered && disable_boot_animation)
      return false;

    return true;
  }

  virtual void UpdateWallpaper(bool clear_cache) OVERRIDE {
    chromeos::WallpaperManager::Get()->UpdateWallpaper(clear_cache);
  }

  virtual void InitializeWallpaper() OVERRIDE {
    chromeos::WallpaperManager::Get()->InitializeWallpaper();
  }

  virtual void OpenSetWallpaperPage() OVERRIDE {
    wallpaper_manager_util::OpenWallpaperManager();
  }

  virtual bool CanOpenSetWallpaperPage() OVERRIDE {
    if (!LoginState::Get()->IsUserAuthenticated())
      return false;
    const User* user = chromeos::UserManager::Get()->GetActiveUser();
    if (!user)
      return false;
    if (chromeos::WallpaperManager::Get()->IsPolicyControlled(user->email()))
      return false;
    return true;
  }

  virtual void OnWallpaperAnimationFinished() OVERRIDE {
    content::NotificationService::current()->Notify(
        chrome::NOTIFICATION_WALLPAPER_ANIMATION_FINISHED,
        content::NotificationService::AllSources(),
        content::NotificationService::NoDetails());
  }

  virtual void OnWallpaperBootAnimationFinished() OVERRIDE {
    // Make sure that boot animation type is used only once.
    boot_animation_finished_ = true;
  }

 private:
  bool boot_animation_finished_;

  // The animation duration to show a new wallpaper if an animation is required.
  int animation_duration_override_in_ms_;

  DISALLOW_COPY_AND_ASSIGN(UserWallpaperDelegate);
};

}  // namespace

ash::UserWallpaperDelegate* CreateUserWallpaperDelegate() {
  return new chromeos::UserWallpaperDelegate();
}

}  // namespace chromeos
