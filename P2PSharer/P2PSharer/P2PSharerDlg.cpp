
// P2PSharerDlg.cpp : 实现文件
//

#include "stdafx.h"

#include <io.h>
//#include <vector>
//
//using namespace std;

#include "P2PSharer.h"
#include "P2PSharerDlg.h"

#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 全局变量
CRITICAL_SECTION csDown;
CRITICAL_SECTION csTransfers;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CP2PSharerDlg 对话框




CP2PSharerDlg::CP2PSharerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CP2PSharerDlg::IDD, pParent),
	m_strWorkDir(_T("")), 
	m_sockLocalToSrv(INVALID_SOCKET), 
	m_paiSrv(NULL), 
	m_paiLocalToSrv(NULL),
	m_paiLocalToCli(NULL),
	m_pListCtrl(NULL),
	m_nSrcNUM(0),
	m_pProgressCtrl(NULL),
	m_strDownFilePath(_T("")),
	m_nDownloadIndex(0),
	m_bDistRSCWithDebug(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CP2PSharerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CP2PSharerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_BROWSE, &CP2PSharerDlg::OnBnClickedBtnBrowse)
	ON_BN_CLICKED(IDC_BTN_OPENSHAREFOLDER, &CP2PSharerDlg::OnBnClickedBtnOpensharefolder)
	ON_BN_CLICKED(IDC_BTN_SRVTEST, &CP2PSharerDlg::OnBnClickedBtnSrvtest)
	ON_BN_CLICKED(IDCANCEL, &CP2PSharerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_DISTRIBUTESOURCE, &CP2PSharerDlg::OnBnClickedBtnDistributesource)
	ON_BN_CLICKED(IDC_BTN_CHNSRVADDR, &CP2PSharerDlg::OnBnClickedBtnChnsrvaddr)
	ON_BN_CLICKED(IDC_BTN_RUNSRC, &CP2PSharerDlg::OnBnClickedBtnRunsrc)
	ON_BN_CLICKED(IDC_BTN_REFRESHSRC, &CP2PSharerDlg::OnBnClickedBtnRefreshsrc)
	ON_BN_CLICKED(IDC_BTN_DOWNLOADSRC, &CP2PSharerDlg::OnBnClickedBtnDownloadsrc)
END_MESSAGE_MAP()


// CP2PSharerDlg 消息处理程序

BOOL CP2PSharerDlg::OnInitDialog()
{
	CDialog::OnInitDialog(); 

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	OleInitialize(NULL);

	TCHAR path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);
	m_strWorkDir = path + m_strWorkDir + _T("\\") + SHARE_FOLDER_NAME + _T("\\");
	SetDlgItemText(IDC_EDIT_SHAREFOLDER, m_strWorkDir);

	// 创建文件夹
	CFileStatus status;
	if (!CFile::GetStatus(m_strWorkDir, status))
	{
		CreateDirectory(m_strWorkDir.GetBuffer(), NULL);
	}
	

	// 初始化IP地址
	SetDlgItemText(IDC_EDIT_SRVADDR, _T("fe80::c1ef:d87e:4586:87bd%11"));

	// 初始化WSA环境
	WSADATA wsaData;
	int nErr(0);
	CString strErr;

	nErr = WSAStartup(MAKEWORD(2,2), &wsaData);

	if (0 != nErr)
	{
		strErr.Format(_T("初始化WSA环境失败。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strErr, _T("错误"), MB_ICONERROR);
		SetNews(strErr);
		return EXIT_FAILURE;
	}

	SetNews(_T("P2PSharer启动..."));

	// 初始化套接字
	InitialSocket();

	// 初始化临界区
	InitializeCriticalSection(&csDown);
	InitializeCriticalSection(&csTransfers);

	// 设置List控件
	m_pListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST_SRCINFO);
	m_pListCtrl->SetTextColor(RGB(0,0,255));
	m_pListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_pListCtrl->InsertColumn(0, _T("资源名称"), LVCFMT_CENTER, 320);
	m_pListCtrl->InsertColumn(1, _T("资源大小(Byte)"), LVCFMT_CENTER, 100);
	m_pListCtrl->InsertColumn(2, _T("资源拥有者"), LVCFMT_CENTER, 80);

	// 设置进度条（Progress control）控件
	m_pProgressCtrl = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS_DOWN);
	m_pProgressCtrl->SetRange(0, 100);
	m_pProgressCtrl->SetPos(0);

	// 设置运行资源按钮不可用

	// 设置下载进度为0
	SetDlgItemText(IDC_EDIT_PROCESS, _T("0"));


	// 启动资源传输请求监听线程
	HANDLE hListenThread = (HANDLE)_beginthreadex(0, 0, ListenThread, this, 0, 0);
	CloseHandle(hListenThread);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CP2PSharerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CP2PSharerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CP2PSharerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CP2PSharerDlg::OnBnClickedBtnBrowse()
{
	TCHAR lpszWorkDir[MAX_PATH] = {0};
	LPITEMIDLIST lpidlRoot;
	LPITEMIDLIST lpidlBrowse;

	::SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &lpidlRoot);	//取得选定的文件夹名

	BROWSEINFO brwsInfo;
	brwsInfo.hwndOwner = NULL;
	brwsInfo.pidlRoot = lpidlRoot;
	brwsInfo.pszDisplayName = lpszWorkDir;
	brwsInfo.lpszTitle = _T("请选择共享目录");
	brwsInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_EDITBOX |BIF_NEWDIALOGSTYLE;
	brwsInfo.lpfn = NULL;
	brwsInfo.lParam = NULL;
	brwsInfo.iImage = NULL;

	lpidlBrowse = ::SHBrowseForFolder(&brwsInfo);

	if (NULL != lpidlBrowse)
	{
		if(::SHGetPathFromIDList(lpidlBrowse,lpszWorkDir))	//取得文件夹名		
		{
			m_strWorkDir.Empty();
			m_strWorkDir = lpszWorkDir;

			if(m_strWorkDir != "")
			{
				if(m_strWorkDir.Right(1) != "\\")
					m_strWorkDir += "\\";				//在末尾时附加"\\"
			}
		}
	}

	SetDlgItemText(IDC_EDIT_SHAREFOLDER, m_strWorkDir);
}

/************************************************************************/
/* 打开工作目录                                                                     */
/************************************************************************/
void CP2PSharerDlg::OnBnClickedBtnOpensharefolder()
{
	CString strProcessCmd = _T("Explorer ") + m_strWorkDir;
	LPTSTR lpszProcessCmd = strProcessCmd.GetBuffer();

	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi;
	CreateProcess(NULL, lpszProcessCmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
}


/************************************************************************/
/* 测试服务器                                                                     */
/************************************************************************/
void CP2PSharerDlg::OnBnClickedBtnSrvtest()
{
	SetDlgItemText(IDC_BTN_SRVTEST, _T("测试中..."));

	if (SrvTest())
	{
		MessageBox(_T("服务器状态良好。"), _T("信息"), MB_ICONINFORMATION);
	}
	else
	{
		MessageBox(_T("无法连接服务器，请检查服务器地址和接口索引是否正确"),
			_T("错误"), MB_ICONERROR);
	}

	SetDlgItemText(IDC_BTN_SRVTEST, _T("测试服务器"));
	GetDlgItem(IDC_BTN_SRVTEST)->EnableWindow(TRUE);
}

/**
* \brief 设置动态信息
*/
void CP2PSharerDlg::SetNews(const CString &strNews)
{
	static int nIndex(0);
	static CString arrStrNews[NEWS_ITEM_NUM_CLI] = {0};

	if (nIndex < NEWS_ITEM_NUM_CLI)
	{
		arrStrNews[nIndex] = strNews;
	}

	CString strShow;

	for (int n(nIndex+1); n < NEWS_ITEM_NUM_CLI; ++n)
	{
		if (!arrStrNews[n].IsEmpty())
		{
			strShow += arrStrNews[n];
			strShow += "\r\n";
		}
	}

	for (int n(0); n < nIndex; ++n)
	{
		if (!arrStrNews[n].IsEmpty())
		{
			strShow += arrStrNews[n];
			strShow += "\r\n";
		}
	}

	strShow += strNews;

	nIndex = (++nIndex) % NEWS_ITEM_NUM_CLI;

	SetDlgItemText(IDC_EDIT_NEWS, strShow);
}

/************************************************************************/
/* 测试服务器状态
/* @return 如果服务器状态正常，返回真；否则，返回假。
/************************************************************************/
bool CP2PSharerDlg::SrvTest()
{
	SOCKADDR_IN6 addrSrv;
	memset(&addrSrv, 0, sizeof(addrSrv));
	int nAddrLen = sizeof(addrSrv);
	char buff[P2PIEP_LEN+1] = {0};

	GetDlgItem(IDC_BTN_SRVTEST)->EnableWindow(FALSE);

	P2PIEP p2piepSrvTest;
	memset(&p2piepSrvTest, 0, sizeof(p2piepSrvTest));
	p2piepSrvTest.Type = P2PIEP_TYPE_SRVTEST;

	int nSendTimes(0);		// 记录发送次数

	bool bSrvOK(FALSE);		// 记录服务器状态是否良好

	SetNews(_T("正在测试服务器状态..."));

	while (RETRY_TIME != nSendTimes++ && !bSrvOK)
	{
		int nErr = sendto(m_sockLocalToSrv, (char*)(&p2piepSrvTest), sizeof(p2piepSrvTest),
			0, m_paiSrv->ai_addr, m_paiSrv->ai_addrlen);

		if (SOCKET_ERROR == nErr)
		{
			continue;
		}

		memset(buff, 0, sizeof(buff));
		int nRecvLen = recvfrom(m_sockLocalToSrv, buff, sizeof(buff), 0, (sockaddr*)&addrSrv, &nAddrLen);

		if (SOCKET_ERROR == nRecvLen)
		{
			continue;
		}

		if (nRecvLen != P2PIEP_LEN)
		{
			SetNews(_T("接收到错误的P2PIEP数据。"));
			continue;
		}

		P2PIEP p2piepRecv;
		memcpy(&p2piepRecv, buff, nRecvLen);

		if (P2PIEP_TYPE_SRVOK == p2piepRecv.Type)
		{
			bSrvOK = TRUE;
			break;
		}
	}

	if (bSrvOK)
	{
		SetNews(_T("服务器状态良好。"));
	}
	else
	{
		SetNews(_T("服务器无法连接。"));
	}

	return bSrvOK;
}


void CP2PSharerDlg::OnBnClickedCancel()
{
	UnDistributeSrc();

	freeaddrinfo(m_paiSrv);
	freeaddrinfo(m_paiLocalToSrv);	

	WSACleanup();
	OnCancel();
}

/**
 * /brief 扫描共享目录中的文件，保存在vcSourceInfo向量中，该函数会清空保存资源信息的向量
 */
void CP2PSharerDlg::ScanSourceInWorkDir()
{
	// 获取工作目录
	GetDlgItemText(IDC_EDIT_SHAREFOLDER, m_strWorkDir);

	if (m_strWorkDir.IsEmpty())
	{
		return;
	}

	CString strSourceName = m_strWorkDir + _T("*.*");

	_wfinddata_t file;
	long lf;

	if ((lf = _wfindfirst(strSourceName.GetBuffer(), &file)) == -1)
	{
		MessageBox(_T("工作目录没有找到。"), _T("错误"), MB_ICONERROR);
		SetNews(_T("工作目录没有找到。"));
		return;
	}
	else
	{
		vcSourceInfo.clear();		// 清空保存资源信息的向量
		do 
		{
			if (file.attrib == _A_ARCH)
			{
				if (MAX_SOURCE_NAME_LEN <= sizeof(file.name))
				{
					SourceInfo sourceInfo;
					memset(&sourceInfo, 0, sizeof(sourceInfo));
					memset(&sourceInfo.szFileName, 0, sizeof(sourceInfo.szFileName));
					memcpy(sourceInfo.szFileName, file.name, sizeof(sourceInfo.szFileName));
					sourceInfo.nSourceSize = file.size;
					vcSourceInfo.push_back(sourceInfo);
				}
				else
				{
					SetNews(_T("资源：%s不能发布，原因：资源名称过长"));
				}				
			}
		} while (0 == _wfindnext(lf, &file));
	}	
}

void CP2PSharerDlg::OnBnClickedBtnDistributesource()
{
	// 获取共享目录下的所有资源
	ScanSourceInWorkDir();

	if (0 == vcSourceInfo.size())
	{
		MessageBox(_T("共享目录中没有可发布的资源"), _T("信息"), MB_ICONINFORMATION);
		return;
	}

	SOCKADDR_IN6 addrSrv;
	memset(&addrSrv, 0, sizeof(addrSrv));
	int nAddrLen = sizeof(addrSrv);
	char buff[P2PIEP_LEN+1] = {0};

	for (size_t stIndex(0); stIndex != vcSourceInfo.size(); ++stIndex)
	{
		P2PIEP p2piep;
		memset(&p2piep, 0, sizeof(p2piep));

		p2piep.Type			= P2PIEP_TYPE_DIST;
		p2piep.SourceLen	= vcSourceInfo.at(stIndex).nSourceSize;
		memcpy(p2piep.SourceName, vcSourceInfo.at(stIndex).szFileName, MAX_SOURCE_NAME_LEN);

		int nSendTimes(0);		// 记录发送次数

		while (RETRY_TIME != nSendTimes++)
		{
			int nErr = sendto(m_sockLocalToSrv, (char*)(&p2piep), sizeof(p2piep),
				0, m_paiSrv->ai_addr, m_paiSrv->ai_addrlen);

			if (SOCKET_ERROR == nErr)
			{
				continue;
			}

			memset(buff, 0, sizeof(buff));
			int nRecvLen = recvfrom(m_sockLocalToSrv, buff, sizeof(buff), 0, (sockaddr*)&addrSrv, &nAddrLen);

			if (SOCKET_ERROR == nRecvLen)
			{
				continue;
			}

			if (nRecvLen != P2PIEP_LEN)
			{
				SetNews(_T("接收到错误的P2PIEP数据。"));
				continue;
			}

			P2PIEP p2piepRecv;
			memcpy(&p2piepRecv, buff, nRecvLen);

			CString strInfo;

			if (!m_bDistRSCWithDebug)
			{
				break;
			}

			if (P2PIEP_TYPE_DIST_OK == p2piepRecv.Type)
			{
				strInfo.Format(_T("发布资源\"%s\"成功。"),vcSourceInfo.at(stIndex).szFileName);
				SetNews(strInfo);
				break;
			}
			else if (P2PIEP_TYPE_DIST_ERR_NOBUF == p2piepRecv.Type)
			{
				strInfo.Format(_T("资源 %s 发布失败，原因：服务器资源数达到最大值。"), 
					vcSourceInfo.at(stIndex).szFileName);
				MessageBox(strInfo, _T("警告"), MB_ICONWARNING);
				SetNews(strInfo);
				break;
			}
			else if (P2PIEP_TYPE_DIST_ERR_DUP == p2piepRecv.Type)
			{
				strInfo.Format(_T("资源 %s 发布失败，原因：资源重复发布。"), 
					vcSourceInfo.at(stIndex).szFileName);
				SetNews(strInfo);
				break;
			}
		}
	}
}

void CP2PSharerDlg::OnBnClickedBtnChnsrvaddr()
{
	CString strDlgItemText;
	GetDlgItemText(IDC_BTN_CHNSRVADDR, strDlgItemText);

	if (_T("修改地址") == strDlgItemText)
	{
		((CEdit*)GetDlgItem(IDC_EDIT_SRVADDR))->SetReadOnly(FALSE);
		SetDlgItemText(IDC_BTN_CHNSRVADDR, _T("确定"));
		closesocket(m_sockLocalToSrv);
	}
	else if ( _T("确定") == strDlgItemText)
	{
		((CEdit*)GetDlgItem(IDC_EDIT_SRVADDR))->SetReadOnly(TRUE);
		SetDlgItemText(IDC_BTN_CHNSRVADDR, _T("修改地址"));
		InitialSocket();		// 服务器地址更改，重新计算
	}
}

void CP2PSharerDlg::InitialSocket()
{
	int nErr(0);
	CString strErr;

	// 获取服务器地址
	TCHAR tszSrvAddr[IPV6_ADDR_MAX_STRLEN] = {0};
	char szSrvAddr[IPV6_ADDR_MAX_STRLEN] = {0};

	GetDlgItemText(IDC_EDIT_SRVADDR, tszSrvAddr, IPV6_ADDR_MAX_STRLEN-1);

	nErr = WideCharToMultiByte(CP_ACP, 0, tszSrvAddr, _countof(tszSrvAddr),
		szSrvAddr, sizeof(szSrvAddr), NULL, NULL);

	if (0 == nErr)
	{
		strErr.Format(_T("字符串转换(Unicode->ANSI)失败。\r\n(错误代码：%d)"),
			GetLastError());
		MessageBox(strErr, _T("错误"), MB_ICONERROR);
		SetNews(strErr);
		return;
	}

	// 获取服务器地址信息
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags		= AI_NUMERICHOST;
	hints.ai_family		= AF_INET6;
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_protocol	= IPPROTO_UDP;


	nErr = getaddrinfo(szSrvAddr, P2PIEP_SRV_PORT_S, &hints, &m_paiSrv);

	if (0 != nErr)
	{
		strErr.Format(_T("获取地址信息(getaddrinfo)失败。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strErr, _T("错误"), MB_ICONERROR);
		SetNews(strErr);
		return;
	}

	// 获取本地地址信息

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags		= AI_PASSIVE;
	hints.ai_family		= AF_INET6;
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_protocol	= IPPROTO_UDP;

	nErr = getaddrinfo(NULL, P2PIEP_CLI_PORT_S, &hints, &m_paiLocalToSrv);

	if (0 != nErr)
	{
		strErr.Format(_T("获取地址信息(getaddrinfo)失败。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strErr, _T("错误"), MB_ICONERROR);
		SetNews(strErr);
		return;
	}

	// 设置用于连接服务器的套接字
	m_sockLocalToSrv = socket(m_paiLocalToSrv->ai_family,
		m_paiLocalToSrv->ai_socktype,
		m_paiLocalToSrv->ai_protocol);

	if (INVALID_SOCKET == m_sockLocalToSrv)
	{
		strErr.Format(_T("创建套接字失败。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strErr, _T("错误"), MB_ICONERROR);
		SetNews(strErr);
		return;
	}

	nErr = bind(m_sockLocalToSrv, m_paiLocalToSrv->ai_addr, m_paiLocalToSrv->ai_addrlen);

	if (0 != nErr)
	{
		strErr.Format(_T("套接字绑定失败。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strErr, _T("错误"), MB_ICONERROR);
		SetNews(strErr);
		return;
	}

	int nTimeOut(TIMEOUT);
	int nOptLen = sizeof(nTimeOut);

	nErr = setsockopt(m_sockLocalToSrv, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, nOptLen);

	if (0 != nErr)
	{
		strErr.Format(_T("设置套接字超时错误。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strErr, _T("错误"), MB_ICONERROR);
		SetNews(strErr);
		return;
	}
}

/**
 * /brief 运行资源
 */
void CP2PSharerDlg::OnBnClickedBtnRunsrc()
{
	if (m_strDownFilePath.IsEmpty())
	{
		MessageBox(_T("没有下载资源"), _T("错误"), MB_ICONERROR);
		SetNews(_T("没有下载资源"));
		return;
	}

	CString strProcessCmd = _T("Explorer ") + m_strDownFilePath;
	STARTUPINFO si = {sizeof(si)};
	PROCESS_INFORMATION pi;
	CreateProcess(NULL, strProcessCmd.GetBuffer(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	(CWnd*)(GetDlgItem(IDC_BTN_RUNSRC))->EnableWindow(FALSE);
}

void CP2PSharerDlg::UnDistributeSrc()
{
	// 获取共享目录下的所有资源
	ScanSourceInWorkDir();

	if (0 == vcSourceInfo.size())
	{
		return;
	}

	SOCKADDR_IN6 addrSrv;
	memset(&addrSrv, 0, sizeof(addrSrv));
	int nAddrLen = sizeof(addrSrv);
	char buff[P2PIEP_LEN+1] = {0};

	for (size_t stIndex(0); stIndex != vcSourceInfo.size(); ++stIndex)
	{
		P2PIEP p2piep;
		memset(&p2piep, 0, sizeof(p2piep));

		p2piep.Type			= P2PIEP_TYPE_UNDIST;
		p2piep.SourceLen	= vcSourceInfo.at(stIndex).nSourceSize;
		memcpy(p2piep.SourceName, vcSourceInfo.at(stIndex).szFileName, MAX_SOURCE_NAME_LEN);

		int nSendTimes(0);		// 记录发送次数
		bool bSuccess(FALSE);	// 记录是否发布成功

		while (RETRY_TIME != nSendTimes++ || bSuccess)
		{
			int nErr = sendto(m_sockLocalToSrv, (char*)(&p2piep), sizeof(p2piep),
				0, m_paiSrv->ai_addr, m_paiSrv->ai_addrlen);

			if (SOCKET_ERROR == nErr)
			{
				continue;
			}

			memset(buff, 0, sizeof(buff));
			int nRecvLen = recvfrom(m_sockLocalToSrv, buff, sizeof(buff), 0, (sockaddr*)&addrSrv, &nAddrLen);

			if (SOCKET_ERROR == nRecvLen)
			{
				continue;
			}

			if (nRecvLen != P2PIEP_LEN)
			{
				SetNews(_T("接收到错误的P2PIEP数据。"));
				continue;
			}

			P2PIEP p2piepRecv;
			memcpy(&p2piepRecv, buff, nRecvLen);

			if (P2PIEP_TYPE_UNDIST_OK == p2piepRecv.Type)
			{
				bSuccess = TRUE;
				break;
			}
			else if (P2PIEP_TYPE_UNDIST_ERR_NOTOWN == p2piepRecv.Type)
			{
				break;
			}
		}

		if (bSuccess)
		{
			CString strInfo;
			strInfo.Format(_T("资源撤销\"%s\"成功。"),vcSourceInfo.at(stIndex).szFileName);
			SetNews(strInfo);
		}
		else
		{
			CString strInfo;
			strInfo.Format(_T("资源撤销\"%s\"失败。"),vcSourceInfo.at(stIndex).szFileName);
			SetNews(strInfo);
		}
	}
}


/**
 * \brief 刷新资源列表
 */
void CP2PSharerDlg::OnBnClickedBtnRefreshsrc()
{	
	SOCKADDR_IN6 addrSrv;
	memset(&addrSrv, 0, sizeof(addrSrv));
	int nAddrLen = sizeof(addrSrv);
	char buff[P2PIEP_LEN+1] = {0};

	P2PIEP p2piepSend;
	memset(&p2piepSend, 0, sizeof(p2piepSend));

	P2PIEP p2piepRecv;
	memset(&p2piepRecv, 0, sizeof(p2piepRecv));

	p2piepSend.Type = P2PIEP_TYPE_SRCINFO_REQ;

	int nRecvLen(0);

	int nSendTimes(0);		// 记录发送次数

	while (RETRY_TIME != nSendTimes++)
	{
		int nErr = sendto(m_sockLocalToSrv, (char*)(&p2piepSend), sizeof(p2piepSend),
			0, m_paiSrv->ai_addr, m_paiSrv->ai_addrlen);

		if (SOCKET_ERROR == nErr)
		{
			continue;
		}

		memset(buff, 0, sizeof(buff));
		nRecvLen = recvfrom(m_sockLocalToSrv, buff, sizeof(buff), 0, (sockaddr*)&addrSrv, &nAddrLen);

		if (SOCKET_ERROR == nRecvLen)
		{
			continue;
		}

		if (nRecvLen != P2PIEP_LEN)
		{
			SetNews(_T("接收到错误的P2PIEP数据。"));
			continue;
		}
		break;
	}

	if (nSendTimes >= RETRY_TIME)
	{
		MessageBox(_T("获取资源列表失败，原因：目标服务器无响应。"), _T("错误"), MB_ICONERROR);
		SetNews(_T("获取资源列表失败，原因：目标服务器无响应。"));
		return;
	}

	// 清理以前的资源
	m_nSrcNUM = 0;
	memset(arrDistSrcInfo, 0, sizeof(arrDistSrcInfo));
	m_pListCtrl->DeleteAllItems();


	memset(&p2piepRecv, 0, sizeof(p2piepRecv));
	memcpy(&p2piepRecv, buff, sizeof(p2piepRecv));

	while(P2PIEP_TYPE_SRCINFO_END != p2piepRecv.Type)
	{
		arrDistSrcInfo[m_nSrcNUM].bIsUse					= TRUE;
		_tcscpy_s(arrDistSrcInfo[m_nSrcNUM].sourceInfo.szFileName,
			p2piepRecv.SourceName);
		arrDistSrcInfo[m_nSrcNUM].sourceInfo.nSourceSize	= p2piepRecv.SourceLen;
		arrDistSrcInfo[m_nSrcNUM].nOwnerNum					= p2piepRecv.SrcOwnerNum;
		for(int n(0); n < p2piepRecv.SrcOwnerNum; ++n)
		{
			arrDistSrcInfo[m_nSrcNUM].srcOwnerInfo.bUse[n]	= TRUE;
			memcpy(&arrDistSrcInfo[m_nSrcNUM].srcOwnerInfo.srcOwner[n],
				&p2piepRecv.srcOwner[n],
				sizeof(SOCKADDR_IN6));
		}

		++m_nSrcNUM;

		nSendTimes = 0;

		while (RETRY_TIME != nSendTimes++)
		{
			memset(&p2piepSend, 0, sizeof(p2piepSend));
			p2piepSend.Type = P2PIEP_TYPE_SRCINFO_OK;

			int nErr = sendto(m_sockLocalToSrv, (char*)(&p2piepSend), sizeof(p2piepSend),
				0, m_paiSrv->ai_addr, m_paiSrv->ai_addrlen);

			if (SOCKET_ERROR == nErr)
			{
				continue;
			}
			break;
		}

		nSendTimes = 0;
		
		while (RETRY_TIME != nSendTimes++)
		{
			memset(buff, 0, sizeof(buff));
			nRecvLen = recvfrom(m_sockLocalToSrv, buff, sizeof(buff), 0, (sockaddr*)&addrSrv, &nAddrLen);

			if (SOCKET_ERROR == nRecvLen)
			{
				continue;
			}

			if (nRecvLen != P2PIEP_LEN)
			{
				SetNews(_T("接收到错误的P2PIEP数据。"));
				continue;
			}
			break;
		}

		if (RETRY_TIME == nSendTimes)
		{
			break;
		}

		memset(&p2piepRecv, 0, sizeof(p2piepRecv));
		memcpy(&p2piepRecv, buff, sizeof(p2piepRecv));
	}

	// 更新List
	for (int n(0); n < m_nSrcNUM; ++n)
	{
		TCHAR wBuff[100] = {0};
		m_pListCtrl->InsertItem(n, arrDistSrcInfo[n].sourceInfo.szFileName);
		_itot_s(arrDistSrcInfo[n].sourceInfo.nSourceSize, wBuff, 10);
		m_pListCtrl->SetItemText(n, 1, wBuff);
		_itot_s(arrDistSrcInfo[n].nOwnerNum, wBuff, 10);
		m_pListCtrl->SetItemText(n, 2, wBuff);
	}
}

void CP2PSharerDlg::OnBnClickedBtnDownloadsrc()
{
	if (0 == m_nSrcNUM)
	{
		MessageBox(_T("没有可以下载的资源，点击旁边的\"刷新资源\"按钮试试看 : )。 "),
			_T("信息"), MB_ICONINFORMATION);
		return;
	}
	// 获得用户的选择资源的索引
	POSITION p = m_pListCtrl->GetFirstSelectedItemPosition();
	int nIndexInList = (int)p;

	if (0 == nIndexInList)
	{
		MessageBox(_T("还没有选择要下载的资源哦 : )。"), _T("信息"), MB_ICONINFORMATION);
		return;
	}

	if ( nIndexInList > m_nSrcNUM)
	{
		MessageBox(_T("崩溃啦，选择的资源不能下载！ "), _T("错误"), MB_ICONERROR);
		return;
	}

	m_nDownloadIndex = nIndexInList - 1;
	ScanSourceInWorkDir();

	for(int n(0); n != vcSourceInfo.size(); ++n)
	{
		if (vcSourceInfo[n].nSourceSize == arrDistSrcInfo[m_nDownloadIndex].sourceInfo.nSourceSize &&
			_tcscmp(vcSourceInfo[n].szFileName, arrDistSrcInfo[m_nDownloadIndex].sourceInfo.szFileName) == 0)
		{
			MessageBox(_T("资源在共享目录中，这个资源就是你的哦 : )"), _T("信息"), MB_ICONINFORMATION);
			return;
		}
	}

	if (0 == arrDistSrcInfo[m_nDownloadIndex].nOwnerNum)
	{
		MessageBox(_T("全部的资源拥有者都不在线，不能下载，点击旁边的\"刷新资源\"按钮试试看 : )。 "), _T("信息"), MB_ICONINFORMATION);
		return;
	}

	HANDLE hDownThread = (HANDLE)_beginthreadex(NULL, 0, DownPrepareThread, this, 0, NULL);
	CloseHandle(hDownThread);
}

unsigned int __stdcall CP2PSharerDlg::DownPrepareThread(LPVOID lparam)
{
	CString strInfo;
	int nErr(0);

	CP2PSharerDlg *pThis = (CP2PSharerDlg*)lparam;
	int nIndex(pThis->m_nDownloadIndex);

	((CWnd*)pThis->GetDlgItem(IDC_BTN_DOWNLOADSRC))->EnableWindow(FALSE);
	// ZoneID解析
	CString strZoneID;

	pThis->GetDlgItemText(IDC_EDIT_SRVADDR, strZoneID);
	strZoneID = strZoneID.Right(strZoneID.GetLength() - strZoneID.Find('%'));
	char szZoneID[IPV6_ADDR_MAX_STRLEN] = {0};

	nErr = WideCharToMultiByte(CP_ACP, 0, strZoneID.GetBuffer(), strZoneID.GetLength(),
		szZoneID, sizeof(szZoneID), NULL, NULL);

	if (0 == nErr)
	{
		strInfo.Format(_T("字符串转换（Unicode->ANSI）失败。\r\n（错误代码：%d）"), GetLastError());
		pThis->MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		pThis->SetNews(strInfo);
		return 1;
	}


	// 创建文件
	pThis->m_strDownFilePath = pThis->m_strWorkDir + CString(pThis->arrDistSrcInfo[nIndex].sourceInfo.szFileName);

	DeleteFile(pThis->m_strDownFilePath.GetBuffer());

	HANDLE hFile = CreateFile(pThis->m_strDownFilePath.GetBuffer(), GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		strInfo.Format(_T("创建文件失败。\r\n（错误代码：%d）"), GetLastError());
		pThis->MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		pThis->SetNews(strInfo);
		return 1;
	}

	CFile file(hFile);
	file.SetLength(pThis->arrDistSrcInfo[nIndex].sourceInfo.nSourceSize);


	int nAvailableSrcOwnerNum(0); // 记录可用资源个数
	for ( int n(0); n < MAX_SRCOWNER_NUM; ++n)
	{
		if (TRUE == pThis->arrDistSrcInfo[nIndex].srcOwnerInfo.bUse[n])
		{
			++nAvailableSrcOwnerNum;
		}
	}

	assert(0 != nAvailableSrcOwnerNum);

	SOCKADDR_IN6 *pOwnerAddr = new SOCKADDR_IN6[nAvailableSrcOwnerNum];

	assert(NULL != pOwnerAddr);

	int nOwnerAddrIndex(0);

	// 对可用的资源拥有者进行地址解析，将ZoneID修改为本地的ZoneID
	for (int n(0); n != nAvailableSrcOwnerNum; ++n)
	{
		memset(&pOwnerAddr[n], 0, sizeof(pOwnerAddr[n]));

		while (nOwnerAddrIndex < MAX_SRCOWNER_NUM)
		{
			if (pThis->arrDistSrcInfo[nIndex].srcOwnerInfo.bUse[nOwnerAddrIndex])
			{		
				char Hostname[NI_MAXHOST] = {0};

				nErr = getnameinfo(
					(sockaddr*)(&(pThis->arrDistSrcInfo[nIndex].srcOwnerInfo.srcOwner[nOwnerAddrIndex])),
					sizeof(pThis->arrDistSrcInfo[nIndex].srcOwnerInfo.srcOwner[nOwnerAddrIndex]),
					Hostname,
					sizeof(Hostname),
					NULL,
					0,
					NI_NUMERICHOST);

				size_t stFind(0);

				for (stFind; stFind < strlen(Hostname); ++stFind)
				{
					if ('%' == Hostname[stFind])
					{
						break;
					}
				}

				if (stFind == strlen(Hostname))
				{
					pThis->MessageBox(_T("无法找到区域ID，下载失败"), _T("警告"), MB_ICONWARNING);
					pThis->SetNews(_T("无法找到区域ID，下载失败"));
					return 1;
				}

				strcpy_s(&Hostname[stFind], sizeof(Hostname)-stFind, szZoneID);

				addrinfo hints;
				memset(&hints, 0, sizeof(hints));
				hints.ai_flags		= AI_NUMERICHOST;
				hints.ai_family		= AF_INET6;
				hints.ai_socktype	= SOCK_DGRAM;
				hints.ai_protocol	= IPPROTO_UDP;

				addrinfo *paiSrcOwner = NULL;

				nErr = getaddrinfo(Hostname, P2PDEP_LISTEN_PORT_S, &hints, &paiSrcOwner);

				assert(0 == nErr);

				memcpy(&pOwnerAddr[n], paiSrcOwner->ai_addr, sizeof(pOwnerAddr[n]));

				++nOwnerAddrIndex;

				break;
			}

			++nOwnerAddrIndex;
		}
	}

	// 对可用的资源拥有者填充OwnerInfo结构
	OwnerInfo *pOwnerInfo = new OwnerInfo[nAvailableSrcOwnerNum];

	assert(NULL != pOwnerInfo);

	int nGetLenFromEachOwner(pThis->arrDistSrcInfo[nIndex].sourceInfo.nSourceSize / nAvailableSrcOwnerNum + 1); // 从每个资源拥有者中获取资源的长度

	for (int n(0); n != nAvailableSrcOwnerNum; ++n)
	{
		memset(&pOwnerInfo[n], 0, sizeof(pOwnerInfo[n]));

		pOwnerInfo[n].nDownState	= DOWN_STATE_READY;
		pOwnerInfo[n].pFile			= &file;

		memcpy(&pOwnerInfo[n].tcsSrcName, 
			pThis->arrDistSrcInfo[nIndex].sourceInfo.szFileName,
			sizeof(pThis->arrDistSrcInfo[nIndex].sourceInfo.szFileName));

		pOwnerInfo[n].pAddrSrcOwner	= &pOwnerAddr[n];
		pOwnerInfo[n].nFileStart	= n * nGetLenFromEachOwner;

		if ((n+1) * nGetLenFromEachOwner >= pThis->arrDistSrcInfo[nIndex].sourceInfo.nSourceSize)
		{			
			pOwnerInfo[n].nFileEnd	= pThis->arrDistSrcInfo[nIndex].sourceInfo.nSourceSize;
		}
		else
		{
			pOwnerInfo[n].nFileEnd	= (n+1) * nGetLenFromEachOwner;
		}
	}


	// 创建下载线程
	for(int n(0); n != nAvailableSrcOwnerNum; ++n)
	{
		HANDLE hDownThread = (HANDLE)_beginthreadex(NULL, 0, DownLoadThread, &pOwnerInfo[n], 0, NULL);
		CloseHandle(hDownThread);
	}

	bool bComplete(FALSE);
	int nConnectedNum(nAvailableSrcOwnerNum);

	time_t timeBegin(clock());

	while (!bComplete)
	{
		Sleep(100);
		CString strDownloadInfo(_T("正在下载 \""));
		strDownloadInfo += pThis->arrDistSrcInfo[nIndex].sourceInfo.szFileName;
		strDownloadInfo += _T("\"...\r\n");

		if (0 == nConnectedNum)
		{
			pThis->MessageBox(_T("所有的资源拥有者都不能连接"), _T("错误"), MB_ICONERROR);
			pThis->SetNews(_T("所有的资源拥有者都不能连接"));
			DeleteFile(pThis->m_strDownFilePath.GetBuffer());
			break;
		}
		int nSum (0);

		for (int n(0); n < nAvailableSrcOwnerNum; ++n)
		{
			CString strTmp;
			if (DOWN_STATE_CANNOT != pOwnerInfo[n].nDownState &&
				pOwnerInfo[n].nDownState >= 0)
			{
				strTmp.Format(_T("线程 %d 已下载 %d 字节\r\n"), n+1, pOwnerInfo[n].nDownState);
				strDownloadInfo += strTmp;

				nSum += pOwnerInfo[n].nDownState;
			}
			else if (DOWN_STATE_CANNOT == pOwnerInfo[n].nDownState)
			{
				--nConnectedNum;
				if (0 == nConnectedNum)
				{
					return 1;
				}
			}
		}

		int nProcess(nSum / (pThis->arrDistSrcInfo[nIndex].sourceInfo.nSourceSize/100));
		pThis->m_pProgressCtrl->SetPos(nProcess);
		pThis->SetDlgItemInt(IDC_EDIT_PROCESS, nProcess);
		pThis->SetDlgItemText(IDC_EDIT_NEWS, strDownloadInfo);

		if (nSum == pThis->arrDistSrcInfo[nIndex].sourceInfo.nSourceSize)
		{
			bComplete = TRUE;
		}
	}

	time_t timeCost(clock() - timeBegin);

	strInfo.Format(_T("资源 \"%s\" 下载完成，用时：%d(ms)。"),
		pThis->arrDistSrcInfo[nIndex].sourceInfo.szFileName,
		timeCost);
	pThis->SetNews(strInfo);

	pThis->m_bDistRSCWithDebug = FALSE;
	pThis->OnBnClickedBtnDistributesource();
	pThis->m_bDistRSCWithDebug = TRUE;
	(CWnd*)(pThis->GetDlgItem(IDC_BTN_RUNSRC))->EnableWindow(TRUE);
	delete [] pOwnerAddr;
	delete [] pOwnerInfo;
	file.Close();
	((CWnd*)pThis->GetDlgItem(IDC_BTN_DOWNLOADSRC))->EnableWindow(TRUE);

	return 0;
}

/**
 *\ brief 资源下载线程
 */
unsigned int __stdcall CP2PSharerDlg::DownLoadThread(LPVOID lparam)
{
	// 解析参数
	OwnerInfo *pOwnerInfo = (OwnerInfo*)lparam;

	CString strInfo;
	int nErr(0);

	P2PDEP p2pdepSend;
	memset(&p2pdepSend, 0, sizeof(p2pdepSend));
	p2pdepSend.type = P2PDEP_TYPE_SRC_REQ;

	SOCKET sockToOwner = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	assert(INVALID_SOCKET != sockToOwner);

	char buff[P2PDEP_LEN+1] = {0};
	memset(buff, 0, sizeof(buff));
	SOCKADDR_IN6 addrOwner;
	int nAddrLen = sizeof(addrOwner);
	int nRecvLen(0);
	
	// 发送REQ请求，并接收应答
	nErr = sendto(sockToOwner, (char*)(&p2pdepSend), sizeof(p2pdepSend),
		0, (sockaddr*)pOwnerInfo->pAddrSrcOwner, sizeof(*(pOwnerInfo->pAddrSrcOwner)));

	assert(SOCKET_ERROR != nErr);

	nRecvLen = recvfrom(sockToOwner, buff, sizeof(buff), 0,
		(sockaddr*)&addrOwner, &nAddrLen);

	assert(SOCKET_ERROR != nRecvLen);
	assert(P2PDEP_LEN == nRecvLen);

	P2PDEP p2pdepRecv;
	memset(&p2pdepRecv, 0, sizeof(p2pdepRecv));
	memcpy(&p2pdepRecv, buff, sizeof(p2pdepRecv));

	// 资源拥有者准备就绪，开始下载
	if (P2PDEP_TYPE_READY == p2pdepRecv.type)
	{
		// 发送请求资源的名字
		memset(&p2pdepSend, 0, sizeof(p2pdepSend));
		p2pdepSend.type		= P2PDEP_TYPE_GET_NAME;
		memcpy(&p2pdepSend.Data, pOwnerInfo->tcsSrcName, sizeof(pOwnerInfo->tcsSrcName));
		nErr = sendto(sockToOwner, (char*)(&p2pdepSend), sizeof(p2pdepSend), 0,
			(sockaddr*)(&addrOwner), sizeof(addrOwner));
		if (SOCKET_ERROR == nErr)
		{
			pOwnerInfo->nDownState = CP2PSharerDlg::DOWN_STATE_CANNOT;
			return 1;
		}
		memset(buff, 0, sizeof(buff));
		nRecvLen = recvfrom(sockToOwner, buff, sizeof(buff), 0,	NULL, NULL);

		if (SOCKET_ERROR == nRecvLen)
		{
			pOwnerInfo->nDownState = CP2PSharerDlg::DOWN_STATE_CANNOT;
			return 1;
		}

		if (P2PDEP_LEN != nRecvLen)
		{
			pOwnerInfo->nDownState = CP2PSharerDlg::DOWN_STATE_CANNOT;
			return 1;
		}

		memset(&p2pdepRecv, 0, sizeof(p2pdepRecv));
		memcpy(&p2pdepRecv, buff, sizeof(p2pdepRecv));

		if (P2PDEP_TYPE_NAME_OK != p2pdepRecv.type)
		{
			pOwnerInfo->nDownState = CP2PSharerDlg::DOWN_STATE_CANNOT;
			return 1;
		}

		int nReqLen = pOwnerInfo->nFileEnd - pOwnerInfo->nFileStart; // 需要下载的资源总长度
		int nReqTimes = (nReqLen + P2PDEP_PAYLOAD_LENGTH -1) / P2PDEP_PAYLOAD_LENGTH;
		int nTotalRecvLen (0);

		for (int n(0); n != nReqTimes; ++n)
		{
			memset(&p2pdepSend, 0, sizeof(p2pdepSend));
			p2pdepSend.type		= P2PDEP_TYPE_GET_DATA;
			p2pdepSend.OffSet	= n * P2PDEP_PAYLOAD_LENGTH + pOwnerInfo->nFileStart;

			if (((n+1)*P2PDEP_PAYLOAD_LENGTH + pOwnerInfo->nFileStart) < pOwnerInfo->nFileEnd)
			{
				p2pdepSend.Length = P2PDEP_PAYLOAD_LENGTH;
			}
			else
			{
				p2pdepSend.Length = pOwnerInfo->nFileEnd - pOwnerInfo->nFileStart - (n*P2PDEP_PAYLOAD_LENGTH);
			}

			nErr = sendto(sockToOwner, (char*)(&p2pdepSend), sizeof(p2pdepSend), 0,
				(sockaddr*)(&addrOwner), sizeof(addrOwner));
			if (SOCKET_ERROR == nErr)
			{
				pOwnerInfo->nDownState = CP2PSharerDlg::DOWN_STATE_CANNOT;
				break;
			}

			memset(buff, 0, sizeof(buff));
			nRecvLen = recvfrom(sockToOwner, buff, sizeof(buff), 0, NULL, NULL);

			if (P2PDEP_LEN != nRecvLen)
			{
				pOwnerInfo->nDownState = CP2PSharerDlg::DOWN_STATE_CANNOT;
				break;
			}

			memset(&p2pdepRecv, 0, sizeof(p2pdepRecv));
			memcpy(&p2pdepRecv, buff, sizeof(p2pdepRecv));

			if (P2PDEP_TYPE_DATA_OK == p2pdepRecv.type)
			{
				EnterCriticalSection(&csDown);				
				pOwnerInfo->pFile->Seek(p2pdepRecv.OffSet, CFile::begin);
				pOwnerInfo->pFile->Write(p2pdepRecv.Data, p2pdepRecv.Length);
				LeaveCriticalSection(&csDown);


				// 处理downstate
				nTotalRecvLen += p2pdepRecv.Length;
				pOwnerInfo->nDownState = nTotalRecvLen;
			}
			else
			{
				pOwnerInfo->nDownState = CP2PSharerDlg::DOWN_STATE_CANNOT;
				break;
			}
		}

		memset(&p2pdepSend, 0, sizeof(p2pdepSend));
		p2pdepSend.type = P2PDEP_TYPE_COMPLETE;

		sendto(sockToOwner, (char*)&p2pdepSend, sizeof(p2pdepSend), 0,
			(sockaddr*)(&addrOwner), sizeof(addrOwner));
	}

	closesocket(sockToOwner);
	return 0;
}

/**
 * \brief 资源获取监听线程
 *
 * \param LPVOID[in] 对象的指针
 */
unsigned int __stdcall CP2PSharerDlg::ListenThread(LPVOID lParam)
{
	CP2PSharerDlg *pThis = static_cast<CP2PSharerDlg*>(lParam);

	CString strInfo;
	int nErr;

	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags		= AI_PASSIVE;
	hints.ai_family		= AF_INET6;
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_protocol	= IPPROTO_UDP;

	addrinfo* paiLocalListen;

	nErr = getaddrinfo(NULL, P2PDEP_LISTEN_PORT_S, &hints, &paiLocalListen);

	if (0 != nErr)
	{
		strInfo.Format(_T("获取地址信息(getaddrinfo)失败。\r\n(错误代码：%d)"),
			WSAGetLastError());
		pThis->MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		pThis->SetNews(strInfo);
		return 1;
	}

	// 设置用于监听的套接字
	SOCKET sockLocalListen = socket(paiLocalListen->ai_family, 
		paiLocalListen->ai_socktype, paiLocalListen->ai_protocol);

	if (INVALID_SOCKET == sockLocalListen)
	{
		strInfo.Format(_T("创建套接字失败。\r\n(错误代码：%d)"),
			WSAGetLastError());
		pThis->MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		pThis->SetNews(strInfo);
		return 1;
	}

	nErr = bind(sockLocalListen, paiLocalListen->ai_addr, paiLocalListen->ai_addrlen);

	if (0 != nErr)
	{
		strInfo.Format(_T("套接字绑定失败。\r\n(错误代码：%d)"),
			WSAGetLastError());
		pThis->MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		pThis->SetNews(strInfo);
		return 1;
	}


	SOCKADDR_IN6 addrCliReq;
	int nCliAddrLen = sizeof(addrCliReq);
	char buff[P2PDEP_LEN+1] = {0};

	while (TRUE)
	{
		memset(&addrCliReq, 0, sizeof(addrCliReq));
		memset(buff, 0, sizeof(buff));

		int nRecvLen = recvfrom(sockLocalListen, buff, sizeof(buff), 0, (sockaddr*)&addrCliReq, &nCliAddrLen);

		if (SOCKET_ERROR == nRecvLen)
		{
			strInfo.Format(_T("接收数据失败。\r\n(错误代码：%d)"),
				WSAGetLastError());
			pThis->SetNews(strInfo);
			continue;
		}

		if (nRecvLen != P2PDEP_LEN)
		{
			strInfo.Format(_T("接收到错误的P2PDEP数据。"));
			pThis->SetNews(strInfo);
			continue;
		}

		P2PDEP p2pdepRecv;
		memset(&p2pdepRecv, 0, sizeof(p2pdepRecv));
		memcpy(&p2pdepRecv, buff, nRecvLen);

		if (P2PDEP_TYPE_SRC_REQ != p2pdepRecv.type)
		{
			continue;
		}

		CliInfo *pCliInof = new CliInfo;
		memcpy(&pCliInof->addrCliRequester, &addrCliReq, sizeof(pCliInof->addrCliRequester));
		pCliInof->pThis = pThis;

		// 创建文件传输线程
		HANDLE hTransfersThread = (HANDLE)_beginthreadex(0, 0, TransfersThread, pCliInof, 0, 0);
		CloseHandle(hTransfersThread);
	}

	closesocket(sockLocalListen);
	freeaddrinfo(paiLocalListen);

	return 0;
}

unsigned int __stdcall CP2PSharerDlg::TransfersThread(LPVOID lparam)
{
	CliInfo *pCliInfo = (CliInfo*)lparam;

	SOCKADDR_IN6 *pAddrRequester = &pCliInfo->addrCliRequester;
	CP2PSharerDlg *pThis = pCliInfo->pThis;

	CString strInfo;
	int nErr(0);

	// 设置用于连接请求资源客户端的套接字
	SOCKET sockToRequester = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	assert (INVALID_SOCKET != sockToRequester);

	P2PDEP p2pdepSend;
	memset(&p2pdepSend, 0, sizeof(p2pdepSend));
	p2pdepSend.type = P2PDEP_TYPE_READY;
	nErr = sendto(sockToRequester, (char*)(&p2pdepSend), sizeof(p2pdepSend), 0,
		(sockaddr*)pAddrRequester, sizeof(*pAddrRequester));

	assert (SOCKET_ERROR != nErr);

	char buff[P2PDEP_LEN+1] = {0};
	memset(buff, 0, sizeof(buff));

	int nRecvLen = recvfrom(sockToRequester, buff, sizeof(buff), 0, NULL, NULL);

	assert(SOCKET_ERROR != nRecvLen);
	assert(P2PDEP_LEN == nRecvLen);

	P2PDEP p2pdepRecv;
	memset(&p2pdepRecv, 0, sizeof(p2pdepRecv));
	
	memcpy(&p2pdepRecv, buff, sizeof(p2pdepRecv));

	assert (P2PDEP_TYPE_GET_NAME == p2pdepRecv.type);

	TCHAR tcsFileName[MAX_SOURCE_NAME_LEN] = {0};
	memset(tcsFileName, 0, sizeof(tcsFileName));

	memcpy(tcsFileName, p2pdepRecv.Data, sizeof(tcsFileName));

	CString strFilePath = pThis->m_strWorkDir + tcsFileName;
	CFile file(strFilePath.GetBuffer(), CFile::modeRead);

	memset(&p2pdepSend, 0, sizeof(p2pdepSend));

	p2pdepSend.type = P2PDEP_TYPE_NAME_OK;

	nErr = sendto(sockToRequester, (char*)&p2pdepSend, sizeof(p2pdepSend), 0,
		(sockaddr*)pAddrRequester, sizeof(*pAddrRequester));

	assert (SOCKET_ERROR != nErr);

	while (true)
	{
		memset(buff, 0, sizeof(buff));
		nRecvLen = recvfrom(sockToRequester, buff, sizeof(buff), 0, NULL, NULL);
		assert(SOCKET_ERROR != nRecvLen);
		assert(P2PDEP_LEN == nRecvLen);

		memset(&p2pdepRecv, 0, sizeof(p2pdepRecv));
		memcpy(&p2pdepRecv, buff, sizeof(p2pdepRecv));

		if (P2PDEP_TYPE_GET_DATA != p2pdepRecv.type ||
			P2PDEP_TYPE_COMPLETE == p2pdepRecv.type)
		{
			break;
		}

		BYTE BuffSend[P2PDEP_PAYLOAD_LENGTH+2] = {0};
		memset(BuffSend, 0, sizeof(BuffSend));

		EnterCriticalSection(&csTransfers);
		file.Seek(p2pdepRecv.OffSet, CFile::begin);
		file.Read(BuffSend, p2pdepRecv.Length);
		LeaveCriticalSection(&csTransfers);

		memset(&p2pdepSend, 0, sizeof(p2pdepSend));
		p2pdepSend.type = P2PDEP_TYPE_DATA_OK;
		p2pdepSend.OffSet = p2pdepRecv.OffSet;
		p2pdepSend.Length = p2pdepRecv.Length;
		memcpy(p2pdepSend.Data, BuffSend, sizeof(p2pdepSend.Data));
		
//		Sleep(1);

		nErr = sendto(sockToRequester, (char*)(&p2pdepSend), sizeof(p2pdepSend), 0,
			(sockaddr*)(pAddrRequester), sizeof(*pAddrRequester));
	}

	closesocket(sockToRequester);
	delete pCliInfo;
	return 0;
}
