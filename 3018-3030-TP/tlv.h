#ifndef		__TLV_H_
#define		__TLV_H_


// TLV结构体
//struct TLVEntity {
//	unsigned char* Tag; //标记
//	unsigned char* Length; //数据长度
//	unsigned char* Value; //数据
//	unsigned int TagSize; //标记占用字节数
//	unsigned int LengthSize; //数据长度占用字节数
//	TLVEntity* Sub_TLVEntity; //子嵌套TLV实体
//};

// TLV结构体
typedef struct TLVEntity{
	unsigned short	Tag; 			//标记
	unsigned long	Length; 		//数据长度
	unsigned char	*Value; 		//数据
	unsigned long 	UpArray; 		//和自己平级的标记的记录数（仅第一个有记录）
	unsigned long	LengthSize; 	//下属的标记记录数，若无则为0
	unsigned long	ArraySize;	//
	struct TLVEntity	*Sub_TLVEntity; 	//子嵌套TLV实体
	//struct TLVEntity		Sub_TLVEntity[20]; 	//子嵌套TLV实体
}TLVEntity_t;


void construct_tlv(unsigned char *in_buf, long in_len, struct TLVEntity *tlv, unsigned char value);
TLVEntity_t * search_tlv(unsigned short *tag_buf, long tag_len, struct TLVEntity *tlv);
struct TLVEntity * search_map_tlv(unsigned short tag, struct TLVEntity *tlv);
char init_gpo(unsigned char *cur_timebcd, unsigned long tranamount, unsigned char cappflag, unsigned char *out_buf, struct TLVEntity *tlv);

#endif