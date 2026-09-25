#pragma once

#include <array>
#include <cstdint>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/ble/IntrusionLog.h"
#include "systemtask/SystemTask.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class IntrusionLog : public Screen {
      public:
        explicit IntrusionLog(Controllers::IntrusionLog& log);
        ~IntrusionLog() override;

        bool OnTouchEvent(TouchEvents event) override;
        void OnButtonEvent(lv_obj_t* object, lv_event_t event);

      private:
        static constexpr uint8_t entriesPerPage = 2;

        void UpdateToggle();
        void ShowPage();
        uint8_t PageCount() const;

        Controllers::IntrusionLog& log;
        uint8_t page = 0;

        lv_obj_t* labelPage;
        lv_obj_t* buttonToggle;
        lv_obj_t* labelToggle;
        lv_obj_t* buttonWipe;
        lv_obj_t* labelEmpty;
        std::array<lv_obj_t*, entriesPerPage> labelStatus;
        std::array<lv_obj_t*, entriesPerPage> labelDetails;
      };
    }

    template <>
    struct AppTraits<Apps::IntrusionLog> {
      static constexpr Apps app = Apps::IntrusionLog;
      static constexpr const char* icon = Screens::Symbols::shieldAlt;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::IntrusionLog(controllers.systemTask->nimble().intrusionLog());
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
