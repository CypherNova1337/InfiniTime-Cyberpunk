#include "displayapp/screens/Matrix.h"

#include <chrono>
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

namespace {
  // Printable ASCII plus the Cyrillic block, both of which exist in jetbrains_mono_bold_20.
  // '#' is excluded because it is the recolor escape character.
  constexpr char asciiGlyphs[] = "0123456789ABCDEFXZ$%&*+<>=?@";

  char* AppendGlyph(char* out, uint16_t glyph) {
    if (glyph < 0x80) {
      *out++ = static_cast<char>(glyph);
    } else {
      *out++ = static_cast<char>(0xC0 | (glyph >> 6));
      *out++ = static_cast<char>(0x80 | (glyph & 0x3F));
    }
    return out;
  }

  char* AppendColor(char* out, const char* color) {
    *out++ = '#';
    for (uint8_t i = 0; i < 6; i++) {
      *out++ = color[i];
    }
    *out++ = ' ';
    return out;
  }
}

Matrix::Matrix(Controllers::DateTime& dateTimeController) {
  rngState = static_cast<uint32_t>(dateTimeController.CurrentDateTime().time_since_epoch().count()) | 1;

  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  for (uint8_t i = 0; i < nbColumns; i++) {
    Column& column = columns[i];
    ResetColumn(column, true);
    for (auto& glyph : column.glyphs) {
      glyph = RandomGlyph();
    }
    column.label = lv_label_create(lv_scr_act(), nullptr);
    lv_label_set_recolor(column.label, true);
    lv_obj_set_style_local_text_line_space(column.label, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_pos(column.label, i * columnWidth, 0);
    RenderColumn(column);
  }

  taskRefresh = lv_task_create(RefreshTaskCallback, 60, LV_TASK_PRIO_MID, this);
}

Matrix::~Matrix() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

uint32_t Matrix::NextRandom() {
  // xorshift32
  rngState ^= rngState << 13;
  rngState ^= rngState >> 17;
  rngState ^= rngState << 5;
  return rngState;
}

uint16_t Matrix::RandomGlyph() {
  const uint32_t r = NextRandom();
  if (r % 3 == 0) {
    return static_cast<uint8_t>(asciiGlyphs[(r >> 2) % (sizeof(asciiGlyphs) - 1)]);
  }
  return 0x410 + ((r >> 2) % 0x40);
}

void Matrix::ResetColumn(Column& column, bool initial) {
  const uint32_t r = NextRandom();
  column.trail = 4 + r % 7;
  column.speed = 1 + (r >> 4) % 3;
  // Start above the screen so drops enter at staggered times
  column.head = initial ? -static_cast<int8_t>((r >> 8) % 16) : -static_cast<int8_t>((r >> 8) % 8);
}

void Matrix::RenderColumn(Column& column) {
  const Palette& palette = palettes[paletteIndex];
  char* out = column.text.data();
  for (int8_t row = 0; row < nbRows; row++) {
    const int8_t distance = column.head - row;
    if (distance < 0 || distance >= column.trail) {
      *out++ = ' ';
    } else {
      const char* color = distance == 0 ? palette.head : (distance < 3 ? palette.bright : palette.dim);
      out = AppendColor(out, color);
      out = AppendGlyph(out, column.glyphs[row]);
      *out++ = '#';
    }
    if (row < nbRows - 1) {
      *out++ = '\n';
    }
  }
  *out = '\0';
  lv_label_set_text_static(column.label, column.text.data());
}

void Matrix::Refresh() {
  tick++;
  for (auto& column : columns) {
    bool changed = false;

    // Glyphs inside the trail flicker every now and then
    if (NextRandom() % 4 == 0) {
      column.glyphs[NextRandom() % nbRows] = RandomGlyph();
      changed = true;
    }

    if (tick % column.speed == 0) {
      column.head++;
      if (column.head - column.trail >= nbRows) {
        ResetColumn(column, false);
      }
      // The new head always gets a fresh glyph
      if (column.head >= 0 && column.head < nbRows) {
        column.glyphs[column.head] = RandomGlyph();
      }
      changed = true;
    }

    if (changed) {
      RenderColumn(column);
    }
  }
}

bool Matrix::OnTouchEvent(TouchEvents event) {
  if (event == TouchEvents::Tap) {
    paletteIndex = (paletteIndex + 1) % palettes.size();
    for (auto& column : columns) {
      RenderColumn(column);
    }
    return true;
  }
  return false;
}
