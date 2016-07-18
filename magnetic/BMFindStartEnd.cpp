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
$Date$			  2015-3-22-2000
------------------------------------------------------------------------
=====================================================================*/
#include <stdafx.h>
#include "BMFindStartEnd.h"
#include "JW_type.h"


int BMVarianceCheck(int M_V[], int start)
{
	int i = 0;
	int varianceV = 0;
	int averageV = 0;
	int M_Length = 400;
	int varThreshold = 1000000;

	for (i = start; i < start+M_Length; i++)
	{
		averageV = averageV + M_V[i];
	}
	averageV = averageV / M_Length;
	for (i = start; i < start+M_Length; i++)
	{
		varianceV += (averageV-M_V[i])*(averageV-M_V[i]);
	}
	varianceV = varianceV / M_Length;

	if (varianceV > varThreshold)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int BMFindStartEnd(Tdn_DataBuf *pTdnDataBuf, Result_RMB *CheckResult, int _RMBCheck_Result[])
{
	int* M_V = pTdnDataBuf->BM_V;
	int* M_W = pTdnDataBuf->BM_W;
	int length = pTdnDataBuf->BM_Count;
	int* _startW = &CheckResult->Moneystartw;
	int* _endW = &CheckResult->Moneyendw;

	int i = 0;
	int flag = 0;
	int LEVEL_H = 2800;
	int LEVEL_L = 400;
	int start[40] = {0};
	int startpoint = 0;
	int endpoint = length-1;
	int varianceV = 0;

	*_startW = 0;
	*_endW = 0;

	///////////////////// Check the Code ///////////////////////////
	for (i = 0; i < length-1; i ++)
	{
		if (M_W[i] == M_W[i+1])
		{
			flag++;
		}
		//if (flag > (length>>1))
		if (flag == (length - 1))
		{
			CheckResult->ERRtype = JW_OTHER_WHEEL_ERROR;
			_RMBCheck_Result[POSITION_CODE] = SIGNAL_ERROR;
			return RETURN_ERROR;
		}
	}
	flag = 0;
	///////////////////// Find start(Buff) and end in BM ///////////////////////////
	for (i = startpoint; i < length-1; i++)
	{
		if ((M_V[i] < LEVEL_H && M_V[i+1] >= LEVEL_H) || (M_V[i] > LEVEL_L && M_V[i+1] <= LEVEL_L))
		{
			if (flag > 39)
			{
				start[39] = i;
			}
			else
			{
				start[flag] = i;
			}		
			flag++;
		}
	}
	for (i = endpoint; i > startpoint; i--)
	{
		if (M_V[i] >= LEVEL_H || M_V[i] <= LEVEL_L)
		{
			endpoint = i;
			break;
		}
	}

	//////////////// Check whether the start(Buff) is the real BM start /////////////////
	flag = 0;
	for (i = 0; i < 40; i++)
	{
		if (BMVarianceCheck(M_V, start[i]))
		{
			startpoint = start[i];
			flag++;
			break;
		}
	}

	////////////// If TRUE return 1 else return 0 ////////////
	if (flag && endpoint - startpoint > 500)
	{
		*_startW = M_W[startpoint];
		*_endW = M_W[endpoint];
		return RETURN_OK;
	}
	else
	{
		CheckResult->ERRtype = JW_BM_HD_NOBM;
		_RMBCheck_Result[POSITION_BM] = SIGNAL_ERROR;
		_RMBCheck_Result[POSITION_HD] = SIGNAL_ERROR;
		return RETURN_ERROR;
	}
}

