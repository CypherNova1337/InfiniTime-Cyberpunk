#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <optional>
#include <displayapp/Controllers.h>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/SimpleWeatherService.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceNetRunner : public Screen {
      public:
        WatchFaceNetRunner(Controllers::DateTime& dateTimeController,
                           const Controllers::Battery& batteryController,
                           const Controllers::Ble& bleController,
                           Controllers::NotificationManager& notificationManager,
                           Controllers::Settings& settingsController,
                           Controllers::HeartRateController& heartRateController,
                           Controllers::MotionController& motionController,
                           Controllers::SimpleWeatherService& weatherService);
        ~WatchFaceNetRunner() override;

        void Refresh() override;

      private:
        void UpdateGlitch();
        void StartGlitch();
        uint32_t NextRandom();

        Utility::DirtyValue<int> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentMinute {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::optional<Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        static constexpr lv_coord_t timeY = 62;
        static constexpr lv_coord_t shadowOffset = 3;

        lv_point_t topLinePoints[4] {{0, 30}, {170, 30}, {180, 38}, {240, 38}};
        lv_point_t bottomLinePoints[4] {{0, 204}, {98, 204}, {108, 196}, {240, 196}};

        lv_obj_t* topLine;
        lv_obj_t* bottomLine;
        lv_obj_t* labelLink;
        lv_obj_t* labelPower;
        lv_obj_t* labelDate;
        lv_obj_t* labelNotification;
        lv_obj_t* labelTimeGhost;
        lv_obj_t* labelTimeShadow;
        lv_obj_t* labelTime;
        lv_obj_t* barSeconds;
        lv_obj_t* labelSeconds;
        lv_obj_t* labelVitals;
        lv_obj_t* labelEnv;
        lv_obj_t* labelHandle;
        lv_obj_t* labelHex;
        lv_obj_t* glitchSlice;

        uint32_t rngState = 0x1337c0de;
        uint8_t glitchFrames = 0;
        const char* amPm = "";

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::NetRunner> {
      static constexpr WatchFace watchFace = WatchFace::NetRunner;
      static constexpr const char* name = "NetRunner";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceNetRunner(controllers.dateTimeController,
                                               controllers.batteryController,
                                               controllers.bleController,
                                               controllers.notificationManager,
                                               controllers.settingsController,
                                               controllers.heartRateController,
                                               controllers.motionController,
                                               *controllers.weatherController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
