#pragma once
#include <windows.h>
#include <stddef.h>
int Wh_GetIntSetting(PCWSTR valueName, ...);
PCWSTR Wh_GetStringSetting(PCWSTR valueName, ...);
void Wh_FreeStringSetting(PCWSTR string);
BOOL Wh_SetFunctionHook(void* targetFunction, void* hookFunction, void** originalFunction);
BOOL Wh_RemoveFunctionHook(void* targetFunction);
BOOL Wh_ApplyHookOperations();
void Wh_Log(PCWSTR format, ...);
size_t Wh_GetStringValue(PCWSTR valueName, PWSTR stringBuffer, size_t bufferChars);
BOOL Wh_SetStringValue(PCWSTR valueName, PCWSTR value);
int Wh_GetIntValue(PCWSTR valueName, int defaultValue);
BOOL Wh_SetIntValue(PCWSTR valueName, int value);
size_t Wh_GetBinaryValue(PCWSTR valueName, void* buffer, size_t bufferSize);
BOOL Wh_SetBinaryValue(PCWSTR valueName, const void* buffer, size_t bufferSize);
BOOL Wh_DeleteValue(PCWSTR valueName);
size_t Wh_GetModStoragePath(PWSTR pathBuffer, size_t bufferChars);
