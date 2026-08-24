#ifndef XA_QR_H
#define XA_QR_H

struct QR_Info
{
	unsigned char Plat;						//1	二维码版本	HEX	1	二维码结构版本号。0x01~0x7F 大连地铁交通行业二维码标准版本，初始版本0x01
	unsigned char genType;					//2	二维码生成类型	HEX	1	定义发码的方式：高半字节：二维码生成算法
											//当前取值0010b:基于对称密钥算法的脱机验证
											//低半字节：二维码生成方式
											//当前取值0001b -联机发码；
											//当前取值0010b -脱机发码；
	unsigned char KID1[2];					//3	发码平台签名密钥索引KID1	HEX	2	用于终端索引用户密钥
	unsigned char KID2[2];					//4	移动应用签名密钥索引KID2	HEX	2	用户终端索引签名密钥
	unsigned char PlatformID[8];			//5	二维码凭证号	HEX	8	二维码标识号，二维码平台内唯一
	unsigned char AppID[8];					//6	移动应用标识	HEX	8	用于申请二维码的移动应用标识
	unsigned char Appcode[4];				//7	移动应用机构代码	HEX	4	用于申请二维码的商户标识
	unsigned char UserID[8];				//8	用户标识	HEX	8	在移动应用内唯一标识商户

	//
	unsigned long GenerateQRtime;			//9	二维码生成时间	HEX	4	经校准的二维码生成时间戳
											//	以4字节无符号整形数字表示。计算从UTC-8时区的2017年1月1日0时0分0秒起流逝的秒数。
	unsigned short qr_Delay;				//10	二维码有效时间	HEX	2	与二维码生成时间一起控制二维码有效时间。以秒为单位，此域在填写时无需带单位
	unsigned char limitBusiness[2];		//11	行业使用范围	HEX	2	地铁应用，暂时固定为：0x0100
	
	unsigned char BusinessLen;				//12	行业自定义	数据长度	HEX	1	由发码平台及行业方定义的信息（13-16）
	unsigned char IDType;					//13	凭证类型	HEX	1	见“二维码凭证类型”表
	unsigned char limitEntryExit;			//14	进出站使用限制	HEX	1	当前二维码可执行业务标志
											//		0x00：不限；0x01：可进站；0x02：可出站
	unsigned char limitStation[8];			//15	站点限制	HEX	8	站点限制，每两个字节标识为一个站点，最多4个站点，若全0则表示不做限制
	unsigned char Travel[8];				//16	行程单号	HEX	8	行程单号，当行程为出站二维码时有效，由出行服务平台生成，用于唯一标识乘客的行程单
	//unsigned char RFU[6];					//17	RFU	HEX	6	预留备用信息
	unsigned char PlatMAC[4];				//18	发码机构签名	HEX	4	使用KID1对第5~11项进行签名
	unsigned char UserMAC[4];				//19	移动用户签名	HEX	4	使用KID2对第12~18项进行签名
}__attribute__( ( packed, aligned(1) ) );
typedef struct QR_Info	QR_Info_t;

QR_Info_t 	tpQR_Info_val;

unsigned char qr_info[2048];
unsigned long qr_len;

unsigned long 	qr_timeout;

unsigned short qr_send_recv(int fd, short *out_len);
unsigned char xa_polling_qr(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);

//unsigned short qr_TellValidateArea();
//unsigned short dl_qr_logic(unsigned char *cmd_buf);
//
//unsigned short dl_qr_entry(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
//unsigned short dl_qr_entry_prepare(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
//unsigned short dl_qr_exit(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);
//unsigned short dl_qr_exit_prepare(unsigned char *cmd_buf, unsigned char *out_buf, unsigned short *out_len);



#endif