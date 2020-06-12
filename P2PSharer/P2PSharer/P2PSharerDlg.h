
// P2PSharerDlg.h : ͷ�ļ�
//

#pragma once

#ifndef	BIF_NEWDIALOGSTYLE
#define		BIF_NEWDIALOGSTYLE		0x0040
#endif

#ifndef	BIF_USENEWUI
#define		BIF_USENEWUI			0x0050
#endif

#define			SHARE_FOLDER_NAME		_T("ShareFolder")

// CP2PSharerDlg �Ի���
class CP2PSharerDlg : public CDialog
{
// ����
public:
	CP2PSharerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_P2PSHARER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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

	static const int DOWN_STATE_READY	= -1;				/**< ׼������ */
	static const int DOWN_STATE_CANNOT	= -2;				/**< �������� */

	CString m_strWorkDir;									/**< ����Ŀ¼ */
	SOCKET m_sockLocalToSrv;								/**< �����׽��֣��������ӷ����� */
	addrinfo *m_paiSrv;
	addrinfo *m_paiLocalToSrv;
	addrinfo *m_paiLocalToCli;
	CListCtrl *m_pListCtrl;									/**< List�ؼ�ָ�� */
	int m_nSrcNUM;											/**< ���õ���Դ���� */
	CProgressCtrl *m_pProgressCtrl;							/**< ָ��Progress Control��ָ�� */
	CString m_strDownFilePath;								/**< ������Դ��·�� */
	int m_nDownloadIndex;									/**< ��Ҫ���ص��ļ���� */
	bool m_bDistRSCWithDebug;

	DistSrcInfo arrDistSrcInfo[MAX_SOURCE_NUM];				/**< ��¼�ӷ�������õ���Դ��Ϣ */

	vector<SourceInfo> vcSourceInfo;						/**< ���汾����Դ��Ϣ */


	void SetNews(const CString &strNews);					/**< ���ö�̬��Ϣ */
	bool SrvTest();											/**< ���Է�����״̬ */
	void ScanSourceInWorkDir();								/**< ��ȡ����Ŀ¼�µ���Դ */
	void InitialSocket();									/**< ��ʼ��socket���� */
	void UnDistributeSrc();									/**< ������Դ���� */

	static unsigned int __stdcall ListenThread(LPVOID);		/**< ��Դ��ȡ�����߳� */
	static unsigned int __stdcall TransfersThread(LPVOID);	/**< ��Դ�����߳� */
	static unsigned int __stdcall DownLoadThread(LPVOID);	/**< ��Դ�����߳� */
	static unsigned int __stdcall DownPrepareThread(LPVOID);/**< ��Դ����׼���߳� */
	

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
