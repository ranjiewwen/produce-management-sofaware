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
$Date$			  2015-1-13-1100
------------------------------------------------------------------------
=====================================================================*/
#ifndef RMBresult_check_H_
#define RMBresult_check_H_

#include "BMHD_AmountMatrixParameter.h"
#include "selfCheck.h"
#include "BMFindStartEnd.h"
#include "BMHD_check.h"
#include "LRSM_check.h"

void WheelScalePor(int *pWheelData,int Count);
void IR_Magnetic_WheelPor(Tdn_DataBuf *pTdnDataBuf);

void RMBresult_check(Tdn_DataBuf *pTdnDataBuf, int CheckType, int *_selfCheck_Result/*[RMB_CHECK_BUFF_SIZE]*/, int *_RMBCheck_Result/*[RMB_CHECK_BUFF_SIZE]*/, Result_RMB* _CheckResult);

#endif