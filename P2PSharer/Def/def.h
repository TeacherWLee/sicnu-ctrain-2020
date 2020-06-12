/**
*
* \file
*
* \brief IPv6对等网资源共享系统 结构、宏定义文件
*
* \author 李巍
* \version 1.0
* \date 2009-08-28
*
* 版权所有(C) 吉林大学计算机科学与技术学院
*
*/

#pragma once
///////////////////////////    Setting    //////////////////////////////

const int MAX_SOURCE_NAME_LEN	(31);					/**< 资源名称最大长度 */

const int MAX_SOURCE_NUM		(5);					/**< 服务器保留的最大资源个数 */
const int MAX_SRCOWNER_NUM		(5);					/**< 服务器保存资源拥有者最大个数 */

const int NEWS_ITEM_NUM_SRV		(6);					/**< 服务器显示的信息条目 */
const int NEWS_ITEM_NUM_CLI		(4);					/**< 客户端显示的信息条目 */

const int TIMEOUT				(2000);					/**< 超时时间 */
const int RETRY_TIME			(3);					/**< 重试次数，为了防止网络传输过程中的丢包 */

const int P2PDEP_PAYLOAD_LENGTH	(1000);					/**< P2PDEP负载长度，根据不同的网络应设置不同值 */

const int BUFFER_SIZE_KB		(100);					/**< 设置缓冲区大小，以KB为单位 */



///////////////////////////    Working    ///////////////////////////////
// 缓冲区大小
const int BUFFER_SIZE						(BUFFER_SIZE_KB*1024);

// P2PIEP格式
struct P2PIEP
{
	UCHAR			Type;								/**< 类型 */
	int				SourceLen;							/**< 资源长度 */
	TCHAR			SourceName[MAX_SOURCE_NAME_LEN+1];	/**< 资源名称 */
	int				SrcOwnerNum;						/**< 资源拥有者数目 */	
	SOCKADDR_IN6	srcOwner[MAX_SRCOWNER_NUM];			/**< 资源拥有者 */
};

const int	P2PIEP_LEN(sizeof(P2PIEP));					/**< P2PIEP协议格式的长度 */

const int	P2PIEP_TYPE_SRVTEST				(0x00);		/**< 测试服务器状态 */
const int	P2PIEP_TYPE_SRVOK				(0x01);		/**< 服务器状态正常 */
const int	P2PIEP_TYPE_SRVEXCEPTION		(0x02);		/**< 服务器状态异常 */
const int	P2PIEP_TYPE_DIST				(0x03);		/**< 发布资源 */
const int	P2PIEP_TYPE_DIST_OK				(0x04);		/**< 资源发布成功 */
const int	P2PIEP_TYPE_DIST_ERR_NOBUF		(0x05);		/**< 资源发布错误：服务器已经达到最大资源数 */
const int	P2PIEP_TYPE_DIST_ERR_DUP		(0x06);		/**< 资源发布错误：客户端重复发布资源 */
const int	P2PIEP_TYPE_DIST_ERR_UNKNOW		(0x07);		/**< 资源发布错误：未知错误 */
const int	P2PIEP_TYPE_UNDIST				(0x08);		/**< 撤销资源发布 */
const int	P2PIEP_TYPE_UNDIST_OK			(0x09);		/**< 资源撤销成功 */
const int	P2PIEP_TYPE_UNDIST_ERR_NOTOWN	(0x0A);		/**< 资源撤销错误：客户不是资源拥有者 */
const int	P2PIEP_TYPE_UNDIST_ERR_UNKNOW	(0x0B);		/**< 资源撤销错误：未知错误 */
const int	P2PIEP_TYPE_SRCINFO_REQ			(0x0C);		/**< 资源信息请求 */
const int	P2PIEP_TYPE_SRCINFO_RES			(0x0D);		/**< 资源信息应答 */
const int	P2PIEP_TYPE_SRCINFO_OK			(0x0E);		/**< 接收资源信息应答成功 */
const int	P2PIEP_TYPE_SRCINFO_END			(0x0F);		/**< 资源信息发送完成 */


// 端口号
static const char	*P2PIEP_SRV_PORT_S		("5893");	/**< P2PIEP协议的服务器端口号 */
const int			P2PIEP_SRV_PORT_N		(htons(5893));
static const char	*P2PIEP_CLI_PORT_S		("5892");	/**< P2PIEP协议的客户端端口号 */
const int			P2PIEP_CLI_PORT_N		(htons(5892));

// P2PDEP格式
struct P2PDEP
{
	int				type;								/**< 类型 */
	int				OffSet;								/**< 偏移 */
	int				Length;								/**< 长度 */
	BYTE			Data[P2PDEP_PAYLOAD_LENGTH];			/**< 负载 */
};

const int	P2PDEP_LEN(sizeof(P2PDEP));					/**< P2PDEP协议格式的长度 */

const int	P2PDEP_TYPE_SRC_REQ				(0x81);		/**< 向资源拥有者请求资源 */
const int	P2PDEP_TYPE_READY				(0x82);		/**< 资源拥有者准备就绪，可以传送文件 */
const int	P2PDEP_TYPE_GET_NAME			(0x83);		/**< 请求资源的名字 */
const int	P2PDEP_TYPE_NAME_OK				(0x84);		/**< 名字请求正确，拥有该资源 */
const int	P2PDEP_TYPE_NAME_ERR			(0x85);		/**< 名字请求错误，拥有者不持有该资源 */
const int	P2PDEP_TYPE_GET_DATA			(0x86);		/**< 请求资源的数据 */
const int	P2PDEP_TYPE_DATA_OK				(0x87);		/**< 数据发送正确 */
const int	P2PDEP_TYPE_DATA_ERR			(0x88);		/**< 数据发送错误 */
const int	P2PDEP_TYPE_COMPLETE			(0x89);		/**< 资源传送完毕 */

static const char	*P2PDEP_LISTEN_PORT_S	("11670");	/**< P2PDEP资源请求监听端口 */
const int			P2PDEP_LISTEN_PORT_N	(htons(11670));
static const char	*P2PDEP_RECV_PORT_S		("11671");	/**< P2PDEP资源接收端口 */
const int			P2PDEP_RECV_PORT_N		(htons(11671));



const int	IPV6_ADDR_MAX_STRLEN			(50);		/**< IPv6地址字符串表示的最大字符串长度 */

// 资源信息
struct SourceInfo
{
	TCHAR			szFileName[MAX_SOURCE_NAME_LEN+1];		/**< 资源名字 */
	int				nSourceSize;							/**< 资源大小 */
};

struct SrcOwnerInfo
{
	int				bUse[MAX_SRCOWNER_NUM];
	SOCKADDR_IN6	srcOwner[MAX_SRCOWNER_NUM];
};

// 保存发布资源的信息
struct DistSrcInfo
{
	int				bIsUse;									// 为真时表示有资源
	SourceInfo		sourceInfo;
	int				nOwnerNum;								// 该资源拥有者数量
	SrcOwnerInfo	srcOwnerInfo;	
};
