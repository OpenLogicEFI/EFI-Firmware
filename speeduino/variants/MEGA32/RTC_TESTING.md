# MEGA32 date/time clock: changes and testing

All changes and tests are in this firmware variant folder. This work does not
change shared Speeduino files, STM32 core variant files, build settings, or
installed libraries.

## Terms used in these notes

- **RTC:** the clock that keeps the date and time, including SD log timestamps.
- **LSE:** the external crystal used by the RTC. It can keep time on backup power.
- **LSI:** the internal clock used when the external crystal cannot start. It is
  less accurate and does not keep running on the RTC battery alone.
- **HAL:** ST's functions for controlling the chip's hardware.
- **Backup domain:** the RTC settings and saved data that can survive on battery
  power. Changing the RTC's clock source can reset this area and erase its data.
- **Valid time:** a saved or newly supplied date/time the firmware can trust.
  A running RTC can still have the wrong or unset date/time.

Neither LSE nor LSI controls CPU speed in this firmware.

## What this first batch does

TODO steps 1 and 2 are written and have passed computer tests. They still need
testing on the boards.

The code tries the external crystal first. If it fails, the code turns it off,
checks that it stopped, and then starts and checks the internal clock. If the
RTC still cannot start, it reports failure and lets the rest of the board start.
It does not call `Error_Handler()`, which could stop firmware startup.

We now check ST's hardware function results ourselves. We keep Speeduino's usual
`rtc_get...` and `rtc_setTime` functions, so the shared firmware does not need
changes. Do not add direct calls to the STM32RTC library in this variant: that
library keeps separate RTC settings and has startup paths that can stop the
firmware when something fails.

A restart using the same healthy clock keeps saved time. Changing clocks can
still erase the saved time and other backup-domain data. For now, the firmware
marks that time invalid instead of trying to restore it. TODO step 3 covers
improving this. A clock found stopped at startup, or a failed RTC setup, read,
or write, also makes the time invalid.

Setting the time now writes the date/time directly and checks that both writes
worked. It does not restart the RTC or retry the crystal. This also covers part
of TODO step 4. Impossible dates are rejected without changing the existing time.
If the RTC has been marked unavailable, further reads/writes are skipped until
reboot; setting time in TunerStudio does not retry RTC startup.

The existing timeout limits remain. A failed crystal startup can take about five
seconds. These waits depend on the chip's normal millisecond timer (SysTick) and
interrupts running during startup. This startup code must not be reused unchanged
for recovery while the engine runs.

**There is no monitoring for a crystal that fails after startup yet.** Starting
successfully does not prove the clock will stay healthy or keep accurate time.
That monitoring and recovery remain TODO step 6.

Without trusted saved time, the RTC starts at 2000-01-01 00:00:00 and can then
advance. If the RTC is unavailable, date/time reads return that fixed placeholder.
Neither means the actual date/time is known. There is no new TunerStudio field
showing validity yet, and SD handling of invalid time still needs work. Use the
status below to distinguish a real time from a placeholder.

## How to check the result

With an ST-Link/debugger, view `g_mega32RtcStatus` after the board starts:

| Field | Meaning |
| --- | --- |
| `state` | Uninitialized=0: not started; Ready=1: setup worked; Unavailable=2: failed. |
| `source` | None=0: no usable RTC; Lse=1: external crystal; Lsi=2: internal clock. |
| `error` | The failure reason, explained below. |
| `timeValid` | True means saved or newly supplied time is considered valid. |
| `lseFailed` | True means the external crystal failed its startup check, even if LSI took over successfully. |
| `startupMs` | Milliseconds spent starting just the RTC, excluding other board startup work. |

Error values:

| Value | Name | Meaning |
| --- | --- | --- |
| 0 | None | No error reported. |
| 1 | BackupAccess | Could not get permission to change battery-backed settings. |
| 2 | LseCleanup | Could not fully turn off the failed external crystal. |
| 3 | LsiStartup | The internal clock did not start successfully. |
| 4 | ClockSelection | The RTC did not accept the selected clock. |
| 5 | Initialization | RTC setup failed. |
| 6 | Read | Reading the date/time failed or returned an impossible value. |
| 7 | Write | Setting the date/time failed. |
| 8 | InvalidDateTime | The supplied date/time was impossible and was rejected. |

`source=None` means the firmware has no usable RTC. After a setup failure, the
chip may still have a clock selected even though the firmware cannot use it.
`Ready` only describes successful setup; it does not watch for later failure.

For a direct hardware check, read these registers without changing them:

- RCC_BDCR bits 9:8 (RTCSEL): 01 means LSE; 10 means LSI.
- RCC_BDCR bit 15 (RTCEN): RTC enabled.
- RCC_BDCR bits 0/1 (LSEON/LSERDY): external crystal switched on/ready.
- RCC_CSR bit 1 (LSIRDY): internal clock ready.

Watch seconds advance as well as checking these flags. Removing crystal load
capacitors may make the crystal unreliable rather than stop it completely, so
verify which clock the modified board actually uses. Do not pause the CPU with
the debugger while measuring startup or engine timing.

## Board tests before proceeding

Use a bench setup with injector/coil outputs disconnected. Record results and
RTC startup time. Repeat power cycles to check for intermittent crystal startup.

1. **Healthy board, fresh start:** remove main and backup power, then power up.
   Expect Ready, LSE, `timeValid=false`, and a placeholder date that advances.
2. **Set time:** use TunerStudio. Check date, weekday and advancing seconds.
   Expect `timeValid=true`. Repeat updates, including midnight and month changes.
3. **Battery-backed restart:** remove main power but keep the RTC battery connected.
   On restart, expect LSE, the saved time plus elapsed time, and `timeValid=true`.
4. **Broken-crystal board, fresh start:** expect Ready, LSI, `lseFailed=true`,
   LSEON/LSERDY cleared, and LSIRDY set. The board must finish starting. The first
   crystal attempt may still add about five seconds; record the delay.
5. **Set time on LSI:** check the supplied date/time and advancing seconds.
   Expect `timeValid=true`; the source stays LSI. Repeated updates should not
   retry the external crystal or repeat its startup delay.
6. **Saved LSE changes to LSI:** first establish valid time with working LSE.
   Remove main power before altering the oscillator. Keep backup power and the
   saved clock selection where practical, then restart with LSE unavailable.
   Expect Ready/LSI and `timeValid=false`, with a placeholder date and no hang.
   If the board work also removed backup power, this instead tests fresh startup;
   record that limitation.
7. **Saved LSI changes to LSE, if practical:** restore the external crystal and
   restart from a saved LSI selection. Expect LSE and `timeValid=false` because
   changing sources reset the saved time. Improving this remains TODO step 3.
8. **Simulated engine signals:** check startup, RPM and communications on both
   boards. Recovery from a crystal failing while running is not included yet.

Investigate before proceeding if startup hangs, the selected clock is unexpected,
seconds do not advance, a healthy LSE restart loses saved time, or a placeholder
is marked valid. A computer test or successful build cannot replace board tests.

## Running the computer tests

Run from the repository's main folder:

```sh
g++ -std=c++14 -Wall -Wextra -Werror \
  -Ispeeduino/variants/MEGA32/tests/stubs \
  -Ispeeduino/variants/MEGA32 -Ispeeduino \
  speeduino/variants/MEGA32/rtc_common.cpp \
  speeduino/variants/MEGA32/tests/rtc_startup_test.cpp \
  -o /tmp/mega32-rtc-test
/tmp/mega32-rtc-test
```

These tests run the real variant code with fake hardware functions that can be
made to fail. They cover startup, saved time, clock changes, and failures while
starting, reading or setting the RTC. They also check invalid dates, reading time
and date together, and setting time without restarting the RTC.

They cannot reproduce a real crystal's electrical behavior or measure actual
board timing. These test files are excluded from the board firmware build.
