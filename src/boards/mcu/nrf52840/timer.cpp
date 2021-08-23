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

using namespace std::chrono_literals;
using namespace std::chrono;

static mbed::Ticker timerTickers[10];	 // calls a callback repeatedly with a timeout
static mbed::Timeout timeoutTickers[10]; // calls a callback once when a timeout expires
uint32_t timerTimes[10];
bool timerInUse[10] = {false, false, false, false, false, false, false, false, false, false};

// External functions

void TimerConfig(void)
{
	/// \todo Nothing to do here for nRF52
}

void TimerInit(TimerEvent_t *obj, void (*callback)(void))
{
	// Look for an available Ticker
	for (int idx = 0; idx < 10; idx++)
	{
		if (timerInUse[idx] == false)
		{
			timerInUse[idx] = true;
			obj->timerNum = idx;
			obj->Callback = callback;
			return;
		}
	}
	/// \todo We run out of tickers, what do we do now???
}

void timerCallback(TimerEvent_t *obj)
{
	// Nothing to do here for the nRF52
}

void TimerStart(TimerEvent_t *obj)
{

	int idx = obj->timerNum;
	if (obj->oneShot)
	{
		timeoutTickers[idx].attach((mbed::callback(obj->Callback)), std::chrono::milliseconds(timerTimes[idx]));
	}
	else
	{
		timerTickers[idx].attach((mbed::callback(obj->Callback)), std::chrono::milliseconds(timerTimes[idx]));
	}
}

void TimerStop(TimerEvent_t *obj)
{
	int idx = obj->timerNum;
	timerTickers[idx].detach();
	timeoutTickers[idx].detach();
}

void TimerReset(TimerEvent_t *obj)
{
	int idx = obj->timerNum;
	timerTickers[idx].detach();
	timeoutTickers[idx].detach();
	if (obj->oneShot)
	{
		//timeoutTickers[idx].attach((mbed::callback(obj->Callback)), timerTimes[idx] * 1ms);
		timeoutTickers[idx].attach((mbed::callback(obj->Callback)), std::chrono::milliseconds(timerTimes[idx]));
	}
	else
	{
		//timerTickers[idx].attach((mbed::callback(obj->Callback)), timerTimes[idx] * 1ms);
		timerTickers[idx].attach((mbed::callback(obj->Callback)), std::chrono::milliseconds(timerTimes[idx]));
	}
}

void TimerSetValue(TimerEvent_t *obj, uint32_t value)
{
	int idx = obj->timerNum;
	timerTimes[idx] = value;
}

TimerTime_t TimerGetCurrentTime(void)
{
	return millis();
}

TimerTime_t TimerGetElapsedTime(TimerTime_t past)
{
	uint32_t nowInTicks = millis();
	uint32_t pastInTicks = past;
	TimerTime_t diff = nowInTicks - pastInTicks;

	return diff;
}

#endif