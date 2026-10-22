#pragma once
#include <cstdint>
using byte = uint8_t;
#define RTC_ENABLED
#define OPENLOGICEFI_MEGA32
#define STM32F407xx
// Fake STM32 hardware for tests run on the computer.
// The board firmware does not use these files.
enum HAL_StatusTypeDef { HAL_OK, HAL_ERROR, HAL_TIMEOUT };
struct FakeRcc { uint32_t BDCR = 0, CSR = 0; };
struct FakePwr { uint32_t CR = 0; };
struct FakeRtc { uint32_t ISR = 0; };
extern FakeRcc fakeRcc;
extern FakePwr fakePwr;
extern FakeRtc fakeRtc;
#define RCC (&fakeRcc)
#define PWR (&fakePwr)
#define RTC (&fakeRtc)
constexpr uint32_t RCC_BDCR_LSEON = 1, RCC_FLAG_LSERDY = 2;
constexpr uint32_t RCC_BDCR_RTCSEL = 0x300, RCC_BDCR_RTCEN = 0x8000;
constexpr uint32_t RCC_RTCCLKSOURCE_LSE = 0x100, RCC_RTCCLKSOURCE_LSI = 0x200;
constexpr uint32_t PWR_CR_DBP = 0x100;
constexpr uint32_t RTC_ISR_INITS = 0x10, RTC_ISR_INIT = 0x80;
constexpr uint32_t RCC_OSCILLATORTYPE_LSE = 1, RCC_OSCILLATORTYPE_LSI = 2;
constexpr uint32_t RCC_LSE_ON = 1, RCC_LSE_OFF = 0, RCC_LSI_ON = 1;
constexpr uint32_t RCC_PLL_NONE = 0, RCC_PERIPHCLK_RTC = 1;
constexpr uint32_t RCC_DBP_TIMEOUT_VALUE = 100;
constexpr uint32_t RTC_HOURFORMAT_24 = 0, RTC_OUTPUT_DISABLE = 0;
constexpr uint32_t RTC_OUTPUT_POLARITY_HIGH = 0, RTC_OUTPUT_TYPE_OPENDRAIN = 0;
constexpr uint32_t LSE_VALUE = 32768, LSI_VALUE = 32000, RTC_FORMAT_BIN = 0;
constexpr uint8_t RTC_WEEKDAY_SATURDAY = 6;
struct RCC_OscInitTypeDef {
  uint32_t OscillatorType = 0, LSEState = 0, LSIState = 0;
  struct { uint32_t PLLState = 0; } PLL;
};
struct RCC_PeriphCLKInitTypeDef { uint32_t PeriphClockSelection = 0, RTCClockSelection = 0; };
struct RTC_HandleTypeDef {
  FakeRtc *Instance = nullptr;
  struct {
    uint32_t HourFormat = 0, OutPut = 0, OutPutPolarity = 0, OutPutType = 0;
    uint32_t AsynchPrediv = 0, SynchPrediv = 0;
  } Init;
};
struct RTC_TimeTypeDef { uint8_t Hours = 0, Minutes = 0, Seconds = 0; };
struct RTC_DateTypeDef { uint8_t Year = 0, Month = 0, Date = 0, WeekDay = 0; };
void fakePowerClock();
void fakeEnableRtc();
bool fakeLseReady();
bool fakeLsiReady();
// Give the two fake clock-ready flags different names/numbers so tests
// can tell whether the code is checking the internal or external clock.
#define RCC_FLAG_LSIRDY 0x10000U
#define __HAL_RCC_PWR_CLK_ENABLE() fakePowerClock()
#define __HAL_RCC_RTC_ENABLE() fakeEnableRtc()
#define __HAL_RCC_GET_FLAG(flag) ((flag) == RCC_FLAG_LSERDY ? fakeLseReady() : fakeLsiReady())
uint32_t HAL_GetTick();
void HAL_PWR_EnableBkUpAccess();
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *);
HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef *);
HAL_StatusTypeDef HAL_RTCEx_DisableBypassShadow(RTC_HandleTypeDef *);
HAL_StatusTypeDef HAL_RTC_Init(RTC_HandleTypeDef *);
HAL_StatusTypeDef HAL_RTC_GetTime(RTC_HandleTypeDef *, RTC_TimeTypeDef *, uint32_t);
HAL_StatusTypeDef HAL_RTC_GetDate(RTC_HandleTypeDef *, RTC_DateTypeDef *, uint32_t);
HAL_StatusTypeDef HAL_RTC_SetTime(RTC_HandleTypeDef *, RTC_TimeTypeDef *, uint32_t);
HAL_StatusTypeDef HAL_RTC_SetDate(RTC_HandleTypeDef *, RTC_DateTypeDef *, uint32_t);
