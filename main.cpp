#include <stdio.h>

#define STRICT
#define NOMINMAX

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <commctrl.h>
#include <shlwapi.h>

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <tchar.h>

#pragma comment(lib, "comctl32.lib")
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib") 

#include "WairCursor.h"
#include "winutil.h"

#include "image.h"

HINSTANCE g_hinst;                          /* This application's HINSTANCE */

HBITMAP g_hBMP;
std::vector<uint8_t> g_bmiBuff(sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256);
//BITMAPINFO g_bmi;
void* g_pBits;
HDC g_hMemDC;

std::wstring g_dir;
std::vector<std::wstring> g_filenames;

struct CacheEntry
{
	std::wstring filename;
    std::shared_ptr<Image> image;
};
std::vector<CacheEntry> g_caches;

int g_indexToDisplay;

#define WINDOW_CLASS_NAME "siv"
#define APP_NAME "Sequential Images Viewer"

/*
 *  OnSize
 *      If we have an inner child, resize it to fit.
 */
void
OnSize(HWND hwnd, UINT state, int cx, int cy)
{
}

/*
 *  OnCreate
 *      Applications will typically override this and maybe even
 *      create a child window.
 */
BOOL
OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	DragAcceptFiles(hwnd, TRUE);

	BITMAPINFO* pBMI = (BITMAPINFO*) &g_bmiBuff[0];
	BITMAPINFO& bmi = *pBMI;
	
	int width = 1920;
	int height = 1080;
	int bitsPerPixel = 32;

	g_hBMP = CreateDIB(width, -height, bitsPerPixel, bmi, g_pBits);
	HDC hWndDC = ::GetDC(hwnd);
	g_hMemDC = ::CreateCompatibleDC(hWndDC);
	::SetMapMode(g_hMemDC, ::GetMapMode(hWndDC));
	::ReleaseDC(hwnd, hWndDC);
	::SelectObject(g_hMemDC, g_hBMP);

    return TRUE;
}

/*
 *  OnDestroy
 *      Post a quit message because our application is over when the
 *      user closes this window.
 */
void
OnDestroy(HWND hwnd)
{
	::DeleteDC(g_hMemDC);
	::DeleteObject(g_hBMP);

    PostQuitMessage(0);
}

/*
 *  PaintContent
 *      Interesting things will be painted here eventually.
 */
void
PaintContent(HWND hwnd, PAINTSTRUCT *pps)
{
	RECT rec = pps->rcPaint;
	::BitBlt(
		pps->hdc,
		rec.left,
		rec.top,
		rec.right - rec.left,
		rec.bottom - rec.top,
		g_hMemDC,
		rec.left,
		rec.top,
		SRCCOPY);

}

/*
 *  OnPaint
 *      Paint the content as part of the paint cycle.
 */
void
OnPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    BeginPaint(hwnd, &ps);
    PaintContent(hwnd, &ps);
    EndPaint(hwnd, &ps);
}

/*
 *  OnPrintClient
 *      Paint the content as requested by USER.
 */
void
OnPrintClient(HWND hwnd, HDC hdc)
{
    PAINTSTRUCT ps;
    ps.hdc = hdc;
    GetClientRect(hwnd, &ps.rcPaint);
    PaintContent(hwnd, &ps);

}

HWND CreateFullscreenWindow(HWND hwnd)
{
	HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hmon, &mi)) {
		return NULL;
	}
	return CreateWindow(
		TEXT("static"),
		TEXT("something interesting might go here"),
		WS_POPUP | WS_VISIBLE,
		mi.rcMonitor.left,
		mi.rcMonitor.top,
		mi.rcMonitor.right - mi.rcMonitor.left,
		mi.rcMonitor.bottom - mi.rcMonitor.top,
		hwnd, NULL, g_hinst, 0
		);
}

static
bool loadImage(const TCHAR* path)
{
	FILE* f = _wfopen(path, _T("rb"));
	if (f) {
		auto& cache = g_caches[g_indexToDisplay];
		cache.image = std::shared_ptr<Image>(new Image());
		auto& image = *cache.image;
		image.load(f);
		fclose(f);
		cache.filename = path;
	}
    return true;
}

void display(HWND hWnd)
{
	if (!g_filenames.size() || g_indexToDisplay >= g_filenames.size()) {
		return;
	}
	if (g_caches.size() != g_filenames.size()) {
		g_caches.resize(g_filenames.size());
	}

	std::vector<TCHAR> filename(1024);
	TCHAR* pf = &filename[0];
	pf[0] = 0;
	PathAppend(pf, g_dir.c_str());
	PathAppend(pf, g_filenames[g_indexToDisplay].c_str());
	if (!PathFileExists(pf)) {
		return;
	}

	CacheEntry& cache = g_caches[g_indexToDisplay];
	if (cache.filename != pf) {
		if (!loadImage(pf)) {
			return;
		}
	}

	if (!cache.image) {
		return;
	}

	auto& image = *cache.image;

	TCHAR title[256];
	_stprintf_s(title, _T("%s - (%u/%u) %s"), TEXT(APP_NAME), g_indexToDisplay + 1, g_filenames.size(), pf);
	::SetWindowText(hWnd, title);

	BITMAPINFO* pBMI = (BITMAPINFO*) &g_bmiBuff[0];
	const BITMAPINFOHEADER& hdr = pBMI->bmiHeader;
	if (image.width > hdr.biWidth && image.height > hdr.biHeight) {
        fprintf(stderr,
				"Image %d x %d is bigger than internal bitmap %d x %d.\n",
				image.width, image.height, hdr.biWidth, hdr.biHeight);
        return;
	}

	const unsigned char* src = image.data;
	int ncomponents = image.ncomponents;
	unsigned char* dst = (unsigned char*) g_pBits;
	for (int y=0; y<image.height; ++y) {
		for (int x=0; x<image.width; ++x) {
			dst[x*4+0] = src[x*ncomponents+2];
			dst[x*4+1] = src[x*ncomponents+1];
			dst[x*4+2] = src[x*ncomponents+0];
		}
		dst += hdr.biWidth * 4;
		src += image.width * ncomponents;
	}

	RECT rec;
	::GetClientRect(hWnd, &rec);
	::InvalidateRect(hWnd, &rec, TRUE);
	::UpdateWindow(hWnd);
}

void OnChar(HWND hwnd, TCHAR ch, int cRepeat)
{
	switch (ch) {
	case ' ':
		CreateFullscreenWindow(hwnd);
		break;
	}
}

void OnKeyDown(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
	if (!g_filenames.size()) {
		return;
	}

	switch (vk) {
	case VK_RIGHT:
		g_indexToDisplay = std::min(g_indexToDisplay + 1, (int)g_filenames.size() - 1);
		display(hwnd);
		break;
	case VK_LEFT:
		g_indexToDisplay = std::max(g_indexToDisplay - 1, 0);
		display(hwnd);
		break;
	case VK_HOME:
		g_indexToDisplay = 0;
		display(hwnd);
		break;
	case VK_END:
		g_indexToDisplay = (int)g_filenames.size() - 1;
		display(hwnd);
		break;
	}
}

bool findFiles(const wchar_t* dir, std::vector<std::wstring>& filenames)
{
	WIN32_FIND_DATA ffd;
	FINDEX_INFO_LEVELS level = FindExInfoBasic; // FindExInfoStandard;
	HANDLE handle = FindFirstFileEx(dir,
									level,
									&ffd,
									FindExSearchNameMatch,
									NULL,
									FIND_FIRST_EX_LARGE_FETCH);
	if (handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	BOOL ret;
	do {
		if (_tcscmp(ffd.cFileName, _T(".")) && _tcscmp(ffd.cFileName, _T(".."))) {
			filenames.emplace_back(ffd.cFileName);
		}
		ret = FindNextFile(handle, &ffd);
	} while (ret);

	return ret == ERROR_NO_MORE_FILES || ret == ERROR_SUCCESS;
}

void OnDropFiles(HWND hWnd, HDROP hDrop)
{
	UINT nchars = DragQueryFile(hDrop, 0, NULL, 0);
	std::unique_ptr<TCHAR[]> buff(new TCHAR[nchars * 2 + 1]);	// UTF-16
	nchars = DragQueryFile(hDrop, 0, buff.get(), nchars + 1);
	if (PathIsDirectory(buff.get())) {
		wchar_t* dir = buff.get();
		g_dir = dir;
		PathAppend(dir, TEXT("\\*.*"));
		g_filenames.clear();
		WaitCursor waiter;
		if (findFiles(dir, g_filenames)) {
			std::sort(g_filenames.begin(), g_filenames.end());
			g_indexToDisplay = 0;
			display(hWnd);
		}
	}
	DragFinish(hDrop);
}

/*
 *  Window procedure
 */
LRESULT CALLBACK
WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uiMsg) {

    HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
    HANDLE_MSG(hwnd, WM_SIZE, OnSize);
    HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
    HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
    HANDLE_MSG(hwnd, WM_CHAR, OnChar);
    HANDLE_MSG(hwnd, WM_KEYDOWN, OnKeyDown);
	HANDLE_MSG(hwnd, WM_DROPFILES, OnDropFiles);
    case WM_PRINTCLIENT: OnPrintClient(hwnd, (HDC)wParam); return 0;
	case WM_ERASEBKGND: return TRUE;
    }

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

BOOL
InitApp(void)
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = g_hinst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = 0; //(HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT(WINDOW_CLASS_NAME);

    if (!RegisterClass(&wc)) return FALSE;

    InitCommonControls();               /* In case we use a common control */

    return TRUE;
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hinstPrev,
                   LPSTR lpCmdLine, int nShowCmd)
{
    MSG msg;
    HWND hwnd;

    g_hinst = hinst;

    if (!InitApp()) return 0;

    if (SUCCEEDED(CoInitialize(NULL))) {/* In case we use COM */

        hwnd = CreateWindow(
            TEXT(WINDOW_CLASS_NAME),		/* Class Name */
            TEXT(APP_NAME),                /* Title */
            WS_OVERLAPPEDWINDOW,            /* Style */
            CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
            CW_USEDEFAULT, CW_USEDEFAULT,   /* Size */
            NULL,                           /* Parent */
            NULL,                           /* No menu */
            hinst,                          /* Instance */
            0);                             /* No special parameters */

        ShowWindow(hwnd, nShowCmd);

        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        CoUninitialize();
    }

    return 0;
}
