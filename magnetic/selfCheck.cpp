/***********************************************************************\ 
*                                                                      *
*            Copyright (C) 2010 - 2015 by Wuhan University             *
*                      All Rights Reserved                             * 
*   Property of Wuhan University. Restricted rights to use, duplicate  * 
*   or disclose this code are granted through contract.                *
*                                                                      *
*   THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF *
*   ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED   *
*   TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A  *
*   PARTICULAR PURPOSE.                                                *
*                                                                      *
------------------------------------------------------------------------
$Id$                                                   
$Revision$		
$Author$          Liuyf                                         
$Date$			  2015-1-6-2000
------------------------------------------------------------------------
=====================================================================*/
#include <stdafx.h>
#include "selfCheck.h"

int BM_codecheck(int BM_W[], int BM_Length, int* _selfCheck_Result)
{
	int i = 0;
	int sum = BMHD_LRSMParameter._CODEParameter._initialSum;
	int min = BMHD_LRSMParameter._CODEParameter._samplePointMin;
	int max = BMHD_LRSMParameter._CODEParameter._samplePointMax;
	*_selfCheck_Result = 0;

	for (i = 0; i < BM_Length-1; i++)
	{
		if (BM_W[i] == BM_W[i+1])
		{
			sum++;
		}
		else
		{
			if ((sum > max || sum < min) && i > min)
			{
				*_selfCheck_Result = SIGNAL_ERROR;
				return RETURN_ERROR;
			}
			sum = 2;
			continue;
		}
		//if (sum > (BM_Length>>1))
		if (sum == (BM_Length - 1))
		{
			*_selfCheck_Result = SIGNAL_ERROR;
			return RETURN_ERROR;
		}
	}
	return RETURN_OK;
}

int varianceCheck(int M_V[], int M_Length, int* _selfCheck_Result)
{
	int i = 0;
	_int64 varianceV = 0;
	int averageV = 0;
	*_selfCheck_Result = 0;

	/***********计算均值**********/
	for (i = 0; i < M_Length; i++)
	{
		averageV = averageV + M_V[i];
	}
	averageV = averageV/M_Length;

	/***********计算方差**********/
	for (i = 0; i < M_Length; i++)
	{
		varianceV += (averageV-M_V[i])*(averageV-M_V[i]);
	}
	varianceV = varianceV/M_Length;

	if (varianceV < BMHD_LRSMParameter._selfCheckInfo._LOW_DV || varianceV > BMHD_LRSMParameter._selfCheckInfo._HIGH_DV 
		|| averageV < BMHD_LRSMParameter._selfCheckInfo._LOW_AV || averageV > BMHD_LRSMParameter._selfCheckInfo._HIGH_AV)
	{
		*_selfCheck_Result = SIGNAL_ERROR;
		return RETURN_ERROR;
	}
	return RETURN_OK;
}

int selfCheck(Tdn_DataBuf* pTdnDataBuf, int _selfCheck_Result[])
{
	int i = 0;

	//码盘有问题
	BM_codecheck(pTdnDataBuf->BM_W, pTdnDataBuf->BM_Count, &_selfCheck_Result[POSITION_CODE]);
	//BM有问题
	varianceCheck(pTdnDataBuf->BM_V, pTdnDataBuf->BM_Count, &_selfCheck_Result[POSITION_BM]);	
	//HD有问题
	varianceCheck(pTdnDataBuf->HD_V, pTdnDataBuf->HD_Count, &_selfCheck_Result[POSITION_HD]);
	//左边磁有问题
	varianceCheck(pTdnDataBuf->LSM_V, pTdnDataBuf->LSM_Count, &_selfCheck_Result[POSITION_LSM]);	
	//右边磁有问题
	varianceCheck(pTdnDataBuf->RSM_V, pTdnDataBuf->RSM_Count, &_selfCheck_Result[POSITION_RSM]);
	//左中磁有问题
	varianceCheck(pTdnDataBuf->LM_V, pTdnDataBuf->LM_Count, &_selfCheck_Result[POSITION_LM]);
	//右中磁有问题
	varianceCheck(pTdnDataBuf->RM_V, pTdnDataBuf->RM_Count, &_selfCheck_Result[POSITION_RM]);

	for (i = 0; i < SELF_CHECK_BUFF_SIZE; i++)
	{
		if (_selfCheck_Result[i] != 0)
		{
			return RETURN_ERROR;
		}
	}
	return RETURN_OK;
}