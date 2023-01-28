#pragma once

#include <cstdint>
#include <math.h>

#define RTC_DATA_ATTR

enum esp_err_t: int {
  ESP_FAIL = -1,
  ESP_OK = 0
  // Add more as needed...
};

enum esp_sleep_ext1_wakeup_mode_t: int {
  ESP_EXT1_WAKEUP_ALL_LOW,
  ESP_EXT1_WAKEUP_ANY_HIGH
};