// Tourne'Icons Kit: what tourne-icons.wh.cpp needs from Windhawk, made by
// kit.cpp itself (the Kit runs without Windhawk).
#pragma once
#include <windows.h>

#define WH_MOD_ID L"tourne-icons-kit"

void KitLog(PCWSTR format, ...);
#define Wh_Log(format, ...) KitLog(format, ##__VA_ARGS__)

int Wh_GetIntSetting(PCWSTR name, ...);
PCWSTR Wh_GetStringSetting(PCWSTR name, ...);
void Wh_FreeStringSetting(PCWSTR string);
BOOL Wh_SetFunctionHook(void* target, void* hook, void** original);
size_t Wh_GetModStoragePath(PWSTR buffer, size_t chars);

typedef struct {
    size_t optionsSize;
    PCWSTR targetFilePath;
} WH_GET_URL_CONTENT_OPTIONS;
typedef struct {
    const char* data;
    size_t length;
    int statusCode;
} WH_URL_CONTENT;
const WH_URL_CONTENT* Wh_GetUrlContent(PCWSTR url, const WH_GET_URL_CONTENT_OPTIONS* options);
void Wh_FreeUrlContent(const WH_URL_CONTENT* content);

namespace WindhawkUtils {
template <typename Prototype>
BOOL SetFunctionHook(Prototype* target, Prototype* hook, Prototype** original) {
    return Wh_SetFunctionHook((void*)target, (void*)hook, (void**)original);
}
}  // namespace WindhawkUtils
