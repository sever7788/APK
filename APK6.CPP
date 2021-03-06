#include <dos.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>

typedef unsigned char byte;
struct VIDEO
{
	byte symb;
	byte attr;
};
unsigned int blinking = 0;					//����, ��������� � ���, ��� �� ������ ����������
int quitFlag = 0;							//���� ������ �� ���������
unsigned int repeat = 1;					//���� ��������� � ������������� �������� �������� ������� ����������
unsigned int attempt = 3;					//���������� �������
void interrupt(*oldInt09)(...);				//������ ���������� ����������

void printDiod(int i)
{
	
	VIDEO far* screen = (VIDEO far *)MK_FP(0xB800, 0);
		
		
		screen->attr = 0x7F;
		screen->symb = 'C';
		screen += 2;
		screen->attr = 0x7F;
		screen->symb = 'N';
		screen += 2;
		screen->attr = 0x7F;
		screen->symb = 'S';
		screen += 76;
	
	if(i!=1)
	{
		screen += i;
		screen->attr = 0x20;
		screen->symb = 0x20;
	}
	else{
		for(int j = 0; j<4; j++){
			screen+=j;
		screen->attr = 0x00;
		screen->symb = 0x20;
		}
	}
	
	
}

//����� ���������� ����������
void interrupt newInt09(...)
{
	unsigned char keyCode;

	keyCode = inp(0x60);					//�������� ������ �� ������ ��-��
	if (keyCode == 0x01)					//���� ������ ������� ESC(���� ��� = 01), �� ����� �� ���������
	{
		quitFlag = 1;						//��������� ����� ������
		outp(0x20, 0x20); 					//�������� ����������� ���������� (master) ������ EOI (end of interrupt)
		outp(0xA0, 0x20);					//�������� ������� ����������� ���������� (slave) ������ EOI (end of interrupt)
		return;
	}
	
	if (keyCode == 0xFE && blinking)		//���� �������� ���� ������� ��� ��������� �����������, �� ��-�� ������ FEh
	{
		if (attempt-- == 0)
		{
			printf("Error. Input register of keyboard is busy.\n");
			quitFlag = 1;
		}
		repeat = 1;							//������ ����, ��������� � ���, ��� ����� ��������� �������� �������
	}
	else
	{
		repeat = 0;
		attempt = 3;						//��������������� ���������� �������
	}

	printf("\t%x", keyCode);				//����� �������� �������
	outp(0x20, 0x20); 						//�������� ����������� ���������� (master) ������ EOI (end of interrupt)
	outp(0xA0, 0x20);						//�������� ������� ����������� ���������� (slave) ������ EOI (end of interrupt)
}

//������� �������� ��������, i - ���� ��������� �����������
void keyBlinking(unsigned char i)
{
	int timeout = 50000;
	repeat = 1;								//��������� ����� �������, ������ ����� ������ ����� ���������� ��-��
	//����� ���������� ������ �������  ���������� �������� �������������� ���(FAh - ��� ������, FEh - � ������� ������)
	while (repeat)
	{
		while (timeout-- > 0)
		{
			//��������� ������� ������ �� ������� ������ ��-��
			//������� ��������� ��-�� � �������� 1-�� ���� �������� ���������
			//1-�� ��� �������� �� ������� ������ �� ������� ������ ��-��.(1 - ������ ����)
			if ((inp(0x64) & 0x02) == 0x00)
			{
				break;						//���� ������ ���, �� ������� �� ��������
			}
		}
		if (!timeout)
		{
			printf("Timeout error.\n");		//���� ����� �������
			disable();
			setvect(0x09, oldInt09);		//��������� ������� �����������
			enable();
			exit(1);						//����� �� ���������
		}
		outp(0x60, 0xED);					//���������� � ���� ������� ���������� ������������
		delay(200);
	}
	timeout = 50000;
	repeat = 1;								//��������� ����� �������, ������ ����� ������ ����� ���������� ��-��
	//����� ���������� ������ �������  ���������� �������� �������������� ���(FAh - ��� ������, FEh - � ������� ������)
	while (repeat)
	{
		//��������� ������� ������ �� ������� ������ ��-��
		//������� ��������� ��-�� � �������� 1-�� ���� �������� ���������
		//1-�� ��� �������� �� ������� ������ �� ������� ������ ��-��.(1 - ������ ����)
		while (timeout-- > 0){
			if ((inp(0x64) & 0x02) == 0x00){
				break;
			}
		}
		if (!timeout)						//�������� �� ����� �� ����� ��������
		{
			printf("Timeout error.\n");
			disable();
			setvect(0x09, oldInt09);		//��������� ������� �����������
			enable();
			exit(1);						//����� �� ���������
		}
		outp(0x60, i);						//���������� ���� ��������� �����������
		delay(200);
	}
	switch(i){
		case 0x04: printDiod(0); break;		//��������� �������� � ����������� �������� "������" 
		case 0x00: printDiod(1); break;
		case 0x01: printDiod(2); break;
		case 0x02: printDiod(4); break;
	}
}

int main()
{
	delay(200);
	disable();								//��������� ����������
	oldInt09 = getvect(0x09);				//���������� ������ ���������� ����������
	setvect(0x09, newInt09);				//��������� ������ ���������� ����������
	enable();								//��������� ����������
	blinking = 1;							//������ ���� �������, ����� ����� ���������� ������������� ��� �� ��-��
	for(int j=0; j<10;j++)					//������� 10 ���
	{
		keyBlinking(0x04);					//������� (Caps Lock)
		keyBlinking(0x00);					//��������� ��� ��������
		keyBlinking(0x01);					//������� (Scroll Lock)
		keyBlinking(0x02);					//������� (Num Lock)
	}
	blinking = 0;
	
	while (!quitFlag);
	disable();								//������ ����������
	setvect(0x09, oldInt09);				//��������� ������� ����������� ���������� ����������
	enable();
	return 0;
}