
#if !defined(AFX_WINTACHOS_H__AA88F7B7_D84D_4D55_BB3D_258AD676D713__INCLUDED_)
#define AFX_WINTACHOS_H__AA88F7B7_D84D_4D55_BB3D_258AD676D713__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <shellapi.h>
#include "resource.h"
#include "pdh.h"
#include "pdhmsg.h"
#pragma comment(lib, "pdh.lib")

#define VERSION					2.00				// �o�[�W����
#define COLORKEY					RGB(1, 0, 1)			// Layered window transparency key

#define MAX_LOADSTRING 100
#define TIMERID					1					// WM_PAINT���s�̂��߂̃^�C�}�[ID
#define	MM_TRAY					(WM_USER + 0)		// �^�X�N�g���C�̃A�C�R���N���b�N��
#define LOGMAX					200					// �L�^���

#ifdef _PS13
	#define WINDOWSZ_X			600					// �T�C�YHuge���̃E�B���h�E�T�C�Y�w
	#define WINDOWSZ_Y			200					// �T�C�YHuge���̃E�B���h�E�T�C�Y�x
	#define SPEEDM_CENTER_X		147					// �X�s�[�h���[�^�[�̒��S���W�w
	#define SPEEDM_CENTER_Y		150					// �X�s�[�h���[�^�[�̒��S���W�x
	#define SPEEDM_CENTER_R		12					// �X�s�[�h���[�^�[�̒��S���̔��a
	#define SPEEDM_LENG			131					// �X�s�[�h���[�^�[�̐j�̒���
	#define SPEEDM_LENG_B		22					// �X�s�[�h���[�^�[�̐j�̒����i���Α��j
	#define SPEED_MIN_R			198.4f				// �X�s�[�h���[�^�[��0km�̈ʒu�̊p�x
	#define SPEED_MAX_R			-18.6f				// �X�s�[�h���[�^�[��180km�̈ʒu�̊p�x
	#define TACHOM_CENTER_X		452					// �^�R���[�^�[�̒��S���W�w
	#define TACHOM_CENTER_Y		150					// �^�R���[�^�[�̒��S���W�x
	#define TACHOM_CENTER_R		12					// �^�R���[�^�[�̒��S���̔��a
	#define TACHOM_LENG			131					// �^�R���[�^�[�̐j�̒���
	#define TACHOM_LENG_B		22					// �^�R���[�^�[�̐j�̒����i���Α��j
	#define TACHO_MIN_R			199.1f				// �^�R���[�^�[��0rpm�̈ʒu�̊p�x
	#define TACHO_MAX_R			-18.0f				// �^�R���[�^�[��9000rpm�̈ʒu�̊p�x
	#define NEEDLE_COLOR		RGB(255,255,255)	// ���[�^�[�̐j�̐F
#endif

class		CWTSetting {					// �ݒ���N���X
public:
	DWORD		dwWindowPosX;				// �E�B���h�E�̈ʒu�w
	DWORD		dwWindowPosY;				// �E�B���h�E�̈ʒu�x
	BOOL		bTopmost;					// ��Ɏ�O�ɕ\��
	UINT		uiSize;						// �T�C�Y 0:Large 1:Normal 2:Small
	UINT		uiTimerElapse;				// �^�C�}�[�Ԋu
	UINT		uiResponse;					// ���X�|���X
}			m_SettingInfo;

class		CWTNeedle {						// �j���N���X
public:
	POINT		poCenter;					// ���S���W
	int			uiCenterR;					// ���S���̔��a
	int			uiLeng;						// �j�̒���
	int			uiLengB;					// �j�̒����i���Α��j
	float		fMinR;						// 0km,0rpm�̈ʒu�̊p�x
	float		fMaxR;						// 180km,9000rpm�̈ʒu�̊p�x
}			*m_NeedleInfo;

// ���̃R�[�h ���W���[���Ɋ܂܂��֐��̑O�錾:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	AboutDlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	SettingDlgProc(HWND, UINT, WPARAM, LPARAM);

HRGN				CreateRgnFromBmp(HBITMAP, COLORREF);
void				MeterDraw(HWND);
void				BuildAlphaMask(HBITMAP, int, int);
void				ShowMyBMP(HWND, HDC);
void				CalcSpTc(void);
void				UpdateSize(HWND);
void				DrawNeedle(HDC);
void				DrawCenterCircle(HDC);
void				ChangeTopmost(HWND);
double				GetCPUUsage_NT(void);
void				PdhStatusCheck(int, PDH_STATUS);
void				ShowReadme(void);
BOOL				IsExistFile(LPCTSTR);
void				MakePopupMenu(HWND);
void				SetInfoToReg(HKEY);

#endif // !defined(AFX_WINTACHOS_H__AA88F7B7_D84D_4D55_BB3D_258AD676D713__INCLUDED_)
