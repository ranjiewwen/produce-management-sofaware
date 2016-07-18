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
#ifndef JW_type_H_
#define JW_type_H_


/************ 安全线 ****************/
#define JW_BM_HD_ZONE			350		//安全线:大磁
#define JW_BM_HD_MISALL			351		//安全线:BMHD_Check 中大磁所有面额都不满足
#define JW_BM_HD_NOBM			352		//安全线:BMHD_Check 中大磁特征宽度不够（无大磁）
#define JW_BM_HD_INVERSE		359		//安全线:BMHD_Check 中大磁安装反
/************ 磁特征 ****************/
#define JW_M_FEATURE_ZONE		450		//磁特征:波形特征
#define JW_M_FEATURE_LCROSS		451		//磁特征:LRSM_Check中左边磁信号弱
#define JW_M_FEATURE_RCROSS		452		//磁特征:LRSM_Check中右边磁信号弱
#define JW_M_FEATURE_NOSM		453		//磁特征:LRSM_Check中左右边磁都宽度过小（严格无边磁）
#define JW_M_FEATURE_LMLESS		461		//磁特征:LRSM_Check中左中磁信号弱
#define JW_M_FEATURE_RMLESS		462		//磁特征:LRSM_Check中右中磁信号弱

/************ 其他 ****************/
#define JW_OTHER_WHEEL_ERROR	562

#endif
