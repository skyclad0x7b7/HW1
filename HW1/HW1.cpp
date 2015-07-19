// BoB_Memory_mapped.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.

#include <stdio.h>
#include "stdafx.h"
#include <malloc.h>
#include <string.h>
#include <Windows.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <stdint.h>
#include <crtdbg.h>


bool is_file_existsW(_In_ const wchar_t* file_path)  
{
   _ASSERTE(NULL != file_path);
   _ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
   if ((NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH))) return false;

   WIN32_FILE_ATTRIBUTE_DATA info = { 0 };

   if (GetFileAttributesExW(file_path, GetFileExInfoStandard, &info) == 0)
      return false;
   else
      return true;
}

bool create_copy_delete()
{
	wchar_t *buf=NULL;																	//  ���� ���丮 ������ ����
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	if(buflen==0)
	{
		printf("GetCurrentDirectoryW() Error!\n");
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR)*buflen);
	if(GetCurrentDirectoryW(buflen, buf)==0)
	{
		printf("GetCurrentDirectoryW() Error! (2)\n");
		free(buf);
		return false;
	}

	wchar_t file_name[4096];
	wchar_t file_name2[4096];
	if(!SUCCEEDED(StringCbPrintfW(												// ���� ���丮 + \���ϸ�
		file_name,
		sizeof(file_name),
		L"%ws\\bob.txt",
		buf)))
	{
		printf("FilenameCreate Error!");
		free(buf);
		return false;
	}

	if (is_file_existsW(file_name)==true)											// ���� ����� ����
    {
        ::DeleteFileW(file_name);
    }

	HANDLE file_handle = CreateFileW(											// ���� �ڵ� ����
		(LPCWSTR)file_name,
		GENERIC_READ | GENERIC_WRITE,
		NULL,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(file_handle == INVALID_HANDLE_VALUE)
	{
		printf("CreateFileW Error!\ngle = %u\n", GetLastError());
		return false;
	}
	char *mtData = "�����ڵ� : String String String ���ڿ� ���ڿ� ���ڿ�";					// ��Ƽ����Ʈ ���ڿ� ����
	wchar_t wcData[1024];																								// UTF-8 ���ڿ� ����
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, mtData, -1, wcData, strlen(mtData));		// UTF-8 ����
	WriteFile(file_handle, wcData, sizeof(wchar_t)*wcslen(wcData), NULL, NULL);				// ���� ����
	CloseHandle(file_handle);

	if(!SUCCEEDED(StringCbPrintfW(													// �纻 ���ϸ� ����
		file_name2,
		sizeof(file_name2),
		L"%ws\\bob2.txt",
		buf)))
	{
		printf("FilenameCreate Error!");
		free(buf);
		return false;
	}

	printf("\n\nCreating File complete!\n");
	system("pause");

	CopyFileW(file_name, file_name2,false);			// ���� ����

	
	HANDLE file_handle2 = CreateFileW((LPCWSTR)file_name2,					// ����� ���� �ڵ� ����
		GENERIC_READ,
		NULL,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	wchar_t content[1024];			// ������ �о�ͼ� ������ ����
	DWORD length;						// ���� ������ ����
	ReadFile(file_handle2, content, 1024, &length, 0);									// ���� �о��. length�� �о�� ����Ʈ �� ����.
	printf("<< ReadFile() >>\n");
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), content, length/2, &length, 0);				// �ֿܼ� ���
	printf("\n");

	LARGE_INTEGER fileSize;													// ���� ũ�� ������ ����
	if(GetFileSizeEx(file_handle2, &fileSize)!=TRUE)				// ���� ũ�� ����/
	{
		printf("GetFileSizeEx Error!\ngle = %u\n", GetLastError());
		CloseHandle(file_handle2);
		return false;
	}
	_ASSERTE(fileSize.HighPart==0);
	if(fileSize.HighPart > 0)
	{
		printf("FileSize is over 4GB. Mapping impossible\n");			// 4GB �̻��� �� ���� �Ұ���
		CloseHandle(file_handle2);
		return false;
	}
	HANDLE file_map = CreateFileMappingW(						// �޸𸮿� ����
		file_handle2,
		NULL,
		PAGE_READONLY,
		0,
		0,
		NULL);
	if(file_map==NULL)
	{
		printf("CreateFileMapping Error!\ngle = %u\n", GetLastError());
		CloseHandle(file_handle2);
		return false;
	}
	
	wchar_t *file_view = (wchar_t *) MapViewOfFile(					// ���� �о�ͼ� ����
		file_map,
		FILE_MAP_READ,
		0,
		0,
		0);
	if(file_view==NULL)
	{
		printf("MapViewOfFile Error!\ngle = %u\n", GetLastError());
		CloseHandle(file_handle2);
		CloseHandle(file_map);
		return false;
	}

	printf("<< Memory Mapped I/O >>\n");
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), file_view, wcslen(file_view), 0, 0);				// �ֿܼ� ���ڼ���ŭ ���
	
	free(buf);												// �Ҵ�� �޸� ����
	buf=NULL;
	UnmapViewOfFile((LPCVOID)file_view);
	CloseHandle(file_handle2);
	CloseHandle(file_map);

	printf("\n\nCopying File & Reading File complete!\n");
	system("pause");

	DeleteFile((LPCWSTR)file_name);
	DeleteFile((LPCWSTR)file_name2);

	printf("\n\nDelete File Complete!\n\n");
	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if(!create_copy_delete()) printf("\n\nFailed!!\n\n");
	return 0;
}

