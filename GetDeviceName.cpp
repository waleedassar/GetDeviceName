
#include "windows.h"
#include "SetupAPI.h"
#include "stdio.h"

#include <intrin.h>
#include <immintrin.h>




int GetDeviceHardwareIdAndDevicePath(const GUID Guid, wchar_t* pHardwareId, wchar_t* pDevicePath)
{
	HDEVINFO hDevInfo =	SetupDiGetClassDevs(&Guid, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		printf("SetupDiGetClassDevs, err: %X\r\n", GetLastError());
		return -1;
	}
	printf("hDevInfo: %I64X\r\n", hDevInfo);

	unsigned long LastError = 0;


	_SP_DEVICE_INTERFACE_DATA Details = { 0 };
	Details.cbSize = sizeof(Details);
	BOOL bRet = SetupDiEnumDeviceInterfaces(hDevInfo, 0, &Guid, 0, &Details);
	LastError = GetLastError();
	printf("SetupDiEnumDeviceInterfaces, bRet: %X, LastError: %X\r\n", bRet, LastError);

	if (bRet != TRUE)
	{
		SetupDiDestroyDeviceInfoList(hDevInfo);
		return -1;
	}

	ULONG index = 0;
	do
	{
		_SP_DEVINFO_DATA  SPDevInfo = { 0 };
		SPDevInfo.cbSize = sizeof(SPDevInfo);

		bRet = SetupDiEnumDeviceInfo(hDevInfo, index, &SPDevInfo);
		LastError = GetLastError();
		printf("SetupDiEnumDeviceInfo, bRet: %X, LastError: %X\r\n", bRet, LastError);

		if (bRet)
		{
			unsigned char* pBuffer = 0;
			unsigned long ReqSize = 0;

			SetupDiGetDeviceRegistryProperty(hDevInfo,
				&SPDevInfo,
				SPDRP_HARDWAREID,
				0,
				0,
				0,
				&ReqSize);
			if(GetLastError()== ERROR_INSUFFICIENT_BUFFER)
			{
				pBuffer = (unsigned char*)LocalAlloc(LMEM_ZEROINIT, ReqSize);
			}
			else
			{
				break;
			}

			SetupDiGetDeviceRegistryProperty(hDevInfo,
				&SPDevInfo,
				SPDRP_HARDWAREID,
				0,
				pBuffer,
				ReqSize,
				&ReqSize);
			wprintf(L"Hardware Id: %s\r\n", pBuffer);

			wchar_t* pDeviceInstanceId = (wchar_t*)LocalAlloc(LMEM_ZEROINIT, 0x400);
			ULONG DeviceInstanceIdSize = 0x30C;

			SetupDiGetDeviceInstanceId(hDevInfo,
				&SPDevInfo,
				pDeviceInstanceId,
				DeviceInstanceIdSize,
				&DeviceInstanceIdSize);


			wprintf(L"Instance Id: %s\r\n", pDeviceInstanceId);
			LocalFree(pDeviceInstanceId);
			
			

			if (   (wcsicmp( (wchar_t*)pBuffer, L"root\\umbus") == 0)  ||
				   (wcsicmp( (wchar_t*)pBuffer, L"umb\\umbus") == 0)       )
			{
				//printf("Hello\r\n");
				//wcscpy(pHardwareId, (wchar_t*)pBuffer);


				ULONG reqSizeXXX = 0;

				SetupDiGetDeviceInterfaceDetailW(hDevInfo, &Details, 0, 0, &reqSizeXXX, 0);
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					SP_DEVICE_INTERFACE_DETAIL_DATA* pDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA*)
						LocalAlloc(LMEM_ZEROINIT, reqSizeXXX);
					pDetailData->cbSize = 0x8;
					if (SetupDiGetDeviceInterfaceDetailW(hDevInfo, &Details, pDetailData, reqSizeXXX, &reqSizeXXX, 0))
					{
						printf("Okay\r\n");
						wchar_t* DevPath = pDetailData->DevicePath;
						wprintf(L"DevicePath: %s\r\n", DevPath);
						wcscpy(pDevicePath, (wchar_t*)DevPath);
					}
					else
					{
						printf("SetupDiGetDeviceInterfaceDetailW, err: %X\r\n", GetLastError());
					}
					LocalFree(pDetailData);
				}
				else
				{
					printf("Errorrrrrrrrrrrrr\r\n");
				}
			}




			LocalFree(pBuffer);
		}

		index++;
	} while (bRet == TRUE);

	if (!SetupDiDestroyDeviceInfoList(hDevInfo))
	{
		printf("SetupDiDestroyDeviceInfoList, err: %X\r\n", GetLastError());
		return -10;
	}
	return 0;
}



int GetNtDeviceName(const GUID Guid,wchar_t* pNtDeviceName)
{
	HDEVINFO hDevInfo =
		SetupDiGetClassDevs(&Guid, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hDevInfo == INVALID_HANDLE_VALUE)
	{
		printf("SetupDiGetClassDevs, err: %X\r\n", GetLastError());
		return -1;
	}

	printf("hDevInfo: %I64X\r\n", hDevInfo);

	unsigned long LastError = 0;
	unsigned long i = 0;
	while (LastError != ERROR_NO_MORE_ITEMS)
	{
		_SP_DEVINFO_DATA  SPDevInfo = { 0 };
		SPDevInfo.cbSize = sizeof(SPDevInfo);

		BOOL bRet = SetupDiEnumDeviceInfo(hDevInfo, i, &SPDevInfo);
		LastError = GetLastError();

		printf("SetupDiEnumDeviceInfo, bRet: %X, LastError: %X\r\n", bRet, LastError);

		if (!bRet) break;
		else
		{
			unsigned char* pBuffer = (unsigned char*)LocalAlloc(LMEM_ZEROINIT, 0x200 + 0x2);
			unsigned long ReqSize = 0;

			if (SetupDiGetDeviceRegistryProperty(hDevInfo,
				&SPDevInfo,
				SPDRP_PHYSICAL_DEVICE_OBJECT_NAME,
				0,
				pBuffer,
				0x200,
				&ReqSize))
			{
				wprintf(L"Hardware Id: %s\r\n", pBuffer);
				wcscpy(pNtDeviceName, (wchar_t*)pBuffer);
				LocalFree(pBuffer);
				break;
			}
			else
			{
				printf("SetupDiGetDeviceRegistryProperty, err: %X\r\n", GetLastError());
				LocalFree(pBuffer);
				return -1;
			}
			LocalFree(pBuffer);
		}

		i++;
	}

	if ((LastError == ERROR_NO_MORE_ITEMS)||(LastError==0))
	{
		printf("Done, enumerated %X items \r\n", i);
	}
	else
	{
		printf("Error while enumerating, err: %X\r\n", GetLastError());
	}

	if (!SetupDiDestroyDeviceInfoList(hDevInfo))
	{
		printf("SetupDiDestroyDeviceInfoList, err: %X\r\n", GetLastError());
		return -10;
	}
	return 0;
}