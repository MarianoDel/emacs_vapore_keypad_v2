//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### RWS317_HARD_CONF.H #################################
//----------------------------------------------------------
#ifndef _RWS317_HARD_CONF_H_
#define _RWS317_HARD_CONF_H_

#include "tim.h"


// Exported Types Constants and Macros -----------------------------------------
#define RxCode_TIM_Disable()	TIM3_IC_CH4_OFF ()
#define RxCode_TIM_Enable() 	TIM3_IC_CH4_ON ()
#define RxCode_TIM_CNT(X)    TIM3_IC_CNT(X)

#define CODES_IN_SST

// Exported Module Functions ---------------------------------------------------

#endif    /* _RWS317_HARD_CONF_H_ */

//--- end of file ---//

