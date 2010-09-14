#ifndef TP_UTIL_WIN_H_INCLUDED
#define TP_UTIL_WIN_H_INCLUDED

#include "exception.h"
#include "auto_release.h"

namespace tp
{
	struct oswin
	{
		static int get_osverion(int* minor_ver)
		{
			OSVERSIONINFOW ovi;
			ovi.dwOSVersionInfoSize = sizeof(ovi);
			if (GetVersionExW(&ovi))
			{
				if (minor_ver) *minor_ver = ovi.dwMinorVersion;
				return ovi.dwMajorVersion;
			}
			return 0;
		}

		/** VISTA/WIN7判断当前进程是否以提升权限模式运行
		 *  
		 *  \ret TRUE 进程是提升权限运行，或者UAC已关闭
		 */
		static bool is_elevated()
		{
			if (get_osverion(0) < 6)
			{
				// XP
				return true;
			}

			HANDLE hToken;
			BOOL bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
			throw_winerr_when(bRet != TRUE);
			ON_LEAVE_1(CloseHandle(hToken), HANDLE, hToken);

			TOKEN_ELEVATION te = {};
			DWORD dwReturnLength = 0;
			bRet = GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &dwReturnLength);
			throw_winerr_when(bRet != TRUE);

			return te.TokenIsElevated? true : false;
		}

		void reg_deletekey(HKEY root, const std::wstring& path, const std::wstring& name)
		{
			HKEY hkey;
			LONG ret = RegOpenKeyW(root, path.c_str(), &hkey);
			throw_if_lasterror(ret);
			ON_LEAVE_1(RegCloseKey(hkey), HKEY, hkey);

			ret = RegDeleteKeyW(hkey, name.c_str());
			throw_if_lasterror(ret);
		}

		void reg_deletekey(HKEY root, const std::wstring& pathname)
		{
			size_t pos = pathname.find_last_of(L'\\');
			throw_when(pos == std::wstring::npos, L"invalid reg path");

			reg_deletekey(root, pathname.substr(0, pos), pathname.substr(pos + 1));
		}

		void reg_write_string(HKEY root, const std::wstring& path, const std::wstring& name, const std::wstring& value)
		{
			HKEY hkey;
			LONG ret = RegCreateKeyExW(root, path.c_str(), 0, NULL, 0, KEY_ALL_ACCESS, 0, &hkey, NULL);
			throw_if_lasterror(ret);
			ON_LEAVE_1(RegCloseKey(hkey), HKEY, hkey);

			ret = RegSetValueExW(hkey, name.c_str(), 0, REG_SZ, (CONST BYTE*)value.c_str(), (value.length() + 1) * 2);
			throw_if_lasterror(ret);
		}

		void reg_write_dword(HKEY root, LPCWSTR path, LPCWSTR name, DWORD value)
		{
			HKEY hkey;
			LONG ret = RegCreateKeyExW(root, path, 0, NULL, 0, KEY_ALL_ACCESS, 0, &hkey, NULL);
			throw_if_lasterror(ret);
			ON_LEAVE_1(RegCloseKey(hkey), HKEY, hkey);

			ret = RegSetValueExW(hkey, name, 0, REG_DWORD, (CONST BYTE*)&value, 4);
			throw_if_lasterror(ret);
		}

		void reg_read_value(HKEY root, const std::wstring& path, const std::wstring& name, DWORD& type, LPBYTE buffer, LPDWORD buflen)
		{
			HKEY hkey;
			LSTATUS ret;
			ret = RegOpenKeyW(root, path.c_str(), &hkey);
			throw_if_lasterror(ret);
			ON_LEAVE_1(RegCloseKey(hkey), HKEY, hkey);

			ret = RegQueryValueExW(hkey, name.c_str(), NULL, &type, buffer, buflen);
			throw_if_lasterror(ret);
		}

		std::wstring reg_read_string(HKEY root, const std::wstring& path, const std::wstring& name, const std::wstring& defval)
		{
			WCHAR buffer[1024] = {};
			DWORD buffer_len = sizeof(buffer);
			DWORD type = 0;
			reg_read_value(root, path, name, type, (LPBYTE)buffer, &buffer_len);

			return buffer;
		}

		DWORD reg_read_dword(HKEY root, const std::wstring& path, const std::wstring& name, const DWORD defval)
		{
			DWORD buffer = 0;
			DWORD buffer_len = sizeof(buffer);
			DWORD type = 0;
			reg_read_value(root, path, name, type, (LPBYTE)buffer, &buffer_len);

			return buffer;
		}

		std::vector<std::wstring> reg_get_subkeys(HKEY root, const std::wstring& path)
		{
			std::vector<std::wstring> keys;

			HKEY hkey;
			LSTATUS ret;
			ret = RegOpenKeyW(HKEY_LOCAL_MACHINE, path.c_str(), &hkey);
			throw_if_lasterror(ret);
			ON_LEAVE_1(RegCloseKey(hkey), HKEY, hkey);

			WCHAR subKey[1024];
			for (int i = 0; ; i++)
			{
				ret = RegEnumKeyW(hkey, i, subKey, _countof(subKey));
				if (ret != ERROR_SUCCESS) break;

				keys.push_back(subKey);
			}

			return keys;
		}

		void reg_copykey(HKEY src, HKEY dest)
		{
			DWORD dwMaxSubkeyLen = 255;
			DWORD dwMaxValueNameLen = 255;
			DWORD dwMaxValueLen = 16383;
			LSTATUS ret;
			ret = RegQueryInfoKeyW(src, NULL, NULL, NULL, NULL, &dwMaxSubkeyLen, NULL, NULL, &dwMaxValueNameLen, &dwMaxValueLen, NULL, NULL);
			throw_if_lasterror(ret);

			wchar_t* key = new wchar_t[dwMaxSubkeyLen + 1];
			ON_LEAVE_1(delete [] key, wchar_t*, key);
			wchar_t* vname = new wchar_t[dwMaxValueNameLen + 1];
			ON_LEAVE_1(delete[] vname, wchar_t*, vname);
			BYTE* value = new BYTE[dwMaxValueLen + 1];
			ON_LEAVE_1(delete[] value, BYTE*, value);

			for (DWORD index = 0;;index++)
			{
				DWORD dwValueNameLen = dwMaxValueNameLen + 1;
				DWORD dwValueLen = dwMaxValueLen + 1;
				DWORD dwType;
				ret = RegEnumValueW(src, index, vname, &dwValueNameLen, NULL, &dwType, value, &dwValueLen);
				if (ret == ERROR_NO_MORE_ITEMS) break;
				throw_if_lasterror(ret);
				ret = RegSetValueExW(dest, vname, NULL, dwType, value, dwValueLen);
				throw_if_lasterror(ret);
			}

			for (DWORD index = 0;;index++)
			{
				DWORD dwKeyLen = dwMaxSubkeyLen + 1;
				ret = RegEnumKeyExW(src, index, key, &dwKeyLen, NULL, NULL, NULL, NULL);
				if (ret == ERROR_NO_MORE_ITEMS) break;
				throw_if_lasterror(ret);
				HKEY newsrc;
				HKEY newdst;
				ret = RegOpenKeyExW(src, key, NULL, KEY_READ, &newsrc);
				throw_if_lasterror(ret);
				ret = RegCreateKeyExW(dest, key, NULL, NULL, 0, KEY_READ|KEY_WRITE, NULL, &newdst, NULL);
				throw_if_lasterror(ret);

				reg_copykey(newsrc, newdst);
			}
		}

	};
}


#endif