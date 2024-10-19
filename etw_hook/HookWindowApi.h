#pragma once
#include <ntifs.h>


EXTERN_C ULONG_PTR GetUserFindWindowEx();

EXTERN_C ULONG_PTR GetUserGetForegroundWindow();

EXTERN_C ULONG_PTR GetUserBuildHwndList();

EXTERN_C ULONG_PTR GetUserQueryWindow();

EXTERN_C ULONG_PTR GetUserWindowFromPoint();


EXTERN_C VOID InitHook();
