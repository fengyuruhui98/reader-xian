#include "xa_ul_operation.h"

unsigned int bit2uint(unsigned char *pData, int bitOffset, int bitLength)
{
unsigned int i, currentData = 0;

	int byteCount = bitOffset%8>0&&bitOffset%8<bitLength?(bitLength+8-bitOffset%8)/8:bitLength/8;

	for(i = 0; i < byteCount+1; i++)
	{
		currentData += pData[bitOffset/8+i]<<((byteCount-i)*8);
	}
	currentData <<= ( (3 - byteCount)*8 + (bitOffset%8) );
	 return currentData>> (32-bitLength);
}

void uint2bit(unsigned int data, unsigned char *pByte, int bitOffset, int bitLength)
{
int i, byteCount;
unsigned char b;

	data <<= (32-bitLength-bitOffset%8);

	byteCount =bitOffset%8>0&&bitOffset%8<bitLength?(bitLength+8-bitOffset%8)/8:bitLength/8;


	for(i = 0; i < byteCount+1; i++)
	{
		b = (data>>(24-i*8))&0xFF;
		pByte[bitOffset/8+i]=b|pByte[bitOffset/8+i];
	}
	return ;
}

void bit2struct_StaticZone(unsigned char *pData, StaticZone* pStruct)
{
unsigned short bitAmount = 0;

	pStruct->Version = (char)bit2uint(pData, bitAmount, Len_ST[0]);
	
	bitAmount += Len_ST[0];
	pStruct->lifecycleCount =  (short)bit2uint(pData, bitAmount, Len_ST[1]);

	bitAmount += Len_ST[1];
	pStruct->keySetNumber =  (char)bit2uint(pData, bitAmount, Len_ST[2]);

	bitAmount += Len_ST[2];
	pStruct->cardBaseDataTime =  (short)bit2uint(pData, bitAmount, Len_ST[3]);

	bitAmount += Len_ST[3];
	pStruct->cardBatchNumber =  (short)bit2uint(pData, bitAmount, Len_ST[4]);

	bitAmount += Len_ST[4];
	pStruct->testMode =  (char)bit2uint(pData, bitAmount, Len_ST[5]);

	bitAmount += Len_ST[5];
	pStruct->productType =  (char)bit2uint(pData, bitAmount, Len_ST[6]);
	
	bitAmount += Len_ST[6];
	pStruct->passengerType =  (char)bit2uint(pData, bitAmount, Len_ST[7]);

	bitAmount += Len_ST[7];
	pStruct->productID =  (char)bit2uint(pData, bitAmount, Len_ST[8]);
	
	bitAmount += Len_ST[8];
	pStruct->purchaseValue =  (long)bit2uint(pData, bitAmount, Len_ST[9]);

	bitAmount += Len_ST[9];
	pStruct->rfu1 =  (long)bit2uint(pData, bitAmount, Len_ST[10]);
	
	bitAmount += Len_ST[10];
	pStruct->lavPaymentMethod =  (char)bit2uint(pData, bitAmount, Len_ST[11]);

	bitAmount += Len_ST[11];
	pStruct->rfu2 =  (char)bit2uint(pData, bitAmount, Len_ST[12]);
	
	bitAmount += Len_ST[12];
	pStruct->validityDuration = (char)bit2uint(pData, bitAmount, Len_ST[13]);

	bitAmount += Len_ST[13];
	pStruct->rfu3 =  (char)bit2uint(pData, bitAmount, Len_ST[14]);
	
	return ;
}

void struct_StaticZone2bit(StaticZone *pStruct, unsigned char *pData)
{
unsigned bitAmount = 0;

	uint2bit(pStruct->Version, pData, bitAmount, Len_ST[0]);

	bitAmount += Len_ST[0];
	uint2bit(pStruct->lifecycleCount, pData, bitAmount, Len_ST[1]);

	bitAmount += Len_ST[1];
	uint2bit(pStruct->keySetNumber, pData, bitAmount, Len_ST[2]);

	bitAmount += Len_ST[2];
	uint2bit(pStruct->cardBaseDataTime, pData, bitAmount, Len_ST[3]);

	bitAmount += Len_ST[3];
	uint2bit(pStruct->cardBatchNumber, pData, bitAmount, Len_ST[4]);

	bitAmount += Len_ST[4];
	uint2bit(pStruct->testMode, pData, bitAmount, Len_ST[5]);

	bitAmount += Len_ST[5];
	uint2bit(pStruct->productType, pData, bitAmount, Len_ST[6]);

	bitAmount += Len_ST[6];
	uint2bit(pStruct->passengerType, pData, bitAmount, Len_ST[7]);

	bitAmount += Len_ST[7];
	uint2bit(pStruct->productID, pData, bitAmount, Len_ST[8]);

	bitAmount += Len_ST[8];
	uint2bit(pStruct->purchaseValue, pData, bitAmount, Len_ST[9]);

	bitAmount += Len_ST[9];
	uint2bit(pStruct->rfu1, pData, bitAmount, Len_ST[10]);

	bitAmount += Len_ST[10];
	uint2bit(pStruct->lavPaymentMethod, pData, bitAmount, Len_ST[11]);

	bitAmount += Len_ST[11];
	uint2bit(pStruct->rfu2, pData, bitAmount, Len_ST[12]);

	bitAmount += Len_ST[12];
	uint2bit(pStruct->validityDuration, pData, bitAmount, Len_ST[13]);

	bitAmount += Len_ST[13];
	uint2bit(pStruct->rfu3, pData, bitAmount, Len_ST[14]);

	return ;
}

