#pragma once

#include <stdint.h>
#include <stdio.h>

#include "dehydrator/interfaces/CharacterDisplay.h"
#include "dehydrator/persistence/UserProfileStore.h"
#include "dehydrator/ui/LcdStatusView.h"
#include "dehydrator/ui/UserProfileActionController.h"

namespace dehydrator {

/**
 * @brief Snapshot rendered by one user-profile detail screen.
 */
struct LcdUserProfileDetailSnapshot {
  /** One-based slot label number. */
  uint8_t slotNumber = 1U;
  /** Whether the slot is occupied. */
  bool occupied = false;
  /** Saved profile payload when occupied. */
  ProfileConfig profile;
  /** Currently selected action. */
  UserProfileAction action = UserProfileAction::Edit;
};

/**
 * @brief Renders one compact read-only user-profile detail screen.
 */
class LcdUserProfileDetailView {
 public:
  /**
   * @brief Creates the detail view.
   *
   * @param display LCD character display interface.
   */
  explicit LcdUserProfileDetailView(CharacterDisplay& display) : display_(display) {}

  /**
   * @brief Renders the current profile summary and selected action.
   *
   * @param snapshot Slot occupancy, profile summary, and action.
   */
  void render(const LcdUserProfileDetailSnapshot& snapshot) {
    char line[LcdStatusView::COLUMNS + 1U] = {};

    fillLine(line);
    snprintf(line, sizeof(line), "Profil %u", snapshot.slotNumber);
    writeLine(0U, line);

    fillLine(line);
    if (!snapshot.occupied) {
      writeToken(line, "Liber", 0U);
    } else {
      writeToken(line, modeLabel(snapshot.profile.mode), 0U);
    }
    writeLine(1U, line);

    fillLine(line);
    if (snapshot.occupied) {
      formatProfileSummary(line, snapshot.profile);
    } else {
      writeToken(line, "Fara profil salvat", 0U);
    }
    writeLine(2U, line);

    fillLine(line);
    line[0] = '>';
    writeToken(line, actionLabel(snapshot.action), 1U);
    writeLine(3U, line);
  }

 private:
  static const char* modeLabel(ProfileMode mode) {
    if (mode == ProfileMode::Boost) {
      return "Boost";
    }
    if (mode == ProfileMode::Fluctuating) {
      return "Fluctuant";
    }
    return "Constant";
  }

  static const char* actionLabel(UserProfileAction action) {
    switch (action) {
      case UserProfileAction::Start:
        return "Pornire";
      case UserProfileAction::Edit:
        return "Editeaza";
      case UserProfileAction::Delete:
        return "Sterge";
      case UserProfileAction::Back:
      default:
        return "Inapoi";
    }
  }

  static void formatProfileSummary(char* line, const ProfileConfig& profile) {
    const uint16_t hours = profile.durationMinutes / 60U;
    const uint16_t minutes = profile.durationMinutes % 60U;
    if (profile.mode == ProfileMode::Boost) {
      snprintf(line, LcdStatusView::COLUMNS + 1U, "%d/%d\xDF""C %uh %um",
               profile.targetTempC, profile.highTempC, hours, minutes);
    } else if (profile.mode == ProfileMode::Fluctuating) {
      snprintf(line, LcdStatusView::COLUMNS + 1U, "%d-%d\xDF""C %uh %um",
               profile.lowTempC, profile.highTempC, hours, minutes);
    } else {
      snprintf(line, LcdStatusView::COLUMNS + 1U, "%d\xDF""C %uh %um",
               profile.targetTempC, hours, minutes);
    }
  }

  static void fillLine(char* line) {
    for (uint8_t index = 0U; index < LcdStatusView::COLUMNS; index++) {
      line[index] = ' ';
    }
    line[LcdStatusView::COLUMNS] = '\0';
  }

  static void writeToken(char* line, const char* token, uint8_t column) {
    if (token == nullptr) {
      return;
    }
    for (size_t index = 0U; token[index] != '\0' &&
                            column < LcdStatusView::COLUMNS;
         index++, column++) {
      line[column] = token[index];
    }
  }

  void writeLine(uint8_t row, const char* line) {
    display_.setCursor(0U, row);
    for (uint8_t column = 0U; column < LcdStatusView::COLUMNS; column++) {
      display_.writeChar(line[column]);
    }
  }

  CharacterDisplay& display_;
};

}  // namespace dehydrator
