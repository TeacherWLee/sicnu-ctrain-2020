
// demo2-chatDlg.h: 头文件
//

#pragma once


// Cdemo2chatDlg 对话框
class Cdemo2chatDlg : public CDialogEx
{
// 构造
public:
	Cdemo2chatDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DEMO2CHAT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonAsServ();
	afx_msg void OnBnClickedButtonAsClient();

private:
	SOCKET socket_serv;
	SOCKET socket_client;
	sockaddr_in si;
	void SetInfo(CString info);
	CString strInfo = _T("");
};
