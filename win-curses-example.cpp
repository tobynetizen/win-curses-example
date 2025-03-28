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

#define BS   0x08
#define CR   0x0D
#define SP   0x20
#define QUO  0x27
#define DQUO 0x22

using namespace std::literals::chrono_literals;

static unsigned int cwWidth  = 120;
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
	HANDLE ConsoleInput  = ECH(GetStdHandle(STD_INPUT_HANDLE));

	CONSOLE_SCREEN_BUFFER_INFO csbInfo;
	GetConsoleScreenBufferInfo(ConsoleOutput, &csbInfo);
	short cwWidth  = csbInfo.srWindow.Right - csbInfo.srWindow.Left + 1;
	short cwHeight = csbInfo.srWindow.Bottom - csbInfo.srWindow.Top + 1;
	short sbWidth  = csbInfo.dwSize.X;
	short sbHeight = csbInfo.dwSize.Y;
	SetConsoleScreenBufferSize(ConsoleOutput, { sbWidth, cwHeight });

	CONSOLE_CURSOR_INFO csrInfo;
	csrInfo.dwSize   = 1;
	csrInfo.bVisible = FALSE;
	
	SetConsoleCursorInfo(ConsoleOutput, &csrInfo);
	SetConsoleMode(ConsoleInput, ENABLE_EXTENDED_FLAGS);
	SetConsoleTitleA(" ");

	DWORD dwBytesWritten = 0;

	std::array<std::wstring, 6> O_LightSymbols = { L"┌", L"└", L"┐", L"┘", L"─", L"│" };
	std::array<std::wstring, 6> O_HeavySymbols = { L"┏", L"┗", L"┓", L"┛", L"━", L"┃" };
	std::array<std::wstring, 4> I_LightSymbols = { L"├", L"┤", L"┬", L"┴"             };
	std::array<std::wstring, 4> I_HeavySymbols = { L"┣", L"┫", L"┳", L"┻"             };
	std::array<std::wstring, 2> CR_Symbols     = { L"┼", L"╋"                         };

	WriteToDisplayBuffer(ConsoleDisplay, 0, 0, O_HeavySymbols[0], O_HeavySymbols[0].size() + 1);
	for (int xt = 1; xt < cwWidth - 2; xt++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, xt, 0, O_HeavySymbols[4], O_HeavySymbols[4].size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, cwWidth - 2, 0, O_HeavySymbols[2], O_HeavySymbols[2].size() + 1);
	for (int yl = 1; yl < cwHeight - 1; yl++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, 0, yl, O_HeavySymbols[5], O_HeavySymbols[5].size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, 0, cwHeight - 1, O_HeavySymbols[1], O_HeavySymbols[1].size() + 1);
	for (int xb = 1; xb < cwWidth - 2; xb++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, xb, cwHeight - 1, O_HeavySymbols[4], O_HeavySymbols[4].size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, cwWidth - 2, cwHeight - 1, O_HeavySymbols[3], O_HeavySymbols[3].size() + 1);
	for (int yr = 1; yr < cwHeight - 1; yr++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, cwWidth - 2, yr, O_HeavySymbols[5], O_HeavySymbols[5].size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, 0, cwHeight - 3, I_HeavySymbols[0], I_HeavySymbols[0].size() + 1);
	for (int xb = 1; xb < cwWidth - 2; xb++)
	{
		WriteToDisplayBuffer(ConsoleDisplay, xb, cwHeight - 3, O_HeavySymbols[4], O_HeavySymbols[4].size() + 1);
	}
	WriteToDisplayBuffer(ConsoleDisplay, cwWidth - 2, cwHeight - 3, I_HeavySymbols[1], I_HeavySymbols[1].size() + 1);
	RenderDisplay(ConsoleOutput, ConsoleDisplay, &dwBytesWritten);

	signed char c;
	std::stringstream ssInputData;
	bool ControlBlock = true;
	unsigned int OutputY = 1;
	unsigned int OutputX = 2;

	for (;;)
	{
		std::thread InputHandler([&]() {
			while (ControlBlock == true)
			{
				c = _getch();
				if ((int)c == (int)CR)
				{
					ControlBlock = false;
					break;
				}
				if ((int)c == (int)BS)
				{
					std::string s = ssInputData.str();
					s.erase(s.size() - 1, s.size());
					ssInputData.str(std::string());
					ssInputData << s;
				}
				ssInputData << c;
				wchar_t wchInputChar = (wchar_t)c;
				if (OutputX < (cwWidth - 3))
				{
					WriteToDisplayBuffer(ConsoleDisplay, OutputX, cwHeight - 2, wchInputChar);
					RenderDisplay(ConsoleOutput, ConsoleDisplay, &dwBytesWritten);
					OutputX++;
				}
				else
				{
					OutputX = 2;
					ClearInputDisplay(ConsoleOutput, ConsoleDisplay, &dwBytesWritten);
				}
			}
			});

		InputHandler.join();
		ssInputData.str(std::string());
		ControlBlock = true;
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
	std::wstring Empty = L"                                                                                                                   ";
	for (int v = 1; v < 27; v++)
	{
		WriteToDisplayBuffer(displayBuffer, 1, v, Empty, Empty.size() + 1);
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
