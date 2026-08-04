#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

#pragma comment(lib, "comctl32.lib")

class Book {
private:
    int id;
    wstring title;
    wstring author;
    bool isBorrowed;

public:
    Book(int id, wstring title, wstring author, bool isBorrowed = false)
        : id(id), title(title), author(author), isBorrowed(isBorrowed) {
    }

    int getId() const { return id; }
    wstring getTitle() const { return title; }
    wstring getAuthor() const { return author; }
    bool getIsBorrowed() const { return isBorrowed; }

    void setBorrowed(bool status) { isBorrowed = status; }
};

vector<Book> books;
vector<size_t> displayedIndices;
constexpr wchar_t kFilename[] = L"library.txt";

HWND hListView, hEditId, hEditTitle, hEditAuthor, hEditSearch;
HFONT hFont;

constexpr int ID_BTN_ADD = 101;
constexpr int ID_BTN_DELETE = 102;
constexpr int ID_BTN_BORROW = 103;
constexpr int ID_BTN_RETURN = 104;
constexpr int ID_BTN_SEARCH = 105;
constexpr int ID_BTN_RESET = 106;
constexpr int ID_BTN_DELETE_ALL = 107;

const wchar_t* kSearchPlaceholder = L"검색할 도서 제목을 입력하세요...";

string WStringToString(const wstring& wstr) {
    if (wstr.empty()) return string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

wstring StringToWString(const string& str) {
    if (str.empty()) return wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void LoadFromFile() {
    ifstream file(WStringToString(kFilename));
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string idStr, titleUtf8, authorUtf8, borrowedStr;
        if (getline(ss, idStr, ',') && getline(ss, titleUtf8, ',') &&
            getline(ss, authorUtf8, ',') && getline(ss, borrowedStr)) {
            books.push_back(Book(stoi(idStr), StringToWString(titleUtf8), StringToWString(authorUtf8), stoi(borrowedStr)));
        }
    }
    file.close();
}

void SaveToFile() {
    ofstream file(WStringToString(kFilename));
    if (!file.is_open()) return;

    for (const auto& book : books) {
        file << book.getId() << ","
            << WStringToString(book.getTitle()) << ","
            << WStringToString(book.getAuthor()) << ","
            << book.getIsBorrowed() << "\n";
    }
    file.close();
}

void RefreshListView(const vector<size_t>& indicesToDisplay) {
    ListView_DeleteAllItems(hListView);
    displayedIndices = indicesToDisplay;

    for (size_t i = 0; i < displayedIndices.size(); ++i) {
        const Book& book = books[displayedIndices[i]];

        wchar_t idBuf[32];
        swprintf_s(idBuf, L"%d", book.getId());

        wstring titleStr = book.getTitle();
        wstring authorStr = book.getAuthor();
        wstring statusStr = book.getIsBorrowed() ? L"대출 중" : L"대출 가능";

        LVITEMW lvItem = { 0 };
        lvItem.mask = LVIF_TEXT;
        lvItem.iItem = (int)i;
        lvItem.iSubItem = 0;
        lvItem.pszText = idBuf;

        int rowIndex = (int)SendMessageW(hListView, LVM_INSERTITEMW, 0, (LPARAM)&lvItem);
        if (rowIndex == -1) continue;

        ListView_SetItemText(hListView, rowIndex, 1, (LPWSTR)titleStr.c_str());
        ListView_SetItemText(hListView, rowIndex, 2, (LPWSTR)authorStr.c_str());
        ListView_SetItemText(hListView, rowIndex, 3, (LPWSTR)statusStr.c_str());
    }
}

void RefreshAll() {
    vector<size_t> allIndices(books.size());
    for (size_t i = 0; i < books.size(); ++i) allIndices[i] = i;
    RefreshListView(allIndices);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        INITCOMMONCONTROLSEX icex = { 0 };
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC = ICC_LISTVIEW_CLASSES;
        InitCommonControlsEx(&icex);

        hFont = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Malgun Gothic");

        // 1. 리스트뷰 (y=15, 높이 210, 폭 660)
        hListView = CreateWindowExW(0, WC_LISTVIEWW, L"",
            WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
            20, 15, 660, 210, hwnd, (HMENU)(INT_PTR)1, NULL, NULL);

        SendMessageW(hListView, CCM_SETUNICODEFORMAT, TRUE, 0);
        SendMessageW(hListView, LVM_SETEXTENDEDLISTVIEWSTYLE,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        SendMessageW(hListView, WM_SETFONT, (WPARAM)hFont, TRUE);

        LVCOLUMNW lvc = { 0 };
        lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        lvc.iSubItem = 0; lvc.pszText = (LPWSTR)L"도서 번호"; lvc.cx = 110;
        SendMessageW(hListView, LVM_INSERTCOLUMNW, 0, (LPARAM)&lvc);
        lvc.iSubItem = 1; lvc.pszText = (LPWSTR)L"제목"; lvc.cx = 230;
        SendMessageW(hListView, LVM_INSERTCOLUMNW, 1, (LPARAM)&lvc);
        lvc.iSubItem = 2; lvc.pszText = (LPWSTR)L"저자"; lvc.cx = 190;
        SendMessageW(hListView, LVM_INSERTCOLUMNW, 2, (LPARAM)&lvc);
        lvc.iSubItem = 3; lvc.pszText = (LPWSTR)L"상태"; lvc.cx = 130;
        SendMessageW(hListView, LVM_INSERTCOLUMNW, 3, (LPARAM)&lvc);

        // 2. 입력 폼 영역 (y = 240)
        HWND hSt1 = CreateWindowW(L"STATIC", L"도서 번호:", WS_CHILD | WS_VISIBLE, 20, 240, 65, 20, hwnd, NULL, NULL, NULL);
        SendMessageW(hSt1, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditId = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 90, 240, 70, 24, hwnd, NULL, NULL, NULL);
        SendMessageW(hEditId, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(hEditId, EM_LIMITTEXT, 10, 0);

        HWND hSt2 = CreateWindowW(L"STATIC", L"제목:", WS_CHILD | WS_VISIBLE, 178, 240, 32, 20, hwnd, NULL, NULL, NULL);
        SendMessageW(hSt2, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditTitle = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 215, 240, 200, 24, hwnd, NULL, NULL, NULL);
        SendMessageW(hEditTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(hEditTitle, EM_LIMITTEXT, 100, 0);

        HWND hSt3 = CreateWindowW(L"STATIC", L"저자:", WS_CHILD | WS_VISIBLE, 433, 240, 32, 20, hwnd, NULL, NULL, NULL);
        SendMessageW(hSt3, WM_SETFONT, (WPARAM)hFont, TRUE);
        hEditAuthor = CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 470, 240, 210, 24, hwnd, NULL, NULL, NULL);
        SendMessageW(hEditAuthor, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(hEditAuthor, EM_LIMITTEXT, 100, 0);

        // 3. 도서 등록 버튼 (y = 275)
        HWND hBtnAdd = CreateWindowW(L"BUTTON", L"도서 등록", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 275, 660, 30, hwnd, (HMENU)(INT_PTR)ID_BTN_ADD, NULL, NULL);
        SendMessageW(hBtnAdd, WM_SETFONT, (WPARAM)hFont, TRUE);

        // 4. 검색 영역 (y = 318)
        hEditSearch = CreateWindowW(L"EDIT", kSearchPlaceholder, WS_CHILD | WS_VISIBLE | WS_BORDER, 20, 318, 460, 26, hwnd, NULL, NULL, NULL);
        SendMessageW(hEditSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessageW(hEditSearch, EM_LIMITTEXT, 100, 0);

        HWND hBtnSearch = CreateWindowW(L"BUTTON", L"검색", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 490, 318, 90, 26, hwnd, (HMENU)(INT_PTR)ID_BTN_SEARCH, NULL, NULL);
        SendMessageW(hBtnSearch, WM_SETFONT, (WPARAM)hFont, TRUE);
        HWND hBtnReset = CreateWindowW(L"BUTTON", L"전체보기", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 590, 318, 90, 26, hwnd, (HMENU)(INT_PTR)ID_BTN_RESET, NULL, NULL);
        SendMessageW(hBtnReset, WM_SETFONT, (WPARAM)hFont, TRUE);

        // 5. 하단 기능 버튼 4개 (y = 358)
        HWND hBtnBorrow = CreateWindowW(L"BUTTON", L"도서 대출", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 20, 358, 150, 34, hwnd, (HMENU)(INT_PTR)ID_BTN_BORROW, NULL, NULL);
        SendMessageW(hBtnBorrow, WM_SETFONT, (WPARAM)hFont, TRUE);
        HWND hBtnReturn = CreateWindowW(L"BUTTON", L"도서 반납", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 190, 358, 150, 34, hwnd, (HMENU)(INT_PTR)ID_BTN_RETURN, NULL, NULL);
        SendMessageW(hBtnReturn, WM_SETFONT, (WPARAM)hFont, TRUE);
        HWND hBtnDelete = CreateWindowW(L"BUTTON", L"도서 삭제", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 360, 358, 150, 34, hwnd, (HMENU)(INT_PTR)ID_BTN_DELETE, NULL, NULL);
        SendMessageW(hBtnDelete, WM_SETFONT, (WPARAM)hFont, TRUE);
        HWND hBtnDeleteAll = CreateWindowW(L"BUTTON", L"전체 삭제", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 530, 358, 150, 34, hwnd, (HMENU)(INT_PTR)ID_BTN_DELETE_ALL, NULL, NULL);
        SendMessageW(hBtnDeleteAll, WM_SETFONT, (WPARAM)hFont, TRUE);

        LoadFromFile();
        RefreshAll();
        break;
    }
    case WM_COMMAND: {
        int id = LOWORD(wParam);
        HWND hControl = (HWND)lParam;

        if (hControl == hEditSearch) {
            if (HIWORD(wParam) == EN_SETFOCUS) {
                wchar_t buf[256] = { 0 };
                GetWindowTextW(hEditSearch, buf, 256);
                if (wcscmp(buf, kSearchPlaceholder) == 0) {
                    SetWindowTextW(hEditSearch, L"");
                }
            }
            else if (HIWORD(wParam) == EN_KILLFOCUS) {
                wchar_t buf[256] = { 0 };
                GetWindowTextW(hEditSearch, buf, 256);
                if (wcslen(buf) == 0) {
                    SetWindowTextW(hEditSearch, kSearchPlaceholder);
                }
            }
        }

        if (id == ID_BTN_ADD) {
            wchar_t idBuf[64] = { 0 };
            wchar_t titleBuf[256] = { 0 };
            wchar_t authorBuf[256] = { 0 };

            GetWindowTextW(hEditId, idBuf, 64);
            GetWindowTextW(hEditTitle, titleBuf, 256);
            GetWindowTextW(hEditAuthor, authorBuf, 256);

            if (wcslen(idBuf) == 0 || wcslen(titleBuf) == 0 || wcslen(authorBuf) == 0) {
                MessageBoxW(hwnd, L"모든 정보를 입력해주세요.", L"경고", MB_OK | MB_ICONWARNING);
                break;
            }

            int bookId = stoi(idBuf);
            for (const auto& b : books) {
                if (b.getId() == bookId) {
                    MessageBoxW(hwnd, L"이미 존재하는 도서 번호입니다.", L"경고", MB_OK | MB_ICONWARNING);
                    return 0;
                }
            }

            books.push_back(Book(bookId, titleBuf, authorBuf));
            SaveToFile();
            RefreshAll();

            SetWindowTextW(hEditId, L"");
            SetWindowTextW(hEditTitle, L"");
            SetWindowTextW(hEditAuthor, L"");
            SetFocus(hEditId);
            MessageBoxW(hwnd, L"도서가 등록되었습니다.", L"성공", MB_OK | MB_ICONINFORMATION);
        }
        else if (id == ID_BTN_SEARCH) {
            wchar_t searchBuf[256] = { 0 };
            GetWindowTextW(hEditSearch, searchBuf, 256);
            wstring keyword(searchBuf);

            if (keyword.empty() || keyword == kSearchPlaceholder) {
                RefreshAll();
                break;
            }

            vector<size_t> filteredIndices;
            for (size_t i = 0; i < books.size(); ++i) {
                if (books[i].getTitle().find(keyword) != wstring::npos) {
                    filteredIndices.push_back(i);
                }
            }
            RefreshListView(filteredIndices);
        }
        else if (id == ID_BTN_RESET) {
            SetWindowTextW(hEditSearch, kSearchPlaceholder);
            RefreshAll();
        }
        else if (id == ID_BTN_DELETE_ALL) {
            if (books.empty()) {
                MessageBoxW(hwnd, L"삭제할 도서가 없습니다.", L"알림", MB_OK | MB_ICONINFORMATION);
                break;
            }

            int result = MessageBoxW(hwnd, L"정말 모든 도서를 삭제하시겠습니까?", L"경고", MB_YESNO | MB_ICONWARNING);
            if (result == IDYES) {
                books.clear();
                SaveToFile();
                RefreshAll();
                MessageBoxW(hwnd, L"모든 도서가 삭제되었습니다.", L"성공", MB_OK | MB_ICONINFORMATION);
            }
        }
        else if (id == ID_BTN_BORROW || id == ID_BTN_RETURN || id == ID_BTN_DELETE) {
            int selectedIndex = (int)SendMessageW(hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
            if (selectedIndex == -1 || selectedIndex >= (int)displayedIndices.size()) {
                MessageBoxW(hwnd, L"목록에서 도서를 선택해주세요.", L"경고", MB_OK | MB_ICONWARNING);
                break;
            }

            size_t bookRealIndex = displayedIndices[selectedIndex];
            Book& targetBook = books[bookRealIndex];

            if (id == ID_BTN_BORROW) {
                if (targetBook.getIsBorrowed()) {
                    MessageBoxW(hwnd, L"이미 대출 중인 도서입니다.", L"알림", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    targetBook.setBorrowed(true);
                    SaveToFile();
                    vector<size_t> currentIndices = displayedIndices;
                    RefreshListView(currentIndices);
                    MessageBoxW(hwnd, L"대출이 완료되었습니다.", L"성공", MB_OK | MB_ICONINFORMATION);
                }
            }
            else if (id == ID_BTN_RETURN) {
                if (!targetBook.getIsBorrowed()) {
                    MessageBoxW(hwnd, L"대출 중이 아닌 도서입니다.", L"알림", MB_OK | MB_ICONINFORMATION);
                }
                else {
                    targetBook.setBorrowed(false);
                    SaveToFile();
                    vector<size_t> currentIndices = displayedIndices;
                    RefreshListView(currentIndices);
                    MessageBoxW(hwnd, L"반납이 완료되었습니다.", L"성공", MB_OK | MB_ICONINFORMATION);
                }
            }
            else if (id == ID_BTN_DELETE) {
                books.erase(books.begin() + bookRealIndex);
                SaveToFile();
                RefreshAll();
                MessageBoxW(hwnd, L"도서가 삭제되었습니다.", L"성공", MB_OK | MB_ICONINFORMATION);
            }
        }
        break;
    }
    case WM_DESTROY:
        if (hFont) DeleteObject(hFont);
        SaveToFile();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    const wchar_t* CLASS_NAME = L"LibraryAppClass";

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = CLASS_NAME;

    RegisterClassExW(&wc);

    // 창 전체 크기를 700x410에 맞게 조정 (타이틀바 여유 공간 계산)
    RECT rc = { 0, 0, 700, 410 };
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"도서관 관리 프로그램",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
        NULL, NULL, hInstance, NULL
    );

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { 0 };
    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_TAB) {
            HWND hFocus = GetFocus();
            if (hFocus == hEditId) {
                SetFocus(hEditTitle);
                continue;
            }
            else if (hFocus == hEditTitle) {
                SetFocus(hEditAuthor);
                continue;
            }
            else if (hFocus == hEditAuthor) {
                SetFocus(hEditId);
                continue;
            }
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}