#include "MyRegistryPlugin.h"

#if defined _M_IX86
  #import "..\bin\x86\Debug\DeviareCOM.dll" raw_interfaces_only, named_guids, raw_dispinterfaces, auto_rename
#elif defined _M_X64
  #import "..\bin\x64\Debug\DeviareCOM64.dll" raw_interfaces_only, named_guids, raw_dispinterfaces, auto_rename
#else
  #error Unsupported platform
#endif

using namespace Deviare2;

#if defined _M_IX86
  #define my_ssize_t long
  #define my_size_t unsigned long
#elif defined _M_X64
  #define my_ssize_t __int64
  #define my_size_t unsigned __int64
#endif


#include <d3d9.h>

IDirect3DDevice9* pDeviceInterface;

UINT_PTR CoGetMethodAddress(LPVOID pv, UINT nIndex)
{
	UINT_PTR *pdwVirtualTable;

	pdwVirtualTable = (UINT_PTR*)*(UINT_PTR*)pv;

	return pdwVirtualTable[nIndex];
}

DWORD WINAPI MyThread( LPVOID lpParam ) 
{ 
	::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	
	Sleep(2000);


	while(TRUE)
	{
		if(GetAsyncKeyState(VK_INSERT))
		{
			pDeviceInterface->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}

		if(GetAsyncKeyState(VK_DELETE))
		{
			pDeviceInterface->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);	
		}

		Sleep(500);
	}
	
	return 0;


	::CoUninitialize();
	
	return 0;
}


//IMPORTANT NOTES:
//---------------
//
//1) Regardless of the functionallity of the plugin, the dll must export: OnLoad, OnUnload, OnHookAdded,
//   OnHookRemoved and OnFunctionCall (Tip: add a .def file to avoid name mangling)
//
//2) Code inside methods should try/catch exceptions to avoid possible crashes in hooked application.
//
//3) Methods that returns an HRESULT value should return S_OK if success or an error value.
//
//   3.1) If a method returns a value less than zero, all hooks will be removed and agent will unload
//        from the process.
//
//   3.2) The recommended way to handle errors is to let the SpyMgr to decide what to do. For e.g. if
//        you hit an error in OnFunctionCall, probably, some custom parameter will not be added to the
//        CustomParams() collection. So, when in your app received the DNktSpyMgrEvents::OnFunctionCall
//        event, you will find the parameters is missing and at this point you can choose what to do.

//-----------------------------------------------------------

BOOL APIENTRY DllMain(__in HMODULE hModule, __in DWORD ulReasonForCall, __in LPVOID lpReserved)
{
  switch (ulReasonForCall)
  {
    case DLL_PROCESS_ATTACH:
		{
			pDeviceInterface = NULL;

			HANDLE hPipe = ::CreateNamedPipe(L"\\\\.\\pipe\\HyperPipe32",
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
			PIPE_UNLIMITED_INSTANCES,
			4096,
			4096,
			0,
			NULL);

			ConnectNamedPipe(hPipe, NULL);


			LPDIRECT3D9 g_pD3D = NULL;
			g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

			UINT_PTR data = CoGetMethodAddress(g_pD3D, 16);
			DWORD bytesWritten = 0;
			WriteFile(hPipe, &data, sizeof(UINT_PTR), &bytesWritten, NULL);

			CreateThread(NULL, NULL, MyThread, NULL, NULL, NULL);


		}break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}

MY_EXPORT HRESULT WINAPI OnLoad()
{
  ::OutputDebugStringA("MyRegistryPlugin::OnLoad called");
  return S_OK;
}

MY_EXPORT VOID WINAPI OnUnload()
{
  ::OutputDebugStringA("MyRegistryPlugin::OnUnLoad called");
  return;
}

MY_EXPORT HRESULT WINAPI OnHookAdded(__in INktHookInfo *lpHookInfo, __in DWORD dwChainIndex,
                                     __in LPCWSTR szParametersW)
{
  CComBSTR functionName;
  my_ssize_t address;
  CHAR szBufA[1024];

  lpHookInfo->get_FunctionName(&functionName);
  lpHookInfo->get_Address(&address);
  sprintf_s(szBufA, 1024, "MyRegistryPlugin::OnHookAdded called [Hook: %S @ 0x%IX / Chain:%lu]",
           functionName, address, dwChainIndex);
  ::OutputDebugStringA(szBufA);
  return S_OK;
}

MY_EXPORT VOID WINAPI OnHookRemoved(__in INktHookInfo *lpHookInfo, __in DWORD dwChainIndex)
{
  CComBSTR functionName;
  my_ssize_t address;
  CHAR szBufA[1024];

  lpHookInfo->get_FunctionName(&functionName);
  lpHookInfo->get_Address(&address);
  sprintf_s(szBufA, 1024, "MyRegistryPlugin::OnHookRemoved called [Hook: %S @ 0x%IX / Chain:%lu]",
            functionName, address, dwChainIndex);
  ::OutputDebugStringA(szBufA);
  return;
}

MY_EXPORT HRESULT WINAPI OnFunctionCall(__in INktHookInfo *lpHookInfo, __in DWORD dwChainIndex,
                                        __in INktHookCallInfoPlugin *lpHookCallInfoPlugin)
{
  CComBSTR functionName, tempBStr[2];
  CComPtr<INktParamsEnum> paramsEnum;
  my_ssize_t address;
  CHAR szBufA[1024];
  WCHAR szBufW[64];
  long i, paramCount;
  HRESULT hRes;

  lpHookInfo->get_FunctionName(&functionName);
  lpHookInfo->get_Address(&address);
  sprintf_s(szBufA, 1024, "MyRegistryPlugin::OnFunctionCall called [Hook: %S @ 0x%IX / Chain:%lu]",
            functionName, address, dwChainIndex);
  ::OutputDebugStringA(szBufA);

   // ::MessageBoxA(NULL, szBufA, "DLL", MB_OK);

  hRes = lpHookCallInfoPlugin->Params(&paramsEnum);
  if (FAILED(hRes))
    return S_OK; //ignore errors

  try
  {
    tempBStr[0] = L"sample name";
    tempBStr[1] = L"HKEY extractor sample";
  }
  catch (...)
  {
    return S_OK; //ignore errors
  }
  hRes = lpHookCallInfoPlugin->AddString(tempBStr[0], tempBStr[1]);
  if (FAILED(hRes))
    return S_OK; //ignore errors

  hRes = paramsEnum->get_Count(&paramCount);
  if (FAILED(hRes))
    return S_OK; //ignore errors


  CComPtr<INktParam> param;
  for (i = 0; i < paramCount; i++)
  {
	paramsEnum->GetAt(i, &param);
  }

  long pointeraddress;

  param->get_PointerVal(&pointeraddress);


CHAR str[100];
sprintf(str, "%li", pointeraddress);
//::MessageBoxA(NULL, str, "DLL", MB_OK);

IDirect3DDevice9** ppDeviceInterface = (IDirect3DDevice9**)pointeraddress;

pDeviceInterface = *ppDeviceInterface;


/*
  for (i = 0; i < paramCount; i++)
  {
    CComPtr<INktParam> param, tempParam;
    VARIANT_BOOL isPointer;

    hRes = paramsEnum->GetAt(i, &param);
    if (FAILED(hRes))
      return S_OK; //ignore errors

    hRes = param->get_IsPointer(&isPointer);
    if (FAILED(hRes))
      isPointer = VARIANT_FALSE;

    if (isPointer == VARIANT_TRUE)
    {
      hRes = param->Evaluate(&tempParam);
      if (FAILED(hRes))
        return S_OK; //ignore errors
      param = tempParam;
    }

    if (param != NULL)
    {
      CComBSTR type, paramNo;
      my_size_t size;

      hRes = param->get_TypeName(&type);
      if (SUCCEEDED(hRes) && type != NULL && wcscmp(type, L"HKEY") == 0)
      {
        hRes = param->get_SizeTVal(&size);
        if (FAILED(hRes))
          return S_OK; //ignore errors
        swprintf_s(szBufW, 64, L"param#%ld", i);
        try
        {
          paramNo = szBufW;
        }
        catch (...)
        {
          return S_OK; //ignore errors
        }
        hRes = lpHookCallInfoPlugin->AddSizeT(paramNo, size);
        if (FAILED(hRes))
          return S_OK; //ignore errors
      }
    }
  }
*/
  return S_OK;
}
