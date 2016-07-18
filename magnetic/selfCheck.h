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

#ifndef selfCheck_H_
#define selfCheck_H_

#include "BMHD_AmountMatrixParameter.h"

int BM_codecheck(int BM_W[], int BM_Length, int* _selfCheck_Result);
int varianceCheck(int M_V[], int M_Length, selfCheck_Info _selfCheckInfo, int* _selfCheck_Result);
int selfCheck(Tdn_DataBuf* pTdnDataBuf, int _selfCheck_Result[]);

#endif