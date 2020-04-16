#include <stdio.h>
#include <conio.h>
#include <dos.h>

void sound(void);
void KD(void);
void SW(void);
void number(void);

void main()
{
char choice;

clrscr();
do
{
printf("1 - Sound\n2 - Division Coefficient\n3 - Status Word\n4 - number\n0 - exit\n");
fflush(stdin);
scanf("%c", &choice);

  switch(choice){
  case '1': sound(); break;
  case '2': KD(); break;
  case '3': SW(); break;
  case '4': number(); break;
  }

}while(choice != '0');
}

#define COUNT 100

void sound()
{
  int frequency[COUNT] = {392,523,392,440,493,329,329,440,392,349,392,261,261,293,293,329,349,349,392,440,493,523,587 };
  int longevity[COUNT] = {400,800,600,200,800,400,400,800,600,200,800,400,400,800,400,400,800,400,400,800,400,400,1200};

  long unsigned base = 1193180;
  int iFreq, kd;

  for(iFreq=0; iFreq <COUNT; iFreq++) //for each frequency
  {
      outp(0x43,0xB6);
      kd = base/frequency[iFreq];
      outp(0x42,kd%256); //Low
      kd /= 256;
      outp(0x42,kd); //High

      outp(0x61, inp(0x61)|3);    //turn ON
      delay(longevity[iFreq]);
      outp(0x61, inp(0x61)&0xFC); //turn OFF
      delay(10);
    }
}

void KD(void)
{
clrscr();
unsigned long j;
int iChannel;
int ports[] = {0x40, 0x41, 0x42};
int controlWords[] = {0, 64, 128};
unsigned kdLow, kdHigh, kd, max, kd2;

printf("Division coefficients:\n");

for(iChannel = 0; iChannel < 3; iChannel++)
{
  kd = 0;
  max = 0;
  if(iChannel == 2) outp(0x61, inp(0x61)|3); //turn ON for kd2

  for(j = 0; j < 65000; j++)
  {
    outp(0x43, controlWords[iChannel]); //select a channel
    kdLow  = inp(ports[iChannel]);
    kdHigh = inp(ports[iChannel]);
    kd = kdHigh*256 + kdLow;

    if(kd > max) max = kd;
  }

  if(iChannel == 2) outp(0x61, inp(0x61)&0xFC); ////turn OFF for kd2
  printf("Channel %d - %4X\n", iChannel, max);
}
}

void SW(void)
{
  clrscr();
  unsigned char state, stateWord[9];
  int iChannel, i;
  int ports[] = {0x40, 0x41, 0x42};
  int controlWords[] = {0xe2, 0xe4, 0xe8};

  for(iChannel = 0; iChannel < 3; iChannel++)
  {
    outp(0x43, controlWords[iChannel]); //select a channel
    state = inp(ports[iChannel]);

    for(i = 7; i >= 0; i --)
    {
      stateWord[i] = state%2 + '0';
      state/= 2;
    }
    stateWord[8] = '\0';
    printf("Channel %d: %s \n", iChannel, stateWord);
  }
}


void number(void)
{
  char choice;
  unsigned int limit, numberLow, numberHigh, number;

  clrscr();
  do
  {
    printf("1 - set limit\n2 - get a number\n0 - exit\n");
    fflush(stdin);
    scanf("%c", &choice);

    switch(choice){
    case '1':
      do
      {
	printf("Enter a limit 1..65535\n");
	fflush(stdin);
      }while(! scanf("%d", &limit));

      outp(0x43,0xB4);
      outp(0x42,limit%256); //Low
      limit /= 256;
      outp(0x42,limit); //High
      outp(0x61, inp(0x61)|1);    //turn ON

      break;

    case '2':
      printf("Random number:\n");

      outp(0x43, 128); //select a channel
      numberLow = inp(0x42);
      numberHigh = inp(0x42);
      number = numberHigh*256 + numberLow;

      printf("Number %u\n", number);

      break;
    }
  }while(choice != '0');

  outp(0x61, inp(0x61)&0xFC); //turn OFF
}
