#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "tlv.h"
#include "time_tools.h"

//void Construct(unsigned char* buffer, unsigned int bufferLength, TLVEntity* tlvEntity, 
//			unsigned int& entityLength, unsigned int status)
//{
//int currentTLVIndex = 0;
//int currentIndex = 0;
//int currentStatus = 'T'; //状态字符
//unsigned long valueSize = 0;
//
//	while(currentIndex < bufferLength)
//	{
//		switch(currentStatus)
//		{
//		case 'T':
//			valueSize = 0;
//			//判断是否单一结构
//			if((status == 1 && buffer[currentIndex] & 0x20) != 0x20)
//			{
//				tlvEntity[currentTLVIndex].Sub_TLVEntity = NULL; //单一结构时将子Tag置空
//				//判断是否多字节Tag
//				if((buffer[currentIndex] & 0x1f) == 0x1f)
//				{
//					int endTagIndex = currentIndex;
//					while((buffer[++endTagIndex] & 0x80) == 0x80); //判断第二个字节的最高位是否为1
//					int tagSize = endTagIndex - currentIndex + 1; //计算Tag包含多少字节
//					tlvEntity[currentTLVIndex].Tag = new unsigned char[tagSize];
//					memcpy(tlvEntity[currentTLVIndex].Tag, buffer + currentIndex, tagSize); 
//					tlvEntity[currentTLVIndex].Tag[tagSize] = 0;
//					tlvEntity[currentTLVIndex].TagSize = tagSize;
//					currentIndex += tagSize;
//				}
//				else
//				{
//					tlvEntity[currentTLVIndex].Tag = new unsigned char[1];
//					memcpy(tlvEntity[currentTLVIndex].Tag, buffer + currentIndex, 1);
//					tlvEntity[currentTLVIndex].Tag[1] = 0;
//					tlvEntity[currentTLVIndex].TagSize = 1;
//					currentIndex += 1;
//				}
//			}
//			else
//			{
//				//判断是否多字节Tag
//				if((buffer[currentIndex] & 0x1f) == 0x1f)
//				{
//					int endTagIndex = currentIndex;
//					while((buffer[++endTagIndex] & 0x80) == 0x80); //判断第二个字节的最高位是否为1
//					int tagSize = endTagIndex - currentIndex + 1; //计算Tag包含多少字节
//					tlvEntity[currentTLVIndex].Tag = new unsigned char[tagSize];
//					memcpy(tlvEntity[currentTLVIndex].Tag, buffer + currentIndex, tagSize); 
//					tlvEntity[currentTLVIndex].Tag[tagSize] = 0;
//					tlvEntity[currentTLVIndex].TagSize = tagSize;
//					currentIndex += tagSize;
//				}
//				else
//				{
//					tlvEntity[currentTLVIndex].Tag = new unsigned char[1];
//					memcpy(tlvEntity[currentTLVIndex].Tag, buffer + currentIndex, 1);
//					tlvEntity[currentTLVIndex].Tag[1] = 0;
//					tlvEntity[currentTLVIndex].TagSize = 1;
//					currentIndex += 1; 
//				}
//				//分析SubTag
//				int subLength = 0; 
//				unsigned char* temp;
//				if((buffer[currentIndex] & 0x80) == 0x80)
//				{
//					for (int index = 0; index < 2; index++)
//					{
//						subLength += buffer[currentIndex + 1 + index] << (index * 8); //计算Length域的长度
//					}
//					temp = new unsigned char[subLength];
//					memcpy(temp, buffer + currentIndex + 3, subLength);
//				}
//				else
//				{
//					subLength = buffer[currentIndex];
//					temp = new unsigned char[subLength];
//					memcpy(temp, buffer + currentIndex + 1, subLength);
//				}
//				temp[subLength] = 0;
//				//memcpy(temp, buffer + currentIndex + 1, subLength);
//				unsigned int oLength;
//				tlvEntity[currentTLVIndex].Sub_TLVEntity = new TLVEntity[1];
//				//Construct(temp, subLength, tlvEntity[currentTLVIndex].Sub_TLVEntity, oLength);
//			}
//			currentStatus = 'L';
//			break;
//		case 'L': 
//			//判断长度字节的最高位是否为1，如果为1，则该字节为长度扩展字节，由下一个字节开始决定长度
//			if((buffer[currentIndex] & 0x80) != 0x80)
//			{
//				tlvEntity[currentTLVIndex].Length = new unsigned char[1];
//				memcpy(tlvEntity[currentTLVIndex].Length, buffer + currentIndex, 1);
//				tlvEntity[currentTLVIndex].Length[1] = 0;
//				tlvEntity[currentTLVIndex].LengthSize = 1;
//				valueSize = tlvEntity[currentTLVIndex].Length[0];
//				currentIndex += 1;
//			}
//			else
//			{
//				//为1的情况
//				unsigned int lengthSize = buffer[currentIndex] & 0x7f;
//				currentIndex += 1; //从下一个字节开始算Length域
//				for (int index = 0; index < lengthSize; index++)
//				{
//					valueSize += buffer[currentIndex + index] << (index * 8); //计算Length域的长度
//				}
//				tlvEntity[currentTLVIndex].Length = new unsigned char[lengthSize];
//				memcpy(tlvEntity[currentTLVIndex].Length, buffer + currentIndex, lengthSize);
//				tlvEntity[currentTLVIndex].Length[lengthSize] = 0;
//				tlvEntity[currentTLVIndex].LengthSize = lengthSize;
//				currentIndex += lengthSize;
//			}
//			currentStatus = 'V';
//			break;
//		case 'V':
//			tlvEntity[currentTLVIndex].Value = new unsigned char[valueSize];
//			memcpy(tlvEntity[currentTLVIndex].Value, buffer + currentIndex, valueSize);
//			tlvEntity[currentTLVIndex].Value[valueSize] = 0;
//			currentIndex += valueSize;
//			//进入下一个TLV构造循环
//			currentTLVIndex += 1;
//			currentStatus = 'T';
//			break;
//		default:
//			return;
//		}
//	}
//	entityLength = currentTLVIndex;
//}
//
//
//
////// 解析TLV
//void Parse(TLVEntity* tlvEntity, unsigned int entityLength, unsigned char* buffer, unsigned long & bufferLength)
//{
//int currentIndex = 0;
//int currentTLVIndex = 0;
//unsigned long valueSize = 0;
//
//	while(currentTLVIndex < entityLength)
//	{
//		valueSize = 0;
//		TLVEntity entity = tlvEntity[currentTLVIndex];
//		memcpy(buffer + currentIndex, entity.Tag, entity.TagSize); //解析Tag
//		currentIndex += entity.TagSize;
//		for (int index = 0; index < entity.LengthSize; index++)
//		{
//			valueSize += entity.Length[index] << (index * 8); //计算Length域的长度
//		}
//		if(valueSize > 127)
//		{
//			buffer[currentIndex] = 0x80 | entity.LengthSize;
//			currentIndex += 1;
//		}
//		memcpy(buffer + currentIndex, entity.Length, entity.LengthSize); //解析Length
//		currentIndex += entity.LengthSize;
//		//判断是否包含子嵌套TLV
//		if(entity.Sub_TLVEntity == NULL)
//		{
//			memcpy(buffer + currentIndex, entity.Value, valueSize); //解析Value
//			currentIndex += valueSize;
//		}
//		else
//		{
//			unsigned long oLength;
//			Parse(entity.Sub_TLVEntity, 1, buffer + currentIndex, oLength); //解析子嵌套TLV
//			currentIndex += oLength;
//		}
//		currentTLVIndex++;
//	}
//	buffer[currentIndex] = 0;
//	bufferLength = currentIndex;
//}



void construct_tlv(unsigned char *in_buf, long in_len, struct TLVEntity *tlv, unsigned char value)
{
long curIndex, curTLVArray, curTagIndex;

	curIndex = curTLVArray = curTagIndex = 0;

	while(curIndex < in_len)
	{
		//
		//tlv->UpArray += 1;
		//TAG
		curTagIndex = curIndex;
		if((in_buf[curIndex] & 0x1F) == 0x1F)
		{//double bytes TAG
			tlv[curTLVArray].Tag = (in_buf[curIndex] << 8) + in_buf[curIndex + 1];
			curIndex += 1;
		}else
			tlv[curTLVArray].Tag = in_buf[curIndex];
		curIndex += 1;
		//LENGTH
		if((in_buf[curIndex] & 0x80) == 0x80)
		{//next bytes is the real length but one bytes enough for PBOC
			curIndex += 1;
			tlv[curTLVArray].Length = in_buf[curIndex];
		}else
			tlv[curTLVArray].Length = in_buf[curIndex];
		curIndex += 1;
		//VALUE
		tlv[curTLVArray].Value = &in_buf[curIndex];
		//MUST be the TAG
		if(((in_buf[curTagIndex] & 0x20) == 0x20) || (tlv[curTLVArray].Tag == 0x9f38))
		{//constructed data object
			if(tlv[curTLVArray].Sub_TLVEntity == NULL)
			{//
				tlv[curTLVArray].Sub_TLVEntity = (struct TLVEntity *)malloc(sizeof(struct TLVEntity) * 20);
				memset(tlv[curTLVArray].Sub_TLVEntity, 0x00, sizeof(struct TLVEntity) * 20);
			}
			else
			{
				//tlv[curTLVArray].Sub_TLVEntity = (struct TLVEntity *)realloc(tlv[curTLVArray].Sub_TLVEntity, sizeof(struct TLVEntity) * (tlv[0].UpArray + 1));
				//tlv[curTLVArray].Sub_TLVEntity[tlv[0].ArraySize].Sub_TLVEntity = NULL;
			}
			tlv[curTLVArray].Sub_TLVEntity[0].UpArray += 1;
			if(tlv[curTLVArray].Tag == 0x9f38)
				construct_tlv(tlv[curTLVArray].Value, tlv[curTLVArray].Length, tlv[curTLVArray].Sub_TLVEntity, 0);
			else
				construct_tlv(tlv[curTLVArray].Value, tlv[curTLVArray].Length, tlv[curTLVArray].Sub_TLVEntity, 1);
			tlv[curTLVArray].ArraySize = tlv[curTLVArray].Sub_TLVEntity[0].UpArray;

			curIndex += tlv[curTLVArray].Length;
			curTLVArray += 1;
			if(curIndex < in_len)
			{
				tlv[0].UpArray += 1;
				//tlv = (struct TLVEntity *)realloc(tlv, sizeof(struct TLVEntity) * (tlv[0].UpArray));
				//memset(&tlv[curTLVArray], 0x00, sizeof(struct TLVEntity));
			}
		}else
		{//primitive data object
			tlv[curTLVArray].Sub_TLVEntity = NULL;
			//
			//tlv[curTLVArray].ArraySize += 1;
			if(value)
				curIndex += tlv[curTLVArray].Length;
			curTagIndex = curIndex;
			curTLVArray += 1;
			//have multi TLV for the same level
			if(curIndex < in_len)
			{
				tlv[0].UpArray += 1;
				//tlv = (struct TLVEntity *)realloc(tlv, sizeof(struct TLVEntity) * (tlv[0].UpArray));
				//memset(&tlv[curTLVArray], 0x00, sizeof(struct TLVEntity));
			}
		}
	}
	return ;
}


struct TLVEntity * search_tlv(unsigned short *tag_buf, long tag_len, struct TLVEntity *tlv)
{
long curIndex, tagIndex;
struct TLVEntity *temptlv;

	if(tag_len <= 0)
		return NULL;
	tagIndex = 0;
	for(curIndex = 0; curIndex < tlv->UpArray; curIndex++)
	{
		//TAG
		if(tlv[curIndex].Tag == tag_buf[tagIndex])
		{
			tagIndex += 1;
			if(tagIndex == tag_len)
				return &tlv[curIndex];
			temptlv = search_tlv(&tag_buf[tagIndex], tag_len - tagIndex, tlv[curIndex].Sub_TLVEntity);
			return temptlv;
		}
	}
	return NULL;
}

struct TLVEntity * search_map_tlv(unsigned short tag, struct TLVEntity *tlv)
{
long curIndex, tagIndex;
struct TLVEntity *temptlv;

	temptlv = NULL;
	//
	if(tag == tlv->Tag)
		return tlv;
	
	for(curIndex = 0; curIndex < tlv->ArraySize; curIndex++)
	{
		//TAG
		if(tlv->Sub_TLVEntity[curIndex].Tag == tag)
		{
			temptlv = &(tlv->Sub_TLVEntity[curIndex]);
			return temptlv;
		}
		if(tlv->Sub_TLVEntity[curIndex].Sub_TLVEntity != NULL)
		{
			temptlv = search_map_tlv(tag, &(tlv->Sub_TLVEntity[curIndex]));
			if(temptlv != NULL)
				return temptlv;
		}
	}
	return temptlv;
}



char init_gpo(unsigned char *cur_timebcd, unsigned long tranamount, unsigned char cappflag, unsigned char *out_buf, struct TLVEntity *tlv)
{
unsigned char curIndex, tagIndex;

	//9f6604 9f0206 9f0306 9f1a02 9505 5f2a02 9a03 9c01 9f3704 df6001 
	tagIndex = 0;
	for(curIndex = 0; curIndex < tlv->ArraySize; curIndex++)
	{
		switch(tlv->Sub_TLVEntity[curIndex].Tag)
		{
		case 0x9f66:	//终端交易属性
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			memcpy(&out_buf[tagIndex], "\x28\x00\x00\x80", 4);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0x9f02:	//授权金额
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			out_buf[tagIndex + 1] = bin2bcd((tranamount / 100000000) % 100);
			out_buf[tagIndex + 2] = bin2bcd((tranamount / 1000000) % 100);
			out_buf[tagIndex + 3] = bin2bcd((tranamount / 10000) % 100);
			out_buf[tagIndex + 4] = bin2bcd((tranamount / 100) % 100);
			out_buf[tagIndex + 5] = bin2bcd((tranamount / 1) % 100);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0x9f03:	//其它金额
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0x9f1a:	//终端国家代码
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			memcpy(&out_buf[tagIndex], "\x01\x56", 2);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0x95:		//终端验证结果
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			memcpy(&out_buf[tagIndex], "\x00\x00\x00\x00\x00", 5);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0x5f2a:	//交易货币代码
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			memcpy(&out_buf[tagIndex], "\x01\x56", 2);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0x9a:		//交易日期
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			memcpy(&out_buf[tagIndex], &cur_timebcd[1], 3);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0x9c:		//交易类型
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			memcpy(&out_buf[tagIndex], "\x00", 1);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0x9f37:	//终端随机数
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			memcpy(&out_buf[tagIndex], "\x11\x22\x33\x44", 4);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		case 0xdf60:	//CAPP交易指示位
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			memcpy(&out_buf[tagIndex], &cappflag, 1);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		default:		//
			memset(&out_buf[tagIndex], 0x00, tlv->Sub_TLVEntity[curIndex].Length);
			tagIndex += tlv->Sub_TLVEntity[curIndex].Length;
			break;
		}
	}
	return tagIndex;
}

void free_tlv(struct TLVEntity *tlv)
{
long curIndex, tagIndex;

	tagIndex = 0;
	for(curIndex = 0; curIndex < tlv->ArraySize; curIndex++)
	{
		//TAG
		if(tlv->Sub_TLVEntity == NULL)
		{
			continue;
		}
		free_tlv(&(tlv->Sub_TLVEntity[curIndex]));
	}
	if(tlv->Sub_TLVEntity != NULL)
		free(tlv->Sub_TLVEntity);
	tlv->Sub_TLVEntity = NULL;
	memset(tlv, 0x00, sizeof(TLVEntity_t));
}
