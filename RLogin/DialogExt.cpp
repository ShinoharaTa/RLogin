// DialogExt.cpp : 実装ファイル
//

#include "stdafx.h"
#include "RLogin.h"
#include "MainFrm.h"
#include "Data.h"
#include "DialogExt.h"

//////////////////////////////////////////////////////////////////////

CShortCutKey::CShortCutKey()
{
	m_MsgID   = 0;
	m_KeyCode = 0;
	m_KeyWith = 0;
	m_CtrlID  = 0;
	m_wParam  = 0;
}
CShortCutKey::~CShortCutKey()
{
}
const CShortCutKey & CShortCutKey::operator = (CShortCutKey &data)
{
	m_MsgID   = data.m_MsgID;
	m_KeyCode = data.m_KeyCode;
	m_KeyWith = data.m_KeyWith;
	m_CtrlID  = data.m_CtrlID;
	m_wParam  = data.m_wParam;

	return *this;
}

//////////////////////////////////////////////////////////////////////
// CDialogExt ダイアログ

IMPLEMENT_DYNAMIC(CDialogExt, CDialog)

CDialogExt::CDialogExt(UINT nIDTemplate, CWnd *pParent)
	: CDialog(nIDTemplate, pParent)
{
	m_nIDTemplate = nIDTemplate;
	m_FontName = ::AfxGetApp()->GetProfileString(_T("Dialog"), _T("FontName"), _T(""));
	m_FontSize = ::AfxGetApp()->GetProfileInt(_T("Dialog"), _T("FontSize"), 9);

	CMainFrame *pMain;

	if ( (pMain = (CMainFrame *)::AfxGetMainWnd()) != NULL )
		m_FontSize = MulDiv(m_FontSize, pMain->m_ScreenDpiY, 96);
}
CDialogExt::~CDialogExt()
{
}
void CDialogExt::SetBackColor(COLORREF color)
{
	m_BkBrush.CreateSolidBrush(color);
}

#pragma pack(push, 1)
	typedef struct _DLGTEMPLATEEX {
		WORD dlgVer;
		WORD signature;
		DWORD helpID;
		DWORD exStyle;
		DWORD style;
		WORD cDlgItems;
		short x;
		short y;
		short cx;
		short cy;
	} DLGTEMPLATEEX, *LPCDLGTEMPLATEEX;
#pragma pack(pop)

BOOL CDialogExt::GetSizeAndText(SIZE *pSize, CString &title)
{
	HGLOBAL hDialog;
	HGLOBAL hInitData = NULL;
	LPCDLGTEMPLATEEX lpDialogTemplate;
	WORD *wp;

	if ( !((CRLoginApp *)AfxGetApp())->LoadResDialog(m_lpszTemplateName, hDialog, hInitData) )
		return FALSE;

	if ( (lpDialogTemplate = (LPCDLGTEMPLATEEX)LockResource(hDialog)) == NULL ) {
		FreeResource(hDialog);
		return FALSE;
	}

	CDialogTemplate dlgTemp((LPCDLGTEMPLATE)lpDialogTemplate);

	if ( IsDefineFont() )
		dlgTemp.SetFont(m_FontName, m_FontSize);

	dlgTemp.GetSizeInPixels(pSize);

	if ( lpDialogTemplate->signature == 0xFFFF )
		wp = (WORD *)((LPCDLGTEMPLATEEX)lpDialogTemplate + 1);
	else
		wp = (WORD *)((LPCDLGTEMPLATE)lpDialogTemplate + 1);

	if ( *wp == (WORD)(-1) )        // Skip menu name string or ordinal
		wp += 2; // WORDs
	else
		while( *(wp++) != 0 );

	if ( *wp == (WORD)(-1) )        // Skip class name string or ordinal
		wp += 2; // WORDs
	else
		while( *(wp++) != 0 );

	// caption string
	title = (WCHAR *)wp;

	UnlockResource(hDialog);
	FreeResource(hDialog);

	if ( hInitData != NULL )
		FreeResource(hInitData);

	return TRUE;
}

BOOL CDialogExt::Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd)
{
	HGLOBAL hDialog;
	HGLOBAL hInitData = NULL;
	void* lpInitData = NULL;
	LPCDLGTEMPLATE lpDialogTemplate;

	m_lpszTemplateName = lpszTemplateName;

	if ( IS_INTRESOURCE(m_lpszTemplateName) && m_nIDHelp == 0 )
		m_nIDHelp = LOWORD((DWORD_PTR)m_lpszTemplateName);

	if ( !((CRLoginApp *)AfxGetApp())->LoadResDialog(m_lpszTemplateName, hDialog, hInitData) )
		return (-1);

	if ( hInitData != NULL )
		lpInitData = (void *)LockResource(hInitData);

	lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialog);

	CDialogTemplate dlgTemp(lpDialogTemplate);

	if ( IsDefineFont() )
		dlgTemp.SetFont(m_FontName, m_FontSize);

	lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(dlgTemp.m_hTemplate);

	BOOL bResult = CreateIndirect(lpDialogTemplate, pParentWnd, lpInitData);

	UnlockResource(dlgTemp.m_hTemplate);

	UnlockResource(hDialog);
	FreeResource(hDialog);

	if ( hInitData != NULL ) {
		UnlockResource(hInitData);
		FreeResource(hInitData);
	}

	return bResult;
}
INT_PTR CDialogExt::DoModal()
{
	HGLOBAL hDialog;
	HGLOBAL hInitData = NULL;
	LPCDLGTEMPLATE lpDialogTemplate;

	if ( !((CRLoginApp *)AfxGetApp())->LoadResDialog(m_lpszTemplateName, hDialog, hInitData) )
		return (-1);

	lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialog);

	CDialogTemplate dlgTemp(lpDialogTemplate);

	if ( IsDefineFont() )
		dlgTemp.SetFont(m_FontName, m_FontSize);

	lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(dlgTemp.m_hTemplate);

	LPCTSTR pSaveTempName = m_lpszTemplateName;
	LPCDLGTEMPLATE pSaveDiaTemp = m_lpDialogTemplate;

	m_lpszTemplateName = NULL;
	m_lpDialogTemplate = NULL;

	InitModalIndirect(lpDialogTemplate, m_pParentWnd, hInitData);
	INT_PTR result = CDialog::DoModal();

	m_lpszTemplateName = pSaveTempName;
	m_lpDialogTemplate = pSaveDiaTemp;

	UnlockResource(dlgTemp.m_hTemplate);

	UnlockResource(hDialog);
	FreeResource(hDialog);

	if ( hInitData != NULL )
		FreeResource(hInitData);

	return result;
}
void CDialogExt::AddShortCutKey(UINT MsgID, UINT KeyCode, UINT KeyWith, UINT CtrlID, WPARAM wParam)
{
	//	AddShortCutKey(0, VK_RETURN, MASK_CTRL, 0, IDOK);
	//	AddShortCutKey(0, VK_RETURN, MASK_CTRL, IDC_BUTTON, BN_CLICKED);

	CShortCutKey data;

	data.m_MsgID   = MsgID;
	data.m_KeyCode = KeyCode;
	data.m_KeyWith = KeyWith;
	data.m_CtrlID  = CtrlID;
	data.m_wParam  = wParam;

	m_Data.Add(data);
}

BEGIN_MESSAGE_MAP(CDialogExt, CDialog)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////
// CDialogExt メッセージ ハンドラー

afx_msg HBRUSH CDialogExt::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if ( m_BkBrush.m_hObject == NULL )
		return hbr;

	switch(nCtlColor) {
	case CTLCOLOR_BTN:			// Button control
	case CTLCOLOR_SCROLLBAR:	// Scroll-bar control
	case CTLCOLOR_LISTBOX:		// List-box control
	case CTLCOLOR_MSGBOX:		// Message box
	case CTLCOLOR_EDIT:			// Edit control
		break;

	case CTLCOLOR_DLG:			// Dialog box
		hbr = m_BkBrush;
		break;

	case CTLCOLOR_STATIC:		// Static control
		hbr = m_BkBrush;
		pDC->SetBkMode(TRANSPARENT);
//		pDC->SetBkColor(GetSysColor(COLOR_WINDOW));
		break;
	}

	return hbr;
}

afx_msg LRESULT CDialogExt::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
	return ((CRLoginApp *)AfxGetApp())->OnIdle((LONG)lParam);
}

LRESULT CDialogExt::OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
	int DpiX = LOWORD(wParam);
	int DpiY = HIWORD(wParam);

	if ( (GetStyle() & WS_SIZEBOX) != 0 )
		MoveWindow((RECT *)lParam, TRUE);

	return TRUE;
}

BOOL CDialogExt::PreTranslateMessage(MSG* pMsg)
{
	int n;
	CShortCutKey *pShortCut;
	CWnd *pWnd = NULL;

	if ( pMsg->message == WM_KEYDOWN ) {
		for ( n = 0 ; n < m_Data.GetSize() ; n++ ) {
			pShortCut = &(m_Data[n]);

			if ( pShortCut->m_KeyCode != (UINT)(pMsg->wParam) )
				continue;

			if ( (pShortCut->m_KeyWith & MASK_SHIFT) != 0 && (GetKeyState(VK_SHIFT)   & 0x80) == 0 )
				continue;
			if ( (pShortCut->m_KeyWith & MASK_CTRL)  != 0 && (GetKeyState(VK_CONTROL) & 0x80) == 0 )
				continue;
			if ( (pShortCut->m_KeyWith & MASK_ALT)   != 0 && (GetKeyState(VK_MENU)    & 0x80) == 0 )
				continue;

			if ( pShortCut->m_MsgID != 0 && ((pWnd = GetDlgItem(pShortCut->m_MsgID)) == NULL || pMsg->hwnd != pWnd->GetSafeHwnd()) )
				continue;

			if ( pShortCut->m_CtrlID != 0 ) {	// Control identifier
				if ( (pWnd = GetDlgItem(pShortCut->m_CtrlID)) == NULL )
					continue;
				PostMessage(WM_COMMAND, MAKEWPARAM(pShortCut->m_CtrlID, pShortCut->m_wParam), (LPARAM)(pWnd->GetSafeHwnd()));

			} else								// IDOK or IDCANCEL ...
				PostMessage(WM_COMMAND, pShortCut->m_wParam);

			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
