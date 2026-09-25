#include "displayapp/screens/Badge.h"

#include <lvgl/lvgl.h>
#include "Identity.h"
#include "displayapp/InfiniTimeTheme.h"
#include "displayapp/screens/BadgeQr.h"

using namespace Pinetime::Applications::Screens;

Badge::Badge(System::SystemTask& systemTask) : wakeLock(systemTask) {
  // Keep the screen on while someone is scanning the code
  wakeLock.Lock();

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::neonMagenta);
  lv_label_set_text_fmt(title, "// %s", Pinetime::Identity::handle);
  lv_obj_align(title, nullptr, LV_ALIGN_IN_TOP_MID, 0, 4);

  const lv_coord_t qrSize = BadgeQr::image.header.w;

  lv_obj_t* frame = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(frame, qrSize + 8, qrSize + 8);
  lv_obj_align(frame, nullptr, LV_ALIGN_CENTER, 0, 2);
  lv_obj_set_style_local_bg_opa(frame, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_radius(frame, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_border_width(frame, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_border_color(frame, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, Colors::neonMagenta);

  lv_obj_t* qr = lv_img_create(lv_scr_act(), nullptr);
  lv_img_set_src(qr, &BadgeQr::image);
  lv_obj_align(qr, frame, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* caption = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(caption, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::neonCyan);
  lv_label_set_text_static(caption, BadgeQr::caption);
  lv_obj_align(caption, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, -4);
}

Badge::~Badge() {
  lv_obj_clean(lv_scr_act());
}
