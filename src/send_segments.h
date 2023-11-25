//------------------------------------------------------
// #### PROYECTO PANEL ALARMA VAPORE - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### SEND_SEGMENTS.H ################################
//------------------------------------------------------
#ifndef _SEND_SEGMENTS_H_
#define _SEND_SEGMENTS_H_

#include "stm32f0xx.h"

// Exported Types Constants and Macros -----------------------------------------


// Exported Module Functions ---------------------------------------------------
#define SendSegment(X) do {\
    GPIOB->BSRR = 0x0000FF00;\
    unsigned char a = ~X;\
    GPIOB->BSRR = (a << 24);\
    } while (0)


#endif    /* _SEND_SEGMENTS_H_ */

//--- end of file ---//

