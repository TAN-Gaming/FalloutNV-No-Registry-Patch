/*
 * No-Registry Patch for Fallout New Vegas
 *
 * Copyright (C) 2021-2023 TANGaming <https://github.com/TAN-Gaming>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <windows.h>
#include <strsafe.h>

#include <MinHook.h>

typedef LONG (WINAPI *PFREGOPENKEYEXW)(HKEY, LPCWSTR, DWORD, REGSAM, PHKEY);
typedef LONG (WINAPI *PFREGQUERYVALUEEXW)(HKEY, LPCWSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
typedef LONG (WINAPI *PFREGCLOSEKEY)(HKEY);

/* For calling original functions */
PFREGOPENKEYEXW Real_RegOpenKeyExW = NULL;
PFREGQUERYVALUEEXW Real_RegQueryValueExW = NULL;
PFREGCLOSEKEY Real_RegCloseKey = NULL;

BOOL bUseHooked = FALSE;
LPWSTR pszGameDir = NULL;

LPWSTR NoRegPatch_GetGameDir(void)
{
    DWORD dwRet;
    LPWSTR pszPath;

    dwRet = GetCurrentDirectoryW(0, NULL);
    if (dwRet == 0)
        return NULL;

    /* NOTE: Included the length of '\' (1) */
    pszPath = HeapAlloc(GetProcessHeap(), 0, (dwRet + 1) * sizeof(WCHAR));
    if (!pszPath)
        return NULL;

    if (GetCurrentDirectoryW(dwRet, pszPath) == 0)
    {
        HeapFree(GetProcessHeap(), 0, pszPath);
        return NULL;
    }

    StringCchCatW(pszPath, dwRet + 1, L"\\");

    return pszPath;
}

/* Hooked RegOpenKeyExW */
LONG WINAPI Hooked_RegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    if (_wcsicmp(lpSubKey, L"Software\\Bethesda Softworks\\FalloutNV") == 0)
    {
        bUseHooked = TRUE;
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
            DWORD cbData = (wcslen(pszGameDir) + 1) * sizeof(WCHAR);

            if (!lpData && lpcbData)
            {
                *lpcbData = cbData;
                return ERROR_SUCCESS;
            }
            else if (lpData && (lpcbData && (*lpcbData >= cbData)))
            {
                memcpy(lpData, pszGameDir, cbData);
                *lpcbData = cbData;
                return ERROR_SUCCESS;
            }
        }
    }

    return Real_RegQueryValueExW(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
}

/* Hooked RegCloseKey */
LONG WINAPI Hooked_RegCloseKey(HKEY hKey)
{
    bUseHooked = FALSE;
    return Real_RegCloseKey(hKey);
}

void NoRegPatch_Init(void)
{
    /* Get game folder path */
    pszGameDir = NoRegPatch_GetGameDir();
    if (!pszGameDir)
    {
        MessageBoxW(NULL, L"Failed to retrieve the game directory.", L"NoRegPatch", MB_ICONERROR | MB_OK);
        TerminateProcess(GetCurrentProcess(), 0);
    }

    /* Initialize MinHook */
    if (MH_Initialize() != MH_OK)
    {
        MessageBoxW(NULL, L"Failed to initialize MinHook.", L"NoRegPatch", MB_ICONERROR | MB_OK);
        TerminateProcess(GetCurrentProcess(), 0);
    }

    MH_CreateHook((LPVOID*)(&RegOpenKeyExW), (LPVOID*)(&Hooked_RegOpenKeyExW), (LPVOID*)(&Real_RegOpenKeyExW));
    MH_CreateHook((LPVOID*)(&RegQueryValueExW), (LPVOID*)(&Hooked_RegQueryValueExW), (LPVOID*)(&Real_RegQueryValueExW));
    MH_CreateHook((LPVOID*)(&RegCloseKey), (LPVOID*)(&Hooked_RegCloseKey), (LPVOID*)(&Real_RegCloseKey));

    MH_EnableHook(MH_ALL_HOOKS);
}

void NoRegPatch_Uninit(void)
{
    MH_DisableHook(MH_ALL_HOOKS);

    /* Uninitialize MinHook. */
    MH_Uninitialize();

    HeapFree(GetProcessHeap(), 0, pszGameDir);
}

/* DLL entry point */
BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        NoRegPatch_Init();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        NoRegPatch_Uninit();
    }

    return TRUE;
}
