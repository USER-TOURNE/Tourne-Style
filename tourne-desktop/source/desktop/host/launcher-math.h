// The launcher's own answers that need no window: arithmetic typed into the search line ("2+3*4", "= 17/4",
// "2^10", "sqrt(2)"), and units ("10 km to mi", "72 f in c", "3.5 gb to mb"). Plain C++; tested in
// desktop\tests. No currency: rates would need asking a server, and Tourne doesn't phone anywhere.
#pragma once

#include <string>

namespace td::launchermath {

// The value of an arithmetic expression: + - * / % ^, parentheses, pi, e, and sqrt abs round floor ceil sin cos
// tan log ln. False if it isn't one (a plain number alone isn't either, so typing "7" still searches apps).
bool Evaluate(const std::wstring& text, double* out);

// "N unit to|in unit": the converted amount, written out ("6.2137 mi"). False if the line isn't a conversion.
bool Convert(const std::wstring& text, std::wstring* answer);

// A number as people write it: up to 10 significant digits, no trailing zeros, no exponent for everyday sizes.
std::wstring Number(double v);

}  // namespace td::launchermath
