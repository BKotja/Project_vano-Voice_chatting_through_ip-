#include "stdafx.h"

#pragma comment(lib, "ws2_32")//�������� ���������� ��� �������
#include <winsock2.h>

#pragma comment(lib,"winmm.lib")
#include <Windows.h>
#include <mmsystem.h>
#include <fstream>
#include <iostream>


using namespace std;

int i_error = 0;//����� ������� ��� ������

				/*/////////////////////////////////////////////////////////////////////////////////////////////////*/

void server() {

	std::cout << "Zaszynam tworzyc server" << endl << endl;
	/********************************************
	0. ��������� ���������� Ws2_32.dll*/
	//� ������ ������ WSAStartup ���������� 0; ����� - ��� ������. 
	WSADATA wsaData;//���������� ����������
	i_error = WSAStartup(MAKEWORD(2, 2), &wsaData);//�����������
	if (i_error)//�������� �� ���������� ���������
	{
		std::cout << "0)  Fatalny blad" << endl;
	}
	else { std::cout << "0)   Biblioteka Ws2_32.dll jest podlaczona" << endl; }

	/********************************************
	1. �������� ���������� ������*/
	//� ������ ������ socket ���������� ����� �����-�����������. ����� -1
	SOCKET SSock;//������� ��������
	SSock = socket(
		AF_INET,//������������ ��� �������� ��������-����������
		SOCK_STREAM,//���������
		IPPROTO_TCP//��� - ��� ���������
	);
	if (SSock == -1) {
		std::cout << "1)   Bland tworzenia socketu" << endl;
	}
	else { std::cout << "1)   Socket jest stworzony " << endl; }

	/********************************************
	2. �������� � ��������� ������*/
	//���������� ����� ������� � IP-������� ����������.
	struct sockaddr_in addr;//������� ��������(addr), ��������� sockaddr_in
	addr.sin_family = AF_INET;//��� �������� ��������-����������
							  //����� ������� ����������� ���������� ���� short [��� htons()] 
							  //� long[��� htonl()], � ������� ��������� ��� ����.
	addr.sin_port = htons(3000);//����
	addr.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY ������ �������� �� ������, � ����� ip

	i_error = 0;	//������� ���������� ��� ������
	i_error = bind(			//� ������ ������ bind ���������� 0, � ��������� ������ - "-1"
		SSock,
		(SOCKADDR*)&addr,	//addr - ��������� �� ��������� sockaddr_in
		sizeof(addr)		//������ ���������.
	);
	if (i_error == -1) {
		std::cout << "2)   Blad powiazania socketa z ip adresem komputera" << endl;
	}
	else { std::cout << "2)   Socket jest powiazany z ip adresem komputera" << endl; }



	/********************************************
	3. ������������� �������� ��������������� �����*/
	//����������-������ ����� ������� �� ����� ������������� �� �������� ����
	std::cout << "3)   Inicializacja porta" << endl;
	listen(
		SSock,
		3 //������� �������� ����� ������ � �������
	);

	/********************************************
	4. ����������� ������� - ��������� ����������� ����������� ����������*/
	//������ ������� ����� ����� � ��������� ��� � �����������, 
	//������������� '���������� ������'
	SOCKET ClientSock;//������� �������� ������ ��� ��������������� �������
	struct sockaddr_in addr_klienta;//������� �������� ��������� ��� ��������������� �������
									//���� ������� accept ����������� ������� - ���������� ����� ����� 
									//��� �������������� ����������, � ��������� ������  INVALID_SOCKET
	int size_client_addr = sizeof(addr_klienta);//������ ��������� ��� �������
	ClientSock = accept(
		SSock,// �����
		(sockaddr*)&addr_klienta, //����� ��������� (!)sockaddr_in 
								  //���� ����� �������� ���������� � �������.
		&size_client_addr // ������ ������
	);
	/*����� Socket.Accept ��������� �� ������� ������ ������ �
	���������� ����� ������ Socket,
	������� ����� ������������ ��� ����������������� �������������� � ��������.
	*/
	/*accept ��� � ��� ������� ����� � addr ����� ���������� ���������� � �������.
	��� ����� ����� ������:
	addr.sin_port � ���� � �������� ���� �����������
	addr.sin_addr � ip ������� � �������� �������. ��� �������������� � ��������� ������ ������������ ������� inet_ntoa. �.�. ip:pchar; ip:=inet_ntoa(addr.sin_addr);
	*/

	if (ClientSock == INVALID_SOCKET) std::cout << "4)   Bland palaczenia klienta" << endl;
	else std::cout << "4)   Klient jest polaczony" << endl;

	///////////////////////////////////////////////////////////////////////////////////////////////

	// WAVEFORMATEX ���������
	WAVEFORMATEX wfx = {};
	wfx.wFormatTag = WAVE_FORMAT_PCM;       // PCM (����������� ������)
	wfx.nChannels = 2;                      // 2 ������
	wfx.nSamplesPerSec = 44100;             // ��������� 44100 Hz
	wfx.wBitsPerSample = 16;                // 16 bit
											// ������ ���������:
	wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;


	// ��������� 'waveIn' ��� ������
	HWAVEIN wi;
	waveInOpen(&wi,            // ��������� 'wi' handle
		WAVE_MAPPER,    // ��������� ������
		&wfx,           // ��������� � ��������� wfx
		NULL, NULL,      // ��� �� ����� callback
		CALLBACK_NULL | WAVE_FORMAT_DIRECT
	);

	// ��� ������ ����� ��� ������, ��� ��� ������ ��� ������
	char buffers[2][44100 * 2 / 2];    // 2 �������
	WAVEHDR headers[2] = { {},{} };           // ������������� 2 ������ � ��������� �� 0
	for (int i = 0; i < 2; ++i)
	{
		headers[i].lpData = buffers[i];             // ���������� �������� � buffers � headers 
		headers[i].dwBufferLength = 44100 * 2 / 4;      // ������ ���� ������� (�������� ����������� ����������� ��� ������, ��� ����� ��� ��������)


														// ������� ������ header
		waveInPrepareHeader(wi, &headers[i], sizeof(headers[i]));

		// ��������� � �������
		//  ��� ������ ��������� ������ �����, ��� ����� ������ ����� ����������� �� =�������=
		waveInAddBuffer(wi, &headers[i], sizeof(headers[i]));
	}


	std::cout << "Poczatek polaczenia" << std::endl;

	// �������� ������
	waveInStart(wi);

	//������ ��������� ���� ����� ��������� � ������ �� ���� ����������.
	//��� ������ ���������� 1 - ������ ������������� �� 2, � � 1�� ����������� �����������.
	//����� ������ ����� ������� � ����� ��������� � ������� � ��� �� ���������...

	char c_data_ot_k[44100 * 2 / 4]; // ������� ����� 

									 // ��������� ��� ��������������� �����
	HWAVEOUT hWaveOut;
	WAVEHDR WaveOutHdr;

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0L, 0L, WAVE_FORMAT_DIRECT);

	WaveOutHdr.lpData = c_data_ot_k;
	WaveOutHdr.dwBufferLength = 44100 * 2 / 5;
	WaveOutHdr.dwBytesRecorded = 0;
	WaveOutHdr.dwFlags = 0L;


	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))  // ���� �� ������ esc
	{
		for (auto &h : headers)      // �������� ������� header
		{
			if (h.dwFlags & WHDR_DONE)           // ����� �� ��? (������ ��?)
			{
				// ���� �� - �� ����� ��������



				//5. �������� ������


				send(//��������
					ClientSock,//�����
					h.lpData,//������
					h.dwBufferLength,
					0//������ �� ����������� ��� ������� write
				);

				recv(//������
					ClientSock,//�����
					c_data_ot_k,//����� ���� ������� ������
					44100 * 2 / 4,//������ ������ ������ - ������� �������
					0//������ �� ����������� ��� ������� read
				);



				// ��������� ����� � ������� (�������)
				h.dwFlags = 0;          // ��� ����������� - ������ ������� ���� ����������
				h.dwBytesRecorded = 0;  // ������ ��� �������� 0 ����

										// ������ ���������  
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
	6. �������� ������*/
	closesocket(SSock);//��������
	WSACleanup();// ��������� WinSock

	system("pause");//���� ������� ������� :)
}


/*/////////////////////////////////////////////////////////////////////////////////////////////////*/


void klient() {

	system("chcp 1251 > text");//������ �������� ������� ���������
							   //--------------
	std::cout << "Zacznam tworzyc klient" << endl << endl;
	/********************************************
	0. ��������� ���������� Ws2_32.dll*/
	//� ������ ������ WSAStartup ���������� 0; ����� - ��� ������. 
	WSADATA wsaData;//���������� ����������
	i_error = WSAStartup(MAKEWORD(2, 2), &wsaData);//�����������
	if (i_error)//�������� �� ���������� ���������
	{
		std::cout << "0)   Fatalny blad" << endl;
	}
	else { std::cout << "0)   Biblioteka Ws2_32.dll jest podlaczona" << endl; }

	/********************************************
	1. �������� ���������� ������*/
	//� ������ ������ socket ���������� ����� �����-�����������. ����� -1
	SOCKET my_sock;//������� ��������
	my_sock = socket(
		AF_INET,//������������ ��� �������� ��������-����������
		SOCK_STREAM,//���������
		IPPROTO_TCP//��� - ��� ���������
	);
	if (my_sock == -1) {
		std::cout << "1)   Bland tworzenia socketu" << endl;
	}
	else { std::cout << "1)   Socket jest stworzony" << endl; }

	/********************************************
	2. �������� � ��������� ������*/

	struct sockaddr_in  dest_addr;//������� ��������(dest_addr), ��������� sockaddr_in
	dest_addr.sin_family = AF_INET;//��� �������� ��������-����������
	dest_addr.sin_port = htons(3000); //����

									  // �������������� IP -������. servIP - ����� �������

	printf("2)   Podaj ip servera:\n");
	char servIP[20];		//������ ip
	scanf("%s", servIP);
	dest_addr.sin_addr.s_addr = inet_addr(servIP);



	/********************************************
	4. ��������� �����*/

	connect(
		my_sock, //�����
		(sockaddr*)&dest_addr, //dest_addr -��������� �� ���������
		sizeof(dest_addr) //������
	);

	// WAVEFORMATEX ���������
	WAVEFORMATEX wfx = {};
	wfx.wFormatTag = WAVE_FORMAT_PCM;       // PCM (����������� ������)
	wfx.nChannels = 2;                      // 2 ������
	wfx.nSamplesPerSec = 44100;             // ��������� 44100 Hz
	wfx.wBitsPerSample = 16;                // 16 bit
											// ������ ���������:
	wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
	wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;



	// ��������� 'waveIn' ��� ������
	HWAVEIN wi;
	waveInOpen(&wi,            // ��������� 'wi' handle
		WAVE_MAPPER,    // ��������� ������
		&wfx,           // ������ ������
		NULL, NULL,      // ��� �� ����� callback
		CALLBACK_NULL | WAVE_FORMAT_DIRECT
	);

	// ��� ������ ����� ��� ������, ��� ��� ������ ��� ������
	char buffers[2][44100 * 2 / 2];    // 2 �������
	WAVEHDR headers[2] = { {},{} };           // ������������� 2 ������ � ��������� �� 0
	for (int i = 0; i < 2; ++i)
	{
		headers[i].lpData = buffers[i];             // ���������� �������� � buffers � headers 
		headers[i].dwBufferLength = 44100 * 2 / 4;      // ������ ���� ������� (�������� ����������� ����������� ��� ������, ��� ����� ��� ��������)


														// ������� ������ header
		waveInPrepareHeader(wi, &headers[i], sizeof(headers[i]));

		// ��������� � �������
		//  ��� ������ ��������� ������ �����, ��� ����� ������ ����� ����������� �� =�������=
		waveInAddBuffer(wi, &headers[i], sizeof(headers[i]));
	}


	std::cout << "Poczatek zwiazku" << std::endl;

	// �������� ������
	waveInStart(wi);


	//������ ��������� ���� ����� ��������� � ������ �� ���� ����������.
	//��� ������ ���������� 1 - ������ ������������� �� 2, � � 1�� ����������� �����������.
	//����� ������ ����� ������� � ����� ��������� � ������� � ��� �� ���������... 

	char c_data_ot_s[44100 * 2 / 4];// ������� ����� 

									// ��������� ��� ��������������� �����

	HWAVEOUT hWaveOut;
	WAVEHDR WaveOutHdr;

	waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0L, 0L, WAVE_FORMAT_DIRECT);

	WaveOutHdr.lpData = c_data_ot_s;
	WaveOutHdr.dwBufferLength = 44100 * 2 / 5;
	WaveOutHdr.dwBytesRecorded = 0;
	WaveOutHdr.dwFlags = 0L;


	while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000))  // ���� �� ������� �� esc
	{
		for (auto& h : headers)      // �������� ������� header
		{
			if (h.dwFlags & WHDR_DONE)           // ����� �� ��? (������ ��?)
			{

				//������
				recv(
					my_sock,//�����
					c_data_ot_s,//����� ���� ������� ������
					44100 * 2 / 4,//������ ������ ������ - ������� �������
					0//������ �� ����������� ��� ������� read
				);

				//��������

				send(//��������
					my_sock,//�����
					h.lpData,//������
					h.dwBufferLength,
					0//������ �� ����������� ��� ������� write		
				);

				h.dwFlags = 0;          // ��� ����������� - ������ ������� ���� ����������
				h.dwBytesRecorded = 0;  // ������ ��� �������� 0 ����

										// ������ ���������  
				waveInPrepareHeader(wi, &h, sizeof(h));
				waveInAddBuffer(wi, &h, sizeof(h));


				waveOutPrepareHeader(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));

				waveOutSetVolume(hWaveOut, 0xFFFFFFFF);

				waveOutWrite(hWaveOut, &WaveOutHdr, sizeof(WAVEHDR));

			}

		}
	}


	/********************************************
	6. �������� ������*/
	closesocket(my_sock);//��������
	WSACleanup();// ��������� WinSock

	waveInStop(wi);//��������� � ��������� �������� ������
	for (auto& h : headers)
	{
		waveInUnprepareHeader(wi, &h, sizeof(h));
	}
	waveInClose(wi);
	waveOutClose(hWaveOut);

	system("pause");//���� ������� ������� :)


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