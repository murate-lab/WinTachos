// WinTachos.cpp : アプリケーション用のエントリ ポイントの定義
//

#include "stdafx.h"
#include "WinTachos.h"

// グローバル変数:
HINSTANCE		hInst;							// 現在のインスタンス
TCHAR			szTitle[MAX_LOADSTRING];		// タイトル バー テキスト
TCHAR			szWindowClass[MAX_LOADSTRING];	// タイトル バー テキスト
HRGN			m_hBaseRgn;						// メーターのベースのリージョン
float			m_fSpeed;						// スピード
float			m_fSpeedDisp;					// スピード表示値
float			m_fSpeedLog[LOGMAX];			// スピード過去の値
float			m_fTacho;						// 回転数
float			m_fTachoDisp;					// 回転数表示値
float			m_fTachoLog[LOGMAX];			// 回転数過去の値
int				m_iLogPos;						// 現在の記録個所
PNOTIFYICONDATA	m_lpni;							// タスクトレイアイコン
WNDPROC			oldLinkProc1 = NULL;			// ＵＲＬのリンク
WNDPROC			oldLinkProc2 = NULL;			// メールのリンク
HFONT			hFontLink;
HCURSOR			hCurHand;						// リンクカーソル
ULONG_PTR		gdiplusToken;

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR     lpCmdLine,
					 int       nCmdShow )
{
	// TODO: この位置にコードを記述してください。
	MSG msg;
	HACCEL hAccelTable;

	// グローバル ストリングを初期化します
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINTACHOS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass( hInstance );

	// アプリケーションの初期化を行います:
	if( !InitInstance( hInstance, nCmdShow ) ) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_WINTACHOS);

	hCurHand = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_HANDP));

	// メイン メッセージ ループ:
	while( GetMessage(&msg, NULL, 0, 0) ) 
	{
		if( !TranslateAccelerator (msg.hwnd, hAccelTable, &msg) ) 
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}

	return msg.wParam;
}

//  関数: MyRegisterClass()
//  用途: ウィンドウ クラスの登録
ATOM MyRegisterClass( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_WINTACHOS);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;//(LPCSTR)IDC_WINTACHOS;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx( &wcex );
}

//   関数: InitInstance(HANDLE, int)
//   用途: インスタンス ハンドルの保存とメイン ウィンドウの作成
//   コメント:
//        この関数では、インスタンス ハンドルをグローバル変数に保存し、プログラムの
//        メイン ウィンドウを作成し表示します。
BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
	HWND hWnd;

	hInst = hInstance; // グローバル変数にインスタンス ハンドルを保存します

	// クラス生成
	m_NeedleInfo = new CWTNeedle[2];

	// メーター値初期化
	m_fSpeed = 0.0f;
	m_fTacho = 0.0f;
	m_fSpeedDisp = 0.0f;
	m_fTachoDisp = 0.0f;
	m_iLogPos = 0;
	for (int i = 0; i < LOGMAX; i++) {
		m_fSpeedLog[i] = 0.0f;
		m_fTachoLog[i] = 0.0f;
	}

	// 設定デフォルト値設定
	m_SettingInfo.dwWindowPosX = 0;
	m_SettingInfo.dwWindowPosY = 0;
	m_SettingInfo.uiTimerElapse = 501;
	m_SettingInfo.uiResponse = 5;
	m_SettingInfo.uiSize = 2;
	m_SettingInfo.bTopmost = TRUE;

	// レジストリからの設定値読み出しor新規作成
	LONG lResult;
	DWORD dwDisposition;
	HKEY hParentKey;
	DWORD dwData = 0;
	
	lResult = RegCreateKeyEx(HKEY_CURRENT_USER,
		"Software\\MurateLab\\WinTachos", NULL,	"",	REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,	NULL, &hParentKey,&dwDisposition);
	if (lResult == ERROR_SUCCESS) {
		if (dwDisposition == REG_CREATED_NEW_KEY) {	// 初回立ち上げ時レジストリエントリ作成＆ReadMe表示
			dwData = (DWORD)(VERSION * 100);
			RegSetValueEx(hParentKey, "Version", 0, REG_DWORD,
				(CONST BYTE *)&dwData, sizeof(DWORD));
			SetInfoToReg(hParentKey);
			ShowReadme();
		} else {	// 前回終了時のウィンドウ位置・設定を復元する
			DWORD dwType = REG_DWORD;
			dwData = sizeof(m_SettingInfo.dwWindowPosX);
			lResult = RegQueryValueEx(hParentKey, "WindowPosX", NULL, &dwType,
				(LPBYTE)&m_SettingInfo.dwWindowPosX, &dwData);
			dwData = sizeof(m_SettingInfo.dwWindowPosY);
			lResult = RegQueryValueEx(hParentKey, "WindowPosY", NULL, &dwType,
				(LPBYTE)&m_SettingInfo.dwWindowPosY, &dwData);
			dwData = sizeof(m_SettingInfo.bTopmost);
			lResult = RegQueryValueEx(hParentKey, "Topmost", NULL, &dwType,
				(LPBYTE)&m_SettingInfo.bTopmost, &dwData);
			dwData = sizeof(m_SettingInfo.uiSize);
			lResult = RegQueryValueEx(hParentKey, "Size", NULL, &dwType,
				(LPBYTE)&m_SettingInfo.uiSize, &dwData);
			dwData = sizeof(m_SettingInfo.uiTimerElapse);
			lResult = RegQueryValueEx(hParentKey, "TimerElapse", NULL, &dwType,
				(LPBYTE)&m_SettingInfo.uiTimerElapse, &dwData);
			dwData = sizeof(m_SettingInfo.uiResponse);
			lResult = RegQueryValueEx(hParentKey, "Response", NULL, &dwType,
				(LPBYTE)&m_SettingInfo.uiResponse, &dwData);
			
			// バージョンチェック
			DWORD dwVersion;
			dwData = sizeof(dwVersion);
			lResult = RegQueryValueEx(hParentKey, "Version", NULL, &dwType,
				(LPBYTE)&dwVersion, &dwData);
			if (dwVersion != (DWORD)(VERSION * 100)) {	// レジストリ上のバージョンと異なっていたらReadMeを表示しレジストリを更新
				ShowReadme();

				dwData = (DWORD)(VERSION * 100);
				RegSetValueEx(hParentKey, "Version", 0, REG_DWORD,
					(CONST BYTE *)&dwData, sizeof(DWORD));
			}
		}
	}	
	RegCloseKey(hParentKey);

	// ウィンドウ生成
	float fScale = (float)(5 - m_SettingInfo.uiSize) / 5.0f;
	hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP | WS_SYSMENU,
	        m_SettingInfo.dwWindowPosX, m_SettingInfo.dwWindowPosY,
	        (int)(WINDOWSZ_X * fScale), (int)(WINDOWSZ_Y * fScale), NULL, NULL, hInstance, NULL);

	if( !hWnd ) {
		return FALSE;
	}

	ChangeTopmost(hWnd);

	// タスクバーを表示しない
	SetWindowLongPtr(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, COLORKEY, 0, LWA_COLORKEY);

	// タスクトレイにアイコンを表示
	m_lpni = new NOTIFYICONDATA;
	HICON hIcon;
	DWORD dwProcessID;
	
	GetWindowThreadProcessId(hWnd, &dwProcessID);
	hIcon = LoadIcon(hInst, (LPCTSTR)IDI_SMALL);
	m_lpni->cbSize = sizeof(NOTIFYICONDATA);
	m_lpni->hIcon = hIcon;
	m_lpni->hWnd = hWnd;
	m_lpni->uCallbackMessage = MM_TRAY;
	m_lpni->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_lpni->uID = dwProcessID;
	strcpy_s(m_lpni->szTip, "WinTachos");
	
	Shell_NotifyIcon(NIM_ADD, m_lpni);

	// ウィンドウ表示
	ShowWindow( hWnd, nCmdShow );
	UpdateWindow( hWnd );

	return TRUE;
}

//  関数: WndProc(HWND, unsigned, WORD, LONG)
//  用途: メイン ウィンドウのメッセージを処理します。
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 終了メッセージの通知とリターン
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch( message ) 
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// メニュー選択の解析:
			switch( wmId ) 
			{
				case ID_MENUITEM_SETTING:
					DialogBox(hInst, (LPCTSTR)IDD_SETTING, hWnd, (DLGPROC)SettingDlgProc);
					break;
				case ID_MENUITEM_SETTINGRESET:
					m_SettingInfo.dwWindowPosX = 0;
					m_SettingInfo.dwWindowPosY = 0;
					m_SettingInfo.uiTimerElapse = 501;
					m_SettingInfo.uiResponse = 5;
					m_SettingInfo.uiSize = 2;
					m_SettingInfo.bTopmost = TRUE;

					UpdateSize(hWnd);

					// レジストリ更新
					LONG lResult;
					HKEY hParentKey;
					lResult = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\MurateLab\\WinTachos",
						NULL,KEY_WRITE, &hParentKey);
					SetInfoToReg(hParentKey);
					RegCloseKey(hParentKey);

					break;
				case ID_MENUITEM_VERSION:
					DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)AboutDlgProc);
					break;
				case ID_MENUITEM_EXIT:
					DestroyWindow( hWnd );
					break;
				default:
				   return DefWindowProc( hWnd, message, wParam, lParam );
			}
			break;

		case WM_PAINT:
			{
				hdc = BeginPaint(hWnd, &ps);

				// TODO: この位置に描画用のコードを追加してください...
				MeterDraw(hWnd, hdc);

				EndPaint(hWnd, &ps);
				break;
			}

		case WM_CREATE:
			{
				UpdateSize(hWnd);	// ウィンドウリージョンを設定

				SetTimer(hWnd, TIMERID, m_SettingInfo.uiTimerElapse, NULL);

				Gdiplus::GdiplusStartupInput gdiplusStartupInput;
				Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

				break;
			}

		case WM_TIMER:
			{
				KillTimer(hWnd, TIMERID);
				CalcSpTc();				// スピード，回転数計算

				// タコメーター動作チェック用
				//m_fTacho += 3;
				//m_fTacho = (float)((int)m_fTacho % 100);
				// タコメーター動作チェック用

				InvalidateRect(hWnd, NULL, FALSE);
				SetTimer(hWnd, TIMERID, m_SettingInfo.uiTimerElapse, NULL);

				break;
			}

		case WM_DESTROY:
			{
				LONG lResult;
				HKEY hParentKey;

				// 現在のウィンドウの位置，各種設定をレジストリに保存
				lResult = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\MurateLab\\WinTachos",
					NULL,KEY_WRITE, &hParentKey);
				SetInfoToReg(hParentKey);
				RegCloseKey(hParentKey);

				KillTimer(hWnd, TIMERID);

				// タスクトレイアイコンの削除
				Shell_NotifyIcon(NIM_DELETE, m_lpni);
				delete m_lpni;
				m_lpni = NULL;

				delete[] m_NeedleInfo;
				m_NeedleInfo = NULL;

				Gdiplus::GdiplusShutdown(gdiplusToken);

				PostQuitMessage( 0 );
				break;
			}

		case WM_NCRBUTTONDOWN:
			{
				MakePopupMenu(hWnd);

				break;
			}

		case MM_TRAY:
			{
				DWORD dwProcessID;
				GetWindowThreadProcessId(hWnd, &dwProcessID);

				if ((wParam == dwProcessID) && (lParam == WM_RBUTTONDOWN)) {
					MakePopupMenu(hWnd);
				}

				break;
			}

		case WM_NCHITTEST:
			{
				// あたかもタイトルバーにマウスがあるようにWindowsを騙す
				wParam = DefWindowProc(hWnd, message, wParam, lParam);
				if (wParam == HTCLIENT)
					return HTCAPTION;
				return wParam;
			}
		case WM_WINDOWPOSCHANGED:
			{
				m_SettingInfo.dwWindowPosX = ((WINDOWPOS*)lParam)->x;
				m_SettingInfo.dwWindowPosY = ((WINDOWPOS*)lParam)->y;
				break;
			}
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}

// ＵＲＬリンク用メッセージ ハンドラ
LRESULT CALLBACK linkProc1(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch(uMessage){
		
		//マウスポインタの変更
		case WM_SETCURSOR:
		{
			SetCursor(hCurHand);
			return 0;
		}
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC         hdc;
			char        szTextTmp[100];
			
			GetWindowText(hWnd, szTextTmp, 100);
			hdc = BeginPaint(hWnd, &ps);
			SelectObject(hdc, hFontLink);
			SetTextColor(hdc, RGB(0, 0, 255));
			SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
			SetBkMode(hdc, OPAQUE);
			TextOut(hdc, 0, 0, szTextTmp, lstrlen(szTextTmp));
			EndPaint(hWnd, &ps);
			return 0;
		}
	}
	return CallWindowProc(oldLinkProc1, hWnd, uMessage, wParam, lParam);
}

// ＵＲＬリンク用メッセージ ハンドラ
LRESULT CALLBACK linkProc2(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	switch(uMessage){
		
		//マウスポインタの変更
		case WM_SETCURSOR:
		{
			SetCursor(hCurHand);
			return 0;
		}
		
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC         hdc;
			char        szTextTmp[100];
			
			GetWindowText(hWnd, szTextTmp, 100);
			hdc = BeginPaint(hWnd, &ps);
			SelectObject(hdc, hFontLink);
			SetTextColor(hdc, RGB(0, 0, 255));
			SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
			SetBkMode(hdc, OPAQUE);
			TextOut(hdc, 0, 0, szTextTmp, lstrlen(szTextTmp));
			EndPaint(hWnd, &ps);
			return 0;
		}
	}
	return CallWindowProc(oldLinkProc2, hWnd, uMessage, wParam, lParam);
}

// バージョン情報ボックス用メッセージ ハンドラ
LRESULT CALLBACK AboutDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_INITDIALOG:
			{
				char cWork[32];

				// ＵＲＬ、メールアドレスのフォントを設定
				LOGFONT logfont;
				hFontLink = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_STATIC_URL), WM_GETFONT, 0, 0);
				GetObject(hFontLink, sizeof(logfont), &logfont);
				logfont.lfUnderline = 1;
				hFontLink = CreateFontIndirect(&logfont);
				SendDlgItemMessage(hDlg, IDC_STATIC_URL, WM_SETFONT, (WPARAM)hFontLink, 0);
				hFontLink = (HFONT)SendMessage(GetDlgItem(hDlg,IDC_STATIC_MAIL), WM_GETFONT, 0, 0);
				GetObject(hFontLink, sizeof(logfont), &logfont);
				logfont.lfUnderline = 1;
				hFontLink = CreateFontIndirect(&logfont);
				SendDlgItemMessage(hDlg, IDC_STATIC_MAIL, WM_SETFONT, (WPARAM)hFontLink, 0);

				//サブクラスハンドラの登録
				oldLinkProc1 = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDC_STATIC_URL), GWLP_WNDPROC);
				SetWindowLongPtr(GetDlgItem(hDlg, IDC_STATIC_URL), GWLP_WNDPROC, (LONG_PTR)linkProc1);
				oldLinkProc2 = (WNDPROC)GetWindowLongPtr(GetDlgItem(hDlg, IDC_STATIC_MAIL), GWLP_WNDPROC);
				SetWindowLongPtr(GetDlgItem(hDlg, IDC_STATIC_MAIL), GWLP_WNDPROC, (LONG_PTR)linkProc2);

				// バージョンをセット
				sprintf_s(cWork, sizeof(cWork), "WinTachos Version %1.2f", (float)VERSION);
				SendMessage(GetDlgItem(hDlg, IDC_STATIC_VERSION), WM_SETTEXT, (WPARAM)0, (LPARAM)cWork);
				return TRUE;
			}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDOK:		// ［ＯＫ］ボタンクリック時
			case IDCANCEL:	// ×ボタンクリック時
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			break;

			case IDC_STATIC_URL:	// ＵＲＬクリック時
				if(HIWORD(wParam) == STN_CLICKED)
				{
					char szTextTmp[100];
					
					GetDlgItemText(hDlg, LOWORD(wParam), szTextTmp, 100);
					ShellExecute(hDlg, NULL, szTextTmp, NULL, NULL, SW_SHOWNORMAL);
				}
				break;

			case IDC_STATIC_MAIL:	// メールアドレスクリック時
				{
					char szTextTmp[100];
					
					strcpy_s(szTextTmp, sizeof(szTextTmp), "mailto:");
					GetDlgItemText(hDlg, LOWORD(wParam), szTextTmp + 7,100);
					ShellExecute(hDlg, NULL, szTextTmp, NULL, NULL, SW_SHOWNORMAL);
				}
				break;
			}
	}
	return FALSE;
}

// 設定ダイアログ用メッセージ ハンドラ
LRESULT CALLBACK SettingDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch( message ) {
		case WM_INITDIALOG:
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_TOPMOST), BM_SETCHECK, (WPARAM)m_SettingInfo.bTopmost, 0L);
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_SIZE), CB_INSERTSTRING, (WPARAM)0, (LPARAM)"Huge");
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_SIZE), CB_INSERTSTRING, (WPARAM)1, (LPARAM)"Large");
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_SIZE), CB_INSERTSTRING, (WPARAM)2, (LPARAM)"Normal");
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_SIZE), CB_INSERTSTRING, (WPARAM)3, (LPARAM)"Small");
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_SIZE), CB_INSERTSTRING, (WPARAM)4, (LPARAM)"Tiny");
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_SIZE), CB_SETCURSEL, (WPARAM)m_SettingInfo.uiSize, 0L);
			char cEditBox[10];
			sprintf_s(cEditBox, sizeof(cEditBox), "%d", m_SettingInfo.uiTimerElapse);
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_TIMERELAPSE), WM_SETTEXT, (WPARAM)0, (WPARAM)cEditBox);
			sprintf_s(cEditBox, sizeof(cEditBox), "%d", m_SettingInfo.uiResponse);
			SendMessage(GetDlgItem(hDlg, IDC_SETTING_RESPONSE), WM_SETTEXT, (WPARAM)0, (WPARAM)cEditBox);
			return TRUE;

		case WM_COMMAND:
			if(LOWORD(wParam) == IDOK) {	// 設定した情報を反映させダイアログを閉じる
				int iEditBox;
				char cEditBox[10];

				// 更新間隔
				SendMessage(GetDlgItem(hDlg, IDC_SETTING_TIMERELAPSE), WM_GETTEXT, (WPARAM)10, (WPARAM)cEditBox);
				sscanf_s(cEditBox, "%d", &iEditBox);
				if (iEditBox < 10 || iEditBox > 10000) {
					MessageBox(NULL, "更新間隔は10〜10000の値を入力して下さい。", "WinTachos", MB_OK);
					break;
				}
				m_SettingInfo.uiTimerElapse = iEditBox;

				// レスポンス
				SendMessage(GetDlgItem(hDlg, IDC_SETTING_RESPONSE), WM_GETTEXT, (WPARAM)10, (WPARAM)cEditBox);
				sscanf_s(cEditBox, "%d", &iEditBox);
				if (iEditBox < 1 || iEditBox > 200) {
					MessageBox(NULL, "レスポンスは1〜200の値を入力して下さい。", "WinTachos", MB_OK);
					break;
				}
				m_SettingInfo.uiResponse = iEditBox;

				// 常に手前に表示
				if (IsDlgButtonChecked(hDlg, IDC_SETTING_TOPMOST) == BST_CHECKED) {  
					m_SettingInfo.bTopmost = TRUE;
				} else {
					m_SettingInfo.bTopmost = FALSE;
				}
				ChangeTopmost(GetParent(hDlg));

				// 大きさ
				m_SettingInfo.uiSize = SendMessage(GetDlgItem(hDlg, IDC_SETTING_SIZE), CB_GETCURSEL, 0L, 0L);
				UpdateSize(GetParent(hDlg));

				// レジストリ更新
				LONG lResult;
				HKEY hParentKey;
				lResult = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\MurateLab\\WinTachos",
					NULL,KEY_WRITE, &hParentKey);
				SetInfoToReg(hParentKey);
				RegCloseKey(hParentKey);

				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			} else if (LOWORD(wParam) == IDCANCEL) {	// ダイアログを閉じる
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
	return FALSE;
}

// 関数 : CreateRgnFromBmp ( HBITMAP, COLORREF )
// 用途 : ビットマップからリージョンを作成する
// 引数 :
//      hBitmap - リージョンを作成するビットマップ
//      cTransparentColor - 透明色(RGB(255,255,255)のように指定する)
//                          デフォルトでは左下隅のドットの色が透明色となる
HRGN CreateRgnFromBmp(HBITMAP hBitmap, COLORREF cTransparentColor = 0xffffffff)
{
	BITMAP bm;                  // BITMAP structure
	HBITMAP hDIBSection = NULL; // Handle to DIB
	COLORREF* pBits = NULL;     // Pointer to the DIB's bit values
	HRGN hRgn = NULL;           // Handle to Region

	// hBitmap のチェック
	if (!hBitmap)
		return NULL;

	// ビットマップのサイズを取得
	if (!GetObject(hBitmap, sizeof(bm), &bm))
		return NULL;

	// BLOCK : DIBの作成
	//         成功すると hDIBSection にビットマップハンドル、pBits にビットへの
	//         ポインタが設定される。(DeleteObject(hDIBSection); を忘れてはいけない)
	//         失敗すると hDIBSection 及び pBits は NULL のまま。
	{
		HDC hMemDC = NULL;          // Handle to a device context : DIB
		HDC hCopyDC = NULL;         // Handle to a device context : DDB -> DIB
		HBITMAP hOldDIBsBmp = NULL; // Handle of the object being replaced : DIB
		HBITMAP hOldDDBsBmp = NULL; // Handle of the object being replaced : DDB
		BITMAPINFOHEADER bmih;      // DIB's BITMAPINFOHEADER structure
		BOOL bResultBitBlt = TRUE;  // Result of BitBlt function

		// メモリデバイスコンテキストを作成
		hMemDC = CreateCompatibleDC(NULL);
		if (!hMemDC)
			goto cleanupdib;

		// DIB(Device Independent Bitmaps : デバイスに依存しないビットマップ)
		// のヘッダ情報を書きこむ
		bmih.biSize             = sizeof(BITMAPINFOHEADER), // size of the structure
		bmih.biWidth            = bm.bmWidth;   // width of the bitmap
		bmih.biHeight           = bm.bmHeight;  // height of the bitmap
		bmih.biPlanes           = 1;        // must be set to 1
		bmih.biBitCount         = 32;       // maximum of 2^32 colors
		bmih.biCompression      = BI_RGB;   // an uncompressed format
		bmih.biSizeImage        = 0;        // must be set to zero for BI_RGB bitmaps
		bmih.biXPelsPerMeter    = 0;        // set to zero
		bmih.biYPelsPerMeter    = 0;        // set to zero
		bmih.biClrUsed          = 0;        // must be set to zero
		bmih.biClrImportant     = 0;        // set to zero, all colors are required

		// DIBの作成
		hDIBSection = CreateDIBSection(
			hMemDC,             // HDC hdc : handle to device context
			(CONST BITMAPINFO*)&bmih,   // CONST BITMAPINFO *pbmi : pointer to BITMAPINFO structure
			DIB_RGB_COLORS, // UINT iUsage : color data type(RGB values)
			(void**)&pBits, // VOID *ppvBits : pointer to receive the bitmap's bit values
			NULL,           // HANDLE hSection : handle to a file mapping object.
			                //                   This parameter can be NULL.
			0);             // DWORD dwOffset : ignored by hSection
		if (!hDIBSection)
			goto cleanupdib;

		// 作成したDIBをDCに選択
		hOldDIBsBmp = (HBITMAP)SelectObject(hMemDC, hDIBSection);

		// 画像の転送元となるメモリデバイスコンテキストを作成
		hCopyDC = CreateCompatibleDC(hMemDC);
		if (!hCopyDC)
			goto cleanupdib;

		// hBitmap で指定されたビットマップをデバイスコンテキストに選択
		hOldDDBsBmp = (HBITMAP)SelectObject(hCopyDC, hBitmap);

		// ビットマップをhMemDCに転送
		bResultBitBlt = BitBlt(hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, hCopyDC, 0, 0, SRCCOPY);

		// クリーンアップ
		cleanupdib:
		if (hOldDDBsBmp)    SelectObject(hCopyDC, hOldDDBsBmp);
		if (hCopyDC)        DeleteDC(hCopyDC);
		if (hOldDIBsBmp)    SelectObject(hMemDC, hOldDIBsBmp);
		if (hMemDC)         DeleteDC(hMemDC);

		// BitBlt 関数が失敗していたらDIBを削除しNULLに設定
		if (!bResultBitBlt)
		{
			DeleteObject(hDIBSection);
			hDIBSection = NULL;
			pBits = NULL;
		}
	}   // End of BLOCK : DIBの作成

	// DIBの作成に失敗していたらリージョンは作れない
	if (!hDIBSection)
		return NULL;

	// BLOCK : リージョンの作成
	{
		HRGN hDotRgn;       // 一時的なドットのリージョン
		int xx;             // x の値を保存

		// 透明色が指定されていなければ、左下隅のドットの色が透明色となる
		if (cTransparentColor == 0xffffffff)
			cTransparentColor = *pBits;

		// 空のリージョンの作成
		hRgn = CreateRectRgn(0,0,0,0);

		// 縦横にビットが透明色かを調べ、透明色で無い部分をリージョンに追加
		for (int y = 0; y < bm.bmHeight; y++) {   // 縦方向
			for (int x = 0; x < bm.bmWidth; x++) {    // 横方向
				if ((*pBits & 0x00ffffff) != (cTransparentColor & 0x00ffffff)) {    // 透明色ではない？
					xx = x;     // x を保存(透明色ではない点の始まりを保存)
					// 連続した透明色ではない領域を求める
					for (x = x + 1; x < bm.bmWidth; x++) {
						// ポインタをインクリメント
						pBits++;

						// 透明色になったら break
						if ((*pBits & 0x00ffffff) == (cTransparentColor & 0x00ffffff))
							break;
					}

					// 一時的な透明色でない領域のリージョンを作成
					hDotRgn = CreateRectRgn(xx ,bm.bmHeight-y ,x ,bm.bmHeight-y-1);
					// hRgn と合成(ビットマップ全体のリージョンを作成)
					CombineRgn(hRgn, hRgn, hDotRgn, RGN_OR);
					// 一時的なリージョンを削除
					DeleteObject(hDotRgn);
				}
				// ポインタをインクリメント
				pBits ++;
			}
		}
	}   // End of BLOCK : リージョンの作成

	// HBITMAP(DIB)を削除
	DeleteObject(hDIBSection);

	// メーターの中心軸の部分のリージョン
	HRGN hCircleRgn;
	for (int i = 0; i < 2; i++) {	// i:0 スピードメーター  i:1 タコメーター
		hCircleRgn = CreateEllipticRgn(
			m_NeedleInfo[i].poCenter.x - m_NeedleInfo[i].uiCenterR, m_NeedleInfo[i].poCenter.y - m_NeedleInfo[i].uiCenterR, 
			m_NeedleInfo[i].poCenter.x + m_NeedleInfo[i].uiCenterR, m_NeedleInfo[i].poCenter.y + m_NeedleInfo[i].uiCenterR );
		CombineRgn(hRgn, hRgn, hCircleRgn, RGN_OR);
		DeleteObject(hCircleRgn);
	}

	// return the handle of the region
	return hRgn;
}

void MeterDraw(HWND hWnd, HDC hdc)
{
	HDC hMemDC;
	HBITMAP hMemBmp;
	HBITMAP hOldBmp;
	RECT rctWnd;

	GetClientRect(hWnd, &rctWnd);
	hMemDC = CreateCompatibleDC(hdc);
	hMemBmp = CreateCompatibleBitmap(hdc, rctWnd.right, rctWnd.bottom);
	hOldBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp);

	HBRUSH hKeyBrush = CreateSolidBrush(COLORKEY);
	FillRect(hMemDC, &rctWnd, hKeyBrush);
	DeleteObject(hKeyBrush);
	SelectClipRgn(hMemDC, m_hBaseRgn);
	ShowMyBMP(hWnd, hMemDC);	// メーター描画
	SelectClipRgn(hMemDC, NULL);

	DrawNeedle(hMemDC);		// 針描画
	DrawCenterCircle(hMemDC);	// 中心軸描画

	BitBlt(hdc, 0, 0, rctWnd.right, rctWnd.bottom, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hOldBmp);
	DeleteObject(hMemBmp);
	DeleteDC(hMemDC);
}

void ShowMyBMP(HWND hWnd, HDC hdc)
{	// メーターの目盛りを描画
	HDC hmdc;
	HBITMAP hBitmap;
	BITMAP bmp;
	HINSTANCE hInst;
	int BMP_W, BMP_H;

	hInst = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);
	switch (m_SettingInfo.uiSize) {
	case 0:
		hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_METER_H));
		break;
	case 1:
		hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_METER_L));
		break;
	case 2:
		hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_METER_N));
		break;
	case 3:
		hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_METER_S));
		break;
	case 4:
		hBitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_METER_T));
		break;
	default:
		hBitmap = NULL;
		break;
	}
	if (!hBitmap)
		return;
	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	BMP_W = (int)bmp.bmWidth;
	BMP_H = (int)bmp.bmHeight;
	hmdc = CreateCompatibleDC(hdc);
	SelectObject(hmdc, hBitmap);
	BitBlt(hdc, 0, 0, BMP_W, BMP_H, hmdc, 0, 0, SRCCOPY);
	DeleteDC(hmdc);
	DeleteObject(hBitmap);
	return;
}

void CalcSpTc(void)
{
	// メモリ使用量取得
	MEMORYSTATUSEX mstMemStat;
	mstMemStat.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&mstMemStat);
	m_fSpeed = (float)mstMemStat.dwMemoryLoad;

	// ＣＰＵ使用率取得
	m_fTacho = (float)GetCPUUsage_NT();

	// 過去の値を記録
	m_fSpeedLog[m_iLogPos] = m_fSpeed;
	m_fTachoLog[m_iLogPos] = m_fTacho;

	// 実際の表示値算出
	float fSpeedSum = m_fSpeed;
	float fTachoSum = m_fTacho;
	for (UINT i = 1; i < m_SettingInfo.uiResponse; i++) {
		fSpeedSum += m_fSpeedLog[(m_iLogPos - i + LOGMAX) % LOGMAX];
		fTachoSum += m_fTachoLog[(m_iLogPos - i + LOGMAX) % LOGMAX];
	}
	m_iLogPos++;  m_iLogPos %= LOGMAX;
	m_fSpeedDisp = fSpeedSum / (float)m_SettingInfo.uiResponse;
	m_fTachoDisp = fTachoSum / (float)m_SettingInfo.uiResponse;
}

void UpdateSize(HWND hWnd)
{
	// 設定されたサイズに合った座標値等の値を針の情報に設定
	float fScale = (float)(5 - m_SettingInfo.uiSize) / 5.0f;
	m_NeedleInfo[0].poCenter.x	= (UINT)((float)SPEEDM_CENTER_X * fScale);
	m_NeedleInfo[0].poCenter.y	= (UINT)((float)SPEEDM_CENTER_Y * fScale);
	m_NeedleInfo[0].uiCenterR	= (UINT)((float)SPEEDM_CENTER_R * fScale);
	m_NeedleInfo[0].uiLeng		= (UINT)((float)SPEEDM_LENG * fScale);
	m_NeedleInfo[0].uiLengB		= (UINT)((float)SPEEDM_LENG_B * fScale);
	m_NeedleInfo[0].fMinR		= SPEED_MIN_R;
	m_NeedleInfo[0].fMaxR		= SPEED_MAX_R;
	m_NeedleInfo[1].poCenter.x	= (UINT)((float)TACHOM_CENTER_X * fScale);
	m_NeedleInfo[1].poCenter.y	= (UINT)((float)TACHOM_CENTER_Y * fScale);
	m_NeedleInfo[1].uiCenterR	= (UINT)((float)TACHOM_CENTER_R * fScale);
	m_NeedleInfo[1].uiLeng		= (UINT)((float)TACHOM_LENG * fScale);
	m_NeedleInfo[1].uiLengB		= (UINT)((float)TACHOM_LENG_B * fScale);
	m_NeedleInfo[1].fMinR		= TACHO_MIN_R;
	m_NeedleInfo[1].fMaxR		= TACHO_MAX_R;

	// ウィンドウサイズ変更
	SetWindowPos(hWnd, NULL, m_SettingInfo.dwWindowPosX, m_SettingInfo.dwWindowPosY,
	   (int)(WINDOWSZ_X * fScale), (int)(WINDOWSZ_Y * fScale), SWP_NOZORDER);

	// リージョン作成用のビットマップを読み込む	
	HBITMAP hBitmap;

	switch (m_SettingInfo.uiSize) {
	case 0:
		hBitmap = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REGION_H)); 
		break;
	case 1:
		hBitmap = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REGION_L)); 
		break;
	case 2:
		hBitmap = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REGION_N)); 
		break;
	case 3:
		hBitmap = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REGION_S)); 
		break;
	case 4:
		hBitmap = (HBITMAP)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_REGION_T)); 
		break;
	default:
		hBitmap = NULL;
		break;
	}

	// ビットマップからリージョンを作成
	if (m_hBaseRgn) DeleteObject(m_hBaseRgn);
	m_hBaseRgn = CreateRgnFromBmp(hBitmap, RGB(0, 0, 0));	// 透過色：黒
	// ビットマップを削除
	DeleteObject(hBitmap);

}

void DrawNeedle(HDC hDC)
{
	Gdiplus::Graphics graphics(hDC);
	graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	Gdiplus::Point pNeedle[6];
	float fAngle, fAngleC_rad, fAngleR_rad, fAngleL_rad;

	float fNeedleWidth = (5 - ((m_SettingInfo.uiSize==3)?2:m_SettingInfo.uiSize)) / 2.0f;

	Gdiplus::SolidBrush black(Gdiplus::Color(255, 0, 0, 0));
	COLORREF nc = NEEDLE_COLOR;
	Gdiplus::SolidBrush needleColor(Gdiplus::Color(255, GetRValue(nc), GetGValue(nc), GetBValue(nc)));

	for (int i = 0; i < 2; i++) {	// i:0 スピードメーター  i:1 タコメーター

		// メータの針の角度計算
		switch (i) {
		case 0:
			fAngle = m_NeedleInfo[i].fMinR - (m_fSpeedDisp / 100.0f) * (m_NeedleInfo[i].fMinR - m_NeedleInfo[i].fMaxR);
			break;
		case 1:
			fAngle = m_NeedleInfo[i].fMinR - (m_fTachoDisp / 100.0f) * (m_NeedleInfo[i].fMinR - m_NeedleInfo[i].fMaxR);
			break;
		default:
			fAngle = m_NeedleInfo[i].fMinR;
			break;
		}
		fAngleC_rad = 6.283185308f / (360.0f / fAngle);
		fAngleR_rad = 6.283185308f / (360.0f / (fAngle - 90.0f));
		fAngleL_rad = 6.283185308f / (360.0f / (fAngle + 90.0f));

		// 針の先端・根元の座標計算
		pNeedle[0].X = m_NeedleInfo[i].poCenter.x + (INT)(m_NeedleInfo[i].uiLeng * cos(fAngleC_rad));
		pNeedle[0].Y = m_NeedleInfo[i].poCenter.y - (INT)(m_NeedleInfo[i].uiLeng * sin(fAngleC_rad));
		pNeedle[1].X = m_NeedleInfo[i].poCenter.x + (INT)(m_NeedleInfo[i].uiCenterR * cos(fAngleC_rad));
		pNeedle[1].Y = m_NeedleInfo[i].poCenter.y - (INT)(m_NeedleInfo[i].uiCenterR * sin(fAngleC_rad));

		// 針描画（淵）
		pNeedle[2].X = m_NeedleInfo[i].poCenter.x + (INT)(4.0f * cos(fAngleR_rad));
		pNeedle[2].Y = m_NeedleInfo[i].poCenter.y - (INT)(4.0f * sin(fAngleR_rad));
		pNeedle[3].X = m_NeedleInfo[i].poCenter.x + (INT)(4.0f * cos(fAngleL_rad));
		pNeedle[3].Y = m_NeedleInfo[i].poCenter.y - (INT)(4.0f * sin(fAngleL_rad));
		pNeedle[4].X = pNeedle[0].X + (INT)(3.0f * cos(fAngleL_rad));
		pNeedle[4].Y = pNeedle[0].Y - (INT)(3.0f * sin(fAngleL_rad));
		pNeedle[5].X = pNeedle[0].X + (INT)(3.0f * cos(fAngleR_rad));
		pNeedle[5].Y = pNeedle[0].Y - (INT)(3.0f * sin(fAngleR_rad));
		graphics.FillPolygon(&black, &pNeedle[2], 4);

		// 針のポリゴン座標計算（中身）
		pNeedle[2].X = pNeedle[1].X + (INT)(fNeedleWidth * cos(fAngleR_rad));
		pNeedle[2].Y = pNeedle[1].Y - (INT)(fNeedleWidth * sin(fAngleR_rad));
		pNeedle[3].X = pNeedle[1].X + (INT)(fNeedleWidth * cos(fAngleL_rad));
		pNeedle[3].Y = pNeedle[1].Y - (INT)(fNeedleWidth * sin(fAngleL_rad));
		pNeedle[4].X = pNeedle[0].X + (INT)(1.0f * cos(fAngleL_rad));
		pNeedle[4].Y = pNeedle[0].Y - (INT)(1.0f * sin(fAngleL_rad));
		pNeedle[5].X = pNeedle[0].X + (INT)(1.0f * cos(fAngleR_rad));
		pNeedle[5].Y = pNeedle[0].Y - (INT)(1.0f * sin(fAngleR_rad));

		// 針の背景ポリゴン
		Gdiplus::Point bgNeedle[4];
		float bgW = fNeedleWidth + 2.0f;
		bgNeedle[0].X = pNeedle[1].X + (INT)(bgW * cos(fAngleR_rad));
		bgNeedle[0].Y = pNeedle[1].Y - (INT)(bgW * sin(fAngleR_rad));
		bgNeedle[1].X = pNeedle[1].X + (INT)(bgW * cos(fAngleL_rad));
		bgNeedle[1].Y = pNeedle[1].Y - (INT)(bgW * sin(fAngleL_rad));
		bgNeedle[2].X = pNeedle[0].X + (INT)(3.0f * cos(fAngleL_rad));
		bgNeedle[2].Y = pNeedle[0].Y - (INT)(3.0f * sin(fAngleL_rad));
		bgNeedle[3].X = pNeedle[0].X + (INT)(3.0f * cos(fAngleR_rad));
		bgNeedle[3].Y = pNeedle[0].Y - (INT)(3.0f * sin(fAngleR_rad));
		graphics.FillPolygon(&black, bgNeedle, 4);

		// 針描画（中身）
		graphics.FillPolygon(&needleColor, &pNeedle[2], 4);

		// 針の先端の座標計算（反対側）
		pNeedle[0].X = m_NeedleInfo[i].poCenter.x + (INT)(-m_NeedleInfo[i].uiLengB * cos(fAngleC_rad));
		pNeedle[0].Y = m_NeedleInfo[i].poCenter.y - (INT)(-m_NeedleInfo[i].uiLengB * sin(fAngleC_rad));
		// 針のポリゴン座標計算（反対側）
		pNeedle[2].X = m_NeedleInfo[i].poCenter.x + (INT)(4.0f * cos(fAngleL_rad));
		pNeedle[2].Y = m_NeedleInfo[i].poCenter.y - (INT)(4.0f * sin(fAngleL_rad));
		pNeedle[3].X = m_NeedleInfo[i].poCenter.x + (INT)(4.0f * cos(fAngleR_rad));
		pNeedle[3].Y = m_NeedleInfo[i].poCenter.y - (INT)(4.0f * sin(fAngleR_rad));
		pNeedle[4].X = pNeedle[0].X + (INT)(4.0f * cos(fAngleR_rad));
		pNeedle[4].Y = pNeedle[0].Y - (INT)(4.0f * sin(fAngleR_rad));
		pNeedle[5].X = pNeedle[0].X + (INT)(4.0f * cos(fAngleL_rad));
		pNeedle[5].Y = pNeedle[0].Y - (INT)(4.0f * sin(fAngleL_rad));

		// 針描画（反対側）
		graphics.FillPolygon(&black, &pNeedle[2], 4);
	}

}

void DrawCenterCircle(HDC hDC)
{	// 針の中心軸を描画	
	HPEN pen, oldpen;
	HBRUSH brush, oldbrush;

	pen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	oldpen = (HPEN)SelectObject(hDC, pen);
	brush = CreateSolidBrush(RGB(0, 0, 0));
	oldbrush = (HBRUSH)SelectObject(hDC, brush);

	for (int i = 0; i < 2; i++) {
		Ellipse( hDC,
			m_NeedleInfo[i].poCenter.x - m_NeedleInfo[i].uiCenterR, m_NeedleInfo[i].poCenter.y - m_NeedleInfo[i].uiCenterR, 
			m_NeedleInfo[i].poCenter.x + m_NeedleInfo[i].uiCenterR, m_NeedleInfo[i].poCenter.y + m_NeedleInfo[i].uiCenterR );
	}

	SelectObject(hDC, oldpen);
	SelectObject(hDC, oldbrush);
	DeleteObject(pen);
	DeleteObject(brush);
}

void ChangeTopmost(HWND hWnd)
{	// 常に手前に表示を切り替え
	HWND hWndInsertAfter;
	if (m_SettingInfo.bTopmost) {
		hWndInsertAfter = HWND_TOPMOST;
	} else {
		hWndInsertAfter = HWND_NOTOPMOST;
	}
	SetWindowPos(hWnd, hWndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

double GetCPUUsage_NT()
{
	static HQUERY hQuery = NULL;
	static HCOUNTER hCounter = NULL;
	PDH_FMT_COUNTERVALUE FmtValue;
	double dResult = 0.0;
	PDH_STATUS pdhStatus;

	if (hQuery == NULL) {	// 初回実行時
		pdhStatus = PdhOpenQuery(NULL, 0, &hQuery);
		PdhStatusCheck(1, pdhStatus);
		pdhStatus = PdhAddCounter(hQuery, "\\Processor Information(_Total)\\% Processor Time", 0, &hCounter);
		PdhStatusCheck(2, pdhStatus);
		pdhStatus = PdhCollectQueryData(hQuery);
		PdhStatusCheck(3, pdhStatus);
	} else {	// ２回目以降
		pdhStatus = PdhCollectQueryData(hQuery);
		PdhStatusCheck(4, pdhStatus);
		pdhStatus = PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL, &FmtValue);
		PdhStatusCheck(5, pdhStatus);
		dResult = FmtValue.doubleValue;
		pdhStatus = PdhCloseQuery(hQuery);
		PdhStatusCheck(6, pdhStatus);
		pdhStatus = PdhOpenQuery(NULL, 0, &hQuery);
		PdhStatusCheck(7, pdhStatus);
		pdhStatus = PdhAddCounter(hQuery, "\\Processor Information(_Total)\\% Processor Time", 0, &hCounter);
		PdhStatusCheck(8, pdhStatus);
		pdhStatus = PdhCollectQueryData(hQuery);
		PdhStatusCheck(9, pdhStatus);
	}

	return dResult;
}

void PdhStatusCheck(int i, PDH_STATUS pdhStatus)
{
	// 時刻を変更したときにエラーが出てしまう件の暫定対策
	if ((pdhStatus == PDH_CALC_NEGATIVE_DENOMINATOR) && (i == 5)) {
		return;
	}

	// エラー処理
	if (pdhStatus!=ERROR_SUCCESS) {
		char* errorMes = new char[100];

		switch (pdhStatus) {
		case PDH_CSTATUS_NO_MACHINE:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CSTATUS_NO_MACHINE) : %d", i);
			break;
		case PDH_CSTATUS_NO_INSTANCE:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CSTATUS_NO_INSTANCE) : %d", i);
			break;
		case PDH_MORE_DATA:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_MORE_DATA) : %d", i);
			break;
		case PDH_CSTATUS_ITEM_NOT_VALIDATED:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CSTATUS_ITEM_NOT_VALIDATED) : %d", i);
			break;
		case PDH_RETRY:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_RETRY) : %d", i);
			break;
		case PDH_NO_DATA:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_NO_DATA) : %d", i);
			break;
		case PDH_CALC_NEGATIVE_DENOMINATOR:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CALC_NEGATIVE_DENOMINATOR) : %d", i);
			break;
		case PDH_CALC_NEGATIVE_TIMEBASE:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CALC_NEGATIVE_TIMEBASE) : %d", i);
			break;
		case PDH_CALC_NEGATIVE_VALUE:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CALC_NEGATIVE_VALUE) : %d", i);
			break;
		case PDH_DIALOG_CANCELLED:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_DIALOG_CANCELLED) : %d", i);
			break;
		case PDH_CSTATUS_NO_OBJECT:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CSTATUS_NO_OBJECT) : %d", i);
			break;
		case PDH_CSTATUS_NO_COUNTER:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CSTATUS_NO_COUNTER) : %d", i);
			break;
		case PDH_CSTATUS_INVALID_DATA:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CSTATUS_INVALID_DATA) : %d", i);
			break;
		case PDH_MEMORY_ALLOCATION_FAILURE:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_MEMORY_ALLOCATION_FAILURE) : %d", i);
			break;
		case PDH_INVALID_HANDLE:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_INVALID_HANDLE) : %d", i);
			break;
		case PDH_INVALID_ARGUMENT:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_INVALID_ARGUMENT) : %d", i);
			break;
		case PDH_FUNCTION_NOT_FOUND:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_FUNCTION_NOT_FOUND) : %d", i);
			break;
		case PDH_CSTATUS_NO_COUNTERNAME:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CSTATUS_NO_COUNTERNAME) : %d", i);
			break;
		case PDH_CSTATUS_BAD_COUNTERNAME:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CSTATUS_BAD_COUNTERNAME) : %d", i);
			break;
		case PDH_INVALID_BUFFER:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_INVALID_BUFFER) : %d", i);
			break;
		case PDH_INSUFFICIENT_BUFFER:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_INSUFFICIENT_BUFFER) : %d", i);
			break;
		case PDH_CANNOT_CONNECT_MACHINE:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CANNOT_CONNECT_MACHINE) : %d", i);
			break;
		case PDH_INVALID_PATH:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_INVALID_PATH) : %d", i);
			break;
		case PDH_INVALID_INSTANCE:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_INVALID_INSTANCE) : %d", i);
			break;
		case PDH_INVALID_DATA:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_INVALID_DATA) : %d", i);
			break;
		case PDH_NO_DIALOG_DATA:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_NO_DIALOG_DATA) : %d", i);
			break;
		case PDH_CANNOT_READ_NAME_STRINGS:
			sprintf_s(errorMes, 100, "PDHステータスエラー(PDH_CANNOT_READ_NAME_STRINGS) : %d", i);
			break;
		default:
			sprintf_s(errorMes, 100, "PDHステータスエラー(unknown) : %d", i);
			break;
		}

		MessageBox(NULL, errorMes, "WinTachos", MB_OK);

		delete[] errorMes;
	}
}

void ShowReadme(void)
{
	if (IsExistFile("ReadMe.txt")) {
		// ReadMeを表示させる
		STARTUPINFO siStartInfo = { sizeof(siStartInfo) };
		PROCESS_INFORMATION piProcInfo;
		CreateProcess(NULL, "notepad ReadMe.txt", NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &siStartInfo, &piProcInfo);
		CloseHandle(piProcInfo.hProcess);
	} else {
		MessageBox(NULL, "ReadMe.txtが見つかりません", "WinTachos", MB_OK);
	}
}

BOOL IsExistFile(LPCTSTR fileName)
{
	BOOL bRet = FALSE;
	WIN32_FIND_DATA findData;
	
	HANDLE  hFind = FindFirstFile(fileName , &findData);
	if (hFind != INVALID_HANDLE_VALUE) {
		bRet = TRUE;
	}
	FindClose(hFind);
	return bRet;
}

void MakePopupMenu(HWND hWnd)
{
	POINT pt;
	HMENU hMenu, hSubMenu;

	// メニュー外でクリックしてもポップアップメニューが消えない対策
	SetForegroundWindow(hWnd);
	
	// ポップアップメニューを出す
	GetCursorPos(&pt);

	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_RCLKMENU));
	hSubMenu = GetSubMenu(hMenu, 0);

	TrackPopupMenu(hSubMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
	               pt.x, pt.y, 0, hWnd, NULL);
}

void SetInfoToReg(HKEY hParentKey)
{
	RegSetValueEx(hParentKey, "WindowPosX", 0, REG_DWORD,
		(CONST BYTE *)&m_SettingInfo.dwWindowPosX, sizeof(DWORD));
	RegSetValueEx(hParentKey, "WindowPosY", 0, REG_DWORD,
		(CONST BYTE *)&m_SettingInfo.dwWindowPosY, sizeof(DWORD));
	RegSetValueEx(hParentKey, "Topmost", 0, REG_DWORD,
		(CONST BYTE *)&m_SettingInfo.bTopmost, sizeof(DWORD));
	RegSetValueEx(hParentKey, "Size", 0, REG_DWORD,
		(CONST BYTE *)&m_SettingInfo.uiSize, sizeof(DWORD));
	RegSetValueEx(hParentKey, "TimerElapse", 0, REG_DWORD,
		(CONST BYTE *)&m_SettingInfo.uiTimerElapse, sizeof(DWORD));
	RegSetValueEx(hParentKey, "Response", 0, REG_DWORD,
		(CONST BYTE *)&m_SettingInfo.uiResponse, sizeof(DWORD));
}
