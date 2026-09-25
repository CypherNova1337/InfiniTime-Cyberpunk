#include "displayapp/screens/IntrusionLog.h"

#include <chrono>
#include <lvgl/lvgl.h>
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonEventHandler(lv_obj_t* object, lv_event_t event) {
    auto* screen = static_cast<IntrusionLog*>(object->user_data);
    screen->OnButtonEvent(object, event);
  }

  lv_obj_t* CreateButton(lv_obj_t** label, const char* text, void* userData) {
    lv_obj_t* button = lv_btn_create(lv_scr_act(), nullptr);
    button->user_data = userData;
    lv_obj_set_event_cb(button, ButtonEventHandler);
    lv_obj_set_style_local_pad_all(button, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
    *label = lv_label_create(button, nullptr);
    lv_label_set_text_static(*label, text);
    return button;
  }
}

IntrusionLog::IntrusionLog(Controllers::IntrusionLog& log) : log {log} {
  // The display task keeps the SPI flash awake, so this is a safe moment to persist pending changes
  if (log.IsDirty()) {
    log.Save();
  }

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::neonMagenta);
  lv_label_set_text_static(title, "// INTRUSIONS");
  lv_obj_set_pos(title, 4, 2);

  labelPage = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelPage, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::neonPurple);

  buttonToggle = CreateButton(&labelToggle, "", this);
  lv_obj_set_size(buttonToggle, 148, 40);
  lv_obj_set_pos(buttonToggle, 2, 30);

  lv_obj_t* labelWipe;
  buttonWipe = CreateButton(&labelWipe, "WIPE", this);
  lv_obj_set_size(buttonWipe, 82, 40);
  lv_obj_set_pos(buttonWipe, 156, 30);

  labelEmpty = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelEmpty, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_label_set_long_mode(labelEmpty, LV_LABEL_LONG_BREAK);
  lv_obj_set_width(labelEmpty, 232);
  lv_label_set_text_static(labelEmpty, "No intrusions\nlogged.\n\nPair your phone\nwith the PIN to\nenable alerts.");
  lv_obj_set_pos(labelEmpty, 4, 84);

  for (uint8_t i = 0; i < entriesPerPage; i++) {
    const lv_coord_t y = 80 + i * 80;
    labelStatus[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_obj_set_pos(labelStatus[i], 4, y);
    labelDetails[i] = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_recolor(labelDetails[i], true);
    lv_obj_set_pos(labelDetails[i], 4, y + 24);
  }

  UpdateToggle();
  ShowPage();
}

IntrusionLog::~IntrusionLog() {
  if (log.IsDirty()) {
    log.Save();
  }
  lv_obj_clean(lv_scr_act());
}

uint8_t IntrusionLog::PageCount() const {
  const size_t count = log.Count();
  return count == 0 ? 1 : (count + entriesPerPage - 1) / entriesPerPage;
}

void IntrusionLog::UpdateToggle() {
  const bool enabled = log.IsEnabled();
  lv_label_set_text_static(labelToggle, enabled ? "ALERTS ON" : "ALERTS OFF");
  lv_obj_set_style_local_text_color(labelToggle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, enabled ? Colors::neonCyan : Colors::gray);
  lv_obj_set_style_local_border_color(buttonToggle, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, enabled ? Colors::neonCyan : Colors::gray);
}

void IntrusionLog::ShowPage() {
  lv_label_set_text_fmt(labelPage, "%d/%d", page + 1, PageCount());
  lv_obj_align(labelPage, nullptr, LV_ALIGN_IN_TOP_RIGHT, -4, 2);
  lv_obj_set_hidden(labelEmpty, log.Count() != 0);

  for (uint8_t i = 0; i < entriesPerPage; i++) {
    const size_t index = page * entriesPerPage + i;
    const bool visible = index < log.Count();
    lv_obj_set_hidden(labelStatus[i], !visible);
    lv_obj_set_hidden(labelDetails[i], !visible);
    if (!visible) {
      continue;
    }

    const auto entry = log.Get(index);
    const bool alerted = (entry.flags & Controllers::IntrusionLog::Alerted) != 0;
    lv_obj_set_style_local_text_color(labelStatus[i],
                                      LV_LABEL_PART_MAIN,
                                      LV_STATE_DEFAULT,
                                      alerted ? Colors::neonMagenta : Colors::lightGray);

    char duration[8];
    if (entry.duration == 0) {
      snprintf(duration, sizeof(duration), "LIVE");
    } else if (entry.duration < 60) {
      snprintf(duration, sizeof(duration), "%ds", entry.duration);
    } else if (entry.duration < 3600) {
      snprintf(duration, sizeof(duration), "%dm", entry.duration / 60);
    } else {
      snprintf(duration, sizeof(duration), "%dh", entry.duration / 3600);
    }
    lv_label_set_text_fmt(labelStatus[i], "%-13s%5s", Controllers::IntrusionLog::Describe(entry), duration);

    const std::chrono::sys_seconds time {std::chrono::seconds {entry.time}};
    const auto days = std::chrono::floor<std::chrono::days>(time);
    const std::chrono::year_month_day date {days};
    const std::chrono::hh_mm_ss clock {time - days};

    char address[18];
    Controllers::IntrusionLog::FormatAddress(entry, address);
    lv_label_set_text_fmt(labelDetails[i],
                          "#fcee0a %02u-%02u %02d:%02d#\n#05d9e8 %s#",
                          static_cast<unsigned>(date.month()),
                          static_cast<unsigned>(date.day()),
                          static_cast<int>(clock.hours().count()),
                          static_cast<int>(clock.minutes().count()),
                          address);
  }
}

void IntrusionLog::OnButtonEvent(lv_obj_t* object, lv_event_t event) {
  if (event != LV_EVENT_CLICKED) {
    return;
  }
  if (object == buttonToggle) {
    log.SetEnabled(!log.IsEnabled());
    UpdateToggle();
  } else if (object == buttonWipe) {
    log.Clear();
    page = 0;
    ShowPage();
  }
}

bool IntrusionLog::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::SwipeUp && page + 1 < PageCount()) {
    page++;
    ShowPage();
    return true;
  }
  if (event == TouchEvents::SwipeDown && page > 0) {
    page--;
    ShowPage();
    return true;
  }
  return false;
}
