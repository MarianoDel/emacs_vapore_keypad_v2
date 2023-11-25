//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### TIM.H #################################
//---------------------------------------------
#ifndef _TIM_H_
#define _TIM_H_


// Module Exported Types Constants and Macros ----------------------------------
#define FREQ_1000HZ    1000
#define FREQ_800HZ    1250

#define DUTY_50_1000    500
#define DUTY_50_800    625


#define TIM1DisableInterrupt     (TIM1->DIER &= ~TIM_DIER_UIE)
#define TIM1EnableInterrupt      (TIM1->DIER |= TIM_DIER_UIE)

#define TIM17DisableInterrupt    (TIM17->DIER &= ~TIM_DIER_UIE)
#define TIM17EnableInterrupt     (TIM17->DIER |= TIM_DIER_UIE)


#define EnablePreload_TIM1_CH1    (TIM1->CCMR1 |= TIM_CCMR1_OC1PE)
#define EnablePreload_TIM1_CH2    (TIM1->CCMR1 |= TIM_CCMR1_OC2PE)
#define EnablePreload_TIM3_CH1    (TIM3->CCMR1 |= TIM_CCMR1_OC1PE)
#define EnablePreload_TIM3_CH2    (TIM3->CCMR1 |= TIM_CCMR1_OC2PE)
#define EnablePreload_TIM3_CH3    (TIM3->CCMR2 |= TIM_CCMR2_OC3PE)
#define EnablePreload_TIM3_CH4    (TIM3->CCMR2 |= TIM_CCMR2_OC4PE)

#define DisablePreload_TIM1_CH1    (TIM1->CCMR1 &= ~TIM_CCMR1_OC1PE)
#define DisablePreload_TIM1_CH2    (TIM1->CCMR1 &= ~TIM_CCMR1_OC2PE)
#define DisablePreload_TIM3_CH1    (TIM3->CCMR1 &= ~TIM_CCMR1_OC1PE)
#define DisablePreload_TIM3_CH2    (TIM3->CCMR1 &= ~TIM_CCMR1_OC2PE)
#define DisablePreload_TIM3_CH3    (TIM3->CCMR2 &= ~TIM_CCMR2_OC3PE)
#define DisablePreload_TIM3_CH4    (TIM3->CCMR2 &= ~TIM_CCMR2_OC4PE)


// Module Exported Functions ---------------------------------------------------
void TIM1_Init (void);
void TIM1_BRK_UP_TRG_COM_IRQHandler (void);

void TIM3_Init(void);
void TIM3_IC_CH1_OFF (void);
void TIM3_IC_CH1_ON (void);
void TIM3_IC_CNT (unsigned short new_counter);
void TIM3_IRQHandler (void);


void TIM_6_Init (void);
void TIM14_IRQHandler (void);
void TIM_14_Init(void);
void TIM16_IRQHandler (void);
void TIM_16_Init(void);
void OneShootTIM16 (unsigned short);
void TIM17_IRQHandler (void);
void TIM_17_Init(void);
void Update_TIM1_CH1 (unsigned short);
void Update_TIM1_CH2 (unsigned short);
void Update_TIM3_CH1 (unsigned short);
void Update_TIM3_CH2 (unsigned short);
void Update_TIM3_CH3 (unsigned short);
void Update_TIM3_CH4 (unsigned short);

void TIM_Timeouts (void);
void Wait_ms (unsigned short wait);

void Power_Ampli_Ena (void);
void Ampli_to_Audio (void);
void Ampli_to_Sirena (void);
void Power_Ampli_Disa (void);
void ChangeAmpli(unsigned short freq, unsigned short pwm);


#endif    /* _TIM_H_ */

//--- end of file ---//
