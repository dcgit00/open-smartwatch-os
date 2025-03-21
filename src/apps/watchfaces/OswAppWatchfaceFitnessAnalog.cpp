#include "apps/watchfaces/OswAppWatchfaceFitnessAnalog.h"
#include "apps/watchfaces/OswAppWatchfaceDigital.h"
#include "apps/watchfaces/OswAppWatchface.h"

#if OSW_PLATFORM_ENVIRONMENT_ACCELEROMETER == 1

#ifdef GIF_BG
#include "./apps/_experiments/gif_player.h"
#endif

#include <config.h>
#include <gfx_util.h>
#include <OswAppV1.h>
#include <osw_config_keys.h>
#include <osw_hal.h>
#include <time.h>

#define CENTER_X (DISP_W / 2)
#define CENTER_Y (DISP_H / 2)

inline uint32_t OswAppWatchfaceFitnessAnalog::calculateDistance(uint32_t steps) {
    float userHeight = OswConfigAllKeys::configHeight.get();
    float avgDist;
    if (userHeight < 180)
        avgDist = userHeight * 0.40f;
    else
        avgDist = userHeight * 0.48f;

    return steps * avgDist * 0.01f;  // cm -> m
}

void OswAppWatchfaceFitnessAnalog::showFitnessTracking(OswHal* hal) {
    uint32_t steps = hal->environment()->getStepsToday();
    uint32_t dists = OswAppWatchfaceFitnessAnalog::calculateDistance(steps);

    uint32_t stepsTarget = OswConfigAllKeys::stepsPerDay.get();
    uint32_t distTarget = OswConfigAllKeys::distPerDay.get();

    uint8_t arcRadius = 6;
    uint16_t yellow = rgb565(255, 255,0);

#ifdef OSW_EMULATOR
    steps = 4000;
    dists = 3000;
#endif

    {
        // draw step arc
        int32_t angle_val = 180.0f * (float)min(steps, stepsTarget) / (float)stepsTarget;
        uint16_t color = yellow;
        uint16_t dimmed_color = changeColor(color, 0.25f);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 92 +arcRadius, arcRadius*2, dimmed_color, 90, 270-angle_val);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y -92, arcRadius, 0, dimmed_color);

        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 92 +arcRadius, arcRadius*2, steps > stepsTarget ? dimmed_color : color, 270-angle_val, 270);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y +92, arcRadius, 0, steps > stepsTarget ? dimmed_color : color);
        int x = CENTER_X + cosf((270-angle_val)*PI/180) * 92.0f;
        int y = CENTER_Y - sinf((270-angle_val)*PI/180) * 92.0f;
        hal->gfx()->drawCircleAA(x, y, arcRadius, 0, color);
    }

    {
        // draw distance arc
        int32_t angle_val = 180.0f * (float)min(dists, distTarget) / (float)distTarget;
        uint16_t color = ui->getInfoColor();
        uint16_t dimmed_color = changeColor(color, 0.25f);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 75 +arcRadius, arcRadius*2, dimmed_color, 90, 270-angle_val);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y -75, arcRadius, 0, dimmed_color, 0.25f);

        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y, 75 +arcRadius, arcRadius*2, steps > stepsTarget ? dimmed_color: color, 270-angle_val, 270);
        hal->gfx()->drawCircleAA(CENTER_X, CENTER_Y +75, arcRadius, 0, steps > stepsTarget ?  dimmed_color: color);
        int x = CENTER_X + cosf((270-angle_val)*PI/180) * 75.0f;
        int y = CENTER_Y - sinf((270-angle_val)*PI/180) * 75.0f;
        hal->gfx()->drawCircleAA(x, y, arcRadius, 0, color);
    }

    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextLeftAligned();

    hal->gfx()->setTextColor(dimColor(yellow, 25));
    hal->gfx()->setTextCursor(CENTER_X + 12, 8+23);
    hal->gfx()->print(steps);
    hal->gfx()->setTextCursor(CENTER_X + 12, DISP_H-23);
    hal->gfx()->print(LANG_WATCHFACE_FITNESS_STEP);

    hal->gfx()->setTextColor(dimColor(ui->getInfoColor(), 24));
    hal->gfx()->setTextCursor(CENTER_X + 12, 8+40);
    hal->gfx()->print(dists);
    hal->gfx()->setTextCursor(CENTER_X + 12, DISP_H-40);
    hal->gfx()->print(LANG_WATCHFACE_FITNESS_DISTANCE);
}

void OswAppWatchfaceFitnessAnalog::drawWatchFace(OswHal* hal, const OswTime& oswTime) {
    // Indices
    hal->gfx()->drawMinuteTicks(CENTER_X, CENTER_Y, 116, 112, ui->getForegroundDimmedColor(), true);
    hal->gfx()->drawHourTicks(CENTER_X, CENTER_Y, 117, 107, ui->getForegroundColor(), true);

    // Hours
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y,  0, 16, (int)(360.0f / 12.0f * (oswTime.hour + oswTime.minute / 60.0f)), 3, ui->getForegroundColor(), true, STRAIGHT_END);
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y, 16, 60, (int)(360.0f / 12.0f * (oswTime.hour + oswTime.minute / 60.0f)), 7, ui->getForegroundColor(), true);

    // Minutes
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y,  0, 16, (int)(360.0f / 60.0f * (oswTime.minute + oswTime.second / 60.0f)), 3, ui->getForegroundColor(), true, STRAIGHT_END);
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y, 16, 105, (int)(360.0f / 60.0f * (oswTime.minute + oswTime.second / 60.0f)), 7, ui->getForegroundColor(), true);

#ifndef GIF_BG
    // Seconds
    hal->gfx()->fillCircleAA(CENTER_X, CENTER_Y, 6, ui->getDangerColor());
    hal->gfx()->drawThickTick(CENTER_X, CENTER_Y, -16, 110, 360 / 60 * oswTime.second, 3, ui->getDangerColor(), true);
#endif
}

void OswAppWatchfaceFitnessAnalog::drawDateFace(OswHal* hal, const OswDate& oswDate, const OswTime& oswTime) {
    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextRightAligned();
    hal->gfx()->setTextCursor(205, 75);
    try {
        const char* weekday = hal->getWeekDay.at(oswDate.weekDay);
        OswAppWatchfaceDigital::displayWeekDay3(weekday);
    } catch (const std::out_of_range& ignore) {
        OSW_LOG_E("getWeekDay is out of range: ", ignore.what());
    }

    hal->gfx()->setTextSize(3);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(CENTER_X - 70, 170);
    OswAppWatchfaceDigital::dateOutput(oswDate);

    hal->gfx()->setTextSize(4);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(CENTER_X - 35, CENTER_Y);

    hal->gfx()->printDecimal(oswTime.hour, 2);
    hal->gfx()->print(":");
    hal->gfx()->printDecimal(oswTime.minute, 2);

    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(215, CENTER_Y);
    hal->gfx()->printDecimal(oswTime.second,2);

    if (!OswConfigAllKeys::timeFormat.get()) {
        const char am[] = "AM";
        const char pm[] = "PM";
        hal->gfx()->setTextCursor(215, 130);
        if (oswTime.afterNoon) {
            hal->gfx()->print(pm);
        } else {
            hal->gfx()->print(am);
        }
    }

#if OSW_PLATFORM_ENVIRONMENT_TEMPERATURE == 1
    /*
        printStatus("Temperature", String(hal->environment()->getTemperature() + String("C")).c_str());
        for(auto& d : *OswTemperatureProvider::getAllTemperatureDevices())
            printStatus((String("  ") + d->getName()).c_str(), String(d->getTemperature() + String("C")).c_str());
    */
    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(DISP_W * 0.2f, DISP_H * 0.2f);
    hal->gfx()->print(hal->environment()->getTemperature(), 1);
    hal->gfx()->print("C");
#endif
}

const char* OswAppWatchfaceFitnessAnalog::getAppId() {
    return OswAppWatchfaceFitnessAnalog::APP_ID;
}

const char* OswAppWatchfaceFitnessAnalog::getAppName() {
    return LANG_AFIT;
}

void OswAppWatchfaceFitnessAnalog::onStart() {
    OswAppV2::onStart();
    OswAppWatchface::addButtonDefaults(this->knownButtonStates);

    // double press on any button to switch to the alternative screen
    this->knownButtonStates[Button::BUTTON_SELECT] = (OswAppV2::ButtonStateNames) (this->knownButtonStates[Button::BUTTON_SELECT] | OswAppV2::ButtonStateNames::DOUBLE_PRESS); // OR to set the bit
    this->knownButtonStates[Button::BUTTON_UP] = (OswAppV2::ButtonStateNames) (this->knownButtonStates[Button::BUTTON_UP] | OswAppV2::ButtonStateNames::DOUBLE_PRESS); // OR to set the bit
    this->knownButtonStates[Button::BUTTON_DOWN] = (OswAppV2::ButtonStateNames) (this->knownButtonStates[Button::BUTTON_DOWN] | OswAppV2::ButtonStateNames::DOUBLE_PRESS); // OR to set the bit

    this->lastTime = time(nullptr); // use
}

void OswAppWatchfaceFitnessAnalog::onLoop() {
    OswAppV2::onLoop();

    this->needsRedraw = this->needsRedraw or time(nullptr) != this->lastTime; // redraw every second
}

void OswAppWatchfaceFitnessAnalog::onDraw() {
    OswAppV2::onDraw();

#ifdef GIF_BG
    if(this->bgGif != nullptr)
        this->bgGif->loop();
#endif

    OswHal* hal = OswHal::getInstance();

    OswDate oswDate = { };
    OswTime oswTime = { };
    hal->getLocalDate(oswDate);
    hal->getLocalTime(oswTime);

    if (this->screen == 0) {
#if OSW_PLATFORM_ENVIRONMENT_ACCELEROMETER == 1
        showFitnessTracking(hal);
#endif

        drawWatchFace(hal, oswTime);
    } else if (this->screen == 1) {
        drawDateFace(hal, oswDate, oswTime);

        static int wait_time = 1;
        if (wait_time >= 0)
            --wait_time;
        else {
            this->screen = 0;
            wait_time = 1;
        }
    }

    this->lastTime = time(nullptr);
}

void OswAppWatchfaceFitnessAnalog::onButton(Button id, bool up, OswAppV2::ButtonStateNames state) {
    OswAppV2::onButton(id, up, state);

    if(!up) return;

    if (state == OswAppV2::ButtonStateNames::DOUBLE_PRESS) {
        if (this->screen < 1)
            ++this->screen;
        return;
    }

    if(OswAppWatchface::onButtonDefaults(*this, id, up, state))
        return; // if the button was handled by the defaults, we are done here
}

void OswAppWatchfaceFitnessAnalog::onStop() {
    OswAppV2::onStop(); // always make sure to call the base class method!
    // This is where you de-initialize stuff, gets called when another app is shown

#ifdef GIF_BG
    if(this->bgGif != nullptr) {
        this->bgGif->stop();
        delete this->bgGif;
        this->bgGif = nullptr;
    }
#endif
}
#endif
