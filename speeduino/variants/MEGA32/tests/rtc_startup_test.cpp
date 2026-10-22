// Test the real MEGA32 RTC code using fake hardware that can be made to fail.
// These tests run on the computer; they do not prove that a real crystal works.
#include "globals.h"
#include "stm32/backup.h"
#include "rtc_clock.h"
#include "rtc_common.h"
#include <cassert>
#include <cstdio>

FakeRcc fakeRcc;
FakePwr fakePwr;
FakeRtc fakeRtc;
namespace {
bool lseWorks, lsiWorks, cleanupWorks, accessWorks, selectionWorks, initWorks;
bool readsWork, timeWriteWorks, dateWriteWorks, bypassWorks;
uint32_t ticks, marker;
int oscillatorCalls, switches, initCalls, writes, reads;
bool timeLocked;
RTC_TimeTypeDef calendarTime;
RTC_DateTypeDef calendarDate;
uint32_t asyncDivider, syncDivider;

void reset() {
  fakeRcc = {}; fakePwr = {}; fakeRtc = {};
  lseWorks = lsiWorks = cleanupWorks = accessWorks = selectionWorks = initWorks = true;
  readsWork = timeWriteWorks = dateWriteWorks = bypassWorks = true;
  ticks = marker = 0; oscillatorCalls = switches = initCalls = writes = reads = 0;
  timeLocked = false; calendarTime = {}; calendarDate = {26,10,22,4};
  g_mega32RtcStatus.state = Mega32RtcState::Uninitialized;
  g_mega32RtcStatus.source = Mega32RtcSource::None;
  g_mega32RtcStatus.error = Mega32RtcError::None;
  g_mega32RtcStatus.timeValid = g_mega32RtcStatus.lseFailed = false;
  g_mega32RtcStatus.startupMs = 0;
}
void retained(uint32_t source) {
  fakeRcc.BDCR = source | RCC_BDCR_RTCEN;
  if (source == RCC_RTCCLKSOURCE_LSE) { fakeRcc.BDCR |= RCC_BDCR_LSEON | 2; }
  else { fakeRcc.CSR = 2; }
  fakeRtc.ISR = RTC_ISR_INITS;
  marker = RTC_BKP_VALUE;
  calendarTime = {12,34,56};
}
void expectUnavailable(Mega32RtcError error) {
  assert(!mega32RtcBegin());
  assert(g_mega32RtcStatus.state == Mega32RtcState::Unavailable);
  assert(g_mega32RtcStatus.source == Mega32RtcSource::None);
  assert(!g_mega32RtcStatus.timeValid && g_mega32RtcStatus.error == error);
  const int previousReads = reads, previousWrites = writes;
  assert(rtc_getYear() == 2000 && rtc_getMonth() == 1 && rtc_getDay() == 1);
  rtc_setTime(0,0,12,22,10,2026);
  assert(reads == previousReads && writes == previousWrites);
}
}
void fakePowerClock() {}
void fakeEnableRtc() { fakeRcc.BDCR |= RCC_BDCR_RTCEN; }
bool fakeLseReady() { return (fakeRcc.BDCR & 2) != 0; }
bool fakeLsiReady() { return (fakeRcc.CSR & 2) != 0; }
uint32_t HAL_GetTick() { return ticks++; }
void HAL_PWR_EnableBkUpAccess() { if (accessWorks) { fakePwr.CR |= PWR_CR_DBP; } }
uint32_t getBackupRegister(uint32_t) { return marker; }
void setBackupRegister(uint32_t, uint32_t value) { assert(fakePwr.CR & PWR_CR_DBP); marker = value; }
HAL_StatusTypeDef HAL_RCC_OscConfig(RCC_OscInitTypeDef *c) {
  ++oscillatorCalls;
  assert(c->PLL.PLLState == RCC_PLL_NONE);
  if (c->OscillatorType == RCC_OSCILLATORTYPE_LSE) {
    if (c->LSEState == RCC_LSE_OFF) {
      if (!cleanupWorks) { return HAL_TIMEOUT; }
      fakeRcc.BDCR &= ~(RCC_BDCR_LSEON | 2); return HAL_OK;
    }
    fakeRcc.BDCR |= RCC_BDCR_LSEON;
    if (lseWorks) { fakeRcc.BDCR |= 2; return HAL_OK; }
    ticks += 5000; return HAL_TIMEOUT;
  }
  if (lsiWorks) { fakeRcc.CSR |= 2; return HAL_OK; }
  return HAL_TIMEOUT;
}
HAL_StatusTypeDef HAL_RCCEx_PeriphCLKConfig(RCC_PeriphCLKInitTypeDef *c) {
  if (!selectionWorks) { return HAL_ERROR; }
  const uint32_t previous = fakeRcc.BDCR & RCC_BDCR_RTCSEL;
  if (previous && previous != c->RTCClockSelection) {
    ++switches; marker = 0; fakeRtc.ISR = 0;
    calendarTime = {}; calendarDate = {};
    // Catch the bug where switching clocks tries the failed crystal again.
    if ((fakeRcc.BDCR & RCC_BDCR_LSEON) && !lseWorks) { assert(false && "LSE left enabled during fallback"); }
  }
  fakeRcc.BDCR = (fakeRcc.BDCR & ~RCC_BDCR_RTCSEL) | c->RTCClockSelection;
  return HAL_OK;
}
HAL_StatusTypeDef HAL_RTCEx_DisableBypassShadow(RTC_HandleTypeDef *) { return bypassWorks ? HAL_OK : HAL_ERROR; }
HAL_StatusTypeDef HAL_RTC_Init(RTC_HandleTypeDef *h) {
  ++initCalls;
  asyncDivider = h->Init.AsynchPrediv; syncDivider = h->Init.SynchPrediv;
  return initWorks ? HAL_OK : HAL_TIMEOUT;
}
HAL_StatusTypeDef HAL_RTC_GetTime(RTC_HandleTypeDef *, RTC_TimeTypeDef *t, uint32_t) {
  ++reads; assert(!timeLocked); timeLocked = true; *t = calendarTime;
  return readsWork ? HAL_OK : HAL_ERROR;
}
HAL_StatusTypeDef HAL_RTC_GetDate(RTC_HandleTypeDef *, RTC_DateTypeDef *d, uint32_t) {
  assert(timeLocked); timeLocked = false; *d = calendarDate;
  return readsWork ? HAL_OK : HAL_ERROR;
}
HAL_StatusTypeDef HAL_RTC_SetTime(RTC_HandleTypeDef *, RTC_TimeTypeDef *t, uint32_t) {
  ++writes; if (!timeWriteWorks) { return HAL_TIMEOUT; }
  calendarTime = *t; return HAL_OK;
}
HAL_StatusTypeDef HAL_RTC_SetDate(RTC_HandleTypeDef *, RTC_DateTypeDef *d, uint32_t) {
  ++writes; if (!dateWriteWorks) { return HAL_TIMEOUT; }
  calendarDate = *d; fakeRtc.ISR |= RTC_ISR_INITS; return HAL_OK;
}

int main() {
  // Fresh startup: use the crystal, then accept valid dates and reject bad ones.
  reset(); assert(mega32RtcBegin());
  assert(g_mega32RtcStatus.source == Mega32RtcSource::Lse && !g_mega32RtcStatus.timeValid);
  assert((asyncDivider + 1) * (syncDivider + 1) == LSE_VALUE);
  assert(rtc_getYear() == 2000 && marker == 0);
  rtc_setTime(56,34,12,22,10,2026);
  assert(g_mega32RtcStatus.timeValid && marker == RTC_BKP_VALUE);
  assert(rtc_getYear() == 2026 && rtc_getDOW() == 4 && rtc_getSecond() == 56);
  const int initialCalls = oscillatorCalls;
  rtc_setTime(1,2,3,29,2,2024); // Valid leap day.
  assert(g_mega32RtcStatus.timeValid && rtc_getDOW() == 4);
  rtc_setTime(1,2,3,29,2,2025); // Reject without clearing valid time.
  assert(g_mega32RtcStatus.error == Mega32RtcError::InvalidDateTime && marker == RTC_BKP_VALUE);
  assert(rtc_getYear() == 2024 && oscillatorCalls == initialCalls && initCalls == 1);
  assert(mega32RtcBegin() && initCalls == 1);

  // A working crystal and saved time must survive a restart.
  reset(); retained(RCC_RTCCLKSOURCE_LSE); assert(mega32RtcBegin());
  assert(switches == 0 && writes == 0 && g_mega32RtcStatus.timeValid);
  assert(rtc_getYear() == 2026 && rtc_getSecond() == 56);

  // Without a working crystal, the internal clock must take over.
  reset(); lseWorks = false; assert(mega32RtcBegin());
  assert(g_mega32RtcStatus.source == Mega32RtcSource::Lsi && g_mega32RtcStatus.lseFailed);
  assert(!g_mega32RtcStatus.timeValid && !(fakeRcc.BDCR & RCC_BDCR_LSEON));
  assert((asyncDivider + 1) * (syncDivider + 1) == LSI_VALUE);
  rtc_setTime(0,0,12,22,10,2026); assert(g_mega32RtcStatus.timeValid);

  // If the previously used crystal is now broken, switch clocks and mark
  // the saved time invalid. Do not try to start the broken crystal twice.
  reset(); retained(RCC_RTCCLKSOURCE_LSE); lseWorks = false; fakeRcc.BDCR &= ~2U;
  assert(mega32RtcBegin() && switches == 1 && !g_mega32RtcStatus.timeValid && marker == 0);
  assert(oscillatorCalls == 3 && g_mega32RtcStatus.source == Mega32RtcSource::Lsi);

  reset(); retained(RCC_RTCCLKSOURCE_LSE); fakeRtc.ISR |= RTC_ISR_INIT;
  assert(mega32RtcBegin() && !g_mega32RtcStatus.timeValid && marker == 0);
  reset(); retained(RCC_RTCCLKSOURCE_LSI); lseWorks = false;
  assert(mega32RtcBegin() && switches == 0 && g_mega32RtcStatus.timeValid);
  reset(); retained(RCC_RTCCLKSOURCE_LSI);
  assert(mega32RtcBegin() && switches == 1 && !g_mega32RtcStatus.timeValid);
  reset(); retained(RCC_RTCCLKSOURCE_LSE); fakeRcc.BDCR &= ~2U;
  assert(mega32RtcBegin() && !g_mega32RtcStatus.timeValid); // Stopped clock isn't trusted.

  // Make each startup step fail in turn. The RTC should report failure
  // without stopping the firmware or continuing to read/write its calendar.
  reset(); accessWorks = false; expectUnavailable(Mega32RtcError::BackupAccess);
  reset(); lseWorks = cleanupWorks = false; expectUnavailable(Mega32RtcError::LseCleanup);
  reset(); lseWorks = lsiWorks = false; expectUnavailable(Mega32RtcError::LsiStartup);
  reset(); selectionWorks = false; expectUnavailable(Mega32RtcError::ClockSelection);
  reset(); retained(RCC_RTCCLKSOURCE_LSE); initWorks = false;
  expectUnavailable(Mega32RtcError::Initialization); assert(marker == 0);
  reset(); bypassWorks = false; expectUnavailable(Mega32RtcError::Initialization);
  reset(); readsWork = false; expectUnavailable(Mega32RtcError::Read);
  reset(); dateWriteWorks = false; expectUnavailable(Mega32RtcError::Initialization);

  // A later read/write error must clear the saved "time is valid" marker.
  reset(); retained(RCC_RTCCLKSOURCE_LSE); assert(mega32RtcBegin()); readsWork = false;
  assert(rtc_getYear() == 2000 && marker == 0 && !g_mega32RtcStatus.timeValid);
  assert(g_mega32RtcStatus.error == Mega32RtcError::Read);

  reset(); assert(mega32RtcBegin()); dateWriteWorks = false;
  rtc_setTime(1,2,3,22,10,2026);
  assert(g_mega32RtcStatus.state == Mega32RtcState::Unavailable && marker == 0);
  assert(g_mega32RtcStatus.error == Mega32RtcError::Write && !g_mega32RtcStatus.timeValid);
  puts("MEGA32 RTC startup/fallback fault tests passed");
}
