//----------------------------------------------------------
// #### VAPORE ALARM WITH KEYPAD PROJECT - Custom Board ####
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ##
// #### MEMORY_UTILS.C #####################################
//----------------------------------------------------------
#include "memory_utils.h"
#include "tim.h"
#include "usart.h"
#include "memory_conf.h"
#include "sst25.h"
#include "parameters.h"

#include <stdio.h>
#include <string.h>


// Externals -------------------------------------------------------------------
extern parameters_typedef param_struct;
extern mem_bkp_typedef memory_backup;
extern filesystem_typedef files;


// Module Private Types Constants and Macros -----------------------------------
void LoadFilesIndex (void);


// Module Functions ---------------------------------------------------
void ShowFileSystem(void)
{
    char str [100];

    Usart1Send((char *) "File System:\r\n");
    Wait_ms(100);
    LoadFilesIndex();

    int filesystem_index = sizeof(files) >> 3;    // /4 for ints, /2 for pos and len
    for (int i = 0; i < filesystem_index; i++)
    {
        unsigned int * pfile = &files.posi0;
        unsigned int * psize = &files.posi0;    //offset comes next

        pfile += (i * 2);
        psize += (i * 2 + 1);
        sprintf(str, "file[%d]: %08x %08x %d\r\n",
                i,
                *pfile,
                *psize,
                *psize);
        
        Usart1Send(str);
        Wait_ms(100);
    }
}


void LoadConfiguration (void)
{
	unsigned char posi [80];
	//unsigned int * p_memory;

	readBufNVM8u(posi, 80, OFFSET_CONFIGURATION);
	//p_memory = (unsigned int *) posi;

	//param_struct.b1t = *p_memory;
	//param_struct.b1r = *(p_memory + 1);
	//param_struct.b2t = *(p_memory + 2);
	//param_struct.b2r = *(p_memory + 3);
	//param_struct.b3t = *(p_memory + 4);
	//param_struct.b3r = *(p_memory + 5);
	//param_struct.b4t = *(p_memory + 6);
	//param_struct.b4r = *(p_memory + 7);

	param_struct.b1t = *posi;
	param_struct.b1r = *(posi + 1);
	param_struct.b2t = *(posi + 2);
	param_struct.b2r = *(posi + 3);
	param_struct.b3t = *(posi + 4);
	param_struct.b3r = *(posi + 5);
	param_struct.b4t = *(posi + 6);
	param_struct.b4r = *(posi + 7);
        param_struct.audio_buttons = *(posi + 8);

	//ahora los tiempos internos
	//param_struct.interdigit = *(unsigned short *) (posi + 8);
	//param_struct.wait_for_code = *(unsigned short *) (posi + 10);

	//ahora los tiempos internos	TODO: por ahora no se configuran
	param_struct.interdigit = 3000;
        //modificacion 09-09-18 doy mas tiempo para recibir un codigo en programacion
	// param_struct.wait_for_code = 10000;
	param_struct.wait_for_code = 60000;
	// param_struct.wait_for_code = 55000;    //doy 55s para que tenga algo de espacio el unlock de 60s

}


void Load16SamplesShort (unsigned short * buf, unsigned int posi)
{
    readBufNVM8u((unsigned char *) buf, 32, posi);
}


void SaveFilesIndex (void)
{
    unsigned char * p_memory;

    p_memory = (unsigned char *) &files.posi0;

    writeBufferNVM (p_memory,
                    sizeof(filesystem_typedef),
                    OFFSET_FILESYSTEM);

}


void ShowConfiguration(void)
{
    char str [100];

    Usart1Send((char *) "Configuration:\r\n");

    sprintf(str, "B1T: %d, B1R: %d\r\n", param_struct.b1t, param_struct.b1r);
    Usart1Send(str);
    Wait_ms(100);

    sprintf(str, "B2T: %d, B2R: %d\r\n", param_struct.b2t, param_struct.b2r);
    Usart1Send(str);
    Wait_ms(100);

    sprintf(str, "B3T: %d, B3R: %d\r\n", param_struct.b3t, param_struct.b3r);
    Usart1Send(str);
    Wait_ms(100);

    sprintf(str, "B4T: %d, B4R: %d\r\n", param_struct.b4t, param_struct.b4r);
    Usart1Send(str);
    Wait_ms(100);

    if (param_struct.audio_buttons & B1_AUDIO_MASK)
        strcpy(str, "B1 audio: 1, ");
    else
        strcpy(str, "B1 audio: 0, ");

    if (param_struct.audio_buttons & B3_AUDIO_MASK)
        strcat(str, "B3 audio: 1\r\n");
    else
        strcat(str, "B3 audio: 0\r\n");
          
    Usart1Send(str);
    Wait_ms(100);
}


void LoadFilesIndex (void)
{
    // read SST memory directly to files struct
    readBufNVM8u((unsigned char *) &files, sizeof(files), OFFSET_FILESYSTEM);
}

//--- end of file ---//

