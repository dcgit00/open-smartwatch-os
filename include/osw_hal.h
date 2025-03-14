#ifndef OSW_HAL_H
#define OSW_HAL_H

#include <memory>
#include <list>
#include <optional>

#include <Arduino.h>
#ifdef OSW_EMULATOR
#include <FakeMe.h>
#else
#include <Arduino_TFT.h>
#endif
#include "Arduino_Canvas_Graphics2D.h"
#include <Wire.h>
#include <Preferences.h>

#include OSW_TARGET_PLATFORM_HEADER
#include "hal/osw_filesystem.h"
#include "hal/buttons.h"
#include <devices/interfaces/OswTimeProvider.h>
#include "osw_config_keys.h"
#include "osw_pins.h"
#if defined(GPS_EDITION) || defined(GPS_EDITION_ROTATED)
#include <NMEAGPS.h>
#endif

#define ERR_SD_MISSING 1
#define ERR_SD_MOUNT_FAILED 2

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool afterNoon;
} OswTime;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekDay;
} OswDate;

class OswHal {
  public:
    static OswHal* getInstance();
    static void resetInstance();

    class Devices;
    Devices* devices() const {
        return this->_devices.get();
    };

#if OSW_PLATFORM_ENVIRONMENT == 1
    class Environment;
    Environment* environment() const {
        return this->_environment.get();
    };
#endif

#ifdef OSW_FEATURE_LUA
    class WakeUpConfig { // Turns out SWIG does not understand final classes
#else
    class WakeUpConfig final {
#endif
      public:
        time_t time = 0;
        // WARNING: These pointers must still be valid e.g. AFTER a deep sleep!
        void (*selected)(void) = nullptr;
        void (*used)(void) = nullptr;
        void (*expired)(void) = nullptr;
      protected:
        size_t id = 0;
        friend class OswHal;
    };

    // Setup
    void setup(bool fromLightSleep);
    void setupFileSystem(void);
    void setupButtons();
    void setupDisplay(bool fromLightSleep);
    void setupPower(bool fromLightSleep);
#if defined(GPS_EDITION) || defined(GPS_EDITION_ROTATED)
    void setupGps(void);
#endif

    // Stop
    void stop(bool toLightSleep);
    void stopDisplay(bool toLightSleep);
    void stopPower();

    // Buttons (Engine-Style)
    void checkButtons();
    bool btnIsDown(Button btn);
    unsigned long btnIsDownFor(Button btn);
    unsigned long btnIsDownSince(Button btn);
    bool btnHasGoneUp(Button btn);
    bool btnHasGoneDown(Button btn);
    void clearButtonState(Button btn);
    bool btnIsTopAligned(Button btn);
    bool btnIsLeftAligned(Button btn);
    void getButtonCoordinates(Button btn, int16_t& x, int16_t& y);

    // DEPRECATED button methods, use OswAppV2::onButton instead
    bool btnIsDoubleClick(Button btn);
    bool btnIsLongPress(Button btn);
    void suppressButtonUntilUp(Button btn);

#if defined(GPS_EDITION) || defined(GPS_EDITION_ROTATED)
    void vibrate(long millis);
#endif

    // Display
    void setBrightness(uint8_t b, bool storeToNVS = true);
    void increaseBrightness(uint8_t v);
    void decreaseBrightness(uint8_t v);
    void displayOff();
    void displayOn();
    void requestDisableDisplayBuffer();
    void requestEnableDisplayBuffer();
    void disableDisplayBuffer();
    void enableDisplayBuffer();
    bool displayBufferEnabled();
    unsigned long screenOnTime();
    unsigned long screenOffTime();

    /**
     * @brief Get the screen brightness from 0 to 255.
     *
     * @param bool checkHardware If true, we check the brightness on the hardware before return it.
     * @return uint8_t
     */
    uint8_t screenBrightness(bool checkHardware = false);

    Arduino_Canvas_Graphics2D* getCanvas(void);
    Graphics2DPrint* gfx();
    void flushCanvas();
    void loadPNGfromProgmem(Graphics2D* target, const unsigned char* array, unsigned int length);

#if defined(GPS_EDITION) || defined(GPS_EDITION_ROTATED)

    // SD
    void loadOsmTile(Graphics2D* target, int8_t z, float tilex, float tiley, int32_t offsetx, int32_t offsety);
    void loadPNGfromSD(Graphics2D* target, const char* path);
    void setPNGAlphaPlaceHolder(uint16_t color);
    bool hasSD(void);
    bool isSDMounted(void);
    uint64_t sdCardSize(void);
    void sdOff(void);

    // GPS
    HardwareSerial getSerialGPS(void);
    void gpsToSerial(void);
    void gpsParse(void);
    void gpsFullOnGps(void);
    void gpsFullOnGpsGlonassBeidu(void);
    void gpsAdvancedPowerSave(void);
    void gpsStandBy(void);
    void gpsBackupMode(void);
    void setDebugGPS(bool on);
    bool isDebugGPS();

    void gpsForceOn(boolean on);
    bool hasGPS(void);
    bool hasGPSFix(void);
    gps_fix* gpsFix(void);
    double gpsLat(void);
    double gpsLon(void);
    uint8_t gpsNumSatellites(void);
    NMEAGPS* gps(void);

#endif
    // Power
    bool isCharging(void);
    uint16_t getBatteryRaw(const uint16_t numAvg = 8);
    // float getBatteryVoltage(void);
    void updatePowerStatistics(uint16_t currBattery);
    uint8_t getBatteryPercent();
    void setCPUClock(uint8_t mhz);
    uint8_t getCPUClock();
    void deepSleep();
    void lightSleep();
    void handleWakeupFromLightSleep();
    void noteUserInteraction();
    void handleDisplayTimout();

    // Power: WakeUpConfigs
    size_t addWakeUpConfig(const WakeUpConfig& config);
    void removeWakeUpConfig(size_t configId);

    // General time stuff
    void updateTimeProvider();
    void updateTimezoneOffsets();

    // UTC Time
    void setUTCTime(const time_t& epoch);
    time_t getUTCTime();
    void getUTCTime(OswTime& oswTime);

    // Offset getters for primary / secondary time (cached!)
    time_t getTimezoneOffsetPrimary();
    time_t getTimezoneOffsetSecondary();

    // New time functions with offset
    time_t getTime(time_t& offset);
    void getTime(time_t& offset, OswTime& oswTime);
    void getDate(time_t& offset, OswDate& oswDate);

    // For backward compatibility: Local time functions (= primary timezone)
    inline void getLocalTime(OswTime& oswTime) {
        this->getTime(this->timezoneOffsetPrimary, oswTime);
    }
    inline uint32_t getLocalTime() {
        return this->getTime(this->timezoneOffsetPrimary);
    }
    inline void getLocalDate(OswDate& oswDate) {
        this->getDate(this->timezoneOffsetPrimary, oswDate);
    };

    // For backward compatibility: Dual time functions (= secondary timezone)
    inline void getDualTime(OswTime& oswTime) {
        this->getTime(this->timezoneOffsetSecondary, oswTime);
    }
    inline uint32_t getDualTime() {
        return this->getTime(this->timezoneOffsetSecondary);
    }
    inline void getDualDate(OswDate& oswDate) {
        this->getDate(this->timezoneOffsetPrimary, oswDate);
    };

    const std::array<const char*, 7> getWeekDay = {
        LANG_SUNDAY,
        LANG_MONDAY,
        LANG_TUESDAY,
        LANG_WEDNESDAY,
        LANG_THURSDAY,
        LANG_FRIDAY,
        LANG_SATURDAY
    };

    bool _requestDisableBuffer = false;
    bool _requestEnableBuffer = false;

  private:
    Arduino_Canvas_Graphics2D* canvas = nullptr;

    static OswHal* instance;
    OswTimeProvider* timeProvider = nullptr;
    unsigned long _screenOnSince = 0;
    unsigned long _screenOffSince = 0;
    unsigned long _lastUserInteraction = 0;

    // array of available buttons for iteration (e.g. handling)
    bool _btnLastState[BTN_NUMBER];
    bool _btnIsDown[BTN_NUMBER];
    bool _btnGoneUp[BTN_NUMBER];
    bool _btnSuppressUntilUpAgain[BTN_NUMBER];
    bool _btnGoneDown[BTN_NUMBER];
    unsigned long _btnIsDownMillis[BTN_NUMBER];
    bool _btnDoubleClickTimeout[BTN_NUMBER];
    unsigned long _btnDoubleClickMillis[BTN_NUMBER];
    bool _btnDoubleClick[BTN_NUMBER];
    uint8_t _btnDetectDoubleClickCount[BTN_NUMBER];
    bool _btnLongPress[BTN_NUMBER];

    uint8_t _brightness = 0;
    bool _hasGPS = false;
    bool _debugGPS = false;
    bool _isLightSleep = false;

    time_t timezoneOffsetPrimary = 0;
    time_t timezoneOffsetSecondary = 0;

    Preferences powerPreferences;
    FileSystemHal* fileSystem;

    std::unique_ptr<Devices> _devices = nullptr;
#if OSW_PLATFORM_ENVIRONMENT == 1
    std::unique_ptr<Environment> _environment = nullptr;
#endif

    std::list<WakeUpConfig> _wakeUpConfigs;
    WakeUpConfig* _lightSleepWakeUpConfig = nullptr;
    size_t _wakeUpConfigIdCounter = 0;
    std::mutex _wakeUpConfigsMutex;

    OswHal(FileSystemHal* fs);
    ~OswHal();
    void doSleep(bool deepSleep);
    uint16_t getBatteryRawMin();
    uint16_t getBatteryRawMax();
    void expireWakeUpConfigs();
    WakeUpConfig* selectWakeUpConfig();
    void persistWakeUpConfig(OswHal::WakeUpConfig* config, bool toLightSleep);
    std::optional<WakeUpConfig> readAndResetWakeUpConfig(bool fromLightSleep);
    void resetWakeUpConfig(bool useLightSleep);
#if OSW_PLATFORM_IS_FLOW3R_BADGE == 1
    uint8_t readGpioExtender(uint8_t address = 0x6D);
#endif
};

#endif

#include "hal/devices.h"
#include "hal/environment.h"
