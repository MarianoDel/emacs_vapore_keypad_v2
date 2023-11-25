//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MEMORY_UTILS.H #####################################
//----------------------------------------------------------
#ifndef __MEMORY_UTILS_H_
#define __MEMORY_UTILS_H_


// Module Exported Types Constants and Macros ----------------------------------


// Module Exported Functions ---------------------------------------------------
void SaveFilesIndex (void);
void LoadConfiguration (void);
void ShowFileSystem(void);
void ShowConfiguration(void);
void Load16SamplesShort (unsigned short * buf, unsigned int posi);

#endif    /* __MEMORY_UTILS_H */

