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
#ifndef BMHD_check_H_
#define BMHD_check_H_

#include "selfCheck.h"

int BM_prepro(int abm[], int abm_length, int BM_Length, int UpDown, int BM_Threshold, Result_RMB *CheckResult);
int BM_check(int AMOUNTMASK[], int DELTAMASK[], int ERRMASK, int valid_dianJugeShu, int WINDOWS, int dianJu[], int deltadianJu[], int* maskflag, int* deltamaskflag);
int BM_codecheck(int BM_W[], int BM_Length);
int BMHD_normalize(int a[], int w[], int length, int func[], int *func_length, int *label);
int BMHD_HDcheck(int HD_V[], int HD_Length, Result_RMB *CheckResult);
int BMHD_checkin(Tdn_DataBuf *pTdnDataBuf, Result_RMB *CheckResult);
int BMHD_sycjudge(Result_RMB *CheckResult);
void BMHD_check(Tdn_DataBuf *pTdnDataBuf, Result_RMB *CheckResult, int _selfCheck_Result[]);

#endif