#ifndef __NETMESSAGE__H
#define __NETMESSAGE__H

#include <Windows.h>

#pragma pack(1)

const int MaxBuffLen = 2044;

static unsigned char g_chSecretKey[128] = "jS9uhwvVUTqiiJR1YdIzjvqutXSbJoypX3XA2rRnkgFL327OYj38DSKI7T36dDt5";

///连接消息
#define MDM_CONNECT						1			///连接消息类型
///辅助处理消息标志
#define ASS_NET_TEST					1			///网络测试
#define ASS_CONNECT_SUCCESS 			3			///连接成功

struct NetMessageHead
{
	UINT uMessageSize; // 包大小
	UINT bMainID;      // 主ID
	UINT bAssistantID; // 辅助ID
	UINT bHandleCode;  // handleCode
	UINT bReserver;    // 保留字
};

///连接成功消息 
struct MSG_S_ConnectSuccess
{
	BYTE						bMaxVer;							///最新版本号码
	BYTE						bLessVer;							///最低版本号码
	BYTE						bReserve[2];						///保留字段
	UINT						i64CheckCode;						///加密后的校验码，由客户端解密在包头中返回
};

#define	MDM_GP_LOGON					100								///大厅登陆
#define ASS_GP_LOGON_BY_NAME			1							    ///通过用户名字登陆
#define ASS_GP_LOGON_SUCCESS			5							    ///登陆成功

///用户登陆（帐号）结构
struct MSG_GP_S_LogonByNameStruct
{
	bool								bForced;							///顶号
	UINT								uRoomVer;							///大厅版本
	char								szName[64];							///登陆名字
	char								TML_SN[128];
	char								szMD5Pass[52];						///登陆密码
	char								szMathineCode[64];					///本机机器码 锁定机器
	char                                szCPUID[24];						///CPU的ID
	char                                szHardID[64];						///硬盘的ID
	char								szIDcardNo[64];						///证件号
	char								szMobileVCode[8];					///手机验证码
	int									gsqPs;
	int									iUserID;							///用户ID登录，如果ID>0用ID登录
};

///大厅登陆返回数据包
struct MSG_GP_R_LogonResult
{
	long int							dwUserID;							///用户 ID 
	long int							dwGamePower;						///游戏权限
	long int							dwMasterPower;						///管理权限
	int									dwMobile;							///手机号码
	int									dwAccID;							///Acc 号码
	ULONG								dwLastLogonIP;						///上次登陆 IP
	ULONG								dwNowLogonIP;						///现在登陆 IP
	int									iLogonType;							//登录类型
	UINT								bLogoID;							///用户头像
	bool								bBoy;								///性别
	char								szName[61];							///用户登录名
	char								TML_SN[128];						///数字签名
	char								szMD5Pass[50];						///用户密码
	char								nickName[32];						///用户昵称
	__int64								i64Money;							///用户金币
	__int64								i64Bank;							///用户财富
	int									iJewels;							//钻石
	int									iLotteries;							///奖券
	int									dwFascination;						///魅力

																			//JianGK 20111107新用户资料
	char								szSignDescr[128];					///个性签名
	char								szRealName[20];						///真实姓名
	char								szIDCardNo[36];						///证件号
	char								szMobileNo[50];						///移动电话
	char								szQQNum[20];						///QQ号码
	char								szAdrNation[50];					///玩家的国藉
	char								szAdrProvince[50];					///玩家所在的省份
	char								szAdrCity[50];						///玩家所在的城市
	char								szZipCode[10];						///邮政编码
	char								szEmail[50];						///电子邮件
	char								szAddress[128];						///联系地址
	char								szSchooling[20];					///文化程度
	char								szHomePage[128];					///个人主页
	char								szTelNo[20];						///固定电话
	char								szMSNID[50];						//MSN帐号
																			//end JianGK 20111107
	char								szHeadUrl[256];						//头像URL
	int									dwTimeIsMoney;						///上次登陆时长所换取的金币
	int									iVipTime;							///
	int									iDoublePointTime;					///双倍积分时间
	int									iProtectTime;						///护身符时间，保留
	bool								bLoginBulletin;						///是否有登录公告，Fred Huang,2008-05-20
	int									iLockMathine;						///当前帐号是否锁定了某台机器，1为锁定，0为未锁定 zxj 2009-11-13
	int									iBindMobile;						///当前帐号是否绑定手机号码，1为绑定，0为未绑定 jianguankun 2012.10.10

	int									iAddFriendType;						///是否允许任何人加为好友
	int									iCutRoomID;							//断线重连房间号
};

// 登录房间
#define MDM_GR_LOGON 100
#define ASS_GR_LOGON_BY_ID 5									///通过用户 ID 登陆
#define ASS_GR_LOGON_SUCCESS			2									///登陆成功

///游戏房间登陆 经度维度不一定有
struct MSG_GR_S_RoomLogon
{
	bool								bForced;							///顶号
	UINT								uNameID;							///名字 ID
	UINT								dwUserID;							///用户 ID
	UINT								uRoomVer;							///大厅版本
	UINT								uGameVer;							///游戏版本
	CHAR								szMD5Pass[50];						///加密密码
	//float                               fLongitude;                         ///经度
	//float                               fLatitude;                          ///纬度
};

/////游戏房间登陆
//struct MSG_GR_R_LogonResult
//{
//	LONG								dwGamePower;						///用户权限
//	LONG								dwMasterPower;						///管理权限
//	LONG								dwRoomRule;							///设置规则
//	UINT								uLessPoint;							///最少经验值
//	UINT								uMaxPoint;							///最多经验值
//	UserInfoStruct						pUserInfoStruct;					///用户信息
//	RECEIVEMONEY                        strRecMoney;                        //非比赛场玩家金币不足自动赠送
//
//																			///wushuqun 2009.6.6
//																			///登录房间时即时获取虚拟玩家人数
//	int									nVirtualUser;
//	int									nPresentCoinNum;  ///< 赠送金币数量
//
//
//
//														  //比赛专用
//	int									iContestID;
//	int									iLowCount;
//	__int64								i64Chip;
//	__int64								i64TimeStart;
//	__int64								i64TimeEnd;
//	__int64								i64LowChip;
//	int									iTimeout;
//	int									iBasePoint;
//
//	bool								bGRMUser;//该玩家可以打开房间管理窗口
//	bool								bGRMRoom;//该房间可以打开房间管理窗口
//
//	MSG_GR_GRM_UpData					GRM_Updata;//管理窗口更新数据
//
//	MSG_GR_R_LogonResult()
//	{
//		memset(this, 0, sizeof(MSG_GR_R_LogonResult));
//	}
//
//
//};















#pragma pack()


#endif
