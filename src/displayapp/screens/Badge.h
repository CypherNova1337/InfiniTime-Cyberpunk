#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class Badge : public Screen {
      public:
        explicit Badge(System::SystemTask& systemTask);
        ~Badge() override;

      private:
        System::WakeLock wakeLock;
      };
    }

    template <>
    struct AppTraits<Apps::Badge> {
      static constexpr Apps app = Apps::Badge;
      static constexpr const char* icon = Screens::Symbols::skull;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Badge(*controllers.systemTask);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
