/*
 *      Copyright (C) 2011 Hendrik Leppkes
 *      http://www.1f0.de
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "stdafx.h"
#include "registry.h"

bool CreateRegistryKey(HKEY hKeyRoot, LPCTSTR pszSubKey)
{
  HKEY hKey;
  LONG  lRet;

  lRet = RegCreateKeyEx(
    hKeyRoot,
    pszSubKey,
    0,
    NULL,
    REG_OPTION_NON_VOLATILE,
    KEY_WRITE,
    NULL,
    &hKey,
    NULL
    );

  if(lRet == ERROR_SUCCESS)
  {
    RegCloseKey(hKey);
    hKey = (HKEY)NULL;
    return true;
  }

  SetLastError((DWORD)lRet);
  return false;
}

CRegistry::CRegistry() : m_key(NULL)
{
}

CRegistry::CRegistry(HKEY hkeyRoot, LPCTSTR pszSubKey, HRESULT &hr)
  : m_key(NULL)
{
  hr = Open(hkeyRoot, pszSubKey);
}

CRegistry::~CRegistry()
{
  if (m_key)
    RegCloseKey(*m_key);
  delete m_key;
}

HRESULT CRegistry::Open(HKEY hkeyRoot, LPCTSTR pszSubKey)
{
  LONG lRet;
  
  if (m_key != NULL) { return E_UNEXPECTED; }
  
  m_key = new HKEY();
  lRet = RegOpenKeyEx(hkeyRoot, pszSubKey, 0, KEY_READ | KEY_WRITE, m_key);
  if (lRet != ERROR_SUCCESS) {
    delete m_key;
    m_key = NULL;
    return E_FAIL;
  }
  return S_OK;
}

std::wstring CRegistry::ReadString(LPCTSTR pszKey, HRESULT &hr)
{
  LONG lRet;
  DWORD dwSize;
  std::wstring result;

  hr = S_OK;

  if (m_key == NULL) { hr = E_UNEXPECTED;  return result; }

  lRet = RegQueryValueEx(*m_key, pszKey, NULL, NULL, NULL, &dwSize);

  if (lRet == ERROR_SUCCESS) {
    // Alloc Buffer to fit the data
    WCHAR *buffer = (WCHAR *)CoTaskMemAlloc(dwSize);
    memset(buffer, 0, dwSize);
    lRet = RegQueryValueEx(*m_key, pszKey, NULL, NULL, (LPBYTE)buffer, &dwSize);
    result = std::wstring(buffer);
    CoTaskMemFree(buffer);
  }
  
  if (lRet != ERROR_SUCCESS) {
    hr = E_FAIL;
  }

  return result;
}

HRESULT CRegistry::WriteString(LPCTSTR pszKey, const LPCTSTR pszValue)
{
  LONG lRet;
  HRESULT hr;

  hr = S_OK;

  if (m_key == NULL) { return E_UNEXPECTED; }

  lRet = RegSetValueEx(*m_key, pszKey, 0, REG_SZ, (const BYTE *)pszValue, (DWORD)((wcslen(pszValue) + 1) * sizeof(WCHAR)));
  if (lRet != ERROR_SUCCESS) {
    return E_FAIL;
  }
  return S_OK;
}

DWORD CRegistry::ReadDWORD(LPCTSTR pszKey, HRESULT &hr)
{
  LONG lRet;
  DWORD dwSize = sizeof(DWORD);
  DWORD dwVal = 0;

  hr = S_OK;

  if (m_key == NULL) { hr = E_UNEXPECTED; return 0; }

  lRet = RegQueryValueEx(*m_key, pszKey, 0, NULL, (LPBYTE)&dwVal, &dwSize);

  if (lRet != ERROR_SUCCESS) {
    hr = E_FAIL;
  }

  return dwVal;
}

HRESULT CRegistry::WriteDWORD(LPCTSTR pszKey, DWORD dwValue)
{
  LONG lRet;
  HRESULT hr;

  hr = S_OK;

  if (m_key == NULL) { return E_UNEXPECTED; }

  lRet = RegSetValueEx(*m_key, pszKey, 0, REG_DWORD, (const BYTE *)&dwValue, sizeof(dwValue));
  if (lRet != ERROR_SUCCESS) {
    return E_FAIL;
  }
  return S_OK;
}

BOOL CRegistry::ReadBOOL(LPCTSTR pszKey, HRESULT &hr)
{
  DWORD dwVal = ReadDWORD(pszKey, hr);
  return dwVal ? TRUE : FALSE;
}

HRESULT CRegistry::WriteBOOL(LPCTSTR pszKey, BOOL bValue)
{
  return WriteDWORD(pszKey, bValue);
}

BYTE *CRegistry::ReadBinary(LPCTSTR pszKey, DWORD &dwSize, HRESULT &hr)
{
  LONG lRet;
  BYTE *result = NULL;

  hr = S_OK;

  if (m_key == NULL) { hr = E_UNEXPECTED;  return result; }

  lRet = RegQueryValueEx(*m_key, pszKey, NULL, NULL, NULL, &dwSize);

  if (lRet == ERROR_SUCCESS) {
    // Alloc Buffer to fit the data
    result = (BYTE *)CoTaskMemAlloc(dwSize);
    memset(result, 0, dwSize);
    lRet = RegQueryValueEx(*m_key, pszKey, NULL, NULL, (LPBYTE)result, &dwSize);
  }

  if (lRet != ERROR_SUCCESS) {
    hr = E_FAIL;
  }

  return result;
}

HRESULT CRegistry::WriteBinary(LPCTSTR pszKey, const BYTE *pbValue, int iLen)
{
  LONG lRet;
  HRESULT hr;

  hr = S_OK;

  if (m_key == NULL) { return E_UNEXPECTED; }

  lRet = RegSetValueEx(*m_key, pszKey, 0, REG_BINARY, (const BYTE *)pbValue, iLen);
  if (lRet != ERROR_SUCCESS) {
    return E_FAIL;
  }
  return S_OK;
}