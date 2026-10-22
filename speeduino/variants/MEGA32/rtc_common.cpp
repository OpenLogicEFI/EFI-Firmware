// MEGA32 date/time clock handling. All changes stay in this variant.
// We use ST's hardware functions (HAL) and check their results ourselves.
// This avoids the STM32RTC library's startup paths that can hide errors or
// stop the firmware. The shared Speeduino files and library stay unchanged.
#include "globals.h"
#include "rtc_common.h"
#include "rtc_clock.h"
#include "stm32/backup.h"

#if !defined(OPENLOGICEFI_MEGA32) || !defined(STM32F407xx)
#error "This RTC backend is only for the MEGA32 STM32F407 variant"
#endif

#ifdef RTC_ENABLED
volatile Mega32RtcStatus g_mega32RtcStatus = {
  Mega32RtcState::Uninitialized, Mega32RtcSource::None,
  Mega32RtcError::None, false, false, 0
};

namespace {
RTC_HandleTypeDef rtcHandle = {};
bool backupWritable = false;

// Stop using the RTC after an error, but let the rest of the firmware run.
// Also clear the saved "time is valid" marker when we have permission to do so.
bool unavailable(Mega32RtcError error)
{
  if (backupWritable) { setBackupRegister(RTC_BKP_INDEX, 0); }
  g_mega32RtcStatus.state = Mega32RtcState::Unavailable;
  g_mega32RtcStatus.source = Mega32RtcSource::None;
  g_mega32RtcStatus.timeValid = false;
  g_mega32RtcStatus.error = error;
  return false;
}

// Turn an RTC clock on or off and report whether the hardware call worked.
bool oscillator(uint32_t type, uint32_t state)
{
  RCC_OscInitTypeDef config = {};
  config.OscillatorType = type;
  config.PLL.PLLState = RCC_PLL_NONE; // Leave the CPU and engine timer clocks alone.
  if (type == RCC_OSCILLATORTYPE_LSE) { config.LSEState = state; }
  else { config.LSIState = state; }
  return HAL_RCC_OscConfig(&config) == HAL_OK;
}

// Divide the clock down to one tick per second, as RTC library 1.2.0 does.
// These dividers cannot correct the internal LSI clock's natural timing error.
void prescalers(uint32_t frequency, uint32_t &async, uint32_t &sync)
{
  async = 127;
  for (int32_t divider = 128; divider > 0; --divider) {
    if ((frequency % divider) == 0 && (frequency / divider) <= 32768) {
      async = divider - 1;
      break;
    }
  }
  sync = frequency / (async + 1) - 1;
}

// Reject impossible dates and times before sending them to the hardware.
bool validDateTime(uint16_t year, uint8_t month, uint8_t day,
                   uint8_t hour, uint8_t minute, uint8_t second)
{
  static const uint8_t days[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if (year < 2000 || year > 2099 || month < 1 || month > 12 ||
      day < 1 || hour > 23 || minute > 59 || second > 59) { return false; }
  const uint8_t maximum = days[month - 1] + ((month == 2 && year % 4 == 0) ? 1 : 0);
  return day <= maximum;
}

bool readCalendar(RTC_TimeTypeDef &time, RTC_DateTypeDef &date)
{
  if (g_mega32RtcStatus.state != Mega32RtcState::Ready) { return false; }
  // The hardware holds a snapshot after we read the time. Reading the date
  // releases it, so always read both, in this order, even if time reading fails.
  const HAL_StatusTypeDef timeResult = HAL_RTC_GetTime(&rtcHandle, &time, RTC_FORMAT_BIN);
  const HAL_StatusTypeDef dateResult = HAL_RTC_GetDate(&rtcHandle, &date, RTC_FORMAT_BIN);
  if (timeResult != HAL_OK || dateResult != HAL_OK ||
      !validDateTime(2000 + date.Year, date.Month, date.Date,
                     time.Hours, time.Minutes, time.Seconds)) {
    return unavailable(Mega32RtcError::Read);
  }
  return true;
}

bool initialize()
{
  // Ask for permission to change the battery-backed RTC settings.
  // Give up if permission is not granted within the hardware timeout.
  backupWritable = false;
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_EnableBkUpAccess();
  const uint32_t start = HAL_GetTick();
  while ((PWR->CR & PWR_CR_DBP) == 0) {
    if (HAL_GetTick() - start >= RCC_DBP_TIMEOUT_VALUE) {
      return unavailable(Mega32RtcError::BackupAccess);
    }
  }

  backupWritable = true;

  // Check what survived the last power cycle BEFORE trying to start a clock.
  // A saved time is not trusted if its clock was stopped or setup unfinished.
  const uint32_t previousSource = RCC->BDCR & RCC_BDCR_RTCSEL;
  const bool previouslyRunning = (RCC->BDCR & RCC_BDCR_RTCEN) != 0 &&
    ((previousSource == RCC_RTCCLKSOURCE_LSE && __HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY)) ||
     (previousSource == RCC_RTCCLKSOURCE_LSI && __HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY)));
  const bool previouslySet = previouslyRunning &&
    (RTC->ISR & RTC_ISR_INITS) != 0 && (RTC->ISR & RTC_ISR_INIT) == 0 &&
    getBackupRegister(RTC_BKP_INDEX) == RTC_BKP_VALUE;

  // Prefer the external crystal. If it fails, try the internal clock.
  Mega32RtcSource source = Mega32RtcSource::Lse;
  uint32_t selection = RCC_RTCCLKSOURCE_LSE;
  if (!oscillator(RCC_OSCILLATORTYPE_LSE, RCC_LSE_ON) ||
      !__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY)) {
    g_mega32RtcStatus.lseFailed = true;
    // Fully turn off the failed crystal first. Otherwise, switching clocks can
    // make the hardware code try that failed crystal again and wait a second
    // time. LSEON is the hardware bit that says the crystal is switched on.
    if (!oscillator(RCC_OSCILLATORTYPE_LSE, RCC_LSE_OFF) ||
        (RCC->BDCR & RCC_BDCR_LSEON) != 0 || __HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY)) {
      return unavailable(Mega32RtcError::LseCleanup);
    }
    if (!oscillator(RCC_OSCILLATORTYPE_LSI, RCC_LSI_ON) ||
        !__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY)) {
      return unavailable(Mega32RtcError::LsiStartup);
    }
    source = Mega32RtcSource::Lsi;
    selection = RCC_RTCCLKSOURCE_LSI;
  }

  // Choose the RTC clock and check that the hardware accepted our choice.
  // Changing this choice may erase the battery-backed time and saved data.
  RCC_PeriphCLKInitTypeDef clocks = {};
  clocks.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  clocks.RTCClockSelection = selection;
  if (HAL_RCCEx_PeriphCLKConfig(&clocks) != HAL_OK ||
      (RCC->BDCR & RCC_BDCR_RTCSEL) != selection) {
    return unavailable(Mega32RtcError::ClockSelection);
  }
  __HAL_RCC_RTC_ENABLE();

  // Keep saved time only if it was trustworthy and the clock did not change.
  const bool retained = previouslySet && previousSource == selection;
  if (!retained) { setBackupRegister(RTC_BKP_INDEX, 0); }

  rtcHandle = {};
  rtcHandle.Instance = RTC;
  rtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  rtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  rtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  rtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  prescalers(source == Mega32RtcSource::Lse ? LSE_VALUE : LSI_VALUE,
             rtcHandle.Init.AsynchPrediv, rtcHandle.Init.SynchPrediv);
  // Enable the hardware snapshot used by readCalendar(). It keeps the time
  // and date from disagreeing if midnight falls between the two reads.
  if (HAL_RTCEx_DisableBypassShadow(&rtcHandle) != HAL_OK ||
      HAL_RTC_Init(&rtcHandle) != HAL_OK) {
    return unavailable(Mega32RtcError::Initialization);
  }

  // If saved time was lost, start at 2000-01-01 so the rest of the firmware
  // receives a usable date. This is a placeholder, not the correct date/time.
  // Leave the saved "time is valid" marker cleared until the user sets time.
  if (!retained) {
    RTC_TimeTypeDef time = {};
    RTC_DateTypeDef date = {};
    date.Year = 0;
    date.Month = 1;
    date.Date = 1;
    date.WeekDay = RTC_WEEKDAY_SATURDAY; // 2000-01-01
    if (HAL_RTC_SetTime(&rtcHandle, &time, RTC_FORMAT_BIN) != HAL_OK ||
        HAL_RTC_SetDate(&rtcHandle, &date, RTC_FORMAT_BIN) != HAL_OK) {
      return unavailable(Mega32RtcError::Initialization);
    }
  }
  g_mega32RtcStatus.state = Mega32RtcState::Ready;
  g_mega32RtcStatus.source = source;
  g_mega32RtcStatus.timeValid = retained;
  RTC_TimeTypeDef time = {};
  RTC_DateTypeDef date = {};
  return readCalendar(time, date);
}
} // namespace

bool mega32RtcBegin()
{
  // Start only once per boot. Do not repeat these waits while the engine runs.
  if (g_mega32RtcStatus.state != Mega32RtcState::Uninitialized) {
    return g_mega32RtcStatus.state == Mega32RtcState::Ready;
  }
  const uint32_t start = HAL_GetTick();
  const bool result = initialize();
  g_mega32RtcStatus.startupMs = HAL_GetTick() - start;
  return result;
}

void initRTC() {} // The board startup code has already started the RTC.

// Keep the date/time functions that the rest of Speeduino already calls.
// If the RTC failed, return 2000-01-01 00:00:00 as a placeholder.
// Do not try to read hardware that failed to start.
uint8_t rtc_getSecond() {
  RTC_TimeTypeDef t = {}; RTC_DateTypeDef d = {};
  return readCalendar(t, d) ? t.Seconds : 0;
}
uint8_t rtc_getMinute() {
  RTC_TimeTypeDef t = {}; RTC_DateTypeDef d = {};
  return readCalendar(t, d) ? t.Minutes : 0;
}
uint8_t rtc_getHour() {
  RTC_TimeTypeDef t = {}; RTC_DateTypeDef d = {};
  return readCalendar(t, d) ? t.Hours : 0;
}
uint8_t rtc_getDay() {
  RTC_TimeTypeDef t = {}; RTC_DateTypeDef d = {};
  return readCalendar(t, d) ? d.Date : 1;
}
uint8_t rtc_getDOW() {
  RTC_TimeTypeDef t = {}; RTC_DateTypeDef d = {};
  return readCalendar(t, d) ? d.WeekDay : RTC_WEEKDAY_SATURDAY;
}
uint8_t rtc_getMonth() {
  RTC_TimeTypeDef t = {}; RTC_DateTypeDef d = {};
  return readCalendar(t, d) ? d.Month : 1;
}
uint16_t rtc_getYear() {
  RTC_TimeTypeDef t = {}; RTC_DateTypeDef d = {};
  return readCalendar(t, d) ? 2000 + d.Year : 2000;
}

void rtc_setTime(byte second, byte minute, byte hour, byte day, byte month, uint16_t year)
{
  if (g_mega32RtcStatus.state != Mega32RtcState::Ready) { return; }
  if (!validDateTime(year, month, day, hour, minute, second)) {
    g_mega32RtcStatus.error = Mega32RtcError::InvalidDateTime;
    return; // Keep the existing time if the new date/time is impossible.
  }
  RTC_TimeTypeDef time = {};
  time.Hours = hour; time.Minutes = minute; time.Seconds = second;
  RTC_DateTypeDef date = {};
  date.Year = year - 2000; date.Month = month; date.Date = day;
  // Calculate the day of the week from the supplied date.
  // The STM32 hardware expects Monday=1 through Sunday=7.
  static const uint8_t offsets[] = {0,3,2,5,0,3,5,1,4,6,2,4};
  const uint16_t y = year - (month < 3 ? 1 : 0);
  const uint8_t dow = (y + y/4 - y/100 + y/400 + offsets[month-1] + day) % 7;
  date.WeekDay = dow == 0 ? 7 : dow;

  // Only mark the new time valid after BOTH the time and date were written.
  // Do not restart the RTC or try the external crystal again here.
  g_mega32RtcStatus.timeValid = false;
  setBackupRegister(RTC_BKP_INDEX, 0);
  if (HAL_RTC_SetTime(&rtcHandle, &time, RTC_FORMAT_BIN) != HAL_OK ||
      HAL_RTC_SetDate(&rtcHandle, &date, RTC_FORMAT_BIN) != HAL_OK) {
    unavailable(Mega32RtcError::Write);
    return;
  }
  setBackupRegister(RTC_BKP_INDEX, RTC_BKP_VALUE);
  g_mega32RtcStatus.timeValid = true;
  g_mega32RtcStatus.error = Mega32RtcError::None;
}
#endif
