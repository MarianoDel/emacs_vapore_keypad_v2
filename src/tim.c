//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TIM.C #################################
//---------------------------------------------

// Includes --------------------------------------------------------------------
#include "tim.h"
#include "hard.h"
#include "rws317.h"


// Module Private Types Constants and Macros -----------------------------------
#define RCC_TIM1_CLK 		(RCC->APB2ENR & 0x00000800)
#define RCC_TIM1_CLK_ON 	(RCC->APB2ENR |= 0x00000800)
#define RCC_TIM1_CLK_OFF 	(RCC->APB2ENR &= ~0x00000800)

#define RCC_TIM3_CLK 		(RCC->APB1ENR & 0x00000002)
#define RCC_TIM3_CLK_ON 	(RCC->APB1ENR |= 0x00000002)
#define RCC_TIM3_CLK_OFF 	(RCC->APB1ENR &= ~0x00000002)

#define RCC_TIM6_CLK 		(RCC->APB1ENR & 0x00000010)
#define RCC_TIM6_CLK_ON 	(RCC->APB1ENR |= 0x00000010)
#define RCC_TIM6_CLK_OFF 	(RCC->APB1ENR &= ~0x00000010)

#define RCC_TIM14_CLK 		(RCC->APB1ENR & 0x00000100)
#define RCC_TIM14_CLK_ON 	(RCC->APB1ENR |= 0x00000100)
#define RCC_TIM14_CLK_OFF 	(RCC->APB1ENR &= ~0x00000100)

#define RCC_TIM15_CLK 		(RCC->APB2ENR & 0x00010000)
#define RCC_TIM15_CLK_ON 	(RCC->APB2ENR |= 0x00010000)
#define RCC_TIM15_CLK_OFF 	(RCC->APB2ENR &= ~0x00010000)

#define RCC_TIM16_CLK 		(RCC->APB2ENR & 0x00020000)
#define RCC_TIM16_CLK_ON 	(RCC->APB2ENR |= 0x00020000)
#define RCC_TIM16_CLK_OFF 	(RCC->APB2ENR &= ~0x00020000)

#define RCC_TIM17_CLK 		(RCC->APB2ENR & 0x00040000)
#define RCC_TIM17_CLK_ON 	(RCC->APB2ENR |= 0x00040000)
#define RCC_TIM17_CLK_OFF 	(RCC->APB2ENR &= ~0x00040000)


// #define TIM3_ARR    666
#define TIM1_ARR    666

#define DUTY_100_PERCENT    1000
// #define USE_FREQ_16KHZ

// #define FREQ_1000HZ    1000
// #define FREQ_800HZ    1250

// #define DUTY_50_FREQ_1000HZ    500
// #define DUTY_50_FREQ_800HZ    625


// Externals -------------------------------------------------------------------
extern volatile short v_samples1[];
extern volatile short v_samples2[];
extern volatile unsigned char update_samples;
extern volatile unsigned char buff_in_use;


// Globals ---------------------------------------------------------------------
volatile unsigned short wait_ms_var = 0;
volatile unsigned char new_sample = 0;


// Module Private Functions ----------------------------------------------------


// Module Functions ------------------------------------------------------------
void Power_Ampli_Ena (void)
{
	//hab general de OC
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void Ampli_to_Audio (void)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;

	TIM1->ARR = 511;
	TIM1->CNT = 0;
	TIM1->PSC = 5;	//prescaler divido 48MHz / (1 + 5)
	// Enable timer ver UDIS
	TIM1->DIER |= TIM_DIER_UIE;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void Ampli_to_Sirena (void)
{
	TIM1->CR1 &= ~TIM_CR1_CEN;

	TIM1->ARR = TIM1_ARR;
	TIM1->CNT = 0;
	TIM1->PSC = 47;	//prescaler divido 48MHz / (1 + 47)

	TIM1->DIER &= ~TIM_DIER_UIE;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void Power_Ampli_Disa (void)
{
	//hab general de OC
	TIM1->BDTR &= 0x7FFF;
	TIM1->CR1 &= ~TIM_CR1_CEN;
}

void ChangeAmpli(unsigned short freq, unsigned short pwm)
{
	TIM1->ARR = freq;
	TIM1->CCR1 = pwm;
}


void Update_TIM1_CH1 (unsigned short a)
{
    TIM1->CCR1 = a;
}

void Update_TIM1_CH2 (unsigned short a)
{
    TIM1->CCR2 = a;
}

void Update_TIM3_CH1 (unsigned short a)
{
    TIM3->CCR1 = a;
}

void Update_TIM3_CH2 (unsigned short a)
{
    TIM3->CCR2 = a;
}

void Update_TIM3_CH3 (unsigned short a)
{
    TIM3->CCR3 = a;
}

void Update_TIM3_CH4 (unsigned short a)
{
    TIM3->CCR4 = a;
}


void TIM_Timeouts (void)
{
    if (wait_ms_var)
        wait_ms_var--;
    
}


void Wait_ms (unsigned short wait)
{
    wait_ms_var = wait;

    while (wait_ms_var);
}


/////////////////////
// TIM1 Functions  //
/////////////////////
void TIM1_Init (void)
{
    if (!RCC_TIM1_CLK)
        RCC_TIM1_CLK_ON;

    //Configuracion del timer.
    // TIM1->CR1 = 0x00;		//clk int / 1; upcounting
    TIM1->CR1 = TIM_CR1_ARPE;    //clk int / 1; upcounting, preload
    TIM1->CR2 |= TIM_CR2_MMS_1;		//UEV -> TRG0

    TIM1->SMCR = 0x0000;
    TIM1->CCMR1 = 0x0060;    // CH1 output PWM mode 1
    TIM1->CCMR2 = 0x0000;
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;

    TIM1->CCER |= TIM_CCER_CC1E |  TIM_CCER_CC1NE;    // original board
    
    TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSI | (128 + 28);    //3.8us @ 48MHz
    
    // TIM1->ARR = FREQ_1000HZ;
    TIM1->ARR = TIM1_ARR;    
    TIM1->CNT = 0;
    TIM1->PSC = 47;    // 1MHz prescaller /(1 + 47)

    // Pins Config
    // Alternate Fuction
    unsigned int temp;
    temp = GPIOA->AFR[0];
    temp &= 0x0FFFFFFF;
    temp |= 0x20000000;    //PA7 -> AF2
    GPIOA->AFR[0] = temp;

    temp = GPIOA->AFR[1];
    temp &= 0xFFFFFFF0;
    temp |= 0x00000002;    //PA8 -> AF2
    GPIOA->AFR[1] = temp;

    // Enable timer interrupt ver UDIS
    // TIM1->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
    NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 2);
    
    TIM1->CR1 |= TIM_CR1_CEN;
}


void TIM1_BRK_UP_TRG_COM_IRQHandler (void)	//16KHz
{
	//cuando arranca siempre in_buff == 1;
	if (buff_in_use == 1)
	{
		if (new_sample < 16)
		{
			//v_pwm[new_sample] = (v_samples1[new_sample] >> 7) + 256;
			//TIM1->CCR1 = v_pwm[new_sample];
			TIM1->CCR1 = (v_samples1[new_sample] >> 7) + 256;
			new_sample++;
		}
		else
		{
			//tengo que cambiar de buffer
			buff_in_use = 2;
			update_samples++;
			TIM1->CCR1 = (v_samples2[0] >> 7) + 256;
			new_sample = 1;
		}
	}
	else if (buff_in_use == 2)
	{
		if (new_sample < 16)
		{
			//v_pwm[new_sample] = (v_samples2[new_sample] >> 7) + 256;
			//TIM1->CCR1 = v_pwm[new_sample];
			TIM1->CCR1 = (v_samples2[new_sample] >> 7) + 256;
			new_sample++;
		}
		else
		{
			//tengo que cambiar de buffer
			buff_in_use = 1;
			update_samples++;
			//v_pwm[0] = (v_samples1[0] >> 7) + 256;
			//TIM1->CCR1 = v_pwm[0];
			TIM1->CCR1 = (v_samples1[0] >> 7) + 256;
			new_sample = 1;
		}
	}


	/*
	if (new_sample < 16)
	{
		TIM1->CCR1 = sin16[new_sample];
		new_sample++;
	}
	else
	{
		TIM1->CCR1 = sin16[0];
		new_sample = 1;
	}
	*/
	//bajar flag
	if (TIM1->SR & 0x01)	//bajo el flag
		TIM1->SR = 0x00;

}

////////////////////
// TIM3 Functions //
////////////////////
void TIM3_Init (void)
{
    if (!RCC_TIM3_CLK)
        RCC_TIM3_CLK_ON;

    //Configuracion del timer.
    TIM3->CR1 = 0x00;    // clk int / 1; upcounting
    TIM3->CR2 = 0x00;    // equal reset

    TIM3->CCMR1 = 0x0000;
    TIM3->CCMR2 = 0x7100;    // CH4 IC filter N=8; no prescaler

    TIM3->ARR = 0xFFFF;    // continuos mode
    TIM3->CNT = 0;
    TIM3->PSC = 47;    // (47 + 1) -> /48 -> 1us

    TIM3->CCER |= TIM_CCER_CC4E | TIM_CCER_CC4P | TIM_CCER_CC4NP;    //input capture CH1 any edge    

    // Pins config    PC9 -> AF0
    // Alternate Fuction
    // unsigned long temp;
    
    // temp = GPIOA->AFR[0];
    // temp &= 0x00FFFFFF;
    // temp |= 0x11000000;			//PA7 -> AF1; PA6 -> AF1
    // GPIOA->AFR[0] = temp;

    // temp = GPIOB->AFR[0];
    // temp &= 0xFF00FF00;                 //PB5 -> AF1; PB4 -> AF1
    // temp |= 0x00110011;			//PB1 -> AF1; PB0 -> AF1
    // GPIOB->AFR[0] = temp;

    // Enable timer ints
    TIM3->DIER |= TIM_DIER_CC4IE;    // CH4 capture/compare int
    TIM3->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM3_IRQn);
    NVIC_SetPriority(TIM3_IRQn, 1);
}


void TIM3_IC_CH1_OFF (void)
{
    //desa int
    TIM3->DIER &= ~(TIM_DIER_CC4IE);
}

void TIM3_IC_CH1_ON (void)
{
    //ena int
    TIM3->DIER |= TIM_DIER_CC4IE;
}


void TIM3_IC_CNT (unsigned short new_counter)
{
    TIM3->CNT = new_counter;
}


void TIM3_IRQHandler (void)	
{
    // reload or overcaptured
    if ((TIM3->SR & TIM_SR_UIF)	||
        (TIM3->SR & TIM_SR_CC4OF))
        TIM3->SR &= ~(TIM_SR_UIF | TIM_SR_CC4OF);

    if (TIM3->SR & TIM_SR_CC4IF)
    {
        Timer_Interrupt_Handler (TIM3->CCR4);
        TIM3->SR &= ~(TIM_SR_CC4IF);
    }
}


/////////////////////
// TIM14 Functions //
/////////////////////
void TIM_14_Init (void)
{
    if (!RCC_TIM14_CLK)
        RCC_TIM14_CLK_ON;

    //Configuracion del timer.
    TIM14->CR1 = 0x00;		//clk int / 1; upcounting; uev
    TIM14->PSC = 47;			//tick cada 1us
    TIM14->ARR = 0xFFFF;			//para que arranque
    TIM14->EGR |= 0x0001;
}


void TIM_16_Init (void)
{
    if (!RCC_TIM16_CLK)
        RCC_TIM16_CLK_ON;

    //Configuracion del timer.
    TIM16->ARR = 0;
    TIM16->CNT = 0;
    TIM16->PSC = 47;

    // Enable timer interrupt ver UDIS
    TIM16->DIER |= TIM_DIER_UIE;
    TIM16->CR1 |= TIM_CR1_URS | TIM_CR1_OPM;	//solo int cuando hay overflow y one shot

    NVIC_EnableIRQ(TIM16_IRQn);
    NVIC_SetPriority(TIM16_IRQn, 7);
}


void TIM16_IRQHandler (void)	//es one shoot
{
    // SendDMXPacket(PCKT_UPDATE);

    if (TIM16->SR & 0x01)
        TIM16->SR = 0x00;    //bajar flag
}


void OneShootTIM16 (unsigned short a)
{
    // TIM16->CNT = 0;
    TIM16->ARR = a;
    TIM16->CR1 |= TIM_CR1_CEN;
}




//100us tick
void TIM_17_Init (void)    //en centanas de microsegundos
{
    if (!RCC_TIM17_CLK)
        RCC_TIM17_CLK_ON;

    //Configuracion del timer.
    TIM17->ARR = 200;
    TIM17->CNT = 0;
    TIM17->PSC = 47;

    //Configuracion canal PWM
    // TIM17->CCMR1 = 0x0060;      //CH1 output PWM mode 1 (channel active TIM->CNT < TIM->CCR1)

    // Enable timer ver UDIS
    // TIM17->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;
    TIM17->DIER |= TIM_DIER_UIE;    
    TIM17->CR1 |= TIM_CR1_CEN;

    NVIC_EnableIRQ(TIM17_IRQn);
    NVIC_SetPriority(TIM17_IRQn, 4);
}



//--- end of file ---//
