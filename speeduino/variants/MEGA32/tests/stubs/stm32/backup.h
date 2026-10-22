#pragma once
#include <cstdint>
constexpr uint32_t RTC_BKP_INDEX = 1, RTC_BKP_VALUE = 0x32F2;
uint32_t getBackupRegister(uint32_t);
void setBackupRegister(uint32_t, uint32_t);
