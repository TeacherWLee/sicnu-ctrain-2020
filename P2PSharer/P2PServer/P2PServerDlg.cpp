
// P2PServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "P2PServer.h"
#include "P2PServerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
















// CP2PServerDlg 对话框



CP2PServerDlg::CP2PServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CP2PServerDlg::IDD, pParent), m_pListCtrl(NULL),
	m_nSrcNum(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CP2PServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CP2PServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CP2PServerDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CP2PServerDlg 消息处理程序

BOOL CP2PServerDlg::OnInitDialog()
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

	// 设置winsock环境
	WSADATA wsaData;
	int nErr(0);
	CString strErr;

	nErr = WSAStartup(MAKEWORD(2,2), &wsaData);


	if (0 != nErr)
	{
		strErr.Format(_T("初始化WSA环境失败。\r\n工作线程即将关闭，服务器将停止工作。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strErr, _T("错误"), MB_ICONERROR);
		SetNews(strErr);
	}

	// 初始化临界区变量
	InitializeCriticalSection(&m_csSrcInfo);

	// 清空资源信息数组
	memset(arrDistSrcInfo, 0, sizeof(arrDistSrcInfo));

	// 设置ListCtrl控件
	m_pListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST_SRCINFO);
	m_pListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_pListCtrl->SetTextColor(RGB(0,0,255));
	m_pListCtrl->InsertColumn(0, _T("资源名称"), LVCFMT_CENTER, 320);
	m_pListCtrl->InsertColumn(1, _T("资源大小(Byte)"), LVCFMT_CENTER, 100);
	m_pListCtrl->InsertColumn(2, _T("资源拥有者"), LVCFMT_CENTER, 80);

	// 创建工作线程 */
	HANDLE hWorkThread = (HANDLE)_beginthreadex(0, 0, WorkThread, this, 0, 0);
	CloseHandle(hWorkThread);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CP2PServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CP2PServerDlg::OnPaint()
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
HCURSOR CP2PServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/**
 * \brief 设置服务器动态信息
 */
void CP2PServerDlg::SetNews(const CString &strNews)
{
	static int nIndex(0);
	static CString arrStrNews[NEWS_ITEM_NUM_SRV] = {0};

	if (nIndex < NEWS_ITEM_NUM_SRV)
	{
		arrStrNews[nIndex] = strNews;
	}

	CString strShow;

	for (int n(nIndex+1); n < NEWS_ITEM_NUM_SRV; ++n)
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

	nIndex = (++nIndex) % NEWS_ITEM_NUM_SRV;

	SetDlgItemText(IDC_EDIT_NEWS, strShow);
}

/**
* \brief 接收客户端发送信息的线程启动函数
*/
unsigned int __stdcall CP2PServerDlg::WorkThread(LPVOID lParam)
{
	CP2PServerDlg *pThis = static_cast<CP2PServerDlg*>(lParam);

	int nErr(0);
	CString strInfo;

	pThis->SetNews(CString(_T("服务器工作线程启动。")));

	// 获取本地地址信息
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags		= AI_PASSIVE;
	hints.ai_family		= AF_INET6;
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_protocol	= IPPROTO_UDP;

	addrinfo *paiLocal = NULL;

	nErr = getaddrinfo(NULL, P2PIEP_SRV_PORT_S, &hints, &paiLocal);

	if (0 != nErr)
	{
		strInfo.Format(_T("获取地址信息(getaddrinfo)失败。\r\n工作线程即将关闭，服务器将停止工作。\r\n(错误代码：%d)"),
			WSAGetLastError());
		pThis->MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		pThis->SetNews(strInfo);
		return 1;
	}

	// 创建套接字并绑定
	SOCKET sockLocal = socket(paiLocal->ai_family, paiLocal->ai_socktype, paiLocal->ai_protocol);

	if (INVALID_SOCKET == sockLocal)
	{
		strInfo.Format(_T("创建套接字失败。\r\n工作线程即将关闭，服务器将停止工作。\r\n(错误代码：%d)"),
			WSAGetLastError());
		pThis->MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		pThis->SetNews(strInfo);
		return 1;
	}

	nErr = bind(sockLocal, paiLocal->ai_addr, paiLocal->ai_addrlen);

	if (0 != nErr)
	{
		strInfo.Format(_T("套接字绑定失败。\r\n工作线程即将关闭，服务器将停止工作。\r\n(错误代码：%d)"),
			WSAGetLastError());
		pThis->MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		pThis->SetNews(strInfo);
		return 1;
	}

	SOCKADDR_IN6 addrCli;
	int nCliAddrLen = sizeof(addrCli);
	char buff[P2PIEP_LEN+1] = {0};

	while (true)
	{
		memset(buff, 0, sizeof(buff));
		memset(&addrCli, 0, sizeof(addrCli));

		int nRecvLen = recvfrom(sockLocal, buff, sizeof(buff), 0, (sockaddr*)&addrCli, &nCliAddrLen);

		if (SOCKET_ERROR == nRecvLen)
		{
			strInfo.Format(_T("接收数据失败。\r\n(错误代码：%d)"),
				WSAGetLastError());
			pThis->SetNews(strInfo);
			continue;
		}

		if (nRecvLen != P2PIEP_LEN)
		{
			strInfo.Format(_T("接收到错误的P2PIEP数据。"));
			pThis->SetNews(strInfo);
			continue;
		}

		P2PIEP p2piepRecv;
		memcpy(&p2piepRecv, buff, nRecvLen);

		switch (p2piepRecv.Type)
		{
		case P2PIEP_TYPE_SRVTEST:	// 服务器测试
			P2PIEP p2piepSrvOK;
			memset(&p2piepSrvOK, 0, sizeof(p2piepSrvOK));
			p2piepSrvOK.Type = P2PIEP_TYPE_SRVOK;
			nErr = sendto(sockLocal, (char*)&p2piepSrvOK, sizeof(p2piepSrvOK),
				0, (sockaddr*)&addrCli, nCliAddrLen);
			if (SOCKET_ERROR == nErr)
			{
				strInfo.Format(_T("发送测试回应失败。\r\n(错误代码：%d)"),
					WSAGetLastError());
				pThis->SetNews(strInfo);
			}
			break;

		case P2PIEP_TYPE_DIST:	//	资源发布
			P2PIEP p2piepDistributeOK;
			memset(&p2piepDistributeOK, 0, sizeof(p2piepDistributeOK));

			p2piepDistributeOK.Type = pThis->DistributeSource(p2piepRecv, addrCli);

			nErr = sendto(sockLocal, (char*)&p2piepDistributeOK, sizeof(p2piepDistributeOK),
				0, (sockaddr*)&addrCli, nCliAddrLen);

			if (SOCKET_ERROR == nErr)
			{
				strInfo.Format(_T("发送发布成功回应失败。\r\n(错误代码：%d)"),
					WSAGetLastError());
				pThis->SetNews(strInfo);
			}
			break;

		case P2PIEP_TYPE_UNDIST:	// 资源撤销
			P2PIEP p2piepUnDistributeOK;
			memset(&p2piepUnDistributeOK, 0, sizeof(p2piepUnDistributeOK));

			p2piepDistributeOK.Type = pThis->UnDistributeSource(p2piepRecv, addrCli);

			nErr = sendto(sockLocal, (char*)&p2piepDistributeOK, sizeof(p2piepDistributeOK),
				0, (sockaddr*)&addrCli, nCliAddrLen);

			if (SOCKET_ERROR == nErr)
			{
				strInfo.Format(_T("发送撤销成功回应失败。\r\n(错误代码：%d)"),
					WSAGetLastError());
				pThis->SetNews(strInfo);
			}
			break;
		case P2PIEP_TYPE_SRCINFO_REQ:
			pThis->SendSrcInfo(sockLocal, addrCli);
			break;
		}
	}

	freeaddrinfo(paiLocal);
	closesocket(sockLocal);

	return 0;
}

void CP2PServerDlg::OnBnClickedCancel()
{
	WSACleanup();
	DeleteCriticalSection(&m_csSrcInfo);
	OnCancel();
}

int CP2PServerDlg::DistributeSource(const P2PIEP &p2piep, const SOCKADDR_IN6 &addrCli)
{
	CString strInfo;
	for (int n(0); n != MAX_SOURCE_NUM; ++n)
	{ 
 		TCHAR buff[100] = {0};
		if (!arrDistSrcInfo[n].bIsUse)
		{
//			EnterCriticalSection(&m_csSrcInfo);
			arrDistSrcInfo[n].bIsUse					= true;
			_tcscpy_s(arrDistSrcInfo[n].sourceInfo.szFileName, p2piep.SourceName);
			arrDistSrcInfo[n].sourceInfo.nSourceSize	= p2piep.SourceLen;
			arrDistSrcInfo[n].srcOwnerInfo.bUse[0]		= true;
			arrDistSrcInfo[n].srcOwnerInfo.srcOwner[0]	= addrCli;
			++arrDistSrcInfo[n].nOwnerNum;

			m_pListCtrl->InsertItem(n, arrDistSrcInfo[n].sourceInfo.szFileName);
			_itot_s(arrDistSrcInfo[n].sourceInfo.nSourceSize, buff, 10);
 			m_pListCtrl->SetItemText(n, 1, buff);
 			_itot_s(arrDistSrcInfo[n].nOwnerNum, buff, 10);
			m_pListCtrl->SetItemText(n, 2, buff);			
//			LeaveCriticalSection(&m_csSrcInfo);

			++m_nSrcNum;  // 新资源发布成功，记录资源数目
			strInfo.Format(_T("资源发布（新资源发布）：%s。"), p2piep.SourceName);
			SetNews(strInfo);
			return P2PIEP_TYPE_DIST_OK;
		}
		else if (arrDistSrcInfo[n].sourceInfo.nSourceSize == p2piep.SourceLen &&
			_tcscmp(arrDistSrcInfo[n].sourceInfo.szFileName, p2piep.SourceName) == 0)
		{
//			EnterCriticalSection(&m_csSrcInfo);
			for (int i(0); i < MAX_SRCOWNER_NUM; ++i)
			{
				if (arrDistSrcInfo[n].srcOwnerInfo.bUse[i] &&
					memcmp(&arrDistSrcInfo[n].srcOwnerInfo.srcOwner[i], &addrCli, sizeof(addrCli)) == 0)
				{
					strInfo.Format(_T("资源发布（重复发布）：%s。"), p2piep.SourceName);
					SetNews(strInfo);
					return P2PIEP_TYPE_DIST_ERR_DUP;
				}
			}

			for (int i(0); i < MAX_SRCOWNER_NUM; ++i)
			{
				if (arrDistSrcInfo[n].srcOwnerInfo.bUse[i] == 0)
				{
					arrDistSrcInfo[n].srcOwnerInfo.bUse[i]		= TRUE;
					arrDistSrcInfo[n].srcOwnerInfo.srcOwner[i]	= addrCli;

					++arrDistSrcInfo[n].nOwnerNum;

					_itot_s(arrDistSrcInfo[n].nOwnerNum, buff, 10);
					m_pListCtrl->SetItemText(n, 2, buff);
					strInfo.Format(_T("资源发布（增加拥有者）：%s。"), p2piep.SourceName);
					SetNews(strInfo);
					return P2PIEP_TYPE_DIST_OK;
				}
			}
//			LeaveCriticalSection(&m_csSrcInfo);
		}
	}

	return P2PIEP_TYPE_DIST_ERR_NOBUF;;
}

/**
 * \brief 撤销资源发布
 *
 * \param p2piep[in] P2PIEP报文
 * \param addrCli 客户端地址
 *
 * \return 返回P2PIEP定义的类型代码
 */
int CP2PServerDlg::UnDistributeSource(const P2PIEP &p2piep, const SOCKADDR_IN6 &addrCli)
{
	CString strInfo;
	strInfo.Format(_T("撤销资源发布：%s。"), p2piep.SourceName);
	SetNews(strInfo);

	for (int n(0); n != MAX_SOURCE_NUM; ++n)
	{
		TCHAR buff[10] = {0};
		if (arrDistSrcInfo[n].bIsUse &&
			arrDistSrcInfo[n].sourceInfo.nSourceSize == p2piep.SourceLen &&
			_tcscmp(arrDistSrcInfo[n].sourceInfo.szFileName, p2piep.SourceName) == 0)
		{
//			EnterCriticalSection(&m_csSrcInfo);
			for (int i(0); i < MAX_SRCOWNER_NUM; ++i)
			{
				if (arrDistSrcInfo[n].srcOwnerInfo.bUse[i] &&
					memcmp(&arrDistSrcInfo[n].srcOwnerInfo.srcOwner[i], &addrCli, sizeof(addrCli)) == 0)
				{
					arrDistSrcInfo[n].srcOwnerInfo.bUse[i]		= FALSE;
					memset(&arrDistSrcInfo[n].srcOwnerInfo.srcOwner[i], 0,
						sizeof(arrDistSrcInfo[n].srcOwnerInfo.srcOwner[i]));

					--arrDistSrcInfo[n].nOwnerNum;

					_itot_s(arrDistSrcInfo[n].nOwnerNum, buff, 10);
					m_pListCtrl->SetItemText(n, 2, buff);

					return P2PIEP_TYPE_UNDIST_OK;
				}
			}
//			LeaveCriticalSection(&m_csSrcInfo);

			return P2PIEP_TYPE_UNDIST_ERR_NOTOWN;
		}
	}
	return P2PIEP_TYPE_UNDIST_ERR_UNKNOW;
}

void CP2PServerDlg::SendSrcInfo(const SOCKET &sock, const SOCKADDR_IN6 &addrCli)
{
	int nTimeOut(TIMEOUT);
	int nOptLen = sizeof(nTimeOut);
	CString strInfo;

	int nErr = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, nOptLen);

	if (0 != nErr)
	{
		strInfo.Format(_T("设置套接字超时错误。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		SetNews(strInfo);
	}

	for (int n(0); n < m_nSrcNum; ++n)
	{
		P2PIEP p2piepSend;
		memset(&p2piepSend, 0, sizeof(p2piepSend));

		p2piepSend.Type = P2PIEP_TYPE_SRCINFO_RES;
		p2piepSend.SourceLen = arrDistSrcInfo[n].sourceInfo.nSourceSize;
		_tcscpy_s(p2piepSend.SourceName, arrDistSrcInfo[n].sourceInfo.szFileName);
		p2piepSend.SrcOwnerNum = arrDistSrcInfo[n].nOwnerNum;
		for (int i(0); i < arrDistSrcInfo[n].nOwnerNum; ++i)
		{
			memcpy(&p2piepSend.srcOwner[i],
				&arrDistSrcInfo[n].srcOwnerInfo.srcOwner[i],
				sizeof(arrDistSrcInfo[n].srcOwnerInfo.srcOwner[i]));
		}

		int nSendTimes(0);
		int nAddrLen(sizeof(SOCKADDR_IN6));

		while(RETRY_TIME != nSendTimes++)
		{
			int nErr = sendto(sock, (char*)(&p2piepSend), sizeof(p2piepSend),
				0, (sockaddr*)&addrCli, sizeof(addrCli));

			if (SOCKET_ERROR == nErr)
			{
				continue;
			}

			char buff[P2PIEP_LEN+1] = {0};
			int nRecvLen = recvfrom(sock, buff, sizeof(buff), 0, 0, 0);

			if (SOCKET_ERROR == nRecvLen)
			{
				continue;
			}

			if (nRecvLen != P2PIEP_LEN)
			{
				continue;
			}

			P2PIEP p2piepRecv;
			memcpy(&p2piepRecv, buff, nRecvLen);

			if (P2PIEP_TYPE_SRCINFO_OK == p2piepRecv.Type)
			{
				break;
			}
		}

		if (RETRY_TIME == nSendTimes)
		{
			SetNews(_T("向客户端发送资源信息失败：客户机没有应答"));

			nTimeOut = 0;

			nErr = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, nOptLen);

			if (0 != nErr)
			{
				strInfo.Format(_T("设置套接字超时错误。\r\n(错误代码：%d)"),
					WSAGetLastError());
				MessageBox(strInfo, _T("错误"), MB_ICONERROR);
				SetNews(strInfo);
			}
			return;
		}
	}

	P2PIEP p2piepSend;
	memset(&p2piepSend, 0, sizeof(p2piepSend));

	p2piepSend.Type = P2PIEP_TYPE_SRCINFO_END;

	int nSendTimes(0);
	int nAddrLen(sizeof(SOCKADDR_IN6));

	while(RETRY_TIME != nSendTimes++)
	{
		int nErr = sendto(sock, (char*)(&p2piepSend), sizeof(p2piepSend),
			0, (sockaddr*)&addrCli, sizeof(addrCli));

		if (SOCKET_ERROR == nErr)
		{
			continue;
		}
		break;
	}

	if (RETRY_TIME == nSendTimes)
	{
		SetNews(_T("向客户端发送资源信息失败：客户机没有应答"));
	}

	nTimeOut = 0;
	nOptLen = sizeof(nTimeOut);

	nErr = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&nTimeOut, nOptLen);

	if (0 != nErr)
	{
		strInfo.Format(_T("设置套接字超时错误。\r\n(错误代码：%d)"),
			WSAGetLastError());
		MessageBox(strInfo, _T("错误"), MB_ICONERROR);
		SetNews(strInfo);
	}
}
