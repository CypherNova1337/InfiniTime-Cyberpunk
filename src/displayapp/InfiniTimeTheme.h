#pragma once

#include <lvgl/lvgl.h>

namespace Colors {
  static constexpr lv_color_t deepOrange = LV_COLOR_MAKE(0xff, 0x40, 0x0);
  static constexpr lv_color_t orange = LV_COLOR_MAKE(0xff, 0xb0, 0x0);
  static constexpr lv_color_t green = LV_COLOR_MAKE(0x0, 0xd2, 0x6a);
  static constexpr lv_color_t blue = LV_COLOR_MAKE(0x7a, 0x2c, 0xff);
  static constexpr lv_color_t lightGray = LV_COLOR_MAKE(0xb0, 0xb0, 0xb0);
  static constexpr lv_color_t gray = LV_COLOR_MAKE(0x50, 0x50, 0x50);

  // Cyberpunk neon palette
  static constexpr lv_color_t neonCyan = LV_COLOR_MAKE(0x05, 0xd9, 0xe8);
  static constexpr lv_color_t neonMagenta = LV_COLOR_MAKE(0xff, 0x2a, 0x6d);
  static constexpr lv_color_t neonYellow = LV_COLOR_MAKE(0xfc, 0xee, 0x0a);
  static constexpr lv_color_t neonGreen = LV_COLOR_MAKE(0x39, 0xff, 0x14);
  static constexpr lv_color_t neonPurple = LV_COLOR_MAKE(0x9d, 0x4e, 0xdd);
  static constexpr lv_color_t dimCyan = LV_COLOR_MAKE(0x0a, 0x4a, 0x55);

  static constexpr lv_color_t bg = LV_COLOR_MAKE(0x2d, 0x1b, 0x4e);
  static constexpr lv_color_t bgAlt = LV_COLOR_MAKE(0x1a, 0x10, 0x2e);
  static constexpr lv_color_t bgDark = LV_COLOR_MAKE(0x0c, 0x07, 0x14);
  static constexpr lv_color_t highlight = neonMagenta;
};

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_pinetime_theme_init();
