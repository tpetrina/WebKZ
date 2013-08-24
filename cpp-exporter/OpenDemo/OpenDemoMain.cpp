/*
	The Main Window

	Copyright (c) 2010, eDark
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name eDark nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.
		* The source code may not be used in any commercial applications.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL eDark BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <windows.h>
#include <wchar.h>
#include "OpenDemoMain.h"

OSVERSIONINFOEX osvi;
int g_Status;
HWND g_Hwnd;
WCHAR g_FileName[MAX_PATH];

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void PlaceWindow(RECT &rWnd);
void PaintLineGradX(HDC hdc, int x1, int x2, int y, UINT gcx, BYTE r, BYTE g, BYTE b);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *) &osvi);

	WNDCLASSEXW wc;
	const WCHAR szClassName[] = L"WindowClass";
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.style		 = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szClassName;

	if(!RegisterClassExW(&wc)) {
		MessageBoxW(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	RECT rWnd = {0, 0, WIN_CX, WIN_CY};
	AdjustWindowRectEx(&rWnd, WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE, NULL);
	PlaceWindow(rWnd);

	g_Hwnd = CreateWindowExW(
		NULL,
		szClassName,
		APP_NAME,
		WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		rWnd.left, rWnd.top, rWnd.right - rWnd.left, rWnd.bottom - rWnd.top,
		NULL, NULL, hInstance, NULL);

	if(g_Hwnd == NULL) {
		MessageBoxW(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(g_Hwnd, nCmdShow);
	UpdateWindow(g_Hwnd);

	MSG Msg;
	while(GetMessageW(&Msg, NULL, 0, 0) > 0)	{
		TranslateMessage(&Msg);
		DispatchMessageW(&Msg);
	}
	return (int)Msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT Ps;
	static HDC hdc, hdcOut;
	static WCHAR c[MAX_PATH] = L"";
	static HWND hBCheck;
	static HFONT ButtonFont, TextFont;
	static HBITMAP hbBuffer;

	switch(msg)
	{
	case WM_CREATE:
		g_Status = 101;
		hbBuffer = CreateCompatibleBitmap(GetDC(NULL), WIN_CX, WIN_CY);

		if(osvi.dwMajorVersion > 5) 
			wcscat_s(c, MAX_PATH, L"Segoe UI");
		else
			wcscat_s(c, MAX_PATH, L"Tahoma");

		ButtonFont = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, c);
		TextFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN, c);
		hBCheck = CreateWindowW(L"button", L"Check Demo", WS_CHILD | BS_PUSHBUTTON | WS_VISIBLE, 
				WIN_CX/2 - 174/2, WIN_CY - 36, 174, 25, hwnd, (HMENU)1, ((LPCREATESTRUCT) lParam)->hInstance, NULL);
		EnableWindow(hBCheck, FALSE);
		SendMessageW(hBCheck, WM_SETFONT, (WPARAM)ButtonFont, MAKELPARAM(TRUE, 0));
		break;

	case WM_PAINT:
		if(g_Status >= 100 || g_Status < 0)
			EnableWindow(hBCheck, TRUE);

		hdcOut = BeginPaint(hwnd, &Ps);
		hdc = CreateCompatibleDC(hdcOut);
		SelectObject(hdc, hbBuffer);
		BitBlt(hdc, 0, 0, WIN_CX, WIN_CY, NULL, 0, 0, WHITENESS);

		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, GetStockObject(DC_PEN));
		SelectObject(hdc, GetStockObject(DC_BRUSH));
		PaintLineGradX(hdc, 0, WIN_CX, WIN_CY - 48, 80, 0, 0, 0);
		PaintLineGradX(hdc, 0, WIN_CX, 40, 80, 0, 0, 0);

		if(g_Status > 0 && g_Status <= 100) {
			//if(g_Status < 100) {
				swprintf(c, 9, L"%d",g_Status);
				wcscat_s(c, MAX_PATH, L"%");
				TextOutW(hdc, 130, 200, c, (int)wcslen(c));
			//}
		}

		BitBlt(hdcOut, 0, 0, WIN_CX, WIN_CY, hdc, 0, 0, SRCCOPY);
		DeleteDC(hdc);
		DeleteDC(hdcOut);
		EndPaint(hwnd, &Ps);
		break;

	case WM_COMMAND:
		switch(wParam) {
		case 1:
			if(g_Status <= 0 || g_Status >= 100)
				SendMessageW(hwnd, WM_OPENFILE, 0, 0);
			break;
		}
		break;

	case WM_OPENFILE:
		g_Hwnd = hwnd;
		DWORD ret;

		do {
			GetExitCodeThread(hThread, &ret);
		} while(ret == STILL_ACTIVE);
		CloseHandle(hThread);

		hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)checkDemo, 0, 0, 0);
		EnableWindow(hBCheck, FALSE);
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void PlaceWindow(RECT &rWnd)
{
	RECT rDesktop;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rDesktop, NULL);
	SIZE sWnd = {rWnd.right - rWnd.left, rWnd.bottom - rWnd.top};
	rWnd.left = rDesktop.left + (rDesktop.right - rDesktop.left)/2 - sWnd.cx/2;
	rWnd.right = rWnd.left + sWnd.cx;
	rWnd.top = rDesktop.top + LONG(((rDesktop.bottom - rDesktop.top) - sWnd.cy)*0.4);
	rWnd.bottom = rWnd.top + sWnd.cy;
}

void PaintLineGradX(HDC hdc, int x1, int x2, int y, UINT gcx, BYTE r, BYTE g, BYTE b)
{
	MoveToEx(hdc, x1, y, NULL);
	for(UINT f = 0; f < gcx; f++) {
		SetDCPenColor(hdc, RGB(255 - f*(255 - r)/gcx, 255 - f*(255 - g)/gcx, 255 - f*(255 - b)/gcx));
		LineTo(hdc, x1 + f, y);
	}
	SetDCPenColor(hdc, RGB(r,g,b));
	LineTo(hdc, x2 - gcx, y);
	for(UINT f = gcx; f > 0; f--) {
		SetDCPenColor(hdc, RGB(255 - f*(255 - r)/gcx, 255 - f*(255 - g)/gcx, 255 - f*(255 - b)/gcx));
		LineTo(hdc, x2 - f, y);
	}
}