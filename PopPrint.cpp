// PopPrint.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "PopPrint.h"
#include <commdlg.h>

BOOL bUserAbort;
HWND hDlgPrint;

BOOL CALLBACK PrintDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_CLOSE, MF_GRAYED);
		return TRUE;

	case WM_COMMAND:
		bUserAbort = TRUE;
		EnableWindow(GetParent(hDlg), TRUE);
		DestroyWindow(hDlg);
		hDlgPrint = NULL;
		return TRUE;
	}
	return FALSE;
}
BOOL CALLBACK AbortProc(HDC hPrinterDC, int iCode)
{
	MSG msg;
	while (!bUserAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (!hDlgPrint || !IsDialogMessage(hDlgPrint, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return !bUserAbort;
}
BOOL PopPrntPrintFile(HINSTANCE hInst, HWND hwnd, HWND hwndEdit, PTSTR szTitleName)
{
	static DOCINFO di = { sizeof(DOCINFO) };
	static PRINTDLG pd;
	BOOL bSuccess;
	int yChar, iCharsPerLine, iLinesPerPage, iTotalLines,
		iTotalPages, iPage, iLine, iLineNum;
	PTSTR pstrBuffer;
	TCHAR szJobName[64 + MAX_PATH];
	TEXTMETRIC tm;
	WORD iColCopy, iNoiColCopy;
	// Invoke Print common dialog box

	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = hwnd;
	pd.hDevMode = NULL;
	pd.hDevNames = NULL;
	pd.hDC = NULL;
	pd.Flags = PD_ALLPAGES | PD_COLLATE |
		PD_RETURNDC | PD_NOSELECTION;
	pd.nFromPage = 0;
	pd.nToPage = 0;
	pd.nMinPage = 0;
	pd.nMaxPage = 0;
	pd.nCopies = 1;
	pd.hInstance = NULL;
	pd.lCustData = 0L;
	pd.lpfnPrintHook = NULL;
	pd.lpfnSetupHook = NULL;
	pd.lpPrintTemplateName = NULL;
	pd.lpSetupTemplateName = NULL;
	pd.hPrintTemplate = NULL;
	pd.hSetupTemplate = NULL;

	if (!PrintDlg(&pd))
		return TRUE;

	if (0 == (iTotalLines = SendMessage(hwndEdit, EM_GETLINECOUNT, 0, 0)))
		return TRUE;
	// Calculate necessary metrics for file

	GetTextMetrics(pd.hDC, &tm);
	yChar = tm.tmHeight + tm.tmExternalLeading;

	iCharsPerLine = GetDeviceCaps(pd.hDC, HORZRES) / tm.tmAveCharWidth;
	iLinesPerPage = GetDeviceCaps(pd.hDC, VERTRES) / yChar;
	iTotalPages = (iTotalLines + iLinesPerPage - 1) / iLinesPerPage;
	// Allocate a buffer for each line of text

	pstrBuffer = (PTSTR)malloc(sizeof(TCHAR) * (iCharsPerLine + 1));
	// Display the printing dialog box

	EnableWindow(hwnd, FALSE);
	bSuccess = TRUE;
	bUserAbort = FALSE;
	hDlgPrint = CreateDialog(hInst, TEXT("PrintDlgBox"), hwnd, PrintDlgProc);
	SetDlgItemText(hDlgPrint, IDC_FILENAME, szTitleName);
	SetAbortProc(pd.hDC, AbortProc);
	// Start the document
	GetWindowText(hwnd, szJobName, sizeof(szJobName));
	di.lpszDocName = szJobName;
	if (StartDoc(pd.hDC, &di) > 0)
	{
		// Collation requires this loop and iNoiColCopy
		for (iColCopy = 0; iColCopy < ((WORD)pd.Flags & PD_COLLATE ? pd.nCopies : 1); ++iColCopy)
		{
			for (iPage = 0; iPage < iTotalPages; iPage++)
			{
				for (iNoiColCopy = 0;
					iNoiColCopy < (pd.Flags & PD_COLLATE ? 1 : pd.nCopies);
					iNoiColCopy++)
				{
					// Start the page
					if (StartPage(pd.hDC) < 0)
					{
						bSuccess =
							FALSE;
						break;
					}
					// For each page, print the lines
					for (iLine = 0; iLine < iLinesPerPage; iLine++)
					{
						iLineNum = iLinesPerPage * iPage + iLine;
						if (iLineNum > iTotalLines)
							break;
						*(int *)pstrBuffer = iCharsPerLine;
						TextOut(pd.hDC, 0, yChar * iLine, pstrBuffer,
							(int)SendMessage(hwndEdit, EM_GETLINE,
							(WPARAM)iLineNum, (LPARAM)pstrBuffer));
					}

					if (EndPage(pd.hDC) < 0)
					{
						bSuccess = FALSE;
						break;
					}

					if (bUserAbort)
						break;
				}

				if (!bSuccess || bUserAbort)
					break;
			}

			if (!bSuccess || bUserAbort)
				break;
		}
	}
	else
		bSuccess = FALSE;
	if (bSuccess)
		EndDoc(pd.hDC);

	if (!bUserAbort)
	{
		EnableWindow(hwnd, TRUE);
		DestroyWindow(hDlgPrint);
	}

	free(pstrBuffer);
	DeleteDC(pd.hDC);

	return bSuccess && !bUserAbort;
}


#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_POPPRINT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_POPPRINT));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_POPPRINT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_POPPRINT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
