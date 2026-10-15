// Explorer's layout options, as StartAllBack's Explorer style has them: the
// details pane along the bottom and This PC's drives grouped by type
// ("// @inline explorer-layout.cpp", in Explorer Style).

////////////////////////////////////////////////////////////////////////////////
// The details pane along the bottom. Each folder type's layout is a small
// DirectUI XML file ("UIFILE" resources: shell32's on current Windows 11, in
// its SystemResources .mun, explorerframe's before), read through dui70's
// DUIXmlParser. Windows 11's put a DetailsContainer on the right; Windows 7's
// put the details in a pane along the bottom, whose parts Explorer still has.
// The layout is read from Windows' own resource and only that container
// swapped, so each folder type keeps the rest of its own.

std::atomic<bool> g_bottomDetails{false};

// Windows 7's details pane, spanning the window's width above the status bar,
// with a sizer on its top edge. Its height goes in the %d (in rp).
constexpr char kBottomDetailsPane[] = R"xml(
<TemplateBackground id="atom(PreviewContainer)" height="%drp" layoutpos="bottom" layout="filllayout()">
<TemplateBackground id="atom(PreviewShineLayer)"/>
<PreviewBackground id="atom(BackgroundClear)" background="ARGB(0, 0, 0, 0)" layout="borderlayout()">
<PreviewThumbnail ModuleID="PreviewThumbnailModule" MaxThumbSize="256" layoutpos="Left"/>
<Element layoutpos="Client" layout="filllayout()">
<PreviewMetadata ModuleID="PreviewMetadataModule" NullSelectPropertyString="prop:*System.OfflineStatus;*System.OfflineAvailability" layoutpos="Client"/>
</Element>
</PreviewBackground>
<Sizer id="atom(PreviewPaneSizer)" sizingtarget="atom(PreviewContainer)" FramePersistType="2" SlidesUpAndDown="true" DownOrRightGrowsTarget="false" layoutpos="top"/>
</TemplateBackground>)xml";

// And the preview pane (Alt+P), which shared Windows 11's container, on the
// right of the whole window as in Windows 7. Its sizer takes space from the
// page first (PageSpaceControlSizer), so it can be dragged both ways.
constexpr char kRightPreviewPane[] = R"xml(
<TemplateBackground id="atom(ReadingPane)" layout="filllayout()" layoutpos="right">
<PreviewThumbnail ModuleID="ReadingPaneThumbnailModule" HideProperty="PreviewHandlerRunning" Vertical="true" layoutpos="client" background="ARGB(0, 0, 0, 0)"/>
<ReadingPaneModule ModuleID="ReadingPaneModule" layoutpos="none"/>
</TemplateBackground>
<Sizer id="atom(ReadingPaneSizer)" sizingtarget="atom(ReadingPane)" GrowTargetFirst="true" SlidesUpAndDown="false" PrioritySizer="atom(PageSpaceControlSizer)" DownOrRightGrowsTarget="false" FramePersistType="2" layoutpos="Right"/>)xml";

// The side pane's sizer (the details pane's, DetailsContainer, or the right-hand preview pane's,
// ReadingPane) has a minimum size that Explorer sets to the width the pane opened at, so the pane could
// be dragged wider but never back narrower than that (seen in a folder window with StartAllBack off;
// its own running code lifts the same floor). shell32's CDUISizerElement::_ComputeBoundedSize keeps
// a size between that minimum and the room there is; for those two panes the minimum is let go, down
// to 24 px (a long drag reached a 100 px floor in one go, and it seemed to stop working).
using ComputeBoundedSize_t = int(WINAPI*)(void* sizer, int size);
using SizerTargetId_t = unsigned short(WINAPI*)(void* sizer);
using SizerMinSize_t = int(WINAPI*)(void* sizer);
ComputeBoundedSize_t ComputeBoundedSize_Original;
SizerTargetId_t SizerTargetId;
SizerMinSize_t SizerMinSize;
std::atomic<unsigned short> g_detailsAtom, g_readingPaneAtom;

int WINAPI ComputeBoundedSize_Hook(void* sizer, int size) {
    int bounded = ComputeBoundedSize_Original(sizer, size);
    // Held up by the minimum (not by a maximised pane, which is held at its most).
    if (size >= bounded || !SizerTargetId || !SizerMinSize || bounded != SizerMinSize(sizer)) return bounded;
    // The ids are dui70's atoms, made by the time a window lays out its panes.
    if (!g_detailsAtom) {
        using StrToID_t = unsigned short(WINAPI*)(PCWSTR);
        HMODULE dui = GetModuleHandleW(L"dui70.dll");
        auto strToId = dui ? (StrToID_t)GetProcAddress(dui, "StrToID") : nullptr;
        if (!strToId) return bounded;
        g_readingPaneAtom = strToId(L"ReadingPane");
        g_detailsAtom = strToId(L"DetailsContainer");
    }
    unsigned short target = SizerTargetId(sizer);
    if (target != g_detailsAtom && target != g_readingPaneAtom) return bounded;
    static int const smallest = MulDiv(24, GetDpiForSystem(), 96);
    return std::max(size, std::min(smallest, bounded));
}

// From the symbol thread (Windhawk fetches shell32's symbols once).
bool HookSizerSymbols() {
    HMODULE shell = GetModuleHandleW(L"shell32.dll");
    if (!shell) return false;
    WindhawkUtils::SYMBOL_HOOK hooks[] = {
        {{LR"(?_ComputeBoundedSize@CDUISizerElement@@AEAAHH@Z)"}, &ComputeBoundedSize_Original, ComputeBoundedSize_Hook,
         true},
        {{LR"(?GetTargetId@CDUISizerElement@@QEAAGXZ)"}, &SizerTargetId, nullptr, true},
        {{LR"(?GetMinSize@CDUISizerElement@@QEAAHXZ)"}, &SizerMinSize, nullptr, true},
    };
    WH_HOOK_SYMBOLS_OPTIONS options = {};
    options.optionsSize = sizeof(options);
    options.noUndecoratedSymbols = TRUE;
    return WindhawkUtils::HookSymbols(shell, hooks, ARRAYSIZE(hooks), &options);
}

// A layout with the details moved, or empty when it has no details container.
std::wstring BottomDetailsLayout(HMODULE frame, UINT id) {
    HRSRC found = FindResourceW(frame, MAKEINTRESOURCEW(id), L"UIFILE");
    HGLOBAL loaded = found ? LoadResource(frame, found) : nullptr;
    auto data = loaded ? (const char*)LockResource(loaded) : nullptr;
    if (!data) return {};
    std::string xml(data, SizeofResource(frame, found));
    constexpr std::string_view kDetails = R"(<DetailsContainer layoutpos="right"/>)";
    constexpr std::string_view kStatusBar = "<StatusBarModule ";
    size_t details = xml.find(kDetails);
    size_t statusBar = xml.find(kStatusBar);
    if (details == std::string::npos || statusBar == std::string::npos) return {};
    size_t statusBarEnd = xml.find("/>", statusBar);
    if (statusBarEnd == std::string::npos || statusBarEnd > details) return {};
    // The details container goes; both panes go after the status bar, at the
    // window's own level, where Windows 7 had them. Layout 6 (pictures) gets
    // a taller details pane, as Windows 7's did.
    char bottom[sizeof(kBottomDetailsPane) + 8];
    snprintf(bottom, sizeof(bottom), kBottomDetailsPane, id == 6 ? 90 : 53);
    xml.erase(details, kDetails.size());
    xml.insert(statusBarEnd + 2, std::string(bottom) + kRightPreviewPane);
    int length = MultiByteToWideChar(CP_UTF8, 0, xml.data(), (int)xml.size(), nullptr, 0);
    std::wstring wide(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, xml.data(), (int)xml.size(), wide.data(), length);
    return wide;
}

using SetXML_t = HRESULT(WINAPI*)(void* parser, PCWSTR xml, HINSTANCE resources, HINSTANCE controls);
using SetXMLFromResource_t = HRESULT(WINAPI*)(void* parser, PCWSTR name, HINSTANCE resources, HINSTANCE controls);
using SetXMLFromResourceWithTheme_t = HRESULT(WINAPI*)(void* parser, UINT id, HINSTANCE load, HINSTANCE resources,
                                                       HINSTANCE controls);
using SetXMLFromResourceId_t = HRESULT(WINAPI*)(void* parser, UINT id, HINSTANCE resources, HINSTANCE controls);
SetXML_t SetXML;
SetXMLFromResource_t SetXMLFromResource_Original;
SetXMLFromResourceWithTheme_t SetXMLFromResourceWithTheme_Original;
SetXMLFromResourceId_t SetXMLFromResourceId_Original;

// The layout to parse instead of Windows' resource, when there is one. The
// folder layouts are shell32's on current Windows 11 (in its SystemResources
// .mun), explorerframe's on older builds: either is taken. Made once per
// layout, the first time a window asks for it; folder layouts have small
// numbers.
constexpr UINT kMaxLayoutId = 64;
PCWSTR BottomDetailsFor(UINT id, HINSTANCE module) {
    if (!g_bottomDetails || !module || !SetXML || id >= kMaxLayoutId) return nullptr;
    int which = module == GetModuleHandleW(L"shell32.dll") ? 0 : module == GetModuleHandleW(L"explorerframe.dll") ? 1 : -1;
    if (which < 0) return nullptr;
    static SRWLOCK lock = SRWLOCK_INIT;
    static std::wstring made[2][kMaxLayoutId];
    static bool tried[2][kMaxLayoutId];
    AcquireSRWLockExclusive(&lock);
    if (!tried[which][id]) {
        made[which][id] = BottomDetailsLayout(module, id);
        tried[which][id] = true;
    }
    ReleaseSRWLockExclusive(&lock);
    return made[which][id].empty() ? nullptr : made[which][id].c_str();
}

HRESULT WINAPI SetXMLFromResource_Hook(void* parser, PCWSTR name, HINSTANCE resources, HINSTANCE controls) {
    PCWSTR layout = IS_INTRESOURCE(name) ? BottomDetailsFor((UINT)(ULONG_PTR)name, resources) : nullptr;
    if (layout && SUCCEEDED(SetXML(parser, layout, resources, controls))) return S_OK;
    return SetXMLFromResource_Original(parser, name, resources, controls);
}

// The form shell32 loads its folder layouts with: a numbered resource.
HRESULT WINAPI SetXMLFromResourceId_Hook(void* parser, UINT id, HINSTANCE resources, HINSTANCE controls) {
    PCWSTR layout = BottomDetailsFor(id, resources);
    if (layout && SUCCEEDED(SetXML(parser, layout, resources, controls))) return S_OK;
    return SetXMLFromResourceId_Original(parser, id, resources, controls);
}

HRESULT WINAPI SetXMLFromResourceWithTheme_Hook(void* parser, UINT id, HINSTANCE load, HINSTANCE resources,
                                               HINSTANCE controls) {
    PCWSTR layout = BottomDetailsFor(id, load);
    if (layout && SUCCEEDED(SetXML(parser, layout, resources, controls))) return S_OK;
    return SetXMLFromResourceWithTheme_Original(parser, id, load, resources, controls);
}

// dui70 exports its parser's methods, so these need no symbols.
void HookLayouts() {
    HMODULE dui = LoadLibraryExW(L"dui70.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (!dui) return;
    SetXML = (SetXML_t)GetProcAddress(dui, "?SetXML@DUIXmlParser@DirectUI@@QEAAJPEBGPEAUHINSTANCE__@@1@Z");
    auto fromResource =
        GetProcAddress(dui, "?SetXMLFromResource@DUIXmlParser@DirectUI@@QEAAJPEBGPEAUHINSTANCE__@@1@Z");
    auto withTheme =
        GetProcAddress(dui, "?SetXMLFromResourceWithTheme@DUIXmlParser@DirectUI@@QEAAJIPEAUHINSTANCE__@@00@Z");
    auto fromId = GetProcAddress(dui, "?SetXMLFromResource@DUIXmlParser@DirectUI@@QEAAJIPEAUHINSTANCE__@@0@Z");
    if (!SetXML) return;
    if (fromId)
        Wh_SetFunctionHook((void*)fromId, (void*)SetXMLFromResourceId_Hook, (void**)&SetXMLFromResourceId_Original);
    if (fromResource)
        Wh_SetFunctionHook((void*)fromResource, (void*)SetXMLFromResource_Hook, (void**)&SetXMLFromResource_Original);
    if (withTheme)
        Wh_SetFunctionHook((void*)withTheme, (void*)SetXMLFromResourceWithTheme_Hook,
                           (void**)&SetXMLFromResourceWithTheme_Original);
}

////////////////////////////////////////////////////////////////////////////////
// This PC's drives by type. Its groups come from Windows' Drive Type
// Categorizer, which since Windows 8 puts every drive in "Devices and
// drives"; here that group is split the way Windows 7 did. The categorizer
// is a COM object: its methods are found through an instance's vtable.

std::atomic<bool> g_driveGrouping{false};

constexpr DWORD kFolders = 9338, kDevicesAndDrives = 9339, kNetworkLocations = 9340, kRedirected = 9341;
constexpr DWORD kHardDisks = 0x54440001, kRemovable = 0x54440002, kOtherDevices = 0x54440003;

using GetCategory_t = HRESULT(STDMETHODCALLTYPE*)(ICategorizer*, UINT, PCUITEMID_CHILD_ARRAY, DWORD*);
using GetCategoryInfo_t = HRESULT(STDMETHODCALLTYPE*)(ICategorizer*, DWORD, CATEGORY_INFO*);
using CompareCategory_t = HRESULT(STDMETHODCALLTYPE*)(ICategorizer*, CATSORT_FLAGS, DWORD, DWORD);
GetCategory_t GetCategory_Original;
GetCategoryInfo_t GetCategoryInfo_Original;
CompareCategory_t CompareCategory_Original;

// A drive's group, from its item ID in This PC (flags 0x20-0x2F, then "C:\").
DWORD DriveGroup(PCUITEMID_CHILD item) {
    auto bytes = (const BYTE*)item;
    if (!item || item->mkid.cb < 6 || (bytes[2] & 0xF0) != 0x20 || bytes[4] != ':') return kOtherDevices;
    WCHAR root[] = {(WCHAR)bytes[3], L':', L'\\', 0};
    switch (GetDriveTypeW(root)) {
        case DRIVE_FIXED:
        case DRIVE_RAMDISK: return kHardDisks;
        case DRIVE_REMOVABLE:
        case DRIVE_CDROM: return kRemovable;
        case DRIVE_REMOTE: return kNetworkLocations;
        default: return kOtherDevices;
    }
}

HRESULT STDMETHODCALLTYPE GetCategory_Hook(ICategorizer* self, UINT count, PCUITEMID_CHILD_ARRAY items, DWORD* ids) {
    HRESULT hr = GetCategory_Original(self, count, items, ids);
    if (FAILED(hr) || !g_driveGrouping || !items || !ids) return hr;
    for (UINT i = 0; i < count; i++) {
        if (ids[i] == kDevicesAndDrives) ids[i] = DriveGroup(items[i]);
    }
    return hr;
}

HRESULT STDMETHODCALLTYPE GetCategoryInfo_Hook(ICategorizer* self, DWORD id, CATEGORY_INFO* info) {
    PCWSTR name = id == kHardDisks      ? L"Hard Disk Drives"
                  : id == kRemovable    ? L"Devices with Removable Storage"
                  : id == kOtherDevices ? L"Other"
                                        : nullptr;
    if (!name || !info) return GetCategoryInfo_Original(self, id, info);
    info->cif = CATINFO_NORMAL;
    wcsncpy_s(info->wszName, name, _TRUNCATE);
    return S_OK;
}

// Folders first, then drives by kind, then the network.
int GroupRank(DWORD id) {
    switch (id) {
        case kFolders: return 0;
        case kHardDisks: return 1;
        case kRemovable: return 2;
        case kDevicesAndDrives: return 3;
        case kOtherDevices: return 4;
        case kNetworkLocations: return 5;
        case kRedirected: return 6;
        default: return -1;
    }
}

bool OurGroup(DWORD id) {
    return id == kHardDisks || id == kRemovable || id == kOtherDevices;
}

HRESULT STDMETHODCALLTYPE CompareCategory_Hook(ICategorizer* self, CATSORT_FLAGS flags, DWORD a, DWORD b) {
    int rankA = GroupRank(a), rankB = GroupRank(b);
    if (!(OurGroup(a) || OurGroup(b)) || rankA < 0 || rankB < 0) return CompareCategory_Original(self, flags, a, b);
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, (USHORT)(short)(rankA < rankB ? -1 : rankA > rankB ? 1 : 0));
}

// The categorizer is apartment-threaded: made in this thread's own apartment
// so the vtable is the object's, not a proxy's.
bool HookDriveGroups() {
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) return false;
    constexpr CLSID kDriveTypeCategorizer = {
        0xB0A8F3CF, 0x4333, 0x4BAB, {0x88, 0x73, 0x1C, 0xCB, 0x1C, 0xAD, 0xA4, 0x8B}};
    ICategorizer* categorizer = nullptr;
    bool hooked = false;
    if (SUCCEEDED(CoCreateInstance(kDriveTypeCategorizer, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&categorizer)))) {
        void** vtable = *(void***)categorizer;
        Wh_SetFunctionHook(vtable[4], (void*)GetCategory_Hook, (void**)&GetCategory_Original);
        Wh_SetFunctionHook(vtable[5], (void*)GetCategoryInfo_Hook, (void**)&GetCategoryInfo_Original);
        Wh_SetFunctionHook(vtable[6], (void*)CompareCategory_Hook, (void**)&CompareCategory_Original);
        categorizer->Release();
        hooked = true;
    }
    CoUninitialize();
    return hooked;
}

void LoadLayoutSettings() {
    PCWSTR value = Wh_GetStringSetting(L"detailsPane");
    g_bottomDetails = value && !wcscmp(value, L"bottom");
    Wh_FreeStringSetting(value);
    // A preset decides it while chosen: along the bottom only for Remastered 7.
    std::wstring preset = ExplorerPreset();
    if (preset == L"proper11" || preset == L"kinda10" || preset == L"remastered7")
        g_bottomDetails = preset == L"remastered7";
    g_driveGrouping = Wh_GetIntSetting(L"driveGrouping") != 0;
    g_styleBars = Wh_GetIntSetting(L"styleBars") != 0;
}
