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
$Date$			  2015-1-13-1500
------------------------------------------------------------------------
=====================================================================*/
#include <stdafx.h>
#include <math.h>
#include "RMBresult_check.h"
#include "JW_type.h"

void WheelScalePor(int *pWheelData,int Count)
{
	int i,j,start,count,wheel,change;
	start = 0;
	wheel = pWheelData[0]*2;

	for(i=0;i<Count;i++)
	{
		pWheelData[i] *= 2;
		if(pWheelData[i]!=wheel)
		{
			count = i-start;
			change = start + (count>>1);
			for(j=change;j<i;j++)
			{
				pWheelData[j]++;
			}
			start = i;
			wheel = pWheelData[i];
		}
	}
}

void IR_Magnetic_WheelPor(Tdn_DataBuf *pTdnDataBuf)
{
	//WheelScalePor(pTdnDataBuf->IR1_W,pTdnDataBuf->IR1_Count);
	//WheelScalePor(pTdnDataBuf->IR2_W,pTdnDataBuf->IR2_Count);
	//WheelScalePor(pTdnDataBuf->IR3_W,pTdnDataBuf->IR3_Count);
	//WheelScalePor(pTdnDataBuf->IR4_W,pTdnDataBuf->IR4_Count);
	//WheelScalePor(pTdnDataBuf->IR5_W,pTdnDataBuf->IR5_Count);
	//WheelScalePor(pTdnDataBuf->IR6_W,pTdnDataBuf->IR6_Count);
	WheelScalePor(pTdnDataBuf->BM_W,pTdnDataBuf->BM_Count);
	WheelScalePor(pTdnDataBuf->LSM_W,pTdnDataBuf->LSM_Count);
	WheelScalePor(pTdnDataBuf->RSM_W,pTdnDataBuf->RSM_Count);
	WheelScalePor(pTdnDataBuf->LM_W,pTdnDataBuf->LM_Count);
	WheelScalePor(pTdnDataBuf->RM_W,pTdnDataBuf->RM_Count);
	WheelScalePor(pTdnDataBuf->HD_W,pTdnDataBuf->HD_Count);

}

void RMBresult_check(Tdn_DataBuf *pTdnDataBuf, int CheckType, int *_selfCheck_Result, int *_RMBCheck_Result, Result_RMB* CheckResult)
{
	//int _selfCheck_Result[SELF_CHECK_BUFF_SIZE];
	//int _RMBCheck_Result[RMB_CHECK_BUFF_SIZE];

	//初始化
	memset((char *)CheckResult, SIGNAL_NORMAL, sizeof(Result_RMB));
	if (_selfCheck_Result != NULL)
	{
		memset(_selfCheck_Result, SIGNAL_NORMAL, SELF_CHECK_BUFF_SIZE * sizeof(int));
	}
	else
	{
		ASSERT(CheckType != 0);
	}

	if (_RMBCheck_Result != NULL)
	{
		memset(_RMBCheck_Result, 0, RMB_CHECK_BUFF_SIZE * sizeof(int));
	}
	else
	{
		ASSERT(CheckType == 0);
	}

	//参数设置
	//////////////////////////////////////////////////////////////////
	// 此处代码仅供测试
	tag_BMHD_LRSM__Parameter testParam; 
	InitialParameter(&testParam);
	//////////////////////////////////////////////////////////////////

	//数据矫正
	IR_Magnetic_WheelPor(pTdnDataBuf);

	if (CheckType == 0)
	{
		//白纸检测
		selfCheck(pTdnDataBuf, _selfCheck_Result);
	}
	else
	{
		//钞票开始结束检测
		BMFindStartEnd(pTdnDataBuf, CheckResult, _RMBCheck_Result);
		if (CheckResult->ERRtype != 0)
		{
			return;
		}
		//大磁检测
		BMHD_check(pTdnDataBuf, CheckResult, _RMBCheck_Result);
		if (CheckResult->ERRtype != 0)
		{
			return;
		}
		//边磁中磁检测
		LRSM_check(pTdnDataBuf, CheckResult, _RMBCheck_Result);

	}
	return;
}

