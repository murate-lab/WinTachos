
#if !defined(AFX_WINTACHOS_H__AA88F7B7_D84D_4D55_BB3D_258AD676D713__INCLUDED_)
#define AFX_WINTACHOS_H__AA88F7B7_D84D_4D55_BB3D_258AD676D713__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <shellapi.h>
#include "resource.h"
#include "pdh.h"
#include "pdhmsg.h"

#define VERSION					2.00				// バージョン
#define COLORKEY					RGB(1, 0, 1)			// Layered window transparency key

#define MAX_LOADSTRING 100
#define TIMERID					1					// WM_PAINT発行のためのタイマーID
#define	MM_TRAY					(WM_USER + 0)		// タスクトレイのアイコンクリック時
#define LOGMAX					200					// 記録上限

#ifdef _PS13
	#define WINDOWSZ_X			600					// サイズHuge時のウィンドウサイズＸ
	#define WINDOWSZ_Y			200					// サイズHuge時のウィンドウサイズＹ
	#define SPEEDM_CENTER_X		147					// スピードメーターの中心座標Ｘ
	#define SPEEDM_CENTER_Y		150					// スピードメーターの中心座標Ｙ
	#define SPEEDM_CENTER_R		12					// スピードメーターの中心軸の半径
	#define SPEEDM_LENG			131					// スピードメーターの針の長さ
	#define SPEEDM_LENG_B		22					// スピードメーターの針の長さ（反対側）
	#define SPEED_MIN_R			198.4f				// スピードメーターの0kmの位置の角度
	#define SPEED_MAX_R			-18.6f				// スピードメーターの180kmの位置の角度
	#define TACHOM_CENTER_X		452					// タコメーターの中心座標Ｘ
	#define TACHOM_CENTER_Y		150					// タコメーターの中心座標Ｙ
	#define TACHOM_CENTER_R		12					// タコメーターの中心軸の半径
	#define TACHOM_LENG			131					// タコメーターの針の長さ
	#define TACHOM_LENG_B		22					// タコメーターの針の長さ（反対側）
	#define TACHO_MIN_R			199.1f				// タコメーターの0rpmの位置の角度
	#define TACHO_MAX_R			-18.0f				// タコメーターの9000rpmの位置の角度
	#define NEEDLE_COLOR		RGB(255,255,255)	// メーターの針の色
#endif

// グローバル変数:
HINSTANCE		hInst;							// 現在のインスタンス
TCHAR			szTitle[MAX_LOADSTRING];		// タイトル バー テキスト
TCHAR			szWindowClass[MAX_LOADSTRING];	// タイトル バー テキスト
HMODULE			m_hDLL;							// Pdh.dllのハンドル
DWORD			m_OS;							// プラットフォームＩＤ
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

class		CWTSetting {					// 設定情報クラス
public:
	DWORD		dwWindowPosX;				// ウィンドウの位置Ｘ
	DWORD		dwWindowPosY;				// ウィンドウの位置Ｙ
	BOOL		bTopmost;					// 常に手前に表示
	UINT		uiSize;						// サイズ 0:Large 1:Normal 2:Small
	UINT		uiTimerElapse;				// タイマー間隔
	UINT		uiResponse;					// レスポンス
}			m_SettingInfo;

class		CWTNeddle {						// 針情報クラス
public:
	POINT		poCenter;					// 中心座標
	int			uiCenterR;					// 中心軸の半径
	int			uiLeng;						// 針の長さ
	int			uiLengB;					// 針の長さ（反対側）
	float		fMinR;						// 0km,0rpmの位置の角度
	float		fMaxR;						// 180km,9000rpmの位置の角度
}			*m_NeedleInfo;

// このコード モジュールに含まれる関数の前宣言:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	SettingDlgProc(HWND, UINT, WPARAM, LPARAM);

HRGN				CreateRgnFromBmp(HBITMAP, COLORREF);
void				MeterDraw(HWND, HDC);
void				ShowMyBMP(HWND, HDC);
void				CalcSpTc(void);
void				UpdateSize(HWND);
void				DrawNeedle(HDC);
void				DrawCenterCircle(HDC);
void				ChangeTopmost(HWND);
int					GetCPUUsage_95(void);
double				GetCPUUsage_NT(void);
void				PdhStatusCheck(int, PDH_STATUS);
void				ShowReadme(void);
BOOL				IsExistFile(LPCTSTR);
void				MakePopupMenu(HWND);
void				SetInfoToReg(HKEY);
PDH_STATUS			(FAR WINAPI *pPdhOpenQuery)(LPVOID, DWORD, HQUERY*);
PDH_STATUS			(FAR WINAPI *pPdhAddCounter)(HQUERY, LPCTSTR, DWORD, HCOUNTER*);
PDH_STATUS			(FAR WINAPI *pPdhCollectQueryData)(HQUERY);
PDH_STATUS			(FAR WINAPI *pPdhGetFormattedCounterValue)(HCOUNTER, DWORD, LPDWORD, PPDH_FMT_COUNTERVALUE);
PDH_STATUS			(FAR WINAPI *pPdhCloseQuery)(HQUERY);

#endif // !defined(AFX_WINTACHOS_H__AA88F7B7_D84D_4D55_BB3D_258AD676D713__INCLUDED_)
