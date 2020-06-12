/**
*
* \file
*
* \brief IPv6�Ե�����Դ����ϵͳ �ṹ���궨���ļ�
*
* \author ��Ρ
* \version 1.0
* \date 2009-08-28
*
* ��Ȩ����(C) ���ִ�ѧ�������ѧ�뼼��ѧԺ
*
*/

#pragma once
///////////////////////////    Setting    //////////////////////////////

const int MAX_SOURCE_NAME_LEN	(31);					/**< ��Դ������󳤶� */

const int MAX_SOURCE_NUM		(5);					/**< �����������������Դ���� */
const int MAX_SRCOWNER_NUM		(5);					/**< ������������Դӵ���������� */

const int NEWS_ITEM_NUM_SRV		(6);					/**< ��������ʾ����Ϣ��Ŀ */
const int NEWS_ITEM_NUM_CLI		(4);					/**< �ͻ�����ʾ����Ϣ��Ŀ */

const int TIMEOUT				(2000);					/**< ��ʱʱ�� */
const int RETRY_TIME			(3);					/**< ���Դ�����Ϊ�˷�ֹ���紫������еĶ��� */

const int P2PDEP_PAYLOAD_LENGTH	(1000);					/**< P2PDEP���س��ȣ����ݲ�ͬ������Ӧ���ò�ֵͬ */

const int BUFFER_SIZE_KB		(100);					/**< ���û�������С����KBΪ��λ */



///////////////////////////    Working    ///////////////////////////////
// ��������С
const int BUFFER_SIZE						(BUFFER_SIZE_KB*1024);

// P2PIEP��ʽ
struct P2PIEP
{
	UCHAR			Type;								/**< ���� */
	int				SourceLen;							/**< ��Դ���� */
	TCHAR			SourceName[MAX_SOURCE_NAME_LEN+1];	/**< ��Դ���� */
	int				SrcOwnerNum;						/**< ��Դӵ������Ŀ */	
	SOCKADDR_IN6	srcOwner[MAX_SRCOWNER_NUM];			/**< ��Դӵ���� */
};

const int	P2PIEP_LEN(sizeof(P2PIEP));					/**< P2PIEPЭ���ʽ�ĳ��� */

const int	P2PIEP_TYPE_SRVTEST				(0x00);		/**< ���Է�����״̬ */
const int	P2PIEP_TYPE_SRVOK				(0x01);		/**< ������״̬���� */
const int	P2PIEP_TYPE_SRVEXCEPTION		(0x02);		/**< ������״̬�쳣 */
const int	P2PIEP_TYPE_DIST				(0x03);		/**< ������Դ */
const int	P2PIEP_TYPE_DIST_OK				(0x04);		/**< ��Դ�����ɹ� */
const int	P2PIEP_TYPE_DIST_ERR_NOBUF		(0x05);		/**< ��Դ�������󣺷������Ѿ��ﵽ�����Դ�� */
const int	P2PIEP_TYPE_DIST_ERR_DUP		(0x06);		/**< ��Դ�������󣺿ͻ����ظ�������Դ */
const int	P2PIEP_TYPE_DIST_ERR_UNKNOW		(0x07);		/**< ��Դ��������δ֪���� */
const int	P2PIEP_TYPE_UNDIST				(0x08);		/**< ������Դ���� */
const int	P2PIEP_TYPE_UNDIST_OK			(0x09);		/**< ��Դ�����ɹ� */
const int	P2PIEP_TYPE_UNDIST_ERR_NOTOWN	(0x0A);		/**< ��Դ�������󣺿ͻ�������Դӵ���� */
const int	P2PIEP_TYPE_UNDIST_ERR_UNKNOW	(0x0B);		/**< ��Դ��������δ֪���� */
const int	P2PIEP_TYPE_SRCINFO_REQ			(0x0C);		/**< ��Դ��Ϣ���� */
const int	P2PIEP_TYPE_SRCINFO_RES			(0x0D);		/**< ��Դ��ϢӦ�� */
const int	P2PIEP_TYPE_SRCINFO_OK			(0x0E);		/**< ������Դ��ϢӦ��ɹ� */
const int	P2PIEP_TYPE_SRCINFO_END			(0x0F);		/**< ��Դ��Ϣ������� */


// �˿ں�
static const char	*P2PIEP_SRV_PORT_S		("5893");	/**< P2PIEPЭ��ķ������˿ں� */
const int			P2PIEP_SRV_PORT_N		(htons(5893));
static const char	*P2PIEP_CLI_PORT_S		("5892");	/**< P2PIEPЭ��Ŀͻ��˶˿ں� */
const int			P2PIEP_CLI_PORT_N		(htons(5892));

// P2PDEP��ʽ
struct P2PDEP
{
	int				type;								/**< ���� */
	int				OffSet;								/**< ƫ�� */
	int				Length;								/**< ���� */
	BYTE			Data[P2PDEP_PAYLOAD_LENGTH];			/**< ���� */
};

const int	P2PDEP_LEN(sizeof(P2PDEP));					/**< P2PDEPЭ���ʽ�ĳ��� */

const int	P2PDEP_TYPE_SRC_REQ				(0x81);		/**< ����Դӵ����������Դ */
const int	P2PDEP_TYPE_READY				(0x82);		/**< ��Դӵ����׼�����������Դ����ļ� */
const int	P2PDEP_TYPE_GET_NAME			(0x83);		/**< ������Դ������ */
const int	P2PDEP_TYPE_NAME_OK				(0x84);		/**< ����������ȷ��ӵ�и���Դ */
const int	P2PDEP_TYPE_NAME_ERR			(0x85);		/**< �����������ӵ���߲����и���Դ */
const int	P2PDEP_TYPE_GET_DATA			(0x86);		/**< ������Դ������ */
const int	P2PDEP_TYPE_DATA_OK				(0x87);		/**< ���ݷ�����ȷ */
const int	P2PDEP_TYPE_DATA_ERR			(0x88);		/**< ���ݷ��ʹ��� */
const int	P2PDEP_TYPE_COMPLETE			(0x89);		/**< ��Դ������� */

static const char	*P2PDEP_LISTEN_PORT_S	("11670");	/**< P2PDEP��Դ��������˿� */
const int			P2PDEP_LISTEN_PORT_N	(htons(11670));
static const char	*P2PDEP_RECV_PORT_S		("11671");	/**< P2PDEP��Դ���ն˿� */
const int			P2PDEP_RECV_PORT_N		(htons(11671));



const int	IPV6_ADDR_MAX_STRLEN			(50);		/**< IPv6��ַ�ַ�����ʾ������ַ������� */

// ��Դ��Ϣ
struct SourceInfo
{
	TCHAR			szFileName[MAX_SOURCE_NAME_LEN+1];		/**< ��Դ���� */
	int				nSourceSize;							/**< ��Դ��С */
};

struct SrcOwnerInfo
{
	int				bUse[MAX_SRCOWNER_NUM];
	SOCKADDR_IN6	srcOwner[MAX_SRCOWNER_NUM];
};

// ���淢����Դ����Ϣ
struct DistSrcInfo
{
	int				bIsUse;									// Ϊ��ʱ��ʾ����Դ
	SourceInfo		sourceInfo;
	int				nOwnerNum;								// ����Դӵ��������
	SrcOwnerInfo	srcOwnerInfo;	
};
