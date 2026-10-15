decltype(&ImageList_DrawIndirect) oIl;
BOOL WINAPI hIl(IMAGELISTDRAWPARAMS* p) { wprintf(L"ImageList_DrawIndirect i %d at %d,%d style 0x%x state 0x%x rgbFg %06lx\n", p->i, p->x, p->y, p->fStyle, p->fState, p->rgbFg); return oIl(p); }
decltype(&FillRect) oFill;
int WINAPI hFill(HDC dc, const RECT* r, HBRUSH b) { if (r->right - r->left < 200) { LOGBRUSH lb{}; GetObjectW(b, sizeof(lb), &lb); wprintf(L"FillRect %ld,%ld,%ld,%ld %06lx\n", r->left, r->top, r->right, r->bottom, (DWORD)lb.lbColor); } return oFill(dc, r, b); }
