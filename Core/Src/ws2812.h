/*
 * ws2812.h
 *
 *  Created on: 23 дек. 2019 г.
 *      Author: ALScode
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WS2812_H_
#define WS2812_H_


/*******************************************************************************
*  Includes
*******************************************************************************/
#pragma once

#include "mcu_header.h"
#include "array.h"
#include "delegate.h"


namespace ws2812 {
/*******************************************************************************
*  Defines
*******************************************************************************/

/*******************************************************************************
*  Constants and varibles
*******************************************************************************/
const uint32_t MCU_FREQ = 72000000; // Defined by user according system clock;

/*******************************************************************************
*  Enums
*******************************************************************************/
typedef enum{
	LED_NUMBER = 16, // Defined by user according number of leds in the ws2812 device(number + 1(!))
	LED_FREQ = 800000, // Hz; Defined by user according of the ws2812 clocking.
	LED_BITS = 24,  // bit per led; Defined by user according of the ws2812 color resolution.
	LED_HIGH = 900, // ns; Defined by user according of the ws2812 timings.
	LED_LOW = 350, // ns; Defined by user according of the ws2812 timings.
	LED_PERIOD = 1250, // ns; Defined by user according of the ws2812 timings.
	LED_SUMMARY = LED_NUMBER * LED_BITS + 1,//!< LED_SUMMARY
}Led_Params;

typedef enum {
	BRIGHT_8 = 1,
	BRIGHT_7 = 2,
	BRIGHT_6 = 4,
	BRIGHT_5 = 8,
	BRIGHT_4 = 16,
	BRIGHT_3 = 32,
	BRIGHT_2 = 64,
	BRIGHT_1 = 128,

}Bright_Params;

typedef enum {
	Black	= 0x00000000,	//(0,0,0)
	White	= 0x00FFFFFF,	//(255,255,255)
	Red		= 0x00FF0000,	//(255,0,0)
	Lime	= 0x0000FF00,	//(0,255,0)
	Blue	= 0x000000FF,	//(0,0,255)
	Yellow	= 0x00FFFF00,	//(255,255,0)
	Aqua	= 0x0000FFFF,	//(0,255,255)
	Magenta = 0x00FF00FF,	//(255,0,255)
	Silver	= 0x00C0C0C0,	//(192,192,192)
	Gray	= 0x00808080,	//(128,128,128)
	Maroon	= 0x00800000,	//(128,0,0)
	Olive	= 0x00808000,	//(128,128,0)
	Green	= 0x00008000,	//(0,128,0)
	Purple	= 0x00800080,	//(128,0,128)
	Teal	= 0x00008080,	//(0,128,128)
	Navy	= 0x00000080,	//(0,0,128)
	BlueViolet 	= 0x008A2BE2, //(138,43,226)
	Gold    	= 0x00FFD700, //(255,215,0)
	OrangeRed 	= 0x00FF4500, //(255,69,0)
	DarkGreen	= 0x00006400, //(0,100,0)
}RGB_24Bit;


/*******************************************************************************
*  Typenames
*******************************************************************************/

typedef uint32_t base_type; // Defined by user according MK architecture;
typedef uint32_t tim_ccr_size; // Defined by user according timer used;
typedef Mcucpp::Atomic atomic; // Defined by user according necessity of atomic access;
typedef
		Mcucpp::Containers::FixedArray<LED_SUMMARY, tim_ccr_size, atomic>& Colours;
typedef
		Mcucpp::Containers::FixedArray<LED_NUMBER, base_type, atomic>& Leds;

/*******************************************************************************
*  Local functions declarations
*******************************************************************************/

void Send_DMA (void);
base_type Get_RNG24bit(void);

/*******************************************************************************
*  Struct / Classes
*******************************************************************************/

typedef struct{
	volatile base_type mode;
	base_type delay;
	Bright_Params bright;
	int counter;
	bool flag;
}WS2812_State;

/*----------------------------------------------------------------------------*/

template <base_type MCU_FREQ>
class Pwm_Params{
public:
	enum { high = (MCU_FREQ / LED_FREQ)*LED_HIGH / LED_PERIOD};
	enum { low = (MCU_FREQ / LED_FREQ)*LED_LOW / LED_PERIOD};
};

/*----------------------------------------------------------------------------*/

class LedManage {
public:
	LedManage();
	LedManage(const LedManage&)=delete;
	LedManage &operator=(const LedManage&)=delete;

	static void set_line (Leds leds, Colours colours, Bright_Params div);

private:
	static void set_color (Colours colours, base_type iterator,
							base_type color, Bright_Params div);
	static void set_led (Colours colours, base_type iterator,
							base_type led, Bright_Params div);
};

/*----------------------------------------------------------------------------*/

template<class LedManage>
class WS2812{
public:
	WS2812(Colours colours_, Leds leds_) :
		colours(colours_),
		leds(leds_)
	{}
	WS2812 &operator=(const WS2812&)=delete;

	inline void next_mode(void){
		Mcucpp::Atomic::FetchAndAdd(&state.mode, 1);
	}

	inline void previous_mode(void){
		Mcucpp::Atomic::FetchAndSub(&state.mode, 1);
	}

	inline base_type get_mode(void){
		return Mcucpp::Atomic::Fetch(&state.mode);
	}

	inline base_type get_delay(void){
		return state.delay;
	}

	template <typename T>
	inline void set_delay (T delay){
			state.delay = static_cast<base_type>(delay);
		}

	inline void set_bright (Bright_Params bright_){
		state.bright = bright_;
	}

	void init (Bright_Params bright_){
		set_bright(bright_);
		leds.assign(LED_NUMBER, Black);
		colours.assign(LED_SUMMARY, Pwm_Params<MCU_FREQ>::low);
		colours[LED_SUMMARY - 1] = 0;

		state.mode = 0;
		state.delay = 250;
		state.bright = bright_;
		state.counter = 0;
		state.flag = true;
	}

	void random_colours(void){
		base_type led = 0, color = 0;

		for (uint8_t i = 0; i != LED_NUMBER; ++i ){

			color = Get_RNG24bit();

			if ((i % 2) || (i == 1)){
				led  |= ( ((color >> 16) & 0xFF)  << 16);
				led  |= ( 0 << 8);
			} else {
				led  |= ( 0  << 16);
				led  |= ( ((color >> 8) & 0xFF) << 8);
			}

			led  |= (color & 0xFF);

			leds[i] = led;

			led = 0;
		}

		LedManage::set_line(leds, colours, state.bright);
		Send_DMA();
	}

	template <typename T>
	void set_led (base_type begin, base_type end, T color){
		std::for_each(&leds[begin], &leds[end], [color](base_type& led)
				{ led = static_cast<base_type>(color); });

		LedManage::set_line(leds, colours, state.bright);
		Send_DMA();
	}

	void bright_float (void){
		static base_type div = 4;
		static bool direction = true;

		if (direction){
			if (div < 128){
				div *= 2;
			} else{
				direction = false;
			}
		} else {
			if (div > 8){
				div /= 2;
			} else{
				direction = true;
			}
		}

		LedManage::set_line(leds, colours, static_cast<Bright_Params>(div));
		Send_DMA();
	}

	void run_line_forward(void){
		uint32_t last = LED_NUMBER - 1;
		uint32_t temp = leds[last];

		for (uint8_t i = 0; i != last; ++i){
			leds[last - i] = leds[last - i - 1];
		}

		leds[0] = temp;

		LedManage::set_line(leds, colours, state.bright);
		Send_DMA();
	}
	void run_line_back(void){
		base_type last = LED_NUMBER - 1;
		uint32_t temp = leds[0];

		for (uint8_t i = 0; i != last; ++i){
			leds[i] = leds[i + 1];
		}

		leds[last] = temp;

		LedManage::set_line(leds, colours, state.bright);
		Send_DMA();
	}

	void jump (int num){
		int last = LED_NUMBER - 1;

		if (state.flag) {
			if (state.counter + num < LED_NUMBER){
				std::swap(leds[state.counter], leds[state.counter + num]);
				++state.counter;
			} else {
				state.flag = false;
				state.counter = 0;
			}
		} else {

			if (last - state.counter - num >= 0){
				std::swap(leds[last - state.counter], leds[last - state.counter - num]);
				++state.counter;
			} else {
				state.flag = true;
				state.counter = 0;
			}
		}
		LedManage::set_line(leds, colours, state.bright);
		Send_DMA();
	}
	void shift_color(void){

		std::for_each(leds.begin(), leds.end(), [](base_type& led)
						{
							base_type temp = led & 0xFF;
							led = (led >> 8) | (temp << 16);
						});

		LedManage::set_line(leds, colours, state.bright);
		Send_DMA();
	}

	void clear_state(void){
		Mcucpp::Atomic::FetchAndAnd(&state.mode, 0);
		state.counter = 0;
		state.flag = true;
	}

private:
	Colours colours;
	Leds leds;
	WS2812_State state;
};


/*******************************************************************************
*  Public functions declarations
*******************************************************************************/

WS2812<LedManage> &ws();


} // End of ws2812 namespase


#endif /* WS2812_H_ */
