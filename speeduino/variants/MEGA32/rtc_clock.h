// MEGA32 RTC source selection from variants/RTCFix.txt.
// Kept local so shared Speeduino board and RTC sources remain unchanged.
#pragma once

#ifdef RTC_ENABLED
void stm32SetRtcClockSource();
#endif
