
// P2PServerDlg.h : 头文件
//

#pragma once


// CP2PServerDlg 对话框
class CP2PServerDlg : public CDialog
{
// 构造
public:
	CP2PServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_P2PSERVER_DIALOG };

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

private:
	DistSrcInfo arrDistSrcInfo[MAX_SOURCE_NUM];
	CRITICAL_SECTION m_csSrcInfo;
	CListCtrl *m_pListCtrl;
	int m_nSrcNum;											/**< 记录资源数目 */

	static unsigned int __stdcall WorkThread(LPVOID);		/**< 服务器工作线程 */
	void SetNews(const CString &strNews);					/**< 设置动态信息 */
	int DistributeSource(const P2PIEP &p2piep, const SOCKADDR_IN6 &addrCli);	/**< 保存信息 */
	int UnDistributeSource(const P2PIEP &p2piep, const SOCKADDR_IN6 &addrCli);	/**< 资源撤销 */
	void SendSrcInfo(const SOCKET &sock, const SOCKADDR_IN6 &addrCli);								/**< 向客户端发送资源信息 */

public:
	afx_msg void OnBnClickedCancel();
};
