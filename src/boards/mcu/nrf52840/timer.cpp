/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic lora driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis, Gregory Cristian and Wael Guibene
Modified for NRF52840 Andrés Sabas @ Electronic Cats
*/

/******************************************************************************
 * @file    timer.c
 * @author  Insight SiP
 * @version V1.0.0
 * @date    02-mars-2018
 * @brief   timer implementation functions for LORA.
 *
 * @attention
 *	THIS SOFTWARE IS PROVIDED BY INSIGHT SIP "AS IS" AND ANY EXPRESS
 *	OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *	OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *	DISCLAIMED. IN NO EVENT SHALL INSIGHT SIP OR CONTRIBUTORS BE
 *	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *	HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *	OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/
#if defined(ARDUINO_ARCH_MBED)
#include "boards/mcu/timer.h"
#include "boards/mcu/board.h"

#include <mbed.h>
#include <rtos.h>
#include <cmsis_os.h>

using namespace rtos;
using namespace mbed;

/** Timer structure */
struct s_timer
{
	int32_t signal = 0;
	bool in_use = false;
	bool active = false;
	time_t duration;
	time_t start_time;
	void (*callback)();
};

/** Array to hold the timers */
s_timer timer[10];

/**
 * @brief Configure timers
 * Starts the background thread to handle timer events
 * Starts the Hardware timer to wakeup the handler thread
 * 
 */
void TimerConfig(void)
{
	
}

/**
 * @brief Initialize a new timer
 * Checks for available timer slot (limited to 10)
 * 
 * @param obj structure with timer settings
 * @param callback callback that the timer should call
 */
void TimerInit(TimerEvent_t *obj, void (*callback)(void))
{
	// Look for an available Ticker
	for (int idx = 0; idx < 10; idx++)
	{
		if (timer[idx].in_use == false)
		{
			timer[idx].signal = 1 << idx; // + 1;
			timer[idx].in_use = true;
			timer[idx].active = false;
			timer[idx].callback = callback;
			timer[idx].duration = obj->ReloadValue * 1000;
			obj->timerNum = idx;
			obj->Callback = callback;
			LOG_LIB("TIM", "Timer %d assigned", idx);
			return;
		}
	}
	LOG_LIB("TIM", "No more timers available!");
}

/**
 * @brief Activate a timer
 * CAUTION requires the timer was initialized before
 * 
 * @param obj structure with timer settings 
 */
void TimerStart(TimerEvent_t *obj)
{
	int idx = obj->timerNum;

	timer[idx].start_time = micros();
	timer[idx].active = true;
	// LOG_LIB("TIM", "Timer %d started with %d ms", idx, timer[idx].duration);
}

/**
 * @brief Deactivate a timer
 * CAUTION requires the timer was initialized before
 * 
 * @param obj structure with timer settings 
 */
void TimerStop(TimerEvent_t *obj)
{
	int idx = obj->timerNum;
	timer[idx].active = false;
	// LOG_LIB("TIM", "Timer %d stopped", idx);
}

/**
 * @brief Restart a timer
 * CAUTION requires the timer was initialized before
 * 
 * @param obj structure with timer settings 
 */
void TimerReset(TimerEvent_t *obj)
{
	int idx = obj->timerNum;
	timer[idx].active = false;
	timer[idx].start_time = micros();
	timer[idx].active = true;
	// LOG_LIB("TIM", "Timer %d reset with %d ms", idx, timerTimes[idx]);
}

/**
 * @brief Set the duration time of a timer
 * CAUTION requires the timer was initialized before
 * 
 * @param obj structure with timer settings 
 * @param value duration time in milliseconds 
 */
void TimerSetValue(TimerEvent_t *obj, uint32_t value)
{
	int idx = obj->timerNum;
	timer[idx].duration = value * 1000;
	// LOG_LIB("TIM", "Timer %d setup to %d ms", idx, value);
}

/**
 * @brief Get current time in milliseconds
 * 
 * @return TimerTime_t time in milliseconds
 */
TimerTime_t TimerGetCurrentTime(void)
{
	return millis();
}

/**
 * @brief Get timers elapsed time
 * 
 * @param past last time the check was done
 * @return TimerTime_t difference between now and last check
 */
TimerTime_t TimerGetElapsedTime(TimerTime_t past)
{
	uint32_t nowInTicks = millis();
	uint32_t pastInTicks = past;
	TimerTime_t diff = nowInTicks - pastInTicks;

	return diff;
}
#endif // ARDUINO_ARCH_MBED