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
unsigned int blinking = 0;					//Флаг, говорящий о том, что мы мигаем лампочками
int quitFlag = 0;							//Флаг выхода из программы
unsigned int repeat = 1;					//Флаг говорящий о необходимости повторно отослать команду клавиатуре
unsigned int attempt = 3;					//Количество попыток
void interrupt(*oldInt09)(...);				//Старый обработчик клавиатуры

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

//Новый обработчик клавиатуры
void interrupt newInt09(...)
{
	unsigned char keyCode;

	keyCode = inp(0x60);					//Получаем данные из буфера кл-ры
	if (keyCode == 0x01)					//Если нажата клафиша ESC(скан код = 01), то выход из программы
	{
		quitFlag = 1;						//Установка флага выхода
		outp(0x20, 0x20); 					//Посылаем контроллеру прерываний (master) сигнал EOI (end of interrupt)
		outp(0xA0, 0x20);					//Посылаем второму контроллеру прерываний (slave) сигнал EOI (end of interrupt)
		return;
	}
	
	if (keyCode == 0xFE && blinking)		//Если значение кода команды или параметра недопустимы, то кл-ра вернет FEh
	{
		if (attempt-- == 0)
		{
			printf("Error. Input register of keyboard is busy.\n");
			quitFlag = 1;
		}
		repeat = 1;							//Ставим флаг, говорящий о том, что нужно повторить отправку команды
	}
	else
	{
		repeat = 0;
		attempt = 3;						//Восстанавливаем количество попыток
	}

	printf("\t%x", keyCode);				//Вывод сканкода символа
	outp(0x20, 0x20); 						//Посылаем контроллеру прерываний (master) сигнал EOI (end of interrupt)
	outp(0xA0, 0x20);						//Посылаем второму контроллеру прерываний (slave) сигнал EOI (end of interrupt)
}

//Функция мерцания лампочки, i - байт состояния индикаторов
void keyBlinking(unsigned char i)
{
	int timeout = 50000;
	repeat = 1;								//Установка флага повтора, котрый потом снимит новое прерывание кл-ры
	//После выполнения каждой команды  клавиатура посылает подтверждающий код(FAh - все хорошо, FEh - в команде ошибка)
	while (repeat)
	{
		while (timeout-- > 0)
		{
			//Проверяем наличие данных на входном буфере кл-ры
			//Читение состояние кл-ры и проверка 1-го бита регистра состояния
			//1-ый бит отвечает за наличие данных на входном буфере кл-ры.(1 - данные есть)
			if ((inp(0x64) & 0x02) == 0x00)
			{
				break;						//Если данных нет, то выходим из ожидания
			}
		}
		if (!timeout)
		{
			printf("Timeout error.\n");		//Если время истекло
			disable();
			setvect(0x09, oldInt09);		//Установка старого обработчика
			enable();
			exit(1);						//Выход из программы
		}
		outp(0x60, 0xED);					//Записываем в порт команду управления индикаторами
		delay(200);
	}
	timeout = 50000;
	repeat = 1;								//Установка флага повтора, котрый потом снимит новое прерывание кл-ры
	//После выполнения каждой команды  клавиатура посылает подтверждающий код(FAh - все хорошо, FEh - в команде ошибка)
	while (repeat)
	{
		//Проверяем наличие данных на входном буфере кл-ры
		//Читение состояние кл-ры и проверка 1-го бита регистра состояния
		//1-ой бит отвечает за наличие данных на входном буфере кл-ры.(1 - данные есть)
		while (timeout-- > 0){
			if ((inp(0x64) & 0x02) == 0x00){
				break;
			}
		}
		if (!timeout)						//Проверка не вышло ли время ожидания
		{
			printf("Timeout error.\n");
			disable();
			setvect(0x09, oldInt09);		//Установка старого обработчика
			enable();
			exit(1);						//Выход из программы
		}
		outp(0x60, i);						//Записываем байт состояния индикаторов
		delay(200);
	}
	switch(i){
		case 0x04: printDiod(0); break;		//Указываем смещение в видеобуфере символов "диодов" 
		case 0x00: printDiod(1); break;
		case 0x01: printDiod(2); break;
		case 0x02: printDiod(4); break;
	}
}

int main()
{
	delay(200);
	disable();								//Запрещаем прерывания
	oldInt09 = getvect(0x09);				//Запоминаем старый обработчик клавиатуры
	setvect(0x09, newInt09);				//Установка нового обработчик клавиатуры
	enable();								//Разрешаем прерывания
	blinking = 1;							//Ставим флаг мигания, чтобы новое прерывание анализировало код из кл-ры
	for(int j=0; j<10;j++)					//Моргаем 10 раз
	{
		keyBlinking(0x04);					//Мигание (Caps Lock)
		keyBlinking(0x00);					//Выключаем все лампочки
		keyBlinking(0x01);					//Мигание (Scroll Lock)
		keyBlinking(0x02);					//Мигание (Num Lock)
	}
	blinking = 0;
	
	while (!quitFlag);
	disable();								//Запрет прерываний
	setvect(0x09, oldInt09);				//Установка старого обработчика прерывания клавиатуры
	enable();
	return 0;
}