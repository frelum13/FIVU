#include "global.h"

#include <stdio.h>
#include <string.h>

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "mon.h"
#include "app.h"
#include "sys.h"

#ifdef GLOBAL_MONITOR

int8_t mon_cmd_info (uint8_t argc, char *argv[]);
int8_t mon_cmd_test (uint8_t argc, char *argv[]);
int8_t mon_cmd_kalibrierung (uint8_t argc, char *argv[]);


void EEPROM_write(unsigned int uiAddress, char ucData)
{

  while(EECR & (1<<EEPE));
  EEAR = uiAddress;
  EEDR = ucData;
  EECR |= (1<<EEMPE);
  EECR |= (1<<EEPE);
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
  while(EECR & (1<<EEPE));
  EEAR = uiAddress;
  EECR |= (1<<EERE);
  return EEDR;
}

const char MON_LINE_WELCOME[] PROGMEM = "Line-Mode: CTRL-X, CTRL-Y, CTRL-C, Return  \n\r";
const char MON_PMEM_CMD_INFO[] PROGMEM = "info\0Application infos\0info";
const char MON_PMEM_CMD_TEST[] PROGMEM = "test\0commando for test\0test";
const char MON_PMEM_CMD_KALIBRIEREN[] PROGMEM = "kal\0Kalibrierung der Messung\0kal <Wert>";

const struct Sys_MonCmdInfo MON_PMEMSTR_CMDS[] PROGMEM =
{
    { MON_PMEM_CMD_INFO, mon_cmd_info }
  , { MON_PMEM_CMD_TEST, mon_cmd_test }
  , { MON_PMEM_CMD_KALIBRIEREN, mon_cmd_kalibrierung}
};

volatile struct Mon mon;

void mon_init (void)
{
  memset((void *)&mon, 0, sizeof(mon));
}


//--------------------------------------------------------

inline void mon_main (void)
{
}

inline uint8_t mon_getCmdCount (void)
{
  return sizeof(MON_PMEMSTR_CMDS)/sizeof(struct Sys_MonCmdInfo);
}


// --------------------------------------------------------
// Monitor commands of the application
// --------------------------------------------------------

int8_t mon_cmd_info (uint8_t argc, char *argv[])
{
  printf("app.flags_u8  : ");
  sys_printHexBin8(sys.flags_u8);
  sys_newline();
  return 0;
}


int8_t mon_cmd_test (uint8_t argc, char *argv[])
{
  uint8_t i;

  for (i = 0; i<argc; i++)
    printf("%u: %s\n\r", i, argv[i]);

  return 0;
}

int8_t mon_cmd_kalibrierung (uint8_t argc, char *argv[])
{

  
  if(argc != 2)
    return 1;
  
  if(strcmp(argv[1],"off")==0)
  {
    printf("Motore werden abgedreht");
    return 0;
  }
  
  uint16_t kal;
  
  if(sscanf(argv[1], "%u", &kal)!=1)
    return 2;
  
  printf("%u\n",kal);
  
  EEPROM_write(2,kal);
  
  
  return 0;
}

// --------------------------------------------------------
// Monitor-Line for continues output
// --------------------------------------------------------

int8_t mon_printLineHeader (uint8_t lineIndex)
{
  if (lineIndex==0)
    sys_printString_P(MON_LINE_WELCOME);
  
  switch (lineIndex)
  {
    case 0: printf("L0 | app.flags_u8"); return 20;
    case 1: printf("L1 | counter  (press 'c' for reset)"); return 40;
    case 2: printf("L2 | Spannung "); return 40;
    default: return -1; // this line index is not valid
  }
}

int8_t mon_printLine   (uint8_t lineIndex, char keyPressed)
{

  switch (lineIndex)
  {
    case 0:
      printf("   | "); sys_printBin(app.flags_u8, 0);
      return 2;

    case 1:
      {
        static uint16_t cnt = 0;
        if (keyPressed=='c')
        cnt = 0;
        printf("   |  0x%04x   ", cnt++);
      }
      return 2;
    case 2:
    {
      uint8_t myByte;
      myByte = EEPROM_read(2);
      
      uint8_t adc = app.spannung;
      _delay_ms(10);
      float vcc = 256*2.56/adc;
      
      printf("%u + %f",myByte, vcc);
    }

    default: return -1;
  }
}

#endif // GLOBAL_MONITOR



