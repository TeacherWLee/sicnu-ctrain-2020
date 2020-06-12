
// P2PSharerDlg.h : 头文件
//

#pragma once

#ifndef	BIF_NEWDIALOGSTYLE
#define		BIF_NEWDIALOGSTYLE		0x0040
#endif

#ifndef	BIF_USENEWUI
#define		BIF_USENEWUI			0x0050
#endif

#define			SHARE_FOLDER_NAME		_T("ShareFolder")

// CP2PSharerDlg 对话框
class CP2PSharerDlg : public CDialog
{
// 构造
public:
	CP2PSharerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_P2PSHARER_DIALOG };

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
	struct CliInfo
	{
		SOCKADDR_IN6		addrCliRequester;
		CP2PSharerDlg		*pThis;
	};

	struct OwnerInfo
	{
		int					nDownState;
		CFile				*pFile;
		TCHAR				tcsSrcName[MAX_SOURCE_NAME_LEN+1];
		SOCKADDR_IN6		*pAddrSrcOwner;
		int					nFileStart;
		int					nFileEnd;
		CRITICAL_SECTION	*pCriticalSection;
	};

	static const int DOWN_STATE_READY	= -1;				/**< 准备下载 */
	static const int DOWN_STATE_CANNOT	= -2;				/**< 不能下载 */

	CString m_strWorkDir;									/**< 工作目录 */
	SOCKET m_sockLocalToSrv;								/**< 本地套接字，用于连接服务器 */
	addrinfo *m_paiSrv;
	addrinfo *m_paiLocalToSrv;
	addrinfo *m_paiLocalToCli;
	CListCtrl *m_pListCtrl;									/**< List控件指针 */
	int m_nSrcNUM;											/**< 可用的资源数量 */
	CProgressCtrl *m_pProgressCtrl;							/**< 指向Progress Control的指针 */
	CString m_strDownFilePath;								/**< 下载资源的路径 */
	int m_nDownloadIndex;									/**< 需要下载的文件编号 */
	bool m_bDistRSCWithDebug;

	DistSrcInfo arrDistSrcInfo[MAX_SOURCE_NUM];				/**< 记录从服务器获得的资源信息 */

	vector<SourceInfo> vcSourceInfo;						/**< 保存本地资源信息 */


	void SetNews(const CString &strNews);					/**< 设置动态信息 */
	bool SrvTest();											/**< 测试服务器状态 */
	void ScanSourceInWorkDir();								/**< 获取共享目录下的资源 */
	void InitialSocket();									/**< 初始化socket环境 */
	void UnDistributeSrc();									/**< 撤销资源发布 */

	static unsigned int __stdcall ListenThread(LPVOID);		/**< 资源获取监听线程 */
	static unsigned int __stdcall TransfersThread(LPVOID);	/**< 资源传送线程 */
	static unsigned int __stdcall DownLoadThread(LPVOID);	/**< 资源下载线程 */
	static unsigned int __stdcall DownPrepareThread(LPVOID);/**< 资源下载准备线程 */
	

public:
	afx_msg void OnBnClickedBtnBrowse();
	afx_msg void OnBnClickedBtnOpensharefolder();
	afx_msg void OnBnClickedBtnSrvtest();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBtnDistributesource();
	afx_msg void OnBnClickedBtnChnsrvaddr();
	afx_msg void OnBnClickedBtnRunsrc();
	afx_msg void OnBnClickedBtnRefreshsrc();
	afx_msg void OnBnClickedBtnDownloadsrc();
};
