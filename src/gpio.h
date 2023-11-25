//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32F030
// ##
// #### GPIO.H ################################
//---------------------------------------------
#ifndef _GPIO_H_
#define _GPIO_H_


// Exported Types Constants and Macros -----------------------------------------



// Exported Module Functions ---------------------------------------------------
void GPIO_Config(void);
void EXTIOff (void);
void EXTIOn (void);
void Gpio_PA7_PA8_Output (void);
void Gpio_PA7_PA8_Alternative (void);

#endif    /* _GPIO_H_ */

//--- end of file ---//
