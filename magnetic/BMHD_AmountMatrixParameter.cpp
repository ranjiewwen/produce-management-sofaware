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
$Date$			  2015-1-9-1100
------------------------------------------------------------------------
=====================================================================*/
#include <stdafx.h>
#include "BMHD_AmountMatrixParameter.h"

int PosOfMax[BMHDSPACE];
int PosOfCount[BMHDSPACE];
int PeakValue[BMHDSPACE];

int weiZhi[BMHDSPACE];
int dianJu[BMHDSPACE];
int deltadianJu[BMHDSPACE];
int abm[WAVELENGTH];

int standX[WAVELENGTH];
int idl[2048];


Result_RMB CheckResult[200];
Tdn_DataBuf TdnDataBuf;

BMHD_LRSM_Parameter BMHD_LRSMParameter;

void InitialParameter(const BMHD_LRSM_Parameter * const _pParam)
{
	ASSERT(_pParam != NULL);

	//自检参数
	BMHD_LRSMParameter._selfCheckInfo._LOW_DV			= _pParam->_selfCheckInfo._LOW_DV;
	BMHD_LRSMParameter._selfCheckInfo._MID_DV			= _pParam->_selfCheckInfo._MID_DV;
	BMHD_LRSMParameter._selfCheckInfo._HIGH_DV			= _pParam->_selfCheckInfo._HIGH_DV;
	BMHD_LRSMParameter._selfCheckInfo._LOW_AV			= _pParam->_selfCheckInfo._LOW_AV;
	BMHD_LRSMParameter._selfCheckInfo._HIGH_AV			= _pParam->_selfCheckInfo._HIGH_AV;

	//码盘参数
	BMHD_LRSMParameter._CODEParameter._initialSum		= _pParam->_CODEParameter._initialSum;
	BMHD_LRSMParameter._CODEParameter._samplePointMin	= _pParam->_CODEParameter._samplePointMin;
	BMHD_LRSMParameter._CODEParameter._samplePointMax	= _pParam->_CODEParameter._samplePointMax;

	//大磁参数
	BMHD_LRSMParameter._BMHDParameter._StartEndOffset	= _pParam->_BMHDParameter._StartEndOffset;
	BMHD_LRSMParameter._BMHDParameter._BMnum_ceil		= _pParam->_BMHDParameter._BMnum_ceil;
	BMHD_LRSMParameter._BMHDParameter._BMnum_floor		= _pParam->_BMHDParameter._BMnum_floor;
	BMHD_LRSMParameter._BMHDParameter._BMdeltamaskThre	= _pParam->_BMHDParameter._BMdeltamaskThre;
	BMHD_LRSMParameter._BMHDParameter._BMmaskThre		= _pParam->_BMHDParameter._BMmaskThre;
	BMHD_LRSMParameter._BMHDParameter._HDdeltaPeak		= _pParam->_BMHDParameter._HDdeltaPeak;
	BMHD_LRSMParameter._BMHDParameter._HDdeltaPosMin	= _pParam->_BMHDParameter._HDdeltaPosMin;
	BMHD_LRSMParameter._BMHDParameter._HDdeltaPosMax	= _pParam->_BMHDParameter._HDdeltaPosMax;

	//边磁参数
	BMHD_LRSMParameter._LRSMParameter._SMOFFEST_H		= _pParam->_LRSMParameter._SMOFFEST_H;
	BMHD_LRSMParameter._LRSMParameter._SMOFFEST_L		= _pParam->_LRSMParameter._SMOFFEST_L;
	BMHD_LRSMParameter._LRSMParameter._SMnum_strong		= _pParam->_LRSMParameter._SMnum_strong;
	BMHD_LRSMParameter._LRSMParameter._SMnum_weak		= _pParam->_LRSMParameter._SMnum_weak;
	//中磁参数
	BMHD_LRSMParameter._LRSMParameter._MIDMOFFEST_H		= _pParam->_LRSMParameter._MIDMOFFEST_H;
	BMHD_LRSMParameter._LRSMParameter._MIDMOFFEST_L		= _pParam->_LRSMParameter._MIDMOFFEST_L;
	BMHD_LRSMParameter._LRSMParameter._deltaMidMthre	= _pParam->_LRSMParameter._deltaMidMthre;
	BMHD_LRSMParameter._LRSMParameter._MidMthre			= _pParam->_LRSMParameter._MidMthre;
	BMHD_LRSMParameter._LRSMParameter._MidMnum_weak		= _pParam->_LRSMParameter._MidMnum_weak;
}

/////////////////////////////// 最新面额识别参数 ////////////////////////////////////////

int HUNDRED[6]		= {140, 140, 140, 140, 140, 140};
int DEL_HUNDRED[5]	= {0,  0,  0,  0,  0};
int ERRBIG			= 16;


