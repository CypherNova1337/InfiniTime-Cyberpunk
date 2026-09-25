#pragma once

#include <array>
#include <cstdint>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Matrix : public Screen {
      public:
        explicit Matrix(Controllers::DateTime& dateTimeController);
        ~Matrix() override;

        void Refresh() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        static constexpr uint8_t nbColumns = 20;
        static constexpr uint8_t nbRows = 10;
        static constexpr uint8_t columnWidth = 12;
        static constexpr uint8_t rowHeight = 24;
        // Worst case per row: "#rrggbb " + 2 byte glyph + "#\n"
        static constexpr uint16_t bufferSize = nbRows * 12 + 1;

        struct Column {
          int8_t head;
          uint8_t trail;
          uint8_t speed;
          std::array<uint16_t, nbRows> glyphs;
          std::array<char, bufferSize> text;
          lv_obj_t* label;
        };

        struct Palette {
          const char* head;
          const char* bright;
          const char* dim;
        };

        static constexpr std::array<Palette, 3> palettes {{
          {"ffffff", "39ff14", "0b6b1a"},
          {"ffffff", "05d9e8", "0a4a55"},
          {"fcee0a", "ff2a6d", "5a1030"},
        }};

        uint32_t NextRandom();
        uint16_t RandomGlyph();
        void ResetColumn(Column& column, bool initial);
        void RenderColumn(Column& column);

        std::array<Column, nbColumns> columns;
        uint32_t rngState;
        uint8_t tick = 0;
        uint8_t paletteIndex = 0;
        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Matrix> {
      static constexpr Apps app = Apps::Matrix;
      static constexpr const char* icon = Screens::Symbols::terminal;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Matrix(controllers.dateTimeController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
