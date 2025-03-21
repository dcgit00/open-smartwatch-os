#include OSW_TARGET_PLATFORM_HEADER
#if OSW_PLATFORM_ENVIRONMENT_ACCELEROMETER == 1

#include "apps/watchfaces/OswAppWatchfaceFitness.h"
#include "apps/watchfaces/OswAppWatchfaceDigital.h"
#include "apps/watchfaces/OswAppWatchface.h"

#include <config.h>
#include <gfx_util.h>
#include <OswAppV1.h>
#include <osw_config_keys.h>
#include <osw_hal.h>
#include <time.h>

float OswAppWatchfaceFitness::calculateDistance(uint32_t steps) {
    uint8_t userHeight = OswConfigAllKeys::configHeight.get();
    uint16_t avgDist = ((userHeight * 0.37f) + (userHeight * 0.45f) + (userHeight - 100)) / 3;
    return steps * avgDist * 0.01f ;  // cm -> m
}

uint32_t OswAppWatchfaceFitness::calculateKcalorie(uint32_t steps) {
    uint8_t kcalSteps = 30;    // Generally, 1 kcal is consumed at 30 steps.
    return steps / kcalSteps;  // There is an error in this. By giving a percentage, people can see more exercise
    // effects by marking fewer calories than they actually consumed.
}

static void dateDisplay(const OswDate& oswDate) {
    OswHal* hal = OswHal::getInstance();
    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextMiddleAligned();
    hal->gfx()->setTextRightAligned();
    hal->gfx()->setTextCursor(205, 90);
    try {
        const char* weekday = hal->getWeekDay.at(oswDate.weekDay);
        OswAppWatchfaceDigital::displayWeekDay3(weekday);
    } catch (const std::out_of_range& ignore) {
        OSW_LOG_E("getWeekDay is out of range: ", ignore.what());
    }

    // Date
    hal->gfx()->setTextSize(2);
    hal->gfx()->setTextMiddleAligned();
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(DISP_W / 2 - 30 + hal->gfx()->getTextOfsetColumns(1), 150);
    OswAppWatchfaceDigital::dateOutput(oswDate);
}

static void timeDisplay(const OswTime& oswTime) {
    OswHal* hal = OswHal::getInstance();
    hal->gfx()->printDecimal(oswTime.hour, 2);
    hal->gfx()->print(":");
    hal->gfx()->printDecimal(oswTime.minute, 2);

    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextMiddleAligned();
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(215, DISP_H / 2);
    hal->gfx()->printDecimal(oswTime.second,2);
}

static void digitalWatchDisplay(const OswTime& oswTime) {
    char am[] = "AM";
    char pm[] = "PM";
    OswHal* hal = OswHal::getInstance();

    hal->gfx()->setTextSize(4);
    hal->gfx()->setTextMiddleAligned();
    hal->gfx()->setTextLeftAligned();
    hal->gfx()->setTextCursor(DISP_W / 2 - 30, DISP_W / 2);

    timeDisplay(oswTime);
    if (!OswConfigAllKeys::timeFormat.get()) {
        hal->gfx()->setTextCursor(215, 130);
        if (oswTime.afterNoon) {
            hal->gfx()->print(pm);
        } else {
            hal->gfx()->print(am);
        }
    }
}

void OswAppWatchfaceFitness::showFitnessTracking() {
    OswHal* hal = OswHal::getInstance();

    uint32_t steps = hal->environment()->getStepsToday();
    float dists = OswAppWatchfaceFitness::calculateDistance(steps);
    uint32_t kcals = OswAppWatchfaceFitness::calculateKcalorie(steps);

    uint32_t stepsTarget = OswConfigAllKeys::stepsPerDay.get();
    uint32_t distTarget = OswConfigAllKeys::distPerDay.get();
    uint32_t kcalTarget = OswConfigAllKeys::kcalPerDay.get();

    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 360, 90, 93, 7, changeColor(ui->getDangerColor(), 0.25f));
    // hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 280, 90, 93, 7, dimColor(rgb565(210, 50, 66), 25));
    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 180 + (180.0f * (float)(steps % stepsTarget) / (float)stepsTarget), 90, 93, 6, steps > stepsTarget ? changeColor(ui->getSuccessColor(),6.25f ): ui->getDangerColor(), true);

    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 360, 90, 75, 7, changeColor(ui->getWarningColor(), 0.25f));
    // hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 180+128, 90, 75, 7, dimColor(rgb565(117, 235, 10), 25));
    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 180 + (180.0f * (float)(kcals % kcalTarget) / (float)kcalTarget), 90, 75, 6,  kcals > kcalTarget ? changeColor(ui->getSuccessColor(),1.25f): ui->getWarningColor(), true);

    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 360, 90, 57, 7, changeColor(ui->getInfoColor(), 0.25f));
    // hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 180+32, 90, 57, 7, dimColor(rgb565(25, 193, 202), 25));
    hal->gfx()->drawArc(DISP_W / 2, DISP_H / 2, 180, 180 + (180.0f * (float)(uint32_t(dists) % distTarget) / (float)distTarget), 90, 57, 6, dists > distTarget  ? changeColor(ui->getSuccessColor(),2.25f) : ui->getInfoColor(), true);

    hal->gfx()->setTextSize(1);
    hal->gfx()->setTextMiddleAligned();
    hal->gfx()->setTextLeftAligned();

    hal->gfx()->setTextColor(dimColor(ui->getDangerColor(), 25));
    hal->gfx()->setTextCursor(DISP_W / 2 + 10, 25);
    hal->gfx()->print(steps);

    hal->gfx()->setTextColor(dimColor(ui->getWarningColor(), 25));
    hal->gfx()->setTextCursor(DISP_W / 2 + 10, 45);
    hal->gfx()->print(kcals);

    hal->gfx()->setTextColor(dimColor(ui->getInfoColor(), 25));
    hal->gfx()->setTextCursor(DISP_W / 2 + 10, 65);
    hal->gfx()->print(dists);

    hal->gfx()->setTextColor(dimColor(ui->getInfoColor(), 25));
    hal->gfx()->setTextCursor(DISP_W / 2 + 10, DISP_H-65);
    hal->gfx()->print(LANG_WATCHFACE_FITNESS_DISTANCE);

    hal->gfx()->setTextColor(dimColor(ui->getWarningColor(), 25));
    hal->gfx()->setTextCursor(DISP_W / 2 + 10, DISP_H-45);
    hal->gfx()->print("kcal");

    hal->gfx()->setTextColor(dimColor(ui->getDangerColor(), 25));
    hal->gfx()->setTextCursor(DISP_W / 2 + 10, DISP_H-25);
    hal->gfx()->print(LANG_WATCHFACE_FITNESS_STEP);
}

const char* OswAppWatchfaceFitness::getAppId() {
    return OswAppWatchfaceFitness::APP_ID;
}

const char* OswAppWatchfaceFitness::getAppName() {
    return LANG_FITNESS;
}

void OswAppWatchfaceFitness::onStart() {
    OswAppV2::onStart();
    OswAppWatchface::addButtonDefaults(this->knownButtonStates);
}

void OswAppWatchfaceFitness::onLoop() {
    OswAppV2::onLoop();

    this->needsRedraw = this->needsRedraw or time(nullptr) != this->lastTime; // redraw every second
}

void OswAppWatchfaceFitness::onDraw() {
    OswAppV2::onDraw();

    OswDate oswDate = { };
    OswTime oswTime = { };
    hal->getLocalDate(oswDate);
    hal->getLocalTime(oswTime);

    dateDisplay(oswDate);
    digitalWatchDisplay(oswTime);

#if OSW_PLATFORM_ENVIRONMENT_ACCELEROMETER == 1
    showFitnessTracking();
#endif

    this->lastTime = time(nullptr);
}

void OswAppWatchfaceFitness::onButton(Button id, bool up, OswAppV2::ButtonStateNames state) {
    OswAppV2::onButton(id, up, state);
    if(OswAppWatchface::onButtonDefaults(*this, id, up, state))
        return; // if the button was handled by the defaults, we are done here
}
#endif
