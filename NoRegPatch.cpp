/* No-Registry Patch for Fallout New Vegas

   Copyright (C) 2021 TANGaming <https://github.com/TAN-Gaming>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE. */

#include <string>

#include <string.h>
#include <wchar.h>
#include <stdlib.h>

#include <windows.h>
#include <winreg.h>

#include <MinHook.h>

std::wstring NoRegPatch_GetGameDir()
{
    static std::wstring path_str;

    if (path_str.length() > 0)
    {
        return path_str;
    }

    size_t path_len = (size_t)(GetCurrentDirectoryW(0, NULL) - 1);
    wchar_t *path = (wchar_t*)malloc((path_len + 1) * sizeof(wchar_t));

    GetCurrentDirectoryW((DWORD)(path_len + 1), path);

    path_str = std::wstring(path, path_len);
    path_str.push_back('\\'); // Add '\' after the path (Fallout NV requires it)

    free(path);

    return path_str;
}

typedef LONG (WINAPI *PFN_RegOpenKeyExW)(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);
typedef LONG (WINAPI *PFN_RegQueryValueExW)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef LONG (WINAPI *PFN_RegCloseKey)(HKEY);

/* For calling original functions */
PFN_RegOpenKeyExW Real_RegOpenKeyExW = NULL;
PFN_RegQueryValueExW Real_RegQueryValueExW = NULL;
PFN_RegCloseKey Real_RegCloseKey = NULL;

static bool bUseHooked = false;

/* Hooked RegOpenKeyExW */
LONG WINAPI Hooked_RegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    if (lstrcmpiW(lpSubKey, L"Software\\Bethesda Softworks\\FalloutNV") == 0)
    {
        bUseHooked = true;

        return ERROR_SUCCESS;
    }

    return Real_RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}

/* Hooked RegQueryValueExW */
LONG WINAPI Hooked_RegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
    if (bUseHooked)
    {
        if (wcscmp(lpValueName, L"Installed Path") == 0)
        {
            if (lpcbData != NULL)
            {
                std::wstring gameDir = NoRegPatch_GetGameDir();

                size_t len = gameDir.length();
                size_t dataSize = (len + 1) * sizeof(wchar_t); // Required data size in bytes

                DWORD cbData = *lpcbData;
                DWORD dwDataSize = (DWORD)dataSize;

                if (lpData != NULL)
                {
                    if (cbData < dwDataSize)
                    {
                        memset(lpcbData, 0, sizeof(dwDataSize));
                        memcpy(lpcbData, &dwDataSize, sizeof(dwDataSize));

                        return ERROR_MORE_DATA;
                    }

                    memset(lpData, 0, (size_t)cbData);
                    memcpy(lpData, gameDir.c_str(), len * sizeof(wchar_t));

                    lpData[len] = L'\0'; // Null-terminated
                }

                else
                {
                    memset(lpcbData, 0, sizeof(dwDataSize));
                    memcpy(lpcbData, &dwDataSize, sizeof(dwDataSize));
                }
            }

            return ERROR_SUCCESS;
        }
    }

    return Real_RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

/* Hooked RegCloseKey */
LONG WINAPI Hooked_RegCloseKey(HKEY hKey)
{
    bUseHooked = false;

    return Real_RegCloseKey(hKey);
}

/* Hook WinAPI functions. */
void NoRegPatch_HookAPIs()
{
    MH_CreateHook((LPVOID*)(&RegOpenKeyExW), (LPVOID*)(&Hooked_RegOpenKeyExW), (LPVOID*)(&Real_RegOpenKeyExW));
    MH_CreateHook((LPVOID*)(&RegQueryValueExW), (LPVOID*)(&Hooked_RegQueryValueExW), (LPVOID*)(&Real_RegQueryValueExW));
    MH_CreateHook((LPVOID*)(&RegCloseKey), (LPVOID*)(&Hooked_RegCloseKey), (LPVOID*)(&Real_RegCloseKey));

    MH_EnableHook(MH_ALL_HOOKS);
}

/* Unhook WinAPI functions. */
void NoRegPatch_UnhookAPIs()
{
    MH_DisableHook(MH_ALL_HOOKS);
}

void NoRegPatch_Init()
{
    /* Initialize MinHook. */
    if (MH_Initialize() != MH_OK)
    {
        MessageBoxA(NULL, "Failed to initialize MinHook.", "Error", MB_ICONERROR | MB_OK);
        TerminateProcess(GetCurrentProcess(), 0);
    }

    NoRegPatch_HookAPIs();
}

void NoRegPatch_Uninit()
{
    NoRegPatch_UnhookAPIs();

    /* Uninitialize MinHook. */
    MH_Uninitialize();
}

/* DLL entry point */
BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
	{
        case DLL_PROCESS_ATTACH:
            NoRegPatch_Init();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            NoRegPatch_Uninit();
            break;
	}

	return TRUE;
}
