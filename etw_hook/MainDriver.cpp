#include <ntifs.h>

#include "ssdt.h"
#include "HookWindowApi.h"

#include <refs.hpp>
#include <etwhook_init.hpp>
#include <etwhook_manager.hpp>

#include <kstl/ksystem_info.hpp>

PVOID ghwnd = NULL;

PVOID MyNtUserGetForegroundWindow()
{
	typedef PVOID(NTAPI* NtUserGetForegroundWindowProc)(VOID);

	NtUserGetForegroundWindowProc NtUserGetForegroundWindowFunc =(NtUserGetForegroundWindowProc) GetUserGetForegroundWindow();

	PVOID hwnd = NtUserGetForegroundWindowFunc();

	if (ghwnd == hwnd)
	{
		//DbgBreakPoint();
		return NULL;
	}

	return hwnd;
}

PVOID MyNtUserFindWindowEx(PVOID desktop1, PVOID desktop2, PUNICODE_STRING tName, PUNICODE_STRING tclassName, ULONG64 x)
{
	typedef PVOID(NTAPI* MyUserFindWindowExProc)(PVOID desktop1, PVOID desktop2, PUNICODE_STRING tName, PUNICODE_STRING tclassName, ULONG64 x);

	MyUserFindWindowExProc MyUserFindWindowExFunc = (MyUserFindWindowExProc)GetUserFindWindowEx();

	PVOID hwnd = MyUserFindWindowExFunc(desktop1, desktop2, tName, tclassName, x);

	if (ghwnd == hwnd)
	{
		//DbgBreakPoint();
		return NULL;
	}

	return hwnd;
}

ULONG64 MyNtUserQueryWindow(PVOID Hwnd, int flags)
{
	typedef ULONG64(NTAPI* MyNtUserQueryWindowProc)(PVOID Hwnd, int flags);

	MyNtUserQueryWindowProc MyNtUserQueryWindowFunc =(MyNtUserQueryWindowProc) GetUserQueryWindow();

	if (Hwnd == ghwnd) return 0;

	return MyNtUserQueryWindowFunc(Hwnd, flags);
}

PVOID MyNtUserWindowFromPoint(PVOID Point)
{
	typedef PVOID(NTAPI* NtUserWindowFromPointProc)(PVOID Point);

	NtUserWindowFromPointProc NtUserWindowFromPointFunc =(NtUserWindowFromPointProc) GetUserWindowFromPoint();

	PVOID Hwnd = NtUserWindowFromPointFunc(Point);

	if (Hwnd == ghwnd) return 0;

	return Hwnd;
}

NTSTATUS MyNtUserBuildHwndList(PVOID a1, PVOID a2, PVOID Address, unsigned int a4, ULONG count, int xxx, PVOID Addressa, PULONG pretCount)
{
	typedef NTSTATUS(NTAPI* MyNtUserBuildHwndListProc)(PVOID a1, PVOID a2, PVOID Address, unsigned int a4, ULONG count, int xxx, PVOID Addressa, PULONG pretCount);

	MyNtUserBuildHwndListProc MyNtUserBuildHwndListFunc =(MyNtUserBuildHwndListProc) GetUserBuildHwndList();

	NTSTATUS status = MyNtUserBuildHwndListFunc(a1, a2, Address, a4, count, xxx, Addressa, pretCount);


	if (NT_SUCCESS(status))
	{

		if (MmIsAddressValid(pretCount) && MmIsAddressValid(Addressa))
		{
			int scount = *pretCount;
			PVOID* arrays = (PVOID*)Addressa;
			for (int i = 0; i < scount; i++)
			{
				if (arrays[i] == ghwnd)
				{
					//������ǵľ�����ǵ�һ��
					if (i == 0)
					{
						DbgPrintEx(77, 0, "[db]:i arrays[i],%llx\r\n", arrays[i]);
						//ֻ��һ�����
						if (scount == 1)
						{
							arrays[i] = 0;
							*pretCount = 0;
							break;
						}

						arrays[i] = arrays[i + 1];
						break;
					}
					else
					{
						DbgPrintEx(77, 0, "[db]:W arrays[i],%llx\r\n", arrays[i]);
						arrays[i] = arrays[i - 1];
						break;
					}

				}
			}
		}

	}

	return status;
}


NTSTATUS detour_NtClose(HANDLE h) {

	//LOG_INFO("ZwClose was Caguth\r\n");

	return NtClose(h);

}
EXTERN_C NTSTATUS DriverEntry(PDRIVER_OBJECT pDriver, PUNICODE_STRING)
{
	auto status = STATUS_SUCCESS;

	//DbgBreakPoint();
	InitHook();
	//SSDTStruct* find = SSSDTFind();
	//IfhOn(HookCallback);
	

	pDriver->DriverUnload = [](PDRIVER_OBJECT) {//�� lambda ���ʽ�У��������Ҫʹ�ô��ݵĲ���������ָֻ���������Ͷ���д��������
		//DriverUnload ��һ������ PDRIVER_OBJECT ���Ͳ����ĺ���ָ�룬������ lambda ���ʽ���������ͬ�Ĳ����б�
		EtwHookManager::get_instance()->destory();
		};

	kstd::Logger::init("etw_hook", nullptr);

	LOG_INFO("init...\r\n");


	status = EtwHookManager::get_instance()->init();



	//EtwHookManager::get_instance()->add_hook(NtCreateFile, detour_NtCreateFile);
	//EtwHookManager::get_instance()->add_hook(, MyNtUserGetForegroundWindow);

	ghwnd = (PVOID)0x0009075E;  //���ھ��

	//��ȡǰ�ô���
	typedef PVOID(NTAPI* NtUserGetForegroundWindowProc)(VOID);
	NtUserGetForegroundWindowProc NtUserGetForegroundWindowFunc =(NtUserGetForegroundWindowProc) GetUserGetForegroundWindow();
	EtwHookManager::get_instance()->add_hook(NtUserGetForegroundWindowFunc, MyNtUserGetForegroundWindow);

	//���Ҵ���
	typedef PVOID(NTAPI* MyUserFindWindowExProc)(PVOID desktop1, PVOID desktop2, PUNICODE_STRING tName, PUNICODE_STRING tclassName, ULONG64 x);
	MyUserFindWindowExProc MyUserFindWindowExFunc = (MyUserFindWindowExProc)GetUserFindWindowEx();
	EtwHookManager::get_instance()->add_hook(MyUserFindWindowExFunc, MyNtUserFindWindowEx);

	//��ѯ����
	typedef ULONG64(NTAPI* MyNtUserQueryWindowProc)(PVOID Hwnd, int flags);
	MyNtUserQueryWindowProc MyNtUserQueryWindowFunc = (MyNtUserQueryWindowProc)GetUserQueryWindow();
	EtwHookManager::get_instance()->add_hook(MyNtUserQueryWindowFunc, MyNtUserQueryWindow);

	//ͨ���������Ҵ���
	typedef PVOID(NTAPI* NtUserWindowFromPointProc)(PVOID Point);
	NtUserWindowFromPointProc NtUserWindowFromPointFunc = (NtUserWindowFromPointProc)GetUserWindowFromPoint();
	EtwHookManager::get_instance()->add_hook(NtUserWindowFromPointFunc, MyNtUserWindowFromPoint);

    //ö�ٴ���   //win10����ط�����������7�� ��һ����6������
	typedef NTSTATUS(NTAPI* MyNtUserBuildHwndListProc)(PVOID a1, PVOID a2, PVOID Address, unsigned int a4, ULONG count,int xxx, PVOID Addressa, PULONG pretCount);
	MyNtUserBuildHwndListProc MyNtUserBuildHwndListFunc = (MyNtUserBuildHwndListProc)GetUserBuildHwndList();
	EtwHookManager::get_instance()->add_hook(MyNtUserBuildHwndListFunc, MyNtUserBuildHwndList);

	EtwHookManager::get_instance()->add_hook(NtClose, detour_NtClose);

	return STATUS_SUCCESS;
}