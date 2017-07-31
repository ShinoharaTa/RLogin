#pragma once

#include "ListCtrlExt.h"
#include "DialogExt.h"

/////////////////////////////////////////////////////////////////////////////
// CPfdListDlg ダイアログ

class CPfdListDlg : public CDialogExt
{
	DECLARE_DYNAMIC(CPfdListDlg)

// コンストラクション
public:
	CPfdListDlg(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	enum { IDD = IDD_PFDLISTDLG };

public:
	class CServerEntry *m_pEntry;
	CListCtrlExt	m_List;
	CStringArrayExt m_PortFwd;
	BOOL m_X11PortFlag;
	CString m_XDisplay;
	BOOL m_x11AuthFlag;
	CString m_x11AuthName;
	CString m_x11AuthData;

	void InitList();

// オーバーライド
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();
	virtual void OnOK();

// インプリメンテーション
protected:
	afx_msg void OnPfdNew();
	afx_msg void OnPfdEdit();
	afx_msg void OnPfdDel();
	afx_msg void OnDblclkPfdlist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditDups();
	afx_msg void OnUpdateEditEntry(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};
