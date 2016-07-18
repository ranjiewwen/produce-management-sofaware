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
  $Date$			2015-1-9-1100
 ------------------------------------------------------------------------
=====================================================================*/

#include <stdafx.h>
#include "BMHD_check.h"
#include "BMHD_AmountMatrixParameter.h"
#include <math.h>
#include "JW_type.h"

extern int abm[WAVELENGTH];
extern int weiZhi[BMHDSPACE];
extern int dianJu[BMHDSPACE];
extern int deltadianJu[BMHDSPACE];

extern int standX[WAVELENGTH];
extern int idl[2048];

extern int PosOfMax[BMHDSPACE];            //极大值点位置
extern int PosOfCount[BMHDSPACE];
extern int PeakValue[BMHDSPACE];

extern int HUNDRED[6];
extern int DEL_HUNDRED[5];
extern int ERRBIG;

//大磁间距计算
int BM_prepro(int abm[], int abm_length, int BM_Length, int UpDown, int BM_Threshold, Result_RMB *CheckResult)
{
	int point = 0;
	int i = 0;
	int dianShu = 0;
	int geShu = 0;
	int dianJugeShu = 0;
	int deltadianJugeShu = 0;
	int jump = 0;

	CheckResult->BMHD_Info.valid_dianJugeShu = 0;

	///////////////////////////// 找超过阈值的点的个数及位置 /////////////////////////////////////////////

	for (dianShu = 0 ; dianShu < abm_length-1; dianShu++)
	{
		if (abm[dianShu] < BM_Threshold && abm[dianShu+1] >= BM_Threshold)
		{

			weiZhi[geShu] = dianShu;
			geShu++;
		}

		if (abm[dianShu] >= BM_Threshold && abm[dianShu+1] < BM_Threshold)
		{

			weiZhi[geShu] = dianShu+1;
			geShu++;
		}

		if (geShu > 57 && BM_Length < 3000)
		{
			CheckResult->Amount = 0;//*ERRtype = JW_BM_HD_MISALL;粘连币
			return -1;
		}
	}

	////////////////////////////////// 根据间距数量分类识别 /////////////////////////////////////////////

	/////////////// 计算出间距 //////////////////

	for (point = 0; point <= geShu-2; point++)																	//曾博间距算法 Gelivable~
	{
		for (i = weiZhi[point]; i <= weiZhi[point+1]; i++)
		{
			if (UpDown == 1)
			{
				if (abm[i] >= BM_Threshold)
				{
					jump++;
				}
			}
			else
			{
				if (abm[i] <= BM_Threshold)
				{
					jump++;
				}
			}		
		}

		if (jump > 0)
		{
			weiZhi[dianJugeShu] = (weiZhi[point] + weiZhi[point+1])/2;											//取峰值中间点
			dianJugeShu++;
		}

		jump = 0;
	}		

	for (point = 0; point < dianJugeShu - 1; point++)
	{
		if (weiZhi[point+1] - weiZhi[point] >= 10)
		{
			dianJu[CheckResult->BMHD_Info.valid_dianJugeShu] = weiZhi[point+1] - weiZhi[point];					//计算间距，滤噪
			CheckResult->BMHD_Info.valid_dianJugeShu++;
		}
	}

	for (point = 0; point < CheckResult->BMHD_Info.valid_dianJugeShu-1; point++)
	{
		deltadianJu[deltadianJugeShu] = abs(dianJu[point+1] - dianJu[point]);								//计算delta
		deltadianJugeShu++;
	}

	return 0;
}

//大磁面额识别
int BM_check(int AMOUNTMASK[], int DELTAMASK[], int ERRMASK, int valid_dianJugeShu, int WINDOWS, int dianJu[], int deltadianJu[], int* maskflag, int* deltamaskflag)
{
	int point = 0;
	int i = 0;
	int position = 0;
	int maskfrontflag = 0;
	int maskbackflag = 0;
	int deltamaskfrontflag = 0;
	int deltamaskbackflag = 0;
	int deltajianJugeShu = valid_dianJugeShu - 1;
	int DELTAWINDOWS = WINDOWS - 1;
	int maskflagplus = 0;

	*deltamaskflag = 0;
	*maskflag = 0;

	//////////////////////////// 间距差值 ///////////////////////////////////////
	for (point = 0; point <= deltajianJugeShu-DELTAWINDOWS;point++)
	{			
		for (i = point; i < point+DELTAWINDOWS; i++)		//i 为 poswindows
		{
			if (abs(deltadianJu[i] - DELTAMASK[i-point]) <= ERRMASK)//ERRMASK[i-point]
			{
				position++;
			}
			else
			{
				break;
			}
			if (position == DELTAWINDOWS)
			{
				deltamaskfrontflag++;			
			}
		}
		position = 0;		

		for (i = point; i < point+DELTAWINDOWS; i++)		//i 为 poswindows
		{
			if (abs(deltadianJu[i] - DELTAMASK[DELTAWINDOWS-(i-point)-1]) <= ERRMASK)
			{
				position++;
			}
			else
			{
				break;
			}
			if (position == DELTAWINDOWS)
			{
				deltamaskbackflag++;
			}
		}
		position = 0;
		
	}

	*deltamaskflag = deltamaskfrontflag + deltamaskbackflag;

	if (deltamaskfrontflag + deltamaskbackflag > BMHD_LRSMParameter._BMHDParameter._BMdeltamaskThre)
	{
		return AMOUNTCOMFIRM;
	}

	//////////////////////////// 间距值 ///////////////////////////////////////
	for (point = 0; point <= valid_dianJugeShu-WINDOWS;point++)
	{			
		for (i = point; i < point+WINDOWS; i++)		//i 为 poswindows
		{
			if (abs(dianJu[i] - AMOUNTMASK[i-point]) <= ERRMASK)
			{
				position++;
			}						
			if (position == WINDOWS-1)
			{
				maskfrontflag++;			
			}
		}
		position = 0;		

		for (i = point; i < point+WINDOWS; i++)		//i 为 poswindows
		{
			if (abs(dianJu[i] - AMOUNTMASK[WINDOWS-(i-point)-1]) <= ERRMASK)
			{
				position++;
			}						
			if (position == WINDOWS-1)
			{
				maskbackflag++;
			}
		}
		position = 0;	
	}

	*maskflag = maskfrontflag + maskbackflag;

	if (maskfrontflag + maskbackflag > BMHD_LRSMParameter._BMHDParameter._BMmaskThre)
	{
		return AMOUNTCOMFIRM;
	}

	return 0;
}

//码盘异常检测
int BM_codecheck(int BM_W[], int BM_Length)
{
  return RETURN_OK;

	int i = 0;
	int sum = BMHD_LRSMParameter._CODEParameter._initialSum;
	int min = BMHD_LRSMParameter._CODEParameter._samplePointMin;
	int max = BMHD_LRSMParameter._CODEParameter._samplePointMax;

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
				return RETURN_ERROR;
			}
			sum = BMHD_LRSMParameter._CODEParameter._initialSum;
			continue;
		}
	}
	return RETURN_OK;
}

//码盘归一化
int BMHD_normalize(int a[], int w[], int length, int func[], int *func_length, int *label)
{
	int N = 8;
	int idl_length;
	int standX_length;
	int w_min;
	int w_max;
	int dcor;
	int u;
	int I;
	int i, j;
	int count;

	/*******添加******/
	int indexflag = 0;
	/*****************/

	*label = 1;

	/*********添加*********/
	w_min = w[0];
	w_max = w[length-1];
	/*********************/

	standX_length = (w_max-w_min-1)*N;
	*func_length = standX_length;

	if ((w_max - w_min > 512) || (w_max - w_min < 0))//码盘跳变 防止爆掉
	{
		*label = BM_CODE;
		return BM_CODE;
	}

	for (i = 0; i < length; i ++)
	{
		if (w[i] != w_min)
		{
			indexflag = i;
			break;
		}
	}

	for (i = w_min+1; i < w_max; i ++)
	{
		count = 0;

		if (w[indexflag] != i)
		{
			if (i == w_min+1)
			{
				for (j = (i-w_min-1)*N; j < (i-w_min)*N; j ++)
				{
					standX[j] = 0;
				}
			}
			else
			{
				for (j = (i-w_min-1)*N; j < (i-w_min)*N; j ++)
				{
					standX[j] = standX[(i-w_min-1)*N-1];
				}
			}
			continue;
		}

		for (j = indexflag; j < length; j ++)
		{
			if (w[j] == i)
			{
				idl[count] = j;
				count ++;
				if (count > 2047)
				{
					count = 2047;
					break;
				}
			}
			else
			{
				indexflag = j;
				break;
			}
		}

		if (count == 1)
		{
			for (j = (i-w_min-1)*N; j < (i-w_min)*N; j ++)
			{
				standX[j] = a[idl[0]];
			}
			continue;
		}

		standX[(i-w_min-1)*N] = a[idl[0]];
		standX[(i-w_min)*N-1] = a[idl[count-1]];

		for (j = 1; j < N-1; j ++)
		{
			dcor = 16384+((18725*j*(count-1))>>3);
			I = dcor>>14;
			u = dcor - (I<<14);
			standX[(i-w_min-1)*N+j] = ((16384-u)*a[idl[I-1]]+u*a[idl[I]])>>14;
		}
	}

	if (standX_length < 512)
	{
		*label = BM_NONE;
		for (i = 0; i < *func_length; i ++)
		{
			func[i] = standX[i];
		}
		return BM_NONE;
	}

	for (i = 0; i < *func_length; i ++)
	{
		func[i] = standX[i];
	}

	return 0;
}

//HD信号检测
int BMHD_HDcheck(int HD_V[], int HD_Length, Result_RMB *CheckResult)
{
	int i = 0;
	int average = 0;
	int point = 0;
	int peakNum = 0;
	int j,k,pos1,pos2,temp;
	int level = 0;
	int newLevel = 0;
	int feature = 0;
	int featureValue = 0;
	int featureNum = 0;
	int flag50 = 0;

	int HDdeltaPeak = BMHD_LRSMParameter._BMHDParameter._HDdeltaPeak;//HD峰值差
	int HDdeltaPosMin = BMHD_LRSMParameter._BMHDParameter._HDdeltaPosMin;//HD间距最小值
	int HDdeltaPosMax = BMHD_LRSMParameter._BMHDParameter._HDdeltaPosMax;//HD间距最大值

	memset(PosOfMax,0,BMHDSPACE*sizeof(int));
	memset(PeakValue,0,BMHDSPACE*sizeof(int));
	memset(PosOfCount,0,BMHDSPACE*sizeof(int));

	if (CheckResult->BMHD_Info.validend - CheckResult->BMHD_Info.validstart > 0)
	{
		for (i = CheckResult->BMHD_Info.validstart; i < CheckResult->BMHD_Info.validend; i++)
		{
			average += HD_V[i];
		}
		average = average/(CheckResult->BMHD_Info.validend - CheckResult->BMHD_Info.validstart);
	}
	else
	{
		average = DEFAULT_LEVELV;
	}

	level = average + 500;

	for(i = 5+CheckResult->BMHD_Info.validstart; i < CheckResult->BMHD_Info.validend-5; i++)
	{
		if((HD_V[i-5] <= HD_V[i]) && (HD_V[i-4] <= HD_V[i]) && (HD_V[i-3] <= HD_V[i]) && (HD_V[i-2] <= HD_V[i]) && (HD_V[i-1] <= HD_V[i]) 
			&& (HD_V[i] >= HD_V[i+1]) && (HD_V[i] >= HD_V[i+2]) && (HD_V[i] >= HD_V[i+3]) && (HD_V[i] >= HD_V[i+4]) && (HD_V[i] >= HD_V[i+5])
			&& (HD_V[i] > level && HD_V[i] <= 5000))
		{
			if (peakNum == 0 || (HD_V[i] != PeakValue[peakNum-1]) || (i - PosOfMax[peakNum-1] > 10))
			{
				PeakValue[peakNum] = HD_V[i];
				peakNum++;
			}		
		}
	}

	///////////////////// 对计算出的峰值排序 //////////////////////	
	for (pos1 = 0; pos1 < peakNum-1; pos1++)
	{
		k = pos1;												/*给记号赋值*/
		for(pos2 = pos1+1; pos2 < peakNum; pos2++)
		{
			if(PeakValue[k] > PeakValue[pos2])
			{
				k = pos2;										/*是k总是指向最小元素*/
			}
		}

		if (pos1 != k)
		{														/*当k!=pos1时才交换，否则a[i]即为最小*/
			temp=PeakValue[pos1];
			PeakValue[pos1]=PeakValue[k];
			PeakValue[k]=temp;
		}
	}
	newLevel = (PeakValue[peakNum/4]+average)/2;//前四分之一的一半作为新阈值
	if (newLevel < level)
	{
		newLevel = level;
	}

	peakNum = 0;
	memset(PeakValue,0,BMHDSPACE*sizeof(int));
	for(i = 5+CheckResult->BMHD_Info.validstart; i < CheckResult->BMHD_Info.validend-5; i++)
	{
		if((HD_V[i-5] <= HD_V[i]) && (HD_V[i-4] <= HD_V[i]) && (HD_V[i-3] <= HD_V[i]) && (HD_V[i-2] <= HD_V[i]) && (HD_V[i-1] <= HD_V[i]) 
			&& (HD_V[i] >= HD_V[i+1]) && (HD_V[i] >= HD_V[i+2]) && (HD_V[i] >= HD_V[i+3]) && (HD_V[i] >= HD_V[i+4]) && (HD_V[i] >= HD_V[i+5])
			&& (HD_V[i] > newLevel && HD_V[i] <= 5000))
		{
			if (peakNum == 0 || (HD_V[i] != PeakValue[peakNum-1]) || (i - PosOfMax[peakNum-1] > 10))
			{
				PosOfMax[peakNum] = i;                                    //存储极大值点的位置
				PeakValue[peakNum] = HD_V[i];
				peakNum++;
			}		
		}
	}

	//100元特征检测
	for (i = 0; i < peakNum-4; i++)//10110
	{
		if((PeakValue[i]-PeakValue[i+1]>HDdeltaPeak) && (PeakValue[i]-PeakValue[i+4]>HDdeltaPeak) 
			&& (PeakValue[i+2]-PeakValue[i+1]>HDdeltaPeak) && (PeakValue[i+2]-PeakValue[i+4]>HDdeltaPeak)
			&& (PeakValue[i+3]-PeakValue[i+1]>HDdeltaPeak) && (PeakValue[i+3]-PeakValue[i+4]>HDdeltaPeak)
			&& (PosOfMax[i+1] - PosOfMax[i] > HDdeltaPosMin) && (PosOfMax[i+1] - PosOfMax[i] < HDdeltaPosMax)
			&& (PosOfMax[i+2] - PosOfMax[i+1] > HDdeltaPosMin) && (PosOfMax[i+2] - PosOfMax[i+1] < HDdeltaPosMax)
			&& (PosOfMax[i+3] - PosOfMax[i+2] > HDdeltaPosMin) && (PosOfMax[i+3] - PosOfMax[i+2] < HDdeltaPosMax) 
			&& (PosOfMax[i+4] - PosOfMax[i+3] > HDdeltaPosMin) && (PosOfMax[i+4] - PosOfMax[i+3] < HDdeltaPosMax))
		{
			return 100;
		}
	}
	for (i = 0; i < peakNum-4; i++)//11010
	{
		if((PeakValue[i]-PeakValue[i+2]>HDdeltaPeak) && (PeakValue[i]-PeakValue[i+4]>HDdeltaPeak) 
			&& (PeakValue[i+1]-PeakValue[i+2]>HDdeltaPeak) && (PeakValue[i+1]-PeakValue[i+4]>HDdeltaPeak)
			&& (PeakValue[i+3]-PeakValue[i+2]>HDdeltaPeak) && (PeakValue[i+3]-PeakValue[i+4]>HDdeltaPeak)
			&& (PosOfMax[i+1] - PosOfMax[i] > HDdeltaPosMin) && (PosOfMax[i+1] - PosOfMax[i] < HDdeltaPosMax)
			&& (PosOfMax[i+2] - PosOfMax[i+1] > HDdeltaPosMin) && (PosOfMax[i+2] - PosOfMax[i+1] < HDdeltaPosMax)
			&& (PosOfMax[i+3] - PosOfMax[i+2] > HDdeltaPosMin) && (PosOfMax[i+3] - PosOfMax[i+2] < HDdeltaPosMax) 
			&& (PosOfMax[i+4] - PosOfMax[i+3] > HDdeltaPosMin) && (PosOfMax[i+4] - PosOfMax[i+3] < HDdeltaPosMax))
		{
			return 100;
		}
	}

	return 0;
}

//大磁检测
int BMHD_checkin(Tdn_DataBuf *pTdnDataBuf, Result_RMB *CheckResult)
{
	int i;	
	int bmlabel = 0;
	int abm_length = 0;
	int point = 0;
	int BMLEVELLINE = 0;
	int PreResult = 0;
	int sum = 0;
	int dianJuMax = 0;
	int dianJuMin = 0;
	int fangChaMin = 0;
	int WINDOWS = 6;
	int sumWindows = 0;
	int averageWindows = 0;
	
	int maskflag = 0;
	int deltamaskflag = 0;
	int answer = 0;

	int* BM_V = pTdnDataBuf->BM_V;
	int* BM_W = pTdnDataBuf->BM_W;
	int* HD_V = pTdnDataBuf->HD_V;
	int* HD_W = pTdnDataBuf->HD_W;
	int BM_Length = pTdnDataBuf->BM_Count;
	int HD_Length = pTdnDataBuf->HD_Count;
	int StartEndOffest = BMHD_LRSMParameter._BMHDParameter._StartEndOffset;

	CheckResult->BMHD_Info.validstart = -1;
	CheckResult->BMHD_Info.validend = -1;
	CheckResult->BMHD_Info.validstart_BM = -1;
	CheckResult->BMHD_Info.validend_BM = -1;
	CheckResult->BMHD_Info.valid_dianJugeShu = 0;
	CheckResult->BMHD_Info.average = -1;
	CheckResult->BMHD_Info.result = -1;

	memset(weiZhi,0,BMHDSPACE*sizeof(int));
	memset(dianJu,0,BMHDSPACE*sizeof(int));
	memset(deltadianJu,0,BMHDSPACE*sizeof(int));
	
	//////////////////////////// 调整范围 //////////////////////////////////////
	if (CheckResult->Moneystartw - StartEndOffest >= BM_W[0])
	{
		CheckResult->Moneystartw = CheckResult->Moneystartw - StartEndOffest;
	}
	if (CheckResult->Moneyendw + StartEndOffest <= BM_W[BM_Length-1])
	{
		CheckResult->Moneyendw = CheckResult->Moneyendw + StartEndOffest;
	}
	//////////////////////////// 调整范围 //////////////////////////////////////

	for (i = 0; i < HD_Length; i++)
	{
		if (HD_W[i] >= CheckResult->Moneystartw)
		{
			CheckResult->BMHD_Info.validstart = i;
			break;
		}
	}
	for (i = HD_Length-1; i >= 0; i--)
	{
		if (HD_W[i] <= CheckResult->Moneyendw)
		{
			CheckResult->BMHD_Info.validend = i;
			break;
		}
	}

	for (i = 0; i < BM_Length; i++)
	{
		if (BM_W[i] >= CheckResult->Moneystartw)
		{
			CheckResult->BMHD_Info.validstart_BM = i;
			break;
		}
	}
	for (i = BM_Length-1; i >= 0; i--)
	{
		if (BM_W[i] <= CheckResult->Moneyendw)
		{
			CheckResult->BMHD_Info.validend_BM = i;
			break;
		}
	}

	////////////////////////////////////// 归一化 ///////////////////////////////////////////////////////
	BMHD_normalize(&BM_V[CheckResult->BMHD_Info.validstart_BM], &BM_W[CheckResult->BMHD_Info.validstart_BM], CheckResult->BMHD_Info.validend_BM-CheckResult->BMHD_Info.validstart_BM+1, abm, &abm_length, &bmlabel);
	if (bmlabel == BM_NONE)
	{
		CheckResult->Amount = BM_NONE;
		return RETURN_ERROR;//*ERRtype = JW_BM_HD_NOBM;
	}
	else if (bmlabel == BM_CODE)
	{
		CheckResult->Amount = BM_CODE;
		return RETURN_ERROR;
	}
	else if (BM_codecheck(BM_W, BM_Length) != RETURN_OK)
	{
		CheckResult->Amount = BM_CODE;
		return RETURN_ERROR;
	}
	
	if (abm_length > 0)
	{
		for (i = 0; i < abm_length; i++)
		{
			BMLEVELLINE += abm[i];
		}
		BMLEVELLINE = BMLEVELLINE/abm_length;
	}
	else
	{
		BMLEVELLINE = DEFAULT_LEVELV;
	}

	PreResult = BM_prepro(abm, abm_length, BM_Length, 1, BMLEVELLINE + BMOFFEST_H, CheckResult);
	if (PreResult != RETURN_OK)
	{
		CheckResult->Amount = BM_UNKNOW;
		return RETURN_ERROR;
	}

	if (CheckResult->BMHD_Info.valid_dianJugeShu >= BMHD_LRSMParameter._BMHDParameter._BMnum_floor && CheckResult->BMHD_Info.valid_dianJugeShu <= BMHD_LRSMParameter._BMHDParameter._BMnum_ceil)
	{
		dianJuMax = dianJu[0];
		dianJuMin = dianJu[0];
		for (point = 0; point <= CheckResult->BMHD_Info.valid_dianJugeShu-1; point++)
		{
			sum = sum + dianJu[point];
			if (dianJu[point] > dianJu[0])
			{
				dianJuMax = dianJu[point];
			}
			if (dianJu[point] < dianJu[0])
			{
				dianJuMin = dianJu[point];
			}
		}
		CheckResult->BMHD_Info.average = (sum-dianJuMax-dianJuMin)/(CheckResult->BMHD_Info.valid_dianJugeShu-2);//求间距平均值

		///////////////////////////// 正式识别 ////////////////////////////////////////

		answer = BM_check(HUNDRED, DEL_HUNDRED, ERRBIG, CheckResult->BMHD_Info.valid_dianJugeShu, WINDOWS, dianJu, deltadianJu, &maskflag, &deltamaskflag);
		if (answer == AMOUNTCOMFIRM && CheckResult->BMHD_Info.average > 25/* && BMHD_HDcheck(HD_V, HD_Length, CheckResult) == 100*/)
		{
			CheckResult->BMHD_Info.result = 100;
		}
    else
		{
			CheckResult->BMHD_Info.result = BM_UNKNOW;
		}
		/*
    else
		{
			PreResult = BM_prepro(abm, abm_length, BM_Length, 0, BMLEVELLINE + BMOFFEST_L, CheckResult);
			if (PreResult != RETURN_OK)
			{
				CheckResult->Amount = BM_UNKNOW;
				return RETURN_ERROR;
			}
			answer = BM_check(HUNDRED, DEL_HUNDRED, ERRBIG, CheckResult->BMHD_Info.valid_dianJugeShu, WINDOWS, dianJu, deltadianJu, &maskflag, &deltamaskflag);
			if (answer == AMOUNTCOMFIRM)
			{
				CheckResult->Amount = BM_INVERSE;
				return RETURN_ERROR;
			}
			else
			{
				CheckResult->BMHD_Info.result = BM_UNKNOW;
			}      
		}
    */
	}
	else if (CheckResult->BMHD_Info.valid_dianJugeShu > BMHD_LRSMParameter._BMHDParameter._BMnum_ceil)
	{
		CheckResult->BMHD_Info.result = BM_UNKNOW;
	}
	else
	{
		CheckResult->BMHD_Info.result = BM_NONE;
	}

	//综合
	CheckResult->Amount = CheckResult->BMHD_Info.result;
	if (CheckResult->Amount > 0)
	{
		return RETURN_OK;
	}
	else
	{
		return RETURN_ERROR;
	}
}

//大磁综合判断
int BMHD_sycjudge(Result_RMB *CheckResult, int _RMBCheck_Result[])
{
	if (CheckResult->Amount == BM_CODE)
	{
		CheckResult->ERRtype = JW_OTHER_WHEEL_ERROR;
		_RMBCheck_Result[POSITION_CODE] = SIGNAL_ERROR;
		return RETURN_ERROR;
	}
	else if (CheckResult->Amount == BM_NONE)
	{
		CheckResult->ERRtype = JW_BM_HD_NOBM;
		_RMBCheck_Result[POSITION_BM] = SIGNAL_ERROR;
		_RMBCheck_Result[POSITION_HD] = SIGNAL_ERROR;
		return RETURN_ERROR;
	}
	else if (CheckResult->Amount == BM_UNKNOW)
	{
		CheckResult->ERRtype = JW_BM_HD_MISALL;
		_RMBCheck_Result[POSITION_BM] = SIGNAL_ERROR;
		_RMBCheck_Result[POSITION_HD] = SIGNAL_ERROR;
		return RETURN_ERROR;
	}
	else if (CheckResult->Amount == BM_UNKNOW)
	{
		CheckResult->ERRtype = JW_BM_HD_INVERSE;
		_RMBCheck_Result[POSITION_BM] = SIGNAL_ERROR;
		_RMBCheck_Result[POSITION_HD] = SIGNAL_ERROR;
		return RETURN_ERROR;
	}
	else
	{
		return RETURN_OK;
	}
}

//大磁主函数
void BMHD_check(Tdn_DataBuf *pTdnDataBuf, Result_RMB *CheckResult, int _RMBCheck_Result[])
{
	CheckResult->Amount = 0;
	CheckResult->ERRtype = 0;
	
	BMHD_checkin(pTdnDataBuf, CheckResult);
	BMHD_sycjudge(CheckResult, _RMBCheck_Result);

}
