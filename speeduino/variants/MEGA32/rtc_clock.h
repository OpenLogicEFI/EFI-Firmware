// MEGA32 RTC status and startup function.
// RTC means the date/time clock. LSE is its external crystal; LSI is the
// internal backup clock. Neither is the CPU clock.
// Use this variant's functions instead of calling STM32RTC directly. That
// library can stop startup on an error and does not use our RTC settings.
#pragma once
#include <stdint.h>

enum class Mega32RtcState : uint8_t { Uninitialized, Ready, Unavailable };
enum class Mega32RtcSource : uint8_t { None, Lse, Lsi };
enum class Mega32RtcError : uint8_t {
  None, BackupAccess, LseCleanup, LsiStartup, ClockSelection,
  Initialization, Read, Write, InvalidDateTime
};

struct Mega32RtcStatus {
  Mega32RtcState state; // Not started yet, started successfully, or failed.
  Mega32RtcSource source; // Clock we can use; None if RTC startup failed.
  Mega32RtcError error; // What went wrong, if anything.
  bool timeValid; // A saved or newly supplied time we can trust.
  bool lseFailed; // The external crystal failed its startup check.
  uint32_t startupMs; // Time spent starting the RTC, in milliseconds.
};

// These details can be viewed with a debugger, such as an ST-Link.
// Ready means startup worked. We do not yet watch for later crystal failure.
// These status details are not added to TunerStudio.
extern volatile Mega32RtcStatus g_mega32RtcStatus;
bool mega32RtcBegin(); // Call at startup only. Clock checks can take seconds.
