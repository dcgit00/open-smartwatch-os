#pragma once
#include <array>

#include <OswIcon.h>
#include <osw_hal.h>

class OswHal;
class OswAppV2 {
  public:
    enum ViewFlags: char {
      NONE = 0,
      NO_OVERLAYS = 1,
      KEEP_DISPLAY_ON = 2,
      NO_FPS_LIMIT = 4
    };
    enum ButtonStateNames: char {
      UNDEFINED = 0,
      SHORT_PRESS = 1,
      LONG_PRESS = 2,
      VERY_LONG_PRESS = 4,
      DOUBLE_PRESS = 8
    };

    OswAppV2();
    virtual ~OswAppV2() = default;

    virtual const char* getAppId() = 0;
    virtual const char* getAppName() = 0;
    virtual OswIcon& getAppIcon();

    virtual void onStart();
    virtual void onLoop();
    virtual void onDraw();
    virtual void onDrawOverlay();
    virtual void onStop();

    virtual void onButton(int id, bool up, ButtonStateNames state);
#ifdef OSW_EMULATOR
    virtual void onLoopDebug(); // By default no debug loop (GUI) is implemented
#endif

    virtual ViewFlags getViewFlags();
    virtual bool getNeedsRedraw();
    virtual void resetNeedsRedraw();
  protected:
    class OswHalProxy {
    public:
      // This will proxy the "->" operator to use the current instance of OswHal
      OswHal* operator->() { return OswHal::getInstance(); };
      // We intentionally do not provide an operation to implicitly convert to OswHal* to prevent accidental use of the wrong instance
    };
    OswHalProxy hal; // You guys are needing that anyways (but you often cache incorrectly), so it is now given to you <3
    std::array<ButtonStateNames, NUM_BUTTONS> knownButtonStates; // Bitmask of known button states (ignores the DOUBLE_PRESS state by default), use this to ignore unhandled button states
    ViewFlags viewFlags = ViewFlags::NONE;
    bool needsRedraw = false;
    OswIcon& getDefaultAppIcon();

  private:
    static OswIcon defaultAppIcon;
    // the button states-variables (↓) are shared between all apps, so they do not detect presses, which were already handled by another app (which may wrapped them)
    static std::array<unsigned long, NUM_BUTTONS> buttonDownSince;
    static std::array<ButtonStateNames, NUM_BUTTONS> buttonLastSentState;
    static std::array<unsigned long, NUM_BUTTONS> buttonDoubleShortTimeout;
    static std::array<float, NUM_BUTTONS> buttonIndicatorProgress;
};