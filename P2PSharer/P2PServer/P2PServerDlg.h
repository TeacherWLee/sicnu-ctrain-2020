
// P2PServerDlg.h : ͷ�ļ�
//

#pragma once


// CP2PServerDlg �Ի���
class CP2PServerDlg : public CDialog
{
// ����
public:
	CP2PServerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_P2PSERVER_DIALOG };

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
	DistSrcInfo arrDistSrcInfo[MAX_SOURCE_NUM];
	CRITICAL_SECTION m_csSrcInfo;
	CListCtrl *m_pListCtrl;
	int m_nSrcNum;											/**< ��¼��Դ��Ŀ */

	static unsigned int __stdcall WorkThread(LPVOID);		/**< �����������߳� */
	void SetNews(const CString &strNews);					/**< ���ö�̬��Ϣ */
	int DistributeSource(const P2PIEP &p2piep, const SOCKADDR_IN6 &addrCli);	/**< ������Ϣ */
	int UnDistributeSource(const P2PIEP &p2piep, const SOCKADDR_IN6 &addrCli);	/**< ��Դ���� */
	void SendSrcInfo(const SOCKET &sock, const SOCKADDR_IN6 &addrCli);								/**< ��ͻ��˷�����Դ��Ϣ */

public:
	afx_msg void OnBnClickedCancel();
};
