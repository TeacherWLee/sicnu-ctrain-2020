
// demo3Dlg.cpp: 实现文件
//

#include <WS2tcpip.h>
#include <altconv.h>

#include "pch.h"
#include "framework.h"
#include "demo3.h"
#include "demo3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// Cdemo3Dlg 对话框



Cdemo3Dlg::Cdemo3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DEMO3_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void Cdemo3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(Cdemo3Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SHOW, &Cdemo3Dlg::OnBnClickedBtnShow)
	ON_BN_CLICKED(IDC_BTN_PROGRESS, &Cdemo3Dlg::OnBnClickedBtnProgress)
	ON_BN_CLICKED(IDC_BTN_IP, &Cdemo3Dlg::OnBnClickedBtnIp)
	ON_BN_CLICKED(IDC_BTN_FILE, &Cdemo3Dlg::OnBnClickedBtnFile)
	ON_BN_CLICKED(IDC_BTN_DLG, &Cdemo3Dlg::OnBnClickedBtnDlg)
	ON_BN_CLICKED(IDC_BTN_SERV, &Cdemo3Dlg::OnBnClickedBtnServ)
	ON_BN_CLICKED(IDC_BTN_SEND, &Cdemo3Dlg::OnBnClickedBtnSend)
END_MESSAGE_MAP()


// Cdemo3Dlg 消息处理程序

BOOL Cdemo3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	IncSize();

	CIPAddressCtrl* pIP = (CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS1);
	pIP->SetAddress(127, 0, 0, 1);

	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST1);
	pListCtrl->SetTextColor(RGB(0, 0, 255));
	pListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	pListCtrl->InsertColumn(0, _T("资源名称"), LVCFMT_CENTER, 200);
	pListCtrl->InsertColumn(1, _T("资源大小"), LVCFMT_CENTER, 80);
	pListCtrl->InsertColumn(2, _T("拥有者"), LVCFMT_CENTER, 80);

	// 初始化WinSocket环境
	// 初始化WSA环境
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	// 创建套接字
	socket_serv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// 设置服务器的地址
	si.sin_family = AF_INET;
	si.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	si.sin_port = htons(10000);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void Cdemo3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void Cdemo3Dlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR Cdemo3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Cdemo3Dlg::IncSize()
{
	nSize += 1;
	printf("%d", nSize);
}



void Cdemo3Dlg::OnBnClickedBtnShow()
{
	// TODO: 在此添加控件通知处理程序代码
	CString str;
	GetDlgItemText(IDC_EDIT_INPUT, str);
	SetDlgItemText(IDC_STATIC_INFO, str);
}


void Cdemo3Dlg::OnBnClickedBtnProgress()
{
	// TODO: 在此添加控件通知处理程序代码
	CProgressCtrl* pProgressCtrl = (CProgressCtrl*)GetDlgItem(IDC_PROGRESS1);
	pProgressCtrl->SetRange(0, 100);
	pProgressCtrl->SetPos(0);

	for (int n = 0; n <= 100; n++)
	{
		pProgressCtrl->SetPos(n);
	}
}


void Cdemo3Dlg::OnBnClickedBtnIp()
{
	// TODO: 在此添加控件通知处理程序代码
	CIPAddressCtrl* pIP = (CIPAddressCtrl*)GetDlgItem(IDC_IPADDRESS1);
	BYTE n1, n2, n3, n4;
	pIP->GetAddress(n1, n2, n3, n4);
	CString str;
	str.Format(_T("%d.%d.%d.%d"), n1, n2, n3, n4);
	SetDlgItemText(IDC_STATIC_INFO, str);
}


void Cdemo3Dlg::OnBnClickedBtnFile()
{
	// TODO: 在此添加控件通知处理程序代码
	CListCtrl* pListCtrl = (CListCtrl*)GetDlgItem(IDC_LIST1);

	pListCtrl->InsertItem(0, _T("勇敢的心.mp4"));
	pListCtrl->SetItemText(0, 1, _T("30GB"));
	pListCtrl->SetItemText(0, 2, _T("李巍"));
}


void Cdemo3Dlg::OnBnClickedBtnDlg()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialog dlg(IDD_DIALOG1);
	dlg.DoModal();
}


void Cdemo3Dlg::OnBnClickedBtnServ()
{
	// TODO: 在此添加控件通知处理程序代码
	// 服务器绑定地址
	bind(socket_serv, (struct sockaddr*)&si, sizeof(si));

	// 创建客户端地址
	SOCKADDR addrCli;
	int nCliAddrLen = sizeof(addrCli);

	// 接收客户端消息缓存
	char buff[100] = { 0 };

	// 接收客户端信息
	while (true)
	{
		recvfrom(socket_serv, buff, sizeof(buff), 0, (sockaddr*)&addrCli, &nCliAddrLen);
		SetDlgItemText(IDC_STATIC_INFO, CString(buff));		// 显示
	}
}


void Cdemo3Dlg::OnBnClickedBtnSend()
{
	SOCKET socket_cli = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	char* buff = "hello";
	sendto(socket_cli, buff, sizeof(buff), 0, (sockaddr*)&si, sizeof(si));
}
