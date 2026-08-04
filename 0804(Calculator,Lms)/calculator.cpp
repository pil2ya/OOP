#include <windows.h>
#include <string>
#include <cmath>
#include <vector>
#include <sstream>
#include <cwctype>
#include <stdexcept>

// 컨트롤 ID 정의
#define ID_EDIT 100
#define ID_BTN_TOGGLE 101

#define ID_BTN_C    0
#define ID_BTN_BS   1
#define ID_BTN_LPAREN 2
#define ID_BTN_RPAREN 3

#define ID_BTN_7    4
#define ID_BTN_8    5
#define ID_BTN_9    6
#define ID_BTN_DIV  7
#define ID_BTN_4    8
#define ID_BTN_5    9
#define ID_BTN_6    10
#define ID_BTN_MUL  11
#define ID_BTN_1    12
#define ID_BTN_2    13
#define ID_BTN_3    14
#define ID_BTN_MINUS 15
#define ID_BTN_0    16
#define ID_BTN_DOT  17
#define ID_BTN_EQ   18
#define ID_BTN_PLUS 19

#define ID_BTN_SIN 20
#define ID_BTN_COS 21
#define ID_BTN_TAN 22
#define ID_BTN_SQRT 23
#define ID_BTN_POW 24
#define ID_BTN_PI 25

// 전역 변수
HWND hEdit;
HWND hToggleBtn;
HWND hButtons[20];
HWND hEngButtons[6];

bool isEngineeringMode = false;
bool clearOnNextInput = false;

// --- [수식 계산 엔진 시작] ---
double EvaluateExpr(const std::wstring& expr, size_t& idx);

double ParseFactor(const std::wstring& expr, size_t& idx) {
    while (idx < expr.length() && expr[idx] == L' ') idx++;
    if (idx >= expr.length()) return 0;

    if (expr[idx] == L'(') {
        idx++; // '(' 스킵
        double val = EvaluateExpr(expr, idx);
        if (idx < expr.length() && expr[idx] == L')') {
            idx++; // ')' 스킵
        }
        return val;
    }

    if (expr[idx] == L'-') {
        idx++;
        return -ParseFactor(expr, idx);
    }
    if (expr[idx] == L'+') {
        idx++;
        return ParseFactor(expr, idx);
    }

    size_t startIdx = idx;
    bool hasDot = false;
    while (idx < expr.length()) {
        wchar_t c = expr[idx];
        if (iswdigit(c)) {
            idx++;
        }
        else if (c == L'.' && !hasDot) {
            hasDot = true;
            idx++;
        }
        else if (c == L'e' || c == L'E') {
            idx++;
            if (idx < expr.length() && (expr[idx] == L'+' || expr[idx] == L'-')) idx++;
        }
        else {
            break;
        }
    }

    if (startIdx == idx) return 0;

    std::wstring numStr = expr.substr(startIdx, idx - startIdx);
    return _wtof(numStr.c_str());
}

double ParseTerm(const std::wstring& expr, size_t& idx) {
    double val = ParseFactor(expr, idx);
    while (idx < expr.length()) {
        while (idx < expr.length() && expr[idx] == L' ') idx++;
        if (idx < expr.length() && expr[idx] == L'*') {
            idx++;
            val *= ParseFactor(expr, idx);
        }
        else if (idx < expr.length() && expr[idx] == L'/') {
            idx++;
            double divisor = ParseFactor(expr, idx);
            if (divisor == 0) throw std::runtime_error("Division by zero");
            val /= divisor;
        }
        else if (idx < expr.length() && expr[idx] == L'^') {
            idx++;
            val = pow(val, ParseFactor(expr, idx));
        }
        else {
            break;
        }
    }
    return val;
}

double EvaluateExpr(const std::wstring& expr, size_t& idx) {
    double val = ParseTerm(expr, idx);
    while (idx < expr.length()) {
        while (idx < expr.length() && expr[idx] == L' ') idx++;
        if (idx < expr.length() && expr[idx] == L'+') {
            idx++;
            val += ParseTerm(expr, idx);
        }
        else if (idx < expr.length() && expr[idx] == L'-') {
            idx++;
            val -= ParseTerm(expr, idx);
        }
        else {
            break;
        }
    }
    return val;
}

// 전처리: 암묵적 곱셈 및 π 처리
std::wstring PreprocessExpression(const std::wstring& rawExpr) {
    // 1단계: 숫자 바로 뒤에 π가 오는 경우 사이에 * 추가 (예: 12π -> 12*π)
    std::wstring step1 = L"";
    for (size_t i = 0; i < rawExpr.length(); ++i) {
        step1 += rawExpr[i];
        if (i + 1 < rawExpr.length()) {
            wchar_t curr = rawExpr[i];
            wchar_t next = rawExpr[i + 1];
            if (iswdigit(curr) && next == L'π') {
                step1 += L'*';
            }
        }
    }

    // 2단계: 'π'를 실제 값으로 치환
    std::wstring expanded = L"";
    for (size_t i = 0; i < step1.length(); ++i) {
        if (step1[i] == L'π') {
            expanded += L"3.141592653589793";
        }
        else {
            expanded += step1[i];
        }
    }

    // 3단계: 괄호 및 숫자 간의 암묵적 곱셈(*) 자동 삽입 (2(5), (2)5, (2)(3) 등)
    std::wstring processed = L"";
    for (size_t i = 0; i < expanded.length(); ++i) {
        processed += expanded[i];
        if (i + 1 < expanded.length()) {
            wchar_t curr = expanded[i];
            wchar_t next = expanded[i + 1];

            if ((iswdigit(curr) && next == L'(') ||
                (curr == L')' && iswdigit(next)) ||
                (curr == L')' && next == L'(')) {
                processed += L'*';
            }
        }
    }
    return processed;
}

double CalculateExpression(const wchar_t* wExpr) {
    try {
        std::wstring rawExpr(wExpr);
        std::wstring expr = PreprocessExpression(rawExpr);
        size_t idx = 0;
        return EvaluateExpr(expr, idx);
    }
    catch (...) {
        return 0;
    }
}
// --- [수식 계산 엔진 끝] ---

// 레이아웃 배치 함수
void UpdateLayout(HWND hwnd) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int margin = 15;
    int spacing = 8;

    MoveWindow(hEdit, margin, margin, width - (margin * 2), 40, TRUE);
    MoveWindow(hToggleBtn, margin, 65, width - (margin * 2), 30, TRUE);

    int topOffset = 105;
    int availableWidth = width - (margin * 2) - (spacing * 3);
    int normalAvailableHeight = height - topOffset - margin - (spacing * 4);

    if (isEngineeringMode) {
        int btnWidth = (width - (margin * 2) - (spacing * 5)) / 6;
        int btnHeight = normalAvailableHeight / 5;

        for (int i = 0; i < 20; ++i) {
            int row = i / 4;
            int col = i % 4;
            int x = margin + col * (btnWidth + spacing);
            int y = topOffset + row * (btnHeight + spacing);
            MoveWindow(hButtons[i], x, y, btnWidth, btnHeight, TRUE);
            ShowWindow(hButtons[i], SW_SHOW);
        }

        int engX = margin + 4 * (btnWidth + spacing);
        int engHeight = normalAvailableHeight / 6;
        for (int i = 0; i < 6; ++i) {
            int y = topOffset + i * (engHeight + (spacing / 2));
            MoveWindow(hEngButtons[i], engX, y, (btnWidth * 2) + spacing, engHeight, TRUE);
            ShowWindow(hEngButtons[i], SW_SHOW);
        }
    }
    else {
        int btnWidth = availableWidth / 4;
        int btnHeight = normalAvailableHeight / 5;

        for (int i = 0; i < 20; ++i) {
            int row = i / 4;
            int col = i % 4;
            int x = margin + col * (btnWidth + spacing);
            int y = topOffset + row * (btnHeight + spacing);
            MoveWindow(hButtons[i], x, y, btnWidth, btnHeight, TRUE);
            ShowWindow(hButtons[i], SW_SHOW);
        }

        for (int i = 0; i < 6; ++i) {
            ShowWindow(hEngButtons[i], SW_HIDE);
        }
    }
}

// 윈도우 프로시저
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        hEdit = CreateWindowW(L"EDIT", L"0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
            15, 15, 310, 40, hwnd, (HMENU)ID_EDIT, NULL, NULL);

        hToggleBtn = CreateWindowW(L"BUTTON", L"공학용 ▶",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            15, 65, 310, 30, hwnd, (HMENU)ID_BTN_TOGGLE, NULL, NULL);

        const wchar_t* btnLabels[20] = {
            L"C",  L"BS", L"(",  L")",
            L"7",  L"8",  L"9",  L"/",
            L"4",  L"5",  L"6",  L"*",
            L"1",  L"2",  L"3",  L"-",
            L"0",  L".",  L"=",  L"+"
        };
        int ids[20] = {
            ID_BTN_C, ID_BTN_BS, ID_BTN_LPAREN, ID_BTN_RPAREN,
            ID_BTN_7, ID_BTN_8, ID_BTN_9, ID_BTN_DIV,
            ID_BTN_4, ID_BTN_5, ID_BTN_6, ID_BTN_MUL,
            ID_BTN_1, ID_BTN_2, ID_BTN_3, ID_BTN_MINUS,
            ID_BTN_0, ID_BTN_DOT, ID_BTN_EQ, ID_BTN_PLUS
        };
        for (int i = 0; i < 20; ++i) {
            hButtons[i] = CreateWindowW(L"BUTTON", btnLabels[i],
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)ids[i], NULL, NULL);
        }

        const wchar_t* engLabels[6] = { L"sin", L"cos", L"tan", L"√", L"x^y", L"π" };
        int engIds[6] = { ID_BTN_SIN, ID_BTN_COS, ID_BTN_TAN, ID_BTN_SQRT, ID_BTN_POW, ID_BTN_PI };
        for (int i = 0; i < 6; ++i) {
            hEngButtons[i] = CreateWindowW(L"BUTTON", engLabels[i],
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                0, 0, 0, 0, hwnd, (HMENU)(INT_PTR)engIds[i], NULL, NULL);
            ShowWindow(hEngButtons[i], SW_HIDE);
        }
        break;
    }
    case WM_SIZE: {
        UpdateLayout(hwnd);
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wp);
        wchar_t buf[256];

        if (id == ID_BTN_TOGGLE) {
            isEngineeringMode = !isEngineeringMode;
            RECT rc;
            GetWindowRect(hwnd, &rc);
            int newWidth = isEngineeringMode ? 560 : 350;
            SetWindowPos(hwnd, NULL, rc.left, rc.top, newWidth, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER);

            if (isEngineeringMode) {
                SetWindowTextW(hToggleBtn, L"일반용 ◀");
            }
            else {
                SetWindowTextW(hToggleBtn, L"공학용 ▶");
            }
            UpdateLayout(hwnd);
        }
        else if (id == ID_BTN_C) {
            SetWindowTextW(hEdit, L"0");
            clearOnNextInput = false;
        }
        else if (id == ID_BTN_BS) {
            GetWindowTextW(hEdit, buf, 256);
            int len = wcslen(buf);
            if (len > 1 && wcscmp(buf, L"Error") != 0) {
                buf[len - 1] = L'\0';
                SetWindowTextW(hEdit, buf);
            }
            else {
                SetWindowTextW(hEdit, L"0");
            }
        }
        else {
            const wchar_t* inputStr = NULL;
            bool allowChain = false;

            if (id >= ID_BTN_7 && id <= ID_BTN_9) {
                wchar_t temp[2] = { (wchar_t)(L'7' + (id - ID_BTN_7)), L'\0' };
                inputStr = temp;
            }
            else if (id >= ID_BTN_4 && id <= ID_BTN_6) {
                wchar_t temp[2] = { (wchar_t)(L'4' + (id - ID_BTN_4)), L'\0' };
                inputStr = temp;
            }
            else if (id >= ID_BTN_1 && id <= ID_BTN_3) {
                wchar_t temp[2] = { (wchar_t)(L'1' + (id - ID_BTN_1)), L'\0' };
                inputStr = temp;
            }
            else if (id == ID_BTN_0) {
                inputStr = L"0";
            }
            else if (id == ID_BTN_DOT) {
                inputStr = L".";
            }
            else if (id == ID_BTN_LPAREN) {
                inputStr = L"("; allowChain = true;
            }
            else if (id == ID_BTN_RPAREN) {
                inputStr = L")"; allowChain = true;
            }
            else if (id == ID_BTN_PLUS) {
                inputStr = L"+"; allowChain = true;
            }
            else if (id == ID_BTN_MINUS) {
                inputStr = L"-"; allowChain = true;
            }
            else if (id == ID_BTN_MUL) {
                inputStr = L"*"; allowChain = true;
            }
            else if (id == ID_BTN_DIV) {
                inputStr = L"/"; allowChain = true;
            }
            else if (id == ID_BTN_POW) {
                inputStr = L"^"; allowChain = true;
            }
            else if (id == ID_BTN_PI) {
                inputStr = L"π"; allowChain = true;
            }

            if (inputStr != NULL) {
                GetWindowTextW(hEdit, buf, 256);

                if (clearOnNextInput) {
                    if (allowChain) {
                        clearOnNextInput = false;
                    }
                    else {
                        SetWindowTextW(hEdit, L"");
                        buf[0] = L'\0';
                        clearOnNextInput = false;
                    }
                }

                if (wcscmp(buf, L"0") == 0 || wcscmp(buf, L"Error") == 0) {
                    SetWindowTextW(hEdit, inputStr);
                }
                else {
                    wcscat_s(buf, 256, inputStr);
                    SetWindowTextW(hEdit, buf);
                }
            }
            else if (id == ID_BTN_EQ) {
                GetWindowTextW(hEdit, buf, 256);
                double result = CalculateExpression(buf);
                swprintf_s(buf, L"%g", result);
                SetWindowTextW(hEdit, buf);
                clearOnNextInput = true;
            }
            else if (id == ID_BTN_SQRT) {
                GetWindowTextW(hEdit, buf, 256);
                double val = CalculateExpression(buf);
                if (val < 0) SetWindowTextW(hEdit, L"Error");
                else {
                    swprintf_s(buf, L"%g", sqrt(val));
                    SetWindowTextW(hEdit, buf);
                }
                clearOnNextInput = true;
            }
            else if (id == ID_BTN_SIN || id == ID_BTN_COS || id == ID_BTN_TAN) {
                GetWindowTextW(hEdit, buf, 256);
                double val = CalculateExpression(buf);
                double rad = val * 3.141592653589793 / 180.0;
                double res = 0;
                if (id == ID_BTN_SIN) res = sin(rad);
                else if (id == ID_BTN_COS) res = cos(rad);
                else if (id == ID_BTN_TAN) res = tan(rad);

                swprintf_s(buf, L"%g", res);
                SetWindowTextW(hEdit, buf);
                clearOnNextInput = true;
            }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"EngCalculatorClass";

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(L"EngCalculatorClass", L"스마트 공학용 계산기",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 350, 520,
        NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}