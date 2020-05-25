#define _SCL_SECURE_NO_WARNINGS
#include <cstdio>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <iostream>
#include <string>
#include <cstdlib>


using namespace std;

//максимальный период простоя для операций чтения/записи
const int MAX_TIME_OUT = 1000;

void COM1(char* path);														// COM1 - передаёт информацию
void COM2();																// COM2 - принимает информацию

int main(int argc, char* argv[])
{
	switch (argc)
	{
	case 1:																	// Если процесс клиента не создан
		COM1(argv[0]);
		break;
	default:
		COM2();
		break;
	}
}

void COM1(char* path)
{
	string name = "COM1";
	STARTUPINFO si;															// Структура для описания процесса клиента.
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION COM2_PROC_INFO;									// Структура для описания процесса COM2.
	ZeroMemory(&COM2_PROC_INFO, sizeof(COM2_PROC_INFO));

	HANDLE Receiver_COM1;
	HANDLE Semaphores[3];

	char buffer[20];			//большой буфер лучше не делать, а разбить информацию на блоки по 20 символов
	int bufferSize = sizeof(buffer);

	string message;

	cout << "-----------------COM1-----------------\n\n";

	Semaphores[0] = CreateSemaphore(NULL, 0, 1, L"SEMAPHORE");				// Семафор выполнения.
	Semaphores[1] = CreateSemaphore(NULL, 0, 1, L"SEMAPHORE_END");			// Семафор завершения.
	Semaphores[2] = CreateSemaphore(NULL, 0, 1, L"SEMAPHORE_EXIT");			// Семафор выхода.

	Receiver_COM1 = CreateFileA(
		name.c_str(),														// Имя открываемого файла.
		GENERIC_READ | GENERIC_WRITE,										// Тип доступа к файлу.
		0,																	// Параметры совместного доступа.
		NULL,																// Атрибуты защиты файла.
		OPEN_EXISTING,														// Режим автосоздания.
		FILE_ATTRIBUTE_NORMAL,												// Асинхронный режим работы.
		NULL																// Описатель файла шаблона.
	);

	CreateProcessA(
		path,																// Имя исполняемого модуля.
		(LPSTR)" COM2",													    // Параметры командной строки.
		NULL,																// Определение атрибутов защиты для нового приложения.
		NULL,																// Определение атрибутов защиты для первого потока созданного приложением.
		FALSE,																// Флаг наследования от процесса производящего запуск.
		CREATE_NEW_CONSOLE,													// Новый процесс получает новую консоль вместо того, чтобы унаследовать родительскую. 
		NULL,																// Указывает на блок среды. Блок среды это список переменных имя=значение в виде строк с нулевым окончанием.
		NULL,																// Указывает текущий диск и каталог.
		(LPSTARTUPINFOA)&si,												// Используется для настройки свойств процесса, например расположения окон и заголовок.
		&COM2_PROC_INFO														// Структура PROCESS_INFORMATION с информацией о процессе. Будет заполнена Windows.
	);

	SetCommMask(Receiver_COM1, EV_RXCHAR);									// Устанавливаем маску на события порта.
	SetupComm(Receiver_COM1, 1500, 1500);									// Инициализирует коммуникационные параметры для заданного устройства (Дескриптор, буфер ввода-вывода)

	COMMTIMEOUTS CommTimeOuts;								// Структура, характеризующая временные параметры последовательного порта.
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;			// Mаксимальное время для интервала между поступлением двух символов по линии связи.
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0;			// Множитель, используемый, чтобы вычислить полный период времени простоя для операций чтения.
	CommTimeOuts.ReadTotalTimeoutConstant = MAX_TIME_OUT;	// Константа, используемая, чтобы вычислить полный (максимальный) период времени простоя для операций чтения.
	CommTimeOuts.WriteTotalTimeoutMultiplier = 0;			// Множитель, используемый, чтобы вычислить полный период времени простоя для операций записи.
	CommTimeOuts.WriteTotalTimeoutConstant = MAX_TIME_OUT;	// Константа, используемая, чтобы вычислить полный период времени простоя для операций записи.

	if (!SetCommTimeouts(Receiver_COM1, &CommTimeOuts))
	{
		CloseHandle(Receiver_COM1);
		Receiver_COM1 = INVALID_HANDLE_VALUE;
		return;
	}

	DCB COM_DCB;										// Структура, характеризующая основные параметры последовательного порта. 
	memset(&COM_DCB, 0, sizeof(COM_DCB));				// Выделение памяти под структуру.
	COM_DCB.DCBlength = sizeof(DCB);					// Задает длину, в байтах, структуры.
	GetCommState(Receiver_COM1, &COM_DCB);				// Извлекает данные о текущих настройках управляющих сигналов для указанного устройства.
	COM_DCB.BaudRate = DWORD(9600);						// Скорость передачи данных.
	COM_DCB.ByteSize = 8;								// Определяет число информационных бит в передаваемых и принимаемых байтах.
	COM_DCB.Parity = NOPARITY;							// Определяет выбор схемы контроля четности (Бит честности отсутствует).
	COM_DCB.StopBits = ONESTOPBIT;						// Задает количество стоповых бит (Один бит). 
	COM_DCB.fAbortOnError = TRUE;						// Задает игнорирование всех операций чтения/записи при возникновении ошибки.
	COM_DCB.fDtrControl = DTR_CONTROL_DISABLE;			// Задает режим управления обменом для сигнала DTR.
	COM_DCB.fRtsControl = RTS_CONTROL_DISABLE;			// Задает режим управления потоком для сигнала RTS.
	COM_DCB.fBinary = TRUE;								// Включает двоичный режим обмена.
	COM_DCB.fParity = FALSE;							// Включает режим контроля четности.
	COM_DCB.fInX = FALSE;								// Задает использование XON/XOFF управления потоком при приеме.
	COM_DCB.fOutX = FALSE;								// Задает использование XON/XOFF управления потоком при передаче.
	COM_DCB.XonChar = 0;								// Задает символ XON используемый как для приема, так и для передачи.
	COM_DCB.XoffChar = (unsigned char)0xFF;				// Задает символ XOFF используемый как для приема, так и для передачи.
	COM_DCB.fErrorChar = FALSE;							//Задает символ, использующийся для замены символов с ошибочной четностью.
	COM_DCB.fNull = FALSE;								// Указывает на необходимость замены символов с ошибкой четности на символ задаваемый полем ErrorChar.
	COM_DCB.fOutxCtsFlow = FALSE;						// Включает режим слежения за сигналом CTS.
	COM_DCB.fOutxDsrFlow = FALSE;						// Включает режим слежения за сигналом DSR.
	COM_DCB.XonLim = 128;								// Задает минимальное число символов в приемном буфере перед посылкой символа XON.
	COM_DCB.XoffLim = 128;								// Определяет максимальное количество байт в приемном буфере перед посылкой символа XOFF.

	//обработка ошибки
	if (!SetCommState(Receiver_COM1, &COM_DCB))
	{
		CloseHandle(Receiver_COM1);
		Receiver_COM1 = INVALID_HANDLE_VALUE;
		return;
	}

	// Цикл передачи мессенджа
	cout << "For exit press [`]" << endl;
	while (true)
	{
		DWORD AmountOfBytes;

		cout << ">>> ";
		cin.clear();
		getline(cin, message);

		if (message == "`")
		{																	// Условие выхода из программы.
			ReleaseSemaphore(Semaphores[2], 1, NULL);						// Реализация семафора выхода (Ставим в сигнальное состояние).
			WaitForSingleObject(COM2_PROC_INFO.hProcess, INFINITE);		// Ожидание сигнала от дочернего процесса.
			break;
		}

		ReleaseSemaphore(Semaphores[0], 1, NULL);							// Реализация семафора выполения (Ставим в сигнальное состояние).

		int AmountOfBlocks = message.size() / bufferSize + 1;				//количество блоков передаваемой инфы
		WriteFile(Receiver_COM1, &AmountOfBlocks, sizeof(AmountOfBlocks), &AmountOfBytes, NULL);

		int size = message.size();
		WriteFile(Receiver_COM1, &size, sizeof(size), &AmountOfBytes, NULL);

		for (int i = 0; i < AmountOfBlocks; i++)
		{
			message.copy(buffer, bufferSize, i * bufferSize);
			if (!WriteFile(Receiver_COM1, buffer, bufferSize, &AmountOfBytes, NULL))
				cout << "Error.";
		}
		WaitForSingleObject(Semaphores[1], INFINITE);						// Ожидание перехода в несигнальное состояние.
	}

	CloseHandle(Receiver_COM1);
	CloseHandle(Semaphores[0]);
	CloseHandle(Semaphores[1]);
	cout << "\n";
	system("pause");
	return;
}

void COM2()
{
	HANDLE Receiver_COM2;
	HANDLE Semaphores[3];

	char buffer[20];
	int bufferSize = sizeof(buffer);

	string message;
	string name = "COM2";

	bool successFlag;
	Semaphores[0] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, L"SEMAPHORE");
	Semaphores[1] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, L"SEMAPHORE_END");
	Semaphores[2] = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, L"SEMAPHORE_EXIT");

	cout << "-----------------COM2-----------------\n\n";

	//Передатчик информации
	Receiver_COM2 = CreateFileA(name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	while (true)
	{
		successFlag = TRUE;
		DWORD AmountOfBytes;
		message.clear();

		int SemaphoreIndex = WaitForMultipleObjects(3, Semaphores, FALSE, INFINITE) - WAIT_OBJECT_0;
		if (SemaphoreIndex == 2)
		{
			break;
		}

		int AmountOfBlocks;
		if (!ReadFile(Receiver_COM2, &AmountOfBlocks, sizeof(AmountOfBlocks), &AmountOfBytes, NULL)) break;

		int size;
		if (!ReadFile(Receiver_COM2, &size, sizeof(size), &AmountOfBytes, NULL)) break;

		for (int i = 0; i < AmountOfBlocks; i++)
		{
			successFlag = ReadFile(Receiver_COM2, buffer, bufferSize, &AmountOfBytes, NULL);
			if (!successFlag)
			{
				break;
			}
			message.append(buffer, bufferSize);
		}

		if (!successFlag)
		{
			break;
		}

		message.resize(size);

		for (int i = 0; i < size; i++)
		{
			cout << message[i];
		}
		cout << endl;
		ReleaseSemaphore(Semaphores[1], 1, NULL);
	}
	CloseHandle(Receiver_COM2);
	CloseHandle(Semaphores[0]);
	CloseHandle(Semaphores[1]);
	return;
}