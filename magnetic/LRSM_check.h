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
#ifndef LRSM_check_H_
#define LRSM_check_H_

#include "BMHD_AmountMatrixParameter.h"

int SMCross(int SM_V[], int SM_W[], int startPosi, int endPosi, int averageV, int* SMthre);
int MagnetCheck(int SM_V[], int SM_W[], int SM_Length, Result_RMB* CheckResult, int* SMthre);
int MidMCheck(int M_V[], int M_W[], int SM_Length, Result_RMB *CheckResult);
void LRSM_check(Tdn_DataBuf *pTdnDataBuf, Result_RMB *CheckResult, int _RMBCheck_Result[]);

#endif