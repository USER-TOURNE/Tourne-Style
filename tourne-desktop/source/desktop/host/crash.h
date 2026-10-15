// When Tourne Desktop crashes: a report, and a way to start without the part
// that crashed it.
//
// Before a part starts (a module, the theme, app styles, glass), its name is
// kept in the registry until it has run for a few seconds. If the program
// dies in that time, the next start leaves that part off and says so, until a
// setting is saved. Any crash also writes crash.txt (in %LOCALAPPDATA%, under
// Tourne, Desktop): the error, where it happened (each frame as module + offset, read
// against the build's TourneDesktop.symbols.exe), what was starting, and the last log lines,
// then says where the file is.
#pragma once

#include <string>

namespace td {

void CrashGuardInstall(const std::wstring& folder);  // first thing: crashes are reported from here on
void CrashGuardBegin();                    // the running copy only: reads what the last run died starting
void CrashNote(const std::wstring& line);  // a log line, kept for the report
void CrashStarting(const std::wstring& part, const std::wstring& title);  // part: "module:taskbar", "glass"...
void CrashStarted();                       // (a few seconds after: the parts started settle)
std::wstring CrashLeftOff();               // the part the last run died starting ("" if none)
std::wstring CrashLeftOffTitle();
void CrashTryAgain();                      // a setting was saved: try that part again next time
bool CrashLeavesOff(const std::wstring& part);

}  // namespace td
