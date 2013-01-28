#ifdef STM32F4XX
	#include "stm32f4xx_conf.h"
	#include <stm32f4xx.h>
	#include <stm32f4xx_rcc.h>
	#include <stm32f4xx_gpio.h>
	#include <stm32f4xx_tim.h>
	#include <stm32f4xx_dma.h>
#else
	#error "Unknown MCU"
#endif

#define LED_NUMBER			1
#define LEDS_BUFFER_SIZE	(LED_NUMBER * 24 + 40)

#define BIT_ZERO		20
#define BIT_ONE			49
#define MAX_COLOR		80

u16 PWM_Buffer[LEDS_BUFFER_SIZE];
uint8_t R = 0,
		G = 0,
		B = 0;


const TIM_TimeBaseInitTypeDef TIM3_BaseInit = {
/*
	0,					//	.TIM_Prescaler		= 0,
	TIM_CounterMode_Up,	//	.TIM_CounterMode	= TIM_CounterMode_Up,
	104,				//	.TIM_Period			= 104,
	TIM_CKD_DIV1,		//	.TIM_ClockDivision	= TIM_CKD_DIV1,
	0					//	.TIM_RepetitionCounter	= 0
*/
	.TIM_Prescaler		= 0,
	.TIM_CounterMode	= TIM_CounterMode_Up,
	.TIM_Period			= 104,
	.TIM_ClockDivision	= TIM_CKD_DIV1,
	.TIM_RepetitionCounter	= 0
};

const TIM_OCInitTypeDef TIM3_OC_Init = {
/*
	TIM_OCMode_PWM1,			//	.TIM_OCMode			= TIM_OCMode_PWM1,
	TIM_OutputState_Enable,		//	.TIM_OutputState	= TIM_OutputState_Enable,
	TIM_OutputNState_Disable,	//	.TIM_OutputNState	= TIM_OutputNState_Disable,
	0,							//	.TIM_Pulse			= 0,
	TIM_OCPolarity_High,		//	.TIM_OCPolarity		= TIM_OCPolarity_High,
	TIM_OCNPolarity_High,		//	.TIM_OCNPolarity	= TIM_OCNPolarity_High,
	TIM_OCIdleState_Reset,		//	.TIM_OCIdleState	= TIM_OCIdleState_Reset,
	TIM_OCNIdleState_Set		//	.TIM_OCNIdleState	= TIM_OCNIdleState_Set
*/
	.TIM_OCMode			= TIM_OCMode_PWM1,
	.TIM_OutputState	= TIM_OutputState_Enable,
	.TIM_OutputNState	= TIM_OutputNState_Disable,
	.TIM_Pulse			= 0,
	.TIM_OCPolarity		= TIM_OCPolarity_High,
	.TIM_OCNPolarity	= TIM_OCNPolarity_High,
	.TIM_OCIdleState	= TIM_OCIdleState_Reset,
	.TIM_OCNIdleState	= TIM_OCNIdleState_Set
};

const GPIO_InitTypeDef GPIO_RGB_DI = {
/*
	GPIO_Pin_9,			//	.GPIO_Pin	= GPIO_Pin_9,
	GPIO_Mode_AF,		//	.GPIO_Mode	= GPIO_Mode_AF,
	GPIO_Speed_100MHz,	//	.GPIO_Speed	= GPIO_Speed_100MHz,
	GPIO_OType_PP,		//	.GPIO_OType	= GPIO_OType_PP,
	GPIO_PuPd_NOPULL	//	.GPIO_PuPd	= GPIO_PuPd_NOPULL
*/
	.GPIO_Pin	= GPIO_Pin_9,
	.GPIO_Mode	= GPIO_Mode_AF,
	.GPIO_Speed	= GPIO_Speed_100MHz,
	.GPIO_OType	= GPIO_OType_PP,
	.GPIO_PuPd	= GPIO_PuPd_NOPULL
};

const DMA_InitTypeDef DMA1_Init = {

#ifdef STM32F4XX
	.DMA_BufferSize 		= LEDS_BUFFER_SIZE,
	.DMA_Channel 			= DMA_Channel_5,
	.DMA_DIR 				= DMA_DIR_MemoryToPeripheral,
	.DMA_FIFOMode 			= DMA_FIFOMode_Disable,
	.DMA_FIFOThreshold 		= DMA_FIFOThreshold_HalfFull,
	.DMA_Memory0BaseAddr 	= (uint32_t) PWM_Buffer,
	.DMA_MemoryBurst 		= DMA_MemoryBurst_Single,
	.DMA_MemoryDataSize 	= DMA_MemoryDataSize_HalfWord,
	.DMA_MemoryInc 			= DMA_MemoryInc_Enable,
	.DMA_Mode 				= DMA_Mode_Circular,
	.DMA_PeripheralBaseAddr = (uint32_t) &TIM3->CCR4,
	.DMA_PeripheralBurst 	= DMA_PeripheralBurst_Single,
	.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
	.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable,
	.DMA_Priority 			= DMA_Priority_Medium
#else
	#error "Unknown MCU"
#endif
};

void Delay_500K(void)
{
	__IO uint32_t count = 500000;
	while(count-- != 0)
		__NOP();
}

uint16_t * FillColorByte(uint8_t color, uint16_t * bits)
{
	uint8_t mask = 0x80;
	while(mask != 0)
	{
		if (color & mask)
			*bits++ = BIT_ONE;
		else
			*bits++ = BIT_ZERO;
		mask >>= 1;
	}
	return bits;
}

void FillColor(void)
{
	uint16_t * bits = &PWM_Buffer[0];

	Delay_500K();

	bits = FillColorByte(G, bits);
	bits = FillColorByte(R, bits);
	bits = FillColorByte(B, bits);
}

int main(void)
{         	
	FillColor();

	// Initialize PC9 and TIM3

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	GPIO_Init(GPIOC, (GPIO_InitTypeDef *)&GPIO_RGB_DI);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_TIM3);

	TIM_TimeBaseInit(TIM3, (TIM_TimeBaseInitTypeDef *)&TIM3_BaseInit);
	TIM_OC4Init(TIM3, (TIM_OCInitTypeDef *)&TIM3_OC_Init);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);

	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM3, ENABLE);

	TIM_CCxCmd(TIM3, TIM_Channel_4, TIM_CCx_Enable);
	TIM_Cmd(TIM3, ENABLE);

	// Initialize DMA1 Stream2
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	TIM_DMACmd(TIM3, TIM_DMA_CC4, ENABLE);

	DMA_Init(DMA1_Stream2, (DMA_InitTypeDef *)&DMA1_Init);
	DMA_Cmd(DMA1_Stream2, ENABLE);
	TIM_DMACmd(TIM3, TIM_DMA_CC4, ENABLE);

	while (1)
	{
		while(++R != MAX_COLOR) { FillColor(); }
		while(--R != 0)			{ FillColor(); }
		while(++G != MAX_COLOR) { FillColor(); }
		while(--G != 0)			{ FillColor(); }
		while(++B != MAX_COLOR) { FillColor(); }
		while(--B != 0)			{ FillColor(); }

		while(++R != MAX_COLOR)	{ ++G; FillColor(); }
		while(--R != 0)			{ --G; FillColor(); }
		while(++R != MAX_COLOR) { ++B; FillColor(); }
		while(--R != 0)			{ --B; FillColor(); }
		while(++G != MAX_COLOR)	{ ++B; FillColor(); }
		while(--G != 0)			{ --B; FillColor(); }

		while(++R != MAX_COLOR)	{ ++G; ++B; FillColor(); }
		while(--R != 0)			{ --G; --B; FillColor(); }
	}
}
