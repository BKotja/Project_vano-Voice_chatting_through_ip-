#include "stdafx.h"

#pragma comment(lib, "ws2_32")//включаем библиотеку для сокетов
#include <winsock2.h>

#pragma comment(lib,"winmm.lib")
#include <Windows.h>
#include <mmsystem.h>
#include <fstream>
#include <iostream>


using namespace std;

int i_error = 0;//будет хранить код ошибки

				/*/////////////////////////////////////////////////////////////////////////////////////////////////*/

void server() {

	std::cout << "Zaszynam tworzyc server" << endl << endl;
	/********************************************
	0. настройка библиотеки Ws2_32.dll*/
	//В случае успеха WSAStartup возвращает 0; иначе - код ошибки. 
	WSADATA wsaData;//определяем переменную
	i_error = WSAStartup(MAKEWORD(2, 2), &wsaData);//настраиваем
	if (i_error)//проверка на успешность настройки
	{
		std::cout << "0)  Fatalny blad" << endl;
	}
	else { std::cout << "0)   Biblioteka Ws2_32.dll jest podlaczona" << endl; }

	/********************************************
	1. создание серверного сокета*/
	//В случае успеха socket возвращает номер сокет-дескриптора. иначе -1
	SOCKET SSock;//создаем прототип
	SSock = socket(
		AF_INET,//используется при создании Интернет-приложений
		SOCK_STREAM,//потоковый
		IPPROTO_TCP//ТСР - для потоковых
	);
	if (SSock == -1) {
		std::cout << "1)   Bland tworzenia socketu" << endl;
	}
	else { std::cout << "1)   Socket jest stworzony " << endl; }

	/********************************************
	2. Привязка к локальным именам*/
	//необходимо сокет связать с IP-адресом компьютера.
	struct sockaddr_in addr;//создаем прототип(addr), структуры sockaddr_in
	addr.sin_family = AF_INET;//при создании Интернет-приложений
							  //далее функции преобразуют переменные типа short [для htons()] 
							  //и long[для htonl()], к формату понятному для сети.
	addr.sin_port = htons(3000);//порт
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY сервер работает НА машине, с любым ip

	i_error = 0;	//очистка переменной для ошибок
	i_error = bind(			//В случае успеха bind возвращает 0, в противном случае - "-1"
		SSock,
		(SOCKADDR*)&addr,	//addr - указатель на структуру sockaddr_in
		sizeof(addr)		//размер структуры.
	);
	if (i_error == -1) {
		std::cout << "2)   Blad powiazania socketa z ip adresem komputera" << endl;
	}
	else { std::cout << "2)   Socket jest powiazany z ip adresem komputera" << endl; }



	/********************************************
	3. Инициализация процесса «прослушивания» порта*/
	//приложение-сервер будет следить за всеми подключениями на заданный порт
	std::cout << "3)   Inicializacja porta" << endl;
	listen(
		SSock,
		3 //сколько запросов могут стоять в очереди
	);

	/********************************************
	4. подключение клиента - Получение дескриптора клиентского соединения*/
	//сервер создает новый сокет и связывает его с ассоциацией, 
	//эквивалентной 'слушающему сокету'
	SOCKET ClientSock;//создаем прототип сокета для подключающегося клиента
	struct sockaddr_in addr_klienta;//создаем прототип структуры для подключающегося клиента
									//Если функция accept выполнилась успешно - возвращает новый сокет 
									//для установленного соединения, в противном случае  INVALID_SOCKET
	int size_client_addr = sizeof(addr_klienta);//размер структуры для клиента
	ClientSock = accept(
		SSock,// сокет
		(sockaddr*)&addr_klienta, //адрес структуры (!)sockaddr_in 
								  //куда будет помещена информация о клиенте.
		&size_client_addr // Длинна адреса
	);
	/*Метод Socket.Accept извлекает из очереди первый запрос и
	возвращает новый объект Socket,
	который можно использовать для коммуникационного взаимодействия с клиентом.
	*/
	/*accept Как я уже говорил ранее в addr будет возвращена информация о клиенте.
	Нам будет важна только:
	addr.sin_port – порт с которого было подключение
	addr.sin_addr – ip клиента в двоичном формате. Для преобразования в строковой формат используется функция inet_ntoa. Т.е. ip:pchar; ip:=inet_ntoa(addr.sin_addr);
	*/

	if (ClientSock == INVALID_SOCKET) std::cout << "4)   Bland palaczenia klienta" << endl;
	else std::cout << "4)   Klient jest polaczony" << endl;

	///////////////////////////////////////////////////////////////////////////////////////////////

	// WAVEFORMATEX структура
	WAVEFORMATEX wfx = {};
	wfx.wFormatTag = WAVE_FORMAT_PCM;       // PCM (стандартный формат)
	wfx.nChannels = 2;                      // 2 канала
	wfx.nSamplesPerSec = 44100;             // Сэмплрейт 44100 Hz
	wfx.wBitsPerSample = 16;                // 16 bit
											// Другие настройки:
	wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;


	// Открываем 'waveIn' для записи
	HWAVEIN wi;
	waveInOpen(&wi,            // наполняем 'wi' handle
		WAVE_MAPPER,    // дефолтный девайс
		&wfx,           // указатель к структуре wfx
		NULL, NULL,      // нам не нужен callback
		CALLBACK_NULL | WAVE_FORMAT_DIRECT
	);

	// Наш девайс готов для записи, даём ему буферы для записи
	char buffers[2][44100 * 2 / 2];    // 2 буффера
	WAVEHDR headers[2] = { {},{} };           // инициализирую 2 буфера в структуре на 0
	for (int i = 0; i < 2; ++i)
	{
		headers[i].lpData = buffers[i];             // присваиваю значиния с buffers в headers 
		headers[i].dwBufferLength = 44100 * 2 / 4;      // размер этих буферов (подобрал максимально оптимальный без потерь, хуй знает как рабоатет)


														// Готовим каждый header
		waveInPrepareHeader(wi, &headers[i], sizeof(headers[i]));

		// Добавляем в очередь
		//  Как только начнеться запись звука, так сразу буфера будут заполняться по =очереди=
		waveInAddBuffer(wi, &headers[i], sizeof(headers[i]));
	}


	std::cout << "Poczatek polaczenia" << std::endl;

	// Стартуем запись
	waveInStart(wi);

	//Теперь записаный звук будет поступать в буферы по мере заполнения.
	//Как только заполнятся 1 - данные записываються во 2, а с 1го отсылаються собеседнику.
	//Далее буферы будут очищены и снова добавлены в очередь и так до посинения...

	char c_data_ot_k[44100 * 2 / 4]; // Сетевой буфер 

									 // Структура для воспроизведения звука
	HWAVEOUT hWaveOut;
	WAVEHDR WaveOutHdr;

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0L, 0L, WAVE_FORMAT_DIRECT);

	WaveOutHdr.lpData = c_data_ot_k;
	WaveOutHdr.dwBufferLength = 44100 * 2 / 5;
	WaveOutHdr.dwBytesRecorded = 0;
	WaveOutHdr.dwFlags = 0L;


	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))  // Пока не ткнешь esc
	{
		for (auto &h : headers)      // Проверка каждого header
		{
			if (h.dwFlags & WHDR_DONE)           // Готов ли он? (полный ли?)
			{
				// Если да - то пошла отправка



				//5. Передача данных


				send(//отправка
					ClientSock,//сокет
					h.lpData,//данные
					h.dwBufferLength,
					0//делаем ее аналогичной как функция write
				);

				recv(//чтение
					ClientSock,//сокет
					c_data_ot_k,//буфер куда поподут данные
					44100 * 2 / 4,//размер буфера чтения - сколько считать
					0//делаем ее аналогичной как функция read
				);



				// Добавляем снова в очередь (очистив)
				h.dwFlags = 0;          // как заполнилось - значит убираем флаг готовности
				h.dwBytesRecorded = 0;  // ставим что записано 0 байт

										// заново добавляем  
				waveInPrepareHeader(wi, &h, sizeof(h));
				waveInAddBuffer(wi, &h, sizeof(h));

				waveOutPrepareHeader(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));

				waveOutSetVolume(hWaveOut, 0xFFFFFFFF);

				waveOutWrite(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));
			}
		}
	}

	waveInStop(wi);
	for (auto& h : headers)
	{
		waveInUnprepareHeader(wi, &h, sizeof(h));
	}
	waveInClose(wi);
	waveOutClose(hWaveOut);

	// All done!
	/********************************************
	6. Закрытие сокета*/
	closesocket(SSock);//закрытие
	WSACleanup();// выгружаем WinSock

	system("pause");//ждем нажатие клавиши :)
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////*/


void klient() {

	system("chcp 1251 > text");//ставим подержку русской кодировки
							   //--------------
	std::cout << "Zacznam tworzyc klient" << endl << endl;
	/********************************************
	0. настройка библиотеки Ws2_32.dll*/
	//В случае успеха WSAStartup возвращает 0; иначе - код ошибки. 
	WSADATA wsaData;//определяем переменную
	i_error = WSAStartup(MAKEWORD(2, 2), &wsaData);//настраиваем
	if (i_error)//проверка на успешность настройки
	{
		std::cout << "0)   Fatalny blad" << endl;
	}
	else { std::cout << "0)   Biblioteka Ws2_32.dll jest podlaczona" << endl; }

	/********************************************
	1. создание серверного сокета*/
	//В случае успеха socket возвращает номер сокет-дескриптора. иначе -1
	SOCKET my_sock;//создаем прототип
	my_sock = socket(
		AF_INET,//используется при создании Интернет-приложений
		SOCK_STREAM,//потоковый
		IPPROTO_TCP//ТСР - для потоковых
	);
	if (my_sock == -1) {
		std::cout << "1)   Bland tworzenia socketu" << endl;
	}
	else { std::cout << "1)   Socket jest stworzony" << endl; }

	/********************************************
	2. Привязка к локальным именам*/

	struct sockaddr_in  dest_addr;//создаем прототип(dest_addr), структуры sockaddr_in
	dest_addr.sin_family = AF_INET;//при создании Интернет-приложений
	dest_addr.sin_port = htons(3000); //порт

									  // Преобразование IP -адреса. servIP - адрес сервера

	printf("2)   Podaj ip servera:\n");
	char servIP[20];		//задаем ip
	scanf("%s", servIP);
	dest_addr.sin_addr.s_addr = inet_addr(servIP);



	/********************************************
	4. Установка связи*/

	connect(
		my_sock, //сокет
		(sockaddr*)&dest_addr, //dest_addr -указатель на структуру
		sizeof(dest_addr) //размер
	);

	// WAVEFORMATEX структура
	WAVEFORMATEX wfx = {};
	wfx.wFormatTag = WAVE_FORMAT_PCM;       // PCM (стандартный формат)
	wfx.nChannels = 2;                      // 2 канала
	wfx.nSamplesPerSec = 44100;             // Сэмплрейт 44100 Hz
	wfx.wBitsPerSample = 16;                // 16 bit
											// Другие настройки:
	wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;



	// Открываем 'waveIn' для записи
	HWAVEIN wi;
	waveInOpen(&wi,            // наполняем 'wi' handle
		WAVE_MAPPER,    // дефолтный девайс
		&wfx,           // говорю формат
		NULL, NULL,      // нам не нужен callback
		CALLBACK_NULL | WAVE_FORMAT_DIRECT
	);

	// Наш девайс готов для записи, даём ему буферы для записи
	char buffers[2][44100 * 2 / 2];    // 2 буффера
	WAVEHDR headers[2] = { {},{} };           // инициализирую 2 буфера в структуре на 0
	for (int i = 0; i < 2; ++i)
	{
		headers[i].lpData = buffers[i];             // присваиваю значиния с buffers в headers 
		headers[i].dwBufferLength = 44100 * 2 / 4;      // размер этих буферов (подобрал максимально оптимальный без потерь, хуй знает как рабоатет)


														// Готовим каждый header
		waveInPrepareHeader(wi, &headers[i], sizeof(headers[i]));

		// Добавляем в очередь
		//  Как только начнеться запись звука, так сразу буфера будут заполняться по =очереди=
		waveInAddBuffer(wi, &headers[i], sizeof(headers[i]));
	}


	std::cout << "Poczatek zwiazku" << std::endl;

	// Стартуем запись
	waveInStart(wi);


	//Теперь записаный звук будет поступать в буферы по мере заполнения.
	//Как только заполнятся 1 - данные записываються во 2, а с 1го отсылаються собеседнику.
	//Далее буферы будут очищены и снова добавлены в очередь и так до посинения... 

	char c_data_ot_s[44100 * 2 / 4];// Сетевой буфер 

									// Структура для воспроизведения звука

	HWAVEOUT hWaveOut;
	WAVEHDR WaveOutHdr;

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0L, 0L, WAVE_FORMAT_DIRECT);

	WaveOutHdr.lpData = c_data_ot_s;
	WaveOutHdr.dwBufferLength = 44100 * 2 / 5;
	WaveOutHdr.dwBytesRecorded = 0;
	WaveOutHdr.dwFlags = 0L;


	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))  // Пока не тыкнешь на esc
	{
		for (auto& h : headers)      // Проверка каждого header
		{
			if (h.dwFlags & WHDR_DONE)           // Готов ли он? (полный ли?)
			{

				//чтение
				recv(
					my_sock,//сокет
					c_data_ot_s,//буфер куда поподут данные
					44100 * 2 / 4,//размер буфера чтения - сколько считать
					0//делаем ее аналогичной как функция read
				);

				//отправка

				send(//отправка
					my_sock,//сокет
					h.lpData,//данные
					h.dwBufferLength,
					0//делаем ее аналогичной как функция write		
				);

				h.dwFlags = 0;          // как заполнилось - значит убираем флаг готовности
				h.dwBytesRecorded = 0;  // ставим что записано 0 байт

										// заново добавляем  
				waveInPrepareHeader(wi, &h, sizeof(h));
				waveInAddBuffer(wi, &h, sizeof(h));


				waveOutPrepareHeader(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));

				waveOutSetVolume(hWaveOut, 0xFFFFFFFF);

				waveOutWrite(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));

			}

		}
	}


	/********************************************
	6. Закрытие сокета*/
	closesocket(my_sock);//закрытие
	WSACleanup();// выгружаем WinSock

	waveInStop(wi);//закрываем и выгружаем средство записи
	for (auto& h : headers)
	{
		waveInUnprepareHeader(wi, &h, sizeof(h));
	}
	waveInClose(wi);
	waveOutClose(hWaveOut);

	system("pause");//ждем нажатие клавиши :)


}

/*/////////////////////////////////////////////////////////////////////////////////////////////////*/

void credits()
{
	short int i;
	for (i = 0; i < 5; i++) {
		printf("..............................................\n\n");
		Sleep(100);
	}

	printf(".........=Created by Vsevolod Panasov=........\n\n");

	for (i = 0; i < 5; i++) {
		printf("..............................................\n\n");
		Sleep(100);
	}
	for (i = 0; i < 7; i++) {
		printf("\n");
	}
	system("pause");
	system("cls");

}

/*/////////////////////////////////////////////////////////////////////////////////////////////////*/

void main() {
	char a;
start:
	printf("Witam w projektu Vano v.01!\n---Zeby utworzyc server nacisni = 1 =\n---Zeby podlaczyc do servera nacisni = 2 =\n---Credits = 3 =\n");
	cin >> a;

	switch ((int)a-48) {
	case 1:
		server();
		break;
	case 2:
		klient();
		break;
	case 3:
		credits();
		goto start;
		break;
	default:
		system("cls");
		goto start;
	}

}