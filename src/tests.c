//---------------------------------------------
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    TEST PLATFORM FOR FIRMWARE
// ##
// #### TESTS.C ###############################
//---------------------------------------------

// Includes Modules for tests --------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <time.h>       // for clock_t, clock(), CLOCKS_PER_SEC

// Externals -------------------------------------------------------------------



// Globals ---------------------------------------------------------------------
unsigned char buffer [1024];


// Testing Functions Declarations ----------------------------------------------
void Test_Write_Buffer (void);


// Module Functions Copys to Test ----------------------------------------------
void writeBufferNVM (unsigned char *buf, unsigned short len, unsigned int address);


// Mocked Functions Declarations -----------------------------------------------
void writePageNVM (unsigned char *buf, unsigned short len, unsigned int addr);


// Module Functions ------------------------------------------------------------
int main(int argc, char *argv[])
{
    Test_Write_Buffer ();

    return 0;
}


///////////////////////
// Testing Functions //
///////////////////////
void Test_Write_Buffer (void)
{
    int len = 2000;
    int address = 0x2000;
    
    printf("my buffer at: %p test with len: %d addr: 0x%04x\n",
           buffer,
           len,
           address);
    
    writeBufferNVM (buffer, len, address);
}


void writeBufferNVM (unsigned char *buf, unsigned short len, unsigned int address)
{
    unsigned short pages = 0;
    unsigned short pages_cnt = 0;
    unsigned short remaining = 0;
    unsigned short bytes_offset = 0;

    pages = len >> 8;

    // send complete pages
    if (pages)
    {
        remaining = len - (pages << 8);
        
        do {
            bytes_offset = (pages_cnt << 8);
            writePageNVM ((buf + bytes_offset), 256, (address + bytes_offset));
            pages_cnt++;
        
        } while (pages_cnt < pages);
    }
    else
        remaining = len;

    // send remaining bytes
    if (remaining)
    {
        bytes_offset = (pages_cnt << 8);
        writePageNVM ((buf + bytes_offset), remaining, (address + bytes_offset));
    }
}


////////////////////
// Mock Functions //
////////////////////
void writePageNVM (unsigned char *buf, unsigned short len, unsigned int addr)
{
    unsigned long long p;
    p = (unsigned long long) buf;
    p -= (unsigned long long) &buffer;
    printf("write to buf at: %p offset: %d len: %d, addr: 0x%04x\n",
           buf,
           p,
           len,
           addr);
}


//--- end of file ---//


