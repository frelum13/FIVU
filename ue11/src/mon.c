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

// defines
// ...

// declarations and definations

int8_t mon_cmd_info (uint8_t argc, char *argv[]);
int8_t mon_cmd_test (uint8_t argc, char *argv[]);
int8_t mon_cmd_hilfe (uint8_t argc, char *argv[]);
int8_t mon_cmd_mot (uint8_t argc, char *argv[]);
int8_t mon_cmd_vkal (uint8_t argc, char *argv[]);

const char MON_LINE_WELCOME[] PROGMEM = "Line-Mode: CTRL-X, CTRL-Y, CTRL-C, Return  \n\r";
const char MON_PMEM_CMD_INFO[] PROGMEM = "info\0Application infos\0info";
const char MON_PMEM_CMD_TEST[] PROGMEM = "test\0commando for test\0test";
const char MON_PMEM_CMD_Hilfe[] PROGMEM = "hilfe\0Kommando nach Vorschlag Moerth\0hilfe arg1 arg2 ...";
const char MON_PMEM_CMD_Mot[] PROGMEM = "mot\0Motor PWM setzen\0mot off | <pwm 0-100>";
const char MON_PMEM_CMD_Vkal[] PROGMEM = "vkal\0Kalibrieren der zu Messenden Spannung\0vkal <wert>";

const struct Sys_MonCmdInfo MON_PMEMSTR_CMDS[] PROGMEM =
{
    { MON_PMEM_CMD_INFO, mon_cmd_info }
  , { MON_PMEM_CMD_TEST, mon_cmd_test }
  , { MON_PMEM_CMD_Hilfe, mon_cmd_hilfe }
  , { MON_PMEM_CMD_Mot, mon_cmd_mot }
  , { MON_PMEM_CMD_Vkal, mon_cmd_vkal}
};

volatile struct Mon mon;

// functions

void mon_init (void)
{
  memset((void *)&mon, 0, sizeof(mon));
}


//--------------------------------------------------------

unsigned char EEPROM_read(unsigned int uiAddress)
{

  while(EECR & (1<<EEWE));
  EEAR = uiAddress;
  EECR |= (1<<EERE);
  return EEDR;
}

void EEPROM_write (unsigned int uiAddress, unsigned char ucData)
  {
    
    while(EECR & (1<<EEWE));
    EEAR = 0 ;
    EEDR = ucData;
    EECR |= (1<<EEMWE);
    EECR |= (1<<EEWE);
}

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

int8_t mon_cmd_hilfe(uint8_t argc, char *argv[])
{
  printf("argc = %u\n", argc);
  uint8_t i;
  for(i = 0;i<argc ; i++)
  {
    printf("argc[%u] = %s\n",i,argv[i]);
  }
}

int8_t mon_cmd_mot(uint8_t argc, char *argv[])
{
  if(argc != 2)
    return 1;
  
  if(strcmp(argv[1],"off")==0)
  {
    printf("Motore werden abgedreht");
    return 0;
  }
  
  uint16_t pwm;
  if(sscanf(argv[1], "%u", &pwm)!=1)
    return 2;
  
  if(pwm>100)
    return 3;
  
  printf("Motore werden auf PWM %u%% gesetzt \n",pwm);
}

int8_t mon_cmd_vkal(uint8_t argc, char *argv[])
{
  char kalibrierung1,kalibrierung2;
  float vkal;
  sscanf(argv[1], "%f", vkal);
        printf("Die Spannung wurde auf %f gesetzt",vkal);
  uint8_t adc = app.spannung;
  _delay_ms(10);
  float vcc = 256*2.56/adc;
  
  kalibrierung1 = (char)vkal;
  kalibrierung2 = kalibrierung1-vcc;
  EEPROM_write(0,kalibrierung);
  
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
    case 2: printf("L2 | T0CNT"); return 25;
    case 3: printf("L3 | time/ms  PWM   (press 'c' for reset Time)"); return 25;
    case 4: printf("L4 | Spannung"); return 25;
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
      printf("   |  0x%02x   %u", TCNT0, app.pwm);
      return 5; // Anzahl der der Lehrzeichen hinter der printf Ausgabe 
    }
    case 3:
    {
      if (keyPressed=='c')
        app.time_MilliSeconds = 0;
      printf("%lu %u",app.time_MilliSeconds, app.pwm);
      return 5;
    }
    case 4:
    {
      uint8_t adc = app.spannung;
      _delay_ms(10);
      float vcc = 256*2.56/adc;
      uint8_t kalibrierung = EEPROM_read(0);
      printf("%.2f",vcc+kalibrierung);
    }
    default: return -1;
  }
}

#endif // GLOBAL_MONITOR



