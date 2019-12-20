
#include "ws2812.h"

using namespace Mcucpp;
using namespace Mcucpp::Containers;

/****************** User Defines ******************************************************************/

/****************** Program Defines ***************************************************************/

/****************** Variables *********************************************************************/
TIM_HandleTypeDef htim5;
RNG_HandleTypeDef hrng;

namespace ws2812 {

static FixedArray<LED_SUMMARY, tim_ccr_size, atomic> __attribute__((section("COLOUR_ARRAY"))) colours;
static FixedArray<LED_NUMBER, base_type, atomic> __attribute__((section("LED_ARRAY"))) leds;

static Delegate1<base_type, RNG_HandleTypeDef*> rng(HAL_RNG_GetRandomNumber);
static Delegate4<HAL_StatusTypeDef, TIM_HandleTypeDef*, base_type, base_type*, uint16_t> dma(HAL_TIM_PWM_Start_DMA);

static WS2812<LedManage> ws2812(colours, leds);

/****************** Private Class Methods *********************************************************/

/****************** Public Class Methods **********************************************************/

/*----------------- LedManage --------------------------------------------------------------------*/

void LedManage::set_color (Colours colours, base_type iterator, base_type color, Bright_Params div){
	base_type tcolor = static_cast<base_type>(color / div);

	for (uint8_t i = 0; i != 8; ++i){

		colours[iterator + i] = (tcolor & 0x80 ?
				static_cast<uint32_t>(Pwm_Params<MCU_FREQ>::high) :
				static_cast<uint32_t>(Pwm_Params<MCU_FREQ>::low) );
		tcolor <<= 1;
	}
}

void LedManage::set_led (Colours colours, base_type iterator, base_type led, Bright_Params div){
	set_color(colours, iterator, (led >> 8) & 0xFF, div);
	set_color(colours, iterator + 8, (led >> 16) & 0xFF, div);
	set_color(colours, iterator + 16, led & 0xFF, div);
}

void LedManage::set_line (Leds leds, Colours colours, Bright_Params div){

	for (uint32_t i = 0; i != LED_NUMBER; ++i){
		set_led(colours, i * LED_BITS, leds[i], div);
	}
}


/****************** Local function definition******************************************************/

void Send_DMA (/*base_type* colour_array, base_type size*/){
	dma(&htim5, TIM_CHANNEL_1, colours.begin(),
			static_cast<uint16_t>( colours.size() ));
}

base_type Get_RNG24bit (void){
	return static_cast<base_type>( rng(&hrng) & 0x00FFFFFF );
}

/****************** Public function definition*****************************************************/

WS2812<LedManage> &ws(){
	return ws2812;
}

} // End of ws2812 namespase

