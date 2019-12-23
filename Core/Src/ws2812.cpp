
/*
 * ws2812.c
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

/*******************************************************************************
*  Includes
*******************************************************************************/
#include "ws2812.h"

/*******************************************************************************
*  Defines
*******************************************************************************/

/*******************************************************************************
*  Constants and varibles
*******************************************************************************/
TIM_HandleTypeDef htim5;
RNG_HandleTypeDef hrng;

namespace ws2812 {

using namespace Mcucpp;
using namespace Mcucpp::Containers;

static FixedArray<LED_SUMMARY, tim_ccr_size, atomic> __attribute__((section("COLOUR_ARRAY"))) colours;
static FixedArray<LED_NUMBER, base_type, atomic> __attribute__((section("LED_ARRAY"))) leds;

static Delegate1<base_type, RNG_HandleTypeDef*> rng(HAL_RNG_GetRandomNumber);
static Delegate4<HAL_StatusTypeDef, TIM_HandleTypeDef*, base_type, base_type*, uint16_t> dma(HAL_TIM_PWM_Start_DMA);

static WS2812<LedManage> ws2812(colours, leds);

/*******************************************************************************
*  Typenames
*******************************************************************************/

/*******************************************************************************
*  Class methods
*******************************************************************************/

void LedManage::set_color (base_type iterator, base_type color, Bright_Params div){
	base_type tcolor = static_cast<base_type>(color / div);

	for (uint8_t i = 0; i != 8; ++i){

		colours[iterator + i] = (tcolor & 0x80 ?
				static_cast<uint32_t>(Pwm_Params<MCU_FREQ>::high) :
				static_cast<uint32_t>(Pwm_Params<MCU_FREQ>::low) );
		tcolor <<= 1;
	}
}

void LedManage::set_led (base_type iterator, base_type led, Bright_Params div){
	set_color(iterator, (led >> 8) & 0xFF, div);
	set_color(iterator + 8, (led >> 16) & 0xFF, div);
	set_color(iterator + 16, led & 0xFF, div);
}

void LedManage::set_line (Bright_Params div){

	for (uint32_t i = 0; i != LED_NUMBER; ++i){
		set_led(i * LED_BITS, leds[i], div);
	}
}


/*******************************************************************************
*  Local functions definitions
*******************************************************************************/

void Send_DMA (/*base_type* colour_array, base_type size*/){
	dma(&htim5, TIM_CHANNEL_1, colours.begin(),
			static_cast<uint16_t>( colours.size() ));
}

base_type Get_RNG24bit (void){
	return static_cast<base_type>( rng(&hrng) & 0x00FFFFFF );
}

/*******************************************************************************
*  Public functions definitions
*******************************************************************************/

WS2812<LedManage> &ws(){
	return ws2812;
}

} // End of ws2812 namespase

