#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <thread>
#include <array>
#include <sstream>
#include <string>
#include <codecvt>

#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN

#define BS 0x08
#define CR 0x0D
#define SP 0x20

using namespace std::literals::chrono_literals;

static unsigned int cwWidth = 120;
static unsigned int cwHeight = 40;
static unsigned int nMaxSize = cwWidth * cwHeight;

HANDLE ECH(HANDLE consoleHandle);
void RefreshDisplay(wchar_t* displayBuffer);
void WriteToDisplayBuffer(wchar_t* displayBuffer, int x, int y, const std::wstring& text, size_t textSize);
void WriteToDisplayBuffer(wchar_t* displayBuffer, int x, int y, wchar_t text);
void RenderDisplay(HANDLE consoleOutputHandle, wchar_t* displayBuffer, LPDWORD dwBytesWritten);
void ClearOutputDisplay(HANDLE consoleOutputHandle, wchar_t* displayBuffer, LPDWORD dwBytesWritten);
void ClearInputDisplay(HANDLE consoleOutputHandle, wchar_t* displayBuffer, LPDWORD dwBytesWritten);

int main()
{
	wchar_t* ConsoleDisplay = new wchar_t[nMaxSize];
	RefreshDisplay(ConsoleDisplay);

	HANDLE ConsoleOutput = ECH(GetStdHandle(STD_OUTPUT_HANDLE));
	HANDLE ConsoleInput = ECH(GetStdHandle(STD_INPUT_HANDLE));

	CONSOLE_SCREEN_BUFFER_INFO csbInfo;
	GetConsoleScreenBufferInfo(ConsoleOutput, &csbInfo);
	short cwWidth = csbInfo.srWindow.Right - csbInfo.srWindow.Left + 1;
	short cwHeight = csbInfo.srWindow.Bottom - csbInfo.srWindow.Top + 1;
	short sbWidth = csbInfo.dwSize.X;
	short sbHeight = csbInfo.dwSize.Y;
	SetConsoleScreenBufferSize(ConsoleOutput, { sbWidth, cwHeight });

	CONSOLE_CURSOR_INFO csrInfo;
	csrInfo.dwSize = 1;
	csrInfo.bVisible = FALSE;
	
	SetConsoleCursorInfo(ConsoleOutput, &csrInfo);
	SetConsoleMode(ConsoleInput, ENABLE_EXTENDED_FLAGS);
	SetConsoleTitleA(" ");

	DWORD dwBytesWritten = 0;

	std::array<int, 6> exLightSymbolsArray = { 0x250C, 0x2514, 0x2510, 0x2518, 0x2500, 0x2502 };
	std::array<int, 6> exHeavySymbolsArray = { 0x250F, 0x2513, 0x2511, 0x251B, 0x2501, 0x2503 };
	std::array<int, 4> inLightSymbolsArray = { 0x251C, 0x2524, 0x252C, 0x2534 };
	std::array<int, 4> inHeavySymbolsArray = { 0x2520, 0x252F, 0x2530, 0x253B };
	std::array<int, 2> crSymbolsArray = { 0x253C, 0x254B };

	WriteToDisplayBuffer(ConsoleDisplay, 0, 0, static_cast<wchar_t>(exHeavySymbolsArray[0]), static_cast<wchar_t>(exHeavySymbolsArray[0]).size() + 1);
	for (int xt = 1; xt < cwWidth - 2; xt++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, xt, 0, static_cast<wchar_t>(exHeavySymbolsArray[4]), static_cast<wchar_t>(exHeavySymbolsArray[4]).size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, cwWidth - 2, 0, exHeavySymbolsArray[2], exHeavySymbolsArray[2].size() + 1);
	for (int yl = 1; yl < cwHeight - 1; yl++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, 0, yl, static_cast<wchar_t>(exHeavySymbolsArray[5]), static_cast<wchar_t>(exHeavySymbolsArray[5]).size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, 0, cwHeight - 1, static_cast<wchar_t>(exHeavySymbolsArray[1]), static_cast<wchar_t>(exHeavySymbolsArray[1]).size() + 1);
	for (int xb = 1; xb < cwWidth - 2; xb++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, xb, cwHeight - 1, static_cast<wchar_t>(exHeavySymbolsArray[4]), static_cast<wchar_t>(exHeavySymbolsArray[4]).size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, cwWidth - 2, cwHeight - 1, static_cast<wchar_t>(exHeavySymbolsArray[3]), static_cast<wchar_t>(exHeavySymbolsArray[3]).size() + 1);
	for (int yr = 1; yr < cwHeight - 1; yr++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, cwWidth - 2, yr, static_cast<wchar_t>(exHeavySymbolsArray[5]), static_cast<wchar_t>(exHeavySymbolsArray[5]).size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, 0, cwHeight - 3, static_cast<wchar_t>(inHeavySymbolsArray[0]), static_cast<wchar_t>(inHeavySymbolsArray[0]).size() + 1);
	for (int xb = 1; xb < cwWidth - 2; xb++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, xb, cwHeight - 3, static_cast<wchar_t>(exHeavySymbolsArray[4]), static_cast<wchar_t>(exHeavySymbolsArray[4]).size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, cwWidth - 2, cwHeight - 3, static_cast<wchar_t>(inHeavySymbolsArray[1]), static_cast<wchar_t>(inHeavySymbolsArray[1]).size() + 1);
	RenderDisplay(ConsoleOutput, ConsoleDisplay, &dwBytesWritten);

	signed char inputChar;
	std::stringstream inputDataStream;
	bool controlBlock = true;
	unsigned int outputY = 1;
	unsigned int outputX = 2;

	for (;;)
	{
		std::thread inputHandlerThread([&]() {
			while (controlBlock == true)
			{
				inputChar = _getch();
				if ((int)inputChar == (int)CR)
				{
					controlBlock = false;
					break;
				}
				if ((int)inputChar == (int)BS)
				{
					std::string inputDataString = inputDataStream.str();
					inputDataString.erase(inputDataString.size() - 1, inputDataString.size());
					inputDataStream.str(std::string());
					inputDataStream << inputDataString;
				}
				inputDataStream << inputChar;
				wchar_t wchInputChar = (wchar_t)inputChar;
				if (outputX < (cwWidth - 3))
				{
					WriteToDisplayBuffer(ConsoleDisplay, outputX, cwHeight - 2, wchInputChar);
					RenderDisplay(ConsoleOutput, ConsoleDisplay, &dwBytesWritten);
					outputX++;
				}
				else
				{
					outputX = 2;
					ClearInputDisplay(ConsoleOutput, ConsoleDisplay, &dwBytesWritten);
				}
			}
			});

		inputHandlerThread.join();
		inputDataStream.str(std::string());
		controlBlock = true;
	}

	ExitProcess(EXIT_SUCCESS);
}

HANDLE ECH(HANDLE consoleHandle)
{
	if (consoleHandle == INVALID_HANDLE_VALUE)
	{
		std::cerr << GetLastError() << std::endl;
		ExitProcess(EXIT_FAILURE);
	}
	else
	{
		return consoleHandle;
	}
}

void RefreshDisplay(wchar_t* displayBuffer)
{
	std::fill_n(displayBuffer, nMaxSize, L' ');
}

void WriteToDisplayBuffer(wchar_t* displayBuffer, int x, int y, const std::wstring& text, size_t textSize)
{
	swprintf(&displayBuffer[y * cwWidth + x], textSize, L"%s", text.c_str());
}

void WriteToDisplayBuffer(wchar_t* displayBuffer, int x, int y, wchar_t text)
{
	displayBuffer[y * cwWidth + x] = text;
}

void RenderDisplay(HANDLE consoleOutputHandle, wchar_t* displayBuffer, LPDWORD dwBytesWritten)
{
	WriteConsoleOutputCharacter(consoleOutputHandle, displayBuffer, nMaxSize, { 0, 0 }, dwBytesWritten);
}

void ClearOutputDisplay(HANDLE consoleOutputHandle, wchar_t* displayBuffer, LPDWORD dwBytesWritten)
{
	std::wstring emptyInlineString(115, L' ');
	for (int v = 1; v < 27; v++)
	{
		WriteToDisplayBuffer(displayBuffer, 1, v, emptyInlineString, emptyInlineString.size() + 1);
	}
	RenderDisplay(consoleOutputHandle, displayBuffer, dwBytesWritten);
}

void ClearInputDisplay(HANDLE consoleOutputHandle, wchar_t* displayBuffer, LPDWORD dwBytesWritten)
{

	for (int v = 2; v < cwWidth - 2; v++)
	{
		WriteToDisplayBuffer(displayBuffer, v, 28, (char)SP);
	}
	RenderDisplay(consoleOutputHandle, displayBuffer, dwBytesWritten);
}
