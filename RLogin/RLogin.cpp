// RLogin.cpp : アプリケーションのクラス動作を定義します。
//

#include "stdafx.h"
#include "RLogin.h"

#ifdef	USE_DWMAPI
#include "DWMApi.h"
#endif

#ifdef	USE_JUMPLIST
#include <Shobjidl.h>
#include <Shlobj.h>
#endif

#include "MainFrm.h"
#include "ChildFrm.h"
#include "RLoginDoc.h"
#include "RLoginView.h"
#include "ExtSocket.h"

#ifdef	_DEBUGXX
#include "Script.h"
#endif

#include <direct.h>
#include <openssl/ssl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// CCommandLineInfoEx

CCommandLineInfoEx::CCommandLineInfoEx()
{
	m_PasStat = 0;
	m_Proto = (-1);
	m_Addr.Empty();
	m_Port.Empty();
	m_User.Empty();
	m_Pass.Empty();
	m_Term.Empty();
	m_Name.Empty();
	m_InUse = FALSE;
}
void CCommandLineInfoEx::ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast)
{
	switch(m_PasStat) {
	case 0:
		if ( !bFlag )
			break;
		if ( _tcsicmp("ip", pszParam) == 0 )
			m_PasStat = 1;
		else if ( _tcsicmp("port", pszParam) == 0 )
			m_PasStat = 2;
		else if ( _tcsicmp("user", pszParam) == 0 )
			m_PasStat = 3;
		else if ( _tcsicmp("pass", pszParam) == 0 )
			m_PasStat = 4;
		else if ( _tcsicmp("term", pszParam) == 0 )
			m_PasStat = 5;
		else if ( _tcsicmp("entry", pszParam) == 0 )
			m_PasStat = 6;
		else if ( _tcsicmp("direct", pszParam) == 0 )
			m_Proto = PROTO_DIRECT;
		else if ( _tcsicmp("login", pszParam) == 0 ) {
			m_Proto = PROTO_LOGIN;
			m_Port  = "login";
		} else if ( _tcsicmp("telnet", pszParam) == 0 ) {
			m_Proto = PROTO_TELNET;
			m_Port  = "telnet";
		} else if ( _tcsicmp("ssh", pszParam) == 0 ) {
			m_Proto = PROTO_SSH;
			m_Port  = "ssh";
		} else if ( _tcsicmp("inuse", pszParam) == 0 )
			m_InUse = TRUE;
		else
			break;
		ParseLast(bLast);
		return;
	case 1:
		m_PasStat = 0;
		if ( bFlag )
			break;
		m_Addr = pszParam;
		ParseLast(bLast);
		return;
	case 2:
		m_PasStat = 0;
		if ( bFlag )
			break;
		m_Port = pszParam;
		ParseLast(bLast);
		return;
	case 3:
		m_PasStat = 0;
		if ( bFlag )
			break;
		m_User = pszParam;
		ParseLast(bLast);
		return;
	case 4:
		m_PasStat = 0;
		if ( bFlag )
			break;
		m_Pass = pszParam;
		ParseLast(bLast);
		return;
	case 5:
		m_PasStat = 0;
		if ( bFlag )
			break;
		m_Term = pszParam;
		ParseLast(bLast);
		return;
	case 6:
		m_PasStat = 0;
		if ( bFlag )
			break;
		m_Name = pszParam;
		ParseLast(bLast);
		return;
	}
	if ( !bFlag && ParseUrl(pszParam) ) {
		ParseLast(bLast);
		return;
	}
	CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
}
BOOL CCommandLineInfoEx::ParseUrl(const TCHAR* pszParam)
{
	// ssh://username:password@hostname:port/path?arg=value#anchor
	int n;
	CString tmp, str;
	static const struct {
		int		type;
		LPCTSTR	name;
		int		len;
	} proto[] = {
		{ PROTO_LOGIN,		_T("login"),	5	},
		{ PROTO_TELNET,		_T("telnet"),	6	},
		{ PROTO_SSH,		_T("ssh"),		3	},
		{ 0,				NULL,			0	},
	};

	for ( n = 0 ; proto[n].name != NULL ; n++ ) {
		if ( _tcsnicmp(proto[n].name, pszParam, proto[n].len) == 0 )
			break;
	}
	if ( proto[n].name == NULL )
		return FALSE;
	pszParam += proto[n].len;
	if ( _tcsnicmp(_T("://"), pszParam, 3) != 0 )
		return FALSE;
	pszParam += 3;

	m_Proto = proto[n].type;
	m_Port  = proto[n].name;

	tmp = pszParam;

	if ( (n = tmp.Find(_T('@'))) >= 0 ) {
		str = tmp.Left(n);
		tmp.Delete(0, n + 1);

		if ( (n = str.Find(_T(':'))) >= 0 ) {
			m_User = str.Left(n);
			m_Pass = str.Mid(n + 1);
		} else
			m_User = str;
	}

	str = tmp.SpanExcluding(_T("/?#"));

	if ( (n = str.Find(_T(':'))) >= 0 ) {
		m_Addr = str.Left(n);
		m_Port = str.Mid(n + 1);
	} else
		m_Addr = str;

	return TRUE;
}
void CCommandLineInfoEx::GetString(CString &str)
{
	CStringArrayExt ary;

	ary.AddVal(m_Proto);
	ary.Add(m_Addr);
	ary.Add(m_Port);
	ary.Add(m_User);
	ary.Add(m_Pass);
	ary.Add(m_Term);
	ary.AddVal(m_InUse);
	ary.Add(m_strFileName);
	ary.AddVal(m_nShellCommand);
	ary.Add(m_Name);

	ary.SetString(str);
}
void CCommandLineInfoEx::SetString(LPCSTR str)
{
	CStringArrayExt ary;

	ary.GetString(str);

	if ( ary.GetSize() <= 9 )
		return;

	m_Proto = ary.GetVal(0);
	m_Addr  = ary.GetAt(1);
	m_Port  = ary.GetAt(2);
	m_User  = ary.GetAt(3);
	m_Pass  = ary.GetAt(4);
	m_Term  = ary.GetAt(5);
	m_InUse = ary.GetVal(6);
	m_strFileName = ary.GetAt(7);
	switch(ary.GetVal(8)) {
	case FileNew:  m_nShellCommand = FileNew;  break;
	case FileOpen: m_nShellCommand = FileOpen; break;
	}
	m_Name = ary.GetAt(9);
}

//////////////////////////////////////////////////////////////////////
// CRLoginApp

BEGIN_MESSAGE_MAP(CRLoginApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CRLoginApp::OnAppAbout)
	// 標準のファイル基本ドキュメント コマンド
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CRLoginApp コンストラクション

CRLoginApp::CRLoginApp()
{
	// TODO: この位置に構築用コードを追加してください。
	// ここに InitInstance 中の重要な初期化処理をすべて記述してください。
	m_NextSock = 0;
#ifdef	USE_DIRECTWRITE
	m_pD2DFactory    = NULL;
	m_pDWriteFactory = NULL;
#endif
}


// 唯一の CRLoginApp オブジェクトです。

CRLoginApp theApp;

#ifdef	USE_DWMAPI
	HMODULE ExDwmApi = NULL;
	BOOL ExDwmEnable = FALSE;
	HRESULT (__stdcall *ExDwmIsCompositionEnabled)(BOOL * pfEnabled) = NULL;
	HRESULT (__stdcall *ExDwmEnableBlurBehindWindow)(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind) = NULL;
	HRESULT (__stdcall *ExDwmExtendFrameIntoClientArea)(HWND hWnd, const MARGINS* pMarInset) = NULL;
#endif

void ExDwmEnableWindow(HWND hWnd)
{
#ifdef	USE_DWMAPI
	if ( ExDwmEnable && ExDwmApi != NULL && ExDwmEnableBlurBehindWindow != NULL && hWnd != NULL ) {
		//Create and populate the BlurBehind structre
		DWM_BLURBEHIND bb = {0};
		MARGINS margin = { -1 };

		//Enable Blur Behind and Blur Region;
		bb.dwFlags = DWM_BB_ENABLE;
		bb.fEnable = true;
		bb.hRgnBlur = NULL;
		bb.fTransitionOnMaximized = false;
		//Enable Blur Behind
		ExDwmEnableBlurBehindWindow(hWnd, &bb);

		//margin.cxLeftWidth = margin.cxRightWidth = 0;
		//margin.cyTopHeight = 40;
		//margin.cyBottomHeight = 0;
		//ExDwmExtendFrameIntoClientArea(hWnd, &margin);
	}
#endif
}

// CRLoginApp 初期化

BOOL CRLoginApp::InitInstance()
{
	//TODO: call AfxInitRichEdit2() to initialize richedit2 library.
	// アプリケーション マニフェストが visual スタイルを有効にするために、
	// ComCtl32.dll Version 6 以降の使用を指定する場合は、
	// Windows XP に InitCommonControlsEx() が必要です。さもなければ、ウィンドウ作成はすべて失敗します。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);

	// アプリケーションで使用するすべてのコモン コントロール クラスを含めるには、
	// これを設定します。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

#ifdef	WINSOCK11
	if ( !AfxSocketInit() ) {
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
#else
	WORD wVersionRequested;
	wVersionRequested = MAKEWORD( 2, 2 );
	if ( WSAStartup( wVersionRequested, &wsaData ) != 0 ) {
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}
#endif

	// 標準初期化
	// これらの機能を使わずに最終的な実行可能ファイルの
	// サイズを縮小したい場合は、以下から不要な初期化
	// ルーチンを削除してください。
	// 設定が格納されているレジストリ キーを変更します。
	// TODO: 会社名または組織名などの適切な文字列に
	// この文字列を変更してください。
	SetRegistryKey(_T("Culti"));
	LoadStdProfileSettings(4);  // 標準の INI ファイルのオプションをロードします (MRU を含む)

#ifdef	USE_DWMAPI
	if ( (ExDwmApi = LoadLibrary("dwmapi.dll")) != NULL ) {
		ExDwmIsCompositionEnabled      = (HRESULT (__stdcall *)(BOOL* pfEnabled))GetProcAddress(ExDwmApi, "DwmIsCompositionEnabled");
		ExDwmEnableBlurBehindWindow    = (HRESULT (__stdcall *)(HWND hWnd, const DWM_BLURBEHIND* pBlurBehind))GetProcAddress(ExDwmApi, "DwmEnableBlurBehindWindow");
		ExDwmExtendFrameIntoClientArea = (HRESULT (__stdcall *)(HWND hWnd, const MARGINS* pMarInset))GetProcAddress(ExDwmApi, "DwmExtendFrameIntoClientArea");

		if ( ExDwmIsCompositionEnabled != NULL )
			ExDwmIsCompositionEnabled(&ExDwmEnable);
	}
#endif

#ifdef	USE_DIRECTWRITE
	if ( SUCCEEDED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory)) )
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(m_pDWriteFactory), reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
#endif

#ifdef	USE_JUMPLIST
	ICustomDestinationList *pJumpList = NULL;

	// COM の初期化
	if ( SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)) ) {
		// インスタンスの作成
		if ( SUCCEEDED(CoCreateInstance(CLSID_DestinationList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pJumpList))) ) {
			// カテゴリの追加
			UINT uMaxSlots;
			IObjectCollection *pObjCol;
	        IShellLink *pSheLink;
			IObjectArray *pObjSlots, *pObjArray;

			if ( SUCCEEDED(pJumpList->BeginList(&uMaxSlots, IID_PPV_ARGS(&pObjSlots))) ) {

				//pJumpList->AppendKnownCategory(KDC_FREQUENT);		// よく使うファイルリスト
				//pJumpList->AppendKnownCategory(KDC_RECENT);		// 最近使ったファイル (Default)
			
				if ( SUCCEEDED(CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pObjCol))) ) {

					if ( SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pSheLink))) ) {
						pSheLink->SetArguments("/inuse");
						pSheLink->SetDescription("inuse");
						pSheLink->SetIconLocation("Path", 0);

						//IPropertyStore *pProStore;
						//if ( SUCCEEDED(pSheLink->QueryInterface(IID_IPropertyStore, (void **)&pProStore)) ) {
						//	pProStore->SetValue(&PKEY_Title, &pv);
						//	pProStore->Release();
						//}

						pObjCol->AddObject(pSheLink);
						pSheLink->Release();
					}

					if ( SUCCEEDED(pObjCol->QueryInterface(IID_PPV_ARGS(&pObjArray))) ) {
						pJumpList->AppendCategory(L"Category", pObjArray);
						pObjArray->Release();
					}
					pObjCol->Release();
				}

				if ( SUCCEEDED(CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pObjCol))) ) {
					if ( SUCCEEDED(pObjCol->QueryInterface(IID_PPV_ARGS(&pObjArray))) ) {
						pJumpList->AddUserTasks(pObjArray);
						pObjArray->Release();
					}
					pObjCol->Release();
				}

				if ( SUCCEEDED(CoCreateInstance(CLSID_EnumerableObjectCollection, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&pObjCol))) ) {
					if ( SUCCEEDED(pObjCol->QueryInterface(IID_PPV_ARGS(&pObjArray))) ) {
						pJumpList->AddUserTasks(pObjArray);
						pObjArray->Release();
					}
					pObjCol->Release();
				}

				// クリーンアップ
				pJumpList->CommitList();
				pObjSlots->Release();
			}

			pJumpList->Release();
		}

		CoUninitialize();
	}
#endif

	// アプリケーション用のドキュメント テンプレートを登録します。ドキュメント テンプレート
	//  はドキュメント、フレーム ウィンドウとビューを結合するために機能します。
	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(IDR_RLOGINTYPE,
		RUNTIME_CLASS(CRLoginDoc),
		RUNTIME_CLASS(CChildFrame), // カスタム MDI 子フレーム
		RUNTIME_CLASS(CRLoginView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// メイン MDI フレーム ウィンドウを作成します。
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	// 接尾辞が存在する場合にのみ DragAcceptFiles を呼び出します。
	//  MDI アプリケーションでは、この呼び出しは、m_pMainWnd を設定した直後に発生しなければなりません。
	// ドラッグ/ドロップ オープンを許可します。
	m_pMainWnd->DragAcceptFiles(FALSE);

	// DDE Execute open を使用可能にします。

	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// DDE、file open など標準のシェル コマンドのコマンド ラインを解析します。
	CCommandLineInfoEx cmdInfo;
	ParseCommandLine(cmdInfo);
	m_pCmdInfo = &cmdInfo;

	if ( cmdInfo.m_InUse && InUseCheck() )
		return FALSE;

	::SetWindowLongPtr(m_pMainWnd->GetSafeHwnd(), GWLP_USERDATA, 0x524c4f47);

	// コマンド ラインで指定されたディスパッチ コマンドです。アプリケーションが
	// /RegServer、/Register、/Unregserver または /Unregister で起動された場合、False を返します。
	// if (!ProcessShellCommand(cmdInfo))
	//	return FALSE;

	char tmp[_MAX_DIR];
	if ( _getcwd(tmp, _MAX_DIR) != NULL )
		m_BaseDir = tmp;
	else {
		int n;
		m_BaseDir = _pgmptr;
		if ( (n = m_BaseDir.ReverseFind('\\')) >= 0 )
			m_BaseDir = m_BaseDir.Left(n);
	}

#ifdef	_DEBUGXX
	CScript script;
	script.ExecFile("script.txt");
#endif

	// メイン ウィンドウが初期化されたので、表示と更新を行います。
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	switch(cmdInfo.m_nShellCommand) {
	case CCommandLineInfo::FileNew:
		OnFileNew();
		break;
	case CCommandLineInfo::FileOpen:
		if ( OpenDocumentFile(cmdInfo.m_strFileName) == NULL )
			OnFileNew();
		break;
	}
	m_pCmdInfo = NULL;

	return TRUE;
}

void CRLoginApp::OpenProcsCmd(CCommandLineInfoEx *pCmdInfo)
{
	m_pCmdInfo = pCmdInfo;
	switch(pCmdInfo->m_nShellCommand) {
	case CCommandLineInfo::FileNew:
		OnFileNew();
		break;
	case CCommandLineInfo::FileOpen:
		if ( OpenDocumentFile(pCmdInfo->m_strFileName) == NULL )
			OnFileNew();
		break;
	}
	m_pCmdInfo = NULL;
}
BOOL CALLBACK RLoginEnumFunc(HWND hwnd, LPARAM lParam)
{
	CWnd wnd;
	CRLoginApp *pApp = (CRLoginApp *)lParam;
	TCHAR title[1024];
	CString cmdLine;

	::GetWindowText(hwnd, title, 1024);

	if ( _tcsncmp(title, "RLogin", 6) == 0 && ::GetWindowLongPtr(hwnd, GWLP_USERDATA) == 0x524c4f47 ) {
		pApp->m_pCmdInfo->m_InUse = FALSE;
		pApp->m_pCmdInfo->GetString(cmdLine);
		COPYDATASTRUCT copyData;
		copyData.dwData = 0x524c4f31;
		copyData.cbData = cmdLine.GetAllocLength();
		copyData.lpData = cmdLine.GetBuffer();
		::SendMessage(hwnd, WM_COPYDATA, (WPARAM)(pApp->m_pMainWnd->GetSafeHwnd()), (LPARAM)&copyData);
		return FALSE;
	}

	return TRUE;
}
BOOL CRLoginApp::InUseCheck()
{
	::EnumWindows(RLoginEnumFunc, (LPARAM)this);
	return (m_pCmdInfo == NULL || m_pCmdInfo->m_InUse ? FALSE : TRUE);
}
void CRLoginApp::SetSocketIdle(class CExtSocket *pSock)
{
	for ( int n = 0 ; n < m_SocketIdle.GetSize() ; n++ ) {
		if ( m_SocketIdle[n] == (void *)pSock )
			return;
	}
	m_SocketIdle.Add((void *)pSock);
}
void CRLoginApp::DelSocketIdle(class CExtSocket *pSock)
{
	for ( int n = 0 ; n < m_SocketIdle.GetSize() ; n++ ) {
		if ( m_SocketIdle[n] == (void *)pSock ) {
			m_SocketIdle.RemoveAt(n);
			break;
		}
	}
}
void CRLoginApp::GetProfileData(LPCTSTR lpszSection, LPCTSTR lpszEntry, void *lpBuf, int nBufLen, void *lpDef)
{
	LPBYTE pData;
	UINT len;

	if ( GetProfileBinary(lpszSection, lpszEntry, &pData, &len) ) {
		if ( len == (UINT)nBufLen )
			memcpy(lpBuf, pData, nBufLen);
		else
			memcpy(lpBuf, lpDef, nBufLen);
		delete pData;
	} else if ( lpDef != NULL )
		memcpy(lpBuf, lpDef, nBufLen);
}
void CRLoginApp::GetProfileBuffer(LPCTSTR lpszSection, LPCTSTR lpszEntry, CBuffer &Buf)
{
	LPBYTE pData;
	UINT len;

	Buf.Clear();
	if ( GetProfileBinary(lpszSection, lpszEntry, &pData, &len) ) {
		Buf.Apend(pData, len);
		delete pData;
	}
}
void CRLoginApp::GetProfileArray(LPCTSTR lpszSection, CStringArrayExt &array)
{
	int n, mx;
	CString entry;
	
	array.RemoveAll();
	mx = GetProfileInt(lpszSection, "ListMax", 0);
	for ( n = 0 ; n < mx ; n++ ) {
		entry.Format("List%02d", n);
		array.Add(GetProfileString(lpszSection, entry, ""));
	}
}
void CRLoginApp::WriteProfileArray(LPCTSTR lpszSection, CStringArrayExt &array)
{
	int n;
	CString entry;

	for ( n = 0 ; n < array.GetSize() ; n++ ) {
		entry.Format("List%02d", n);
		WriteProfileString(lpszSection, entry, array.GetAt(n));
	}
	WriteProfileInt(lpszSection, "ListMax", n);
}
int CRLoginApp::GetProfileSeqNum(LPCTSTR lpszSection, LPCTSTR lpszEntry)
{
	CMutexLock Mutex(lpszEntry);
	int num = GetProfileInt(lpszSection, lpszEntry, 0) ;
	WriteProfileInt(lpszSection, lpszEntry, num + 1);
	return num;
}
void CRLoginApp::GetProfileKeys(LPCTSTR lpszSection, CStringArrayExt &array)
{
	array.RemoveAll();
	HKEY hAppKey;
	if ( (hAppKey = GetAppRegistryKey()) == NULL )
		return;
	HKEY hSecKey;
	if ( RegOpenKeyEx(hAppKey, lpszSection, 0, KEY_READ, &hSecKey) == ERROR_SUCCESS && hSecKey != NULL ) {
		TCHAR name[1024];
		DWORD len = 1024;
		for ( int n = 0 ; RegEnumValue(hSecKey, n, name, &len, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS ; n++, len = 1024 )
			array.Add(name);
		RegCloseKey(hSecKey);
	}
	RegCloseKey(hAppKey);
}
void CRLoginApp::DelProfileEntry(LPCTSTR lpszSection, LPCTSTR lpszEntry)
{
	HKEY hSecKey;
	if ( (hSecKey = GetSectionKey(lpszSection)) == NULL )
		return;
	RegDeleteValue(hSecKey, lpszEntry);
	RegCloseKey(hSecKey);
}

void CRLoginApp::RegisterShellProtocol(LPCSTR pSection, LPCSTR pOption)
{
	HKEY hKey[4];
	DWORD val;
	CString strTemp;
	CString strPathName;
	CString oldDefine;
	CBuffer buf;
	CRegKey reg;

	AfxGetModuleShortFileName(AfxGetInstanceHandle(), strPathName);

	CString strOpenCommandLine;
	CString strDefaultIconCommandLine;

	strDefaultIconCommandLine.Format("%s,%d", strPathName, 1);
	strOpenCommandLine.Format("%s %s %%1", strPathName, pOption);

	//	HKEY_CLASSES_ROOT or HKEY_CURRENT_USER\Software\Classes
	//
	//	[HKEY_CLASSES_ROOT\ssh]
	//	@ = "URL: ssh Protocol"
	//	BrowserFlags = dword:00000008
	//	EditFlags = dword:00000002
	//	URL Protocol = ""

	//	[HKEY_CLASSES_ROOT\ssh\DefaultIcon]
	//	@ = "RLogin.exe,1"

	//	[HKEY_CLASSES_ROOT\ssh\shell]
	//	[HKEY_CLASSES_ROOT\ssh\shell\open]
	//	[HKEY_CLASSES_ROOT\ssh\shell\open\command]
	//	@ = "RLogin.exe /term xterm /inuse %1"

	strTemp.Format("Software\\Classes\\%s", pSection);

	if ( reg.Open(HKEY_CURRENT_USER, strTemp) == ERROR_SUCCESS ) {
		ULONG len = 0;
		if ( reg.QueryBinaryValue("OldDefine", NULL, &len) == ERROR_SUCCESS )
			reg.QueryBinaryValue("OldDefine", buf.PutSpc(len), &len);
		else
			RegisterSave(HKEY_CURRENT_USER, strTemp, buf);
		buf.Clear();
		reg.Close();
	}

	if( AfxRegCreateKey(HKEY_CURRENT_USER, strTemp, &(hKey[0])) == ERROR_SUCCESS ) {

		strTemp = "URL: ssh Protocol";
		RegSetValueEx(hKey[0], "", 0, REG_SZ, (const LPBYTE)(LPCSTR)strTemp, strTemp.GetLength() + 1);
		val = 8;
		RegSetValueEx(hKey[0], "BrowserFlags", 0, REG_DWORD, (const LPBYTE)(&val), sizeof(val));
		val = 2;
		RegSetValueEx(hKey[0], "EditFlags", 0, REG_DWORD, (const LPBYTE)(&val), sizeof(val));
		strTemp = "";
		RegSetValueEx(hKey[0], "URL Protocol", 0, REG_SZ, (const LPBYTE)(LPCSTR)strTemp, strTemp.GetLength() + 1);

		RegSetValueEx(hKey[0], "OldDefine", 0, REG_BINARY, (const LPBYTE)buf.GetPtr(), buf.GetSize());

		if( AfxRegCreateKey(hKey[0], "DefaultIcon", &(hKey[1])) == ERROR_SUCCESS ) {
			RegSetValueEx(hKey[1], "", 0, REG_SZ, (const LPBYTE)(LPCSTR)strDefaultIconCommandLine, strDefaultIconCommandLine.GetLength() + 1);
			RegCloseKey(hKey[1]);
		}

		if( AfxRegCreateKey(hKey[0], "shell", &(hKey[1])) == ERROR_SUCCESS ) {
			if( AfxRegCreateKey(hKey[1], "open", &(hKey[2])) == ERROR_SUCCESS ) {
				if( AfxRegCreateKey(hKey[2], "command", &(hKey[3])) == ERROR_SUCCESS ) {
					RegSetValueEx(hKey[3], "", 0, REG_SZ, (const LPBYTE)(LPCSTR)strOpenCommandLine, strOpenCommandLine.GetLength() + 1);
					RegCloseKey(hKey[3]);
				}
				RegCloseKey(hKey[2]);
			}
			RegCloseKey(hKey[1]);
		}

		RegCloseKey(hKey[0]);
	}
}
void CRLoginApp::RegisterDelete(HKEY hKey, LPCSTR pSection, LPCSTR pKey)
{
	CRegKey reg;

	if ( reg.Open(hKey, pSection) != ERROR_SUCCESS )
		return;

	reg.RecurseDeleteKey(pKey);
	reg.Close();
}
void CRLoginApp::RegisterSave(HKEY hKey, LPCSTR pSection, CBuffer &buf)
{
	int n;
	CRegKey reg;
	DWORD type, len;
	CHAR *work;
	CStringArray menba;

	if ( reg.Open(hKey, pSection) != ERROR_SUCCESS )
		return;

	len = 0;
	reg.QueryValue("", &type, NULL, &len);
	if ( len < 1024 )
		len = 1024;
	work = new CHAR[len];
	reg.QueryValue("", &type, work, &len);

	buf.Put32Bit(type);
	buf.PutBuf((LPBYTE)work, len);

	for ( n = 0 ; ; n++ ) {
		len = 1020;
		if ( reg.EnumKey(n, work, &len) != ERROR_SUCCESS )
			break;
		work[len] = '\0';
		menba.Add(work);
	}

	delete work;

	buf.Put32Bit(menba.GetSize());
	for ( n = 0 ; n < menba.GetSize() ; n++ ) {
		buf.PutStr(menba[n]);
		RegisterSave(reg.m_hKey, menba[n], buf);
	}

	reg.Close();
}
void CRLoginApp::RegisterLoad(HKEY hKey, LPCSTR pSection, CBuffer &buf)
{
	int n;
	CRegKey reg;
	DWORD type, len;
	CString name;
	CBuffer work;

	if ( buf.GetSize() < 4 )
		return;

	if ( reg.Create(hKey, pSection) != ERROR_SUCCESS )
		return;

	type = buf.Get32Bit();
	buf.GetBuf(&work);
	reg.SetValue("", type, work.GetPtr(), work.GetSize());

	len = buf.Get32Bit();
	for ( n = 0 ; n < len ; n++ ) {
		buf.GetStr(name);
		RegisterLoad(reg.m_hKey, name, buf);
	}

	reg.Close();
}

// アプリケーションのバージョン情報に使われる CAboutDlg ダイアログ

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ダイアログ データ
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// ダイアログを実行するためのアプリケーション コマンド
void CRLoginApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CRLoginApp メッセージ ハンドラ

BOOL CRLoginApp::OnIdle(LONG lCount) 
{
	if ( CWinApp::OnIdle(lCount) )
		return TRUE;

	for ( int n = 0 ; n < m_SocketIdle.GetSize() ; n++ ) {
		if ( m_NextSock >= m_SocketIdle.GetSize() )
			m_NextSock = 0;
		CExtSocket *pSock = (CExtSocket *)(m_SocketIdle[m_NextSock]);
		if ( ++m_NextSock >= m_SocketIdle.GetSize() )
			m_NextSock = 0;
		if ( pSock->OnIdle() )
			return TRUE;
	}

	return FALSE;
}

int CRLoginApp::ExitInstance() 
{
	CRYPTO_cleanup_all_ex_data();
#ifndef	WINSOCK11
	WSACleanup();
#endif

#ifdef	USE_DWMAPI
	if ( ExDwmApi != NULL )
		FreeLibrary(ExDwmApi);
#endif

#ifdef	USE_DIRECTWRITE
	if ( m_pDWriteFactory != NULL )
		m_pDWriteFactory->Release();
	if ( m_pD2DFactory != NULL )
		m_pD2DFactory->Release();
#endif

	return CWinApp::ExitInstance();
}

BOOL CRLoginApp::SaveAllModified() 
{
	CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	if ( pMain != NULL && !pMain->SaveModified() )
		return FALSE;

	return CWinApp::SaveAllModified();
}
void CRLoginApp::SSL_Init()
{
	static BOOL bLoadAlgo = FALSE;

	if ( bLoadAlgo )
		return;
	bLoadAlgo = TRUE;

	SSLeay_add_all_algorithms();
	SSLeay_add_ssl_algorithms();
}