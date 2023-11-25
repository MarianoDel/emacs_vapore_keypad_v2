//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MEMORY_CONF.H ######################################
//----------------------------------------------------------
#ifndef __MEMORY_CONF_H_
#define __MEMORY_CONF_H_


// Module Exported Types Constants and Macros ----------------------------------
#define CONFIGURATION_IN_SST
#define CODES_IN_SST
#define INFO_IN_SST
#define OFFSET_FILESYSTEM	0x0000
#define OFFSET_CONFIGURATION	0x1000
#define OFFSET_FIRST_FILE	0x6000


#define FLASH_PAGE_SIZE    1024
#define FLASH_PAGE_SIZE_DIV2    512
#define FLASH_PAGE_SIZE_DIV4    256

typedef union mem_bkp {
    unsigned int v_bkp [FLASH_PAGE_SIZE_DIV4];
    unsigned char v_bkp_8u [FLASH_PAGE_SIZE];
    unsigned short v_bkp_16u [FLASH_PAGE_SIZE_DIV2];
} mem_bkp_typedef;


// at least 2 bytes aligned
typedef struct filesystem {

	//para num0
	unsigned int posi0;
	unsigned int length0;
	//para num1
	unsigned int posi1;
	unsigned int length1;
	//para num2
	unsigned int posi2;
	unsigned int length2;
	//para num3
	unsigned int posi3;
	unsigned int length3;
	//para num4
	unsigned int posi4;
	unsigned int length4;
	//para num5
	unsigned int posi5;
	unsigned int length5;
	//para num6
	unsigned int posi6;
	unsigned int length6;
	//para num7
	unsigned int posi7;
	unsigned int length7;
	//para num8
	unsigned int posi8;
	unsigned int length8;
	//para num9
	unsigned int posi9;
	unsigned int length9;
	// for audio in Button 1
	unsigned int posi10;
	unsigned int length10;
	// for audio in Button 3
	unsigned int posi11;
	unsigned int length11;

} filesystem_typedef;


#endif    /* __MEMORY_CONF_H */

