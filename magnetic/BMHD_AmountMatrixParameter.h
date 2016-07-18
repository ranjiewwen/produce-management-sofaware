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
#ifndef BMHD_AmountMatrixParameter_H_
#define BMHD_AmountMatrixParameter_H_

#define		WAVELENGTH					1024*12
#define		BMHDSPACE				64

#define		SIGNAL_NORMAL			0
#define		SIGNAL_ERROR			-1
#define		RETURN_OK				0
#define		RETURN_ERROR			-1

#define		POSITION_CODE			0
#define		POSITION_BM				1
#define		POSITION_HD				2
#define		POSITION_LSM			3
#define		POSITION_RSM			4
#define		POSITION_LM				5
#define		POSITION_RM				6

#define		BM_UNKNOW				0
#define		BM_NONE					-1
#define		BM_CODE					-2
#define		BM_INVERSE				-3

#define		SM_OK					0
#define		SM_NONE					-1
#define		SM_SMALL				-2

#define		MIDM_OK					0
#define		MIDM_NONE				-1

#define		SELF_CHECK_BUFF_SIZE	7
#define		RMB_CHECK_BUFF_SIZE		7

#define		AMOUNTCOMFIRM			185
#define		BMOFFEST_H				1400
#define		BMOFFEST_L				600
#define		DEFAULT_LEVELV			1500

#define		SM_CONFIRM_LSM			13
#define		SM_CONFIRM_RSM			24

/************************************************************************/
/*							源数据结构体								*/
/************************************************************************/
typedef struct tagTdn_DataBuf
{
	int *BM_W;
	int *BM_T;
	int *BM_V;
	int *LSM_W;
	int *LSM_T;
	int *LSM_V;
	int *RSM_W;
	int *RSM_T;
	int *RSM_V;
	int *LM_W;
	int *LM_T;
	int *LM_V;
	int *RM_W;
	int *RM_T;
	int *RM_V;
	int *EIR0_W;
	int *EIR0_T;
	int *EIR0_V;
	int *EIR1_W;
	int *EIR1_T;
	int *EIR1_V;
	int *EIR2_W;
	int *EIR2_T;
	int *EIR2_V;
	int *EIR3_W;
	int *EIR3_T;
	int *EIR3_V;
	int *EIR4_W;
	int *EIR4_T;
	int *EIR4_V;
	int *EIR5_W;
	int *EIR5_T;
	int *EIR5_V;
	int *EIR6_W;
	int *EIR6_T;
	int *EIR6_V;
	int *EIR7_W;
	int *EIR7_T;
	int *EIR7_V;
	int *IR1_W;
	int *IR1_T;
	int *IR1_V;
	int *IR2_W;
	int *IR2_T;
	int *IR2_V;
	int *IR3_W;
	int *IR3_T;
	int *IR3_V;
	int *IR4_W;
	int *IR4_T;
	int *IR4_V;
	int *IR5_W;
	int *IR5_T;
	int *IR5_V;
	int *IR6_W;
	int *IR6_T;
	int *IR6_V;
	int *UV_W;
	int *UV_T;
	int *UV_V;
	int *UVL_W;
	int *UVL_T;
	int *UVL_V;
	int *UVR_W;
	int *UVR_T;
	int *UVR_V;
	int *HD_W;
	int *HD_T;
	int *HD_V;
	int *IR_NORM_V;

	int BM_Count;
	int LSM_Count;
	int RSM_Count;
	int LM_Count;
	int RM_Count;
	int EIR0_Count;
	int EIR1_Count;
	int EIR2_Count;
	int EIR3_Count;
	int EIR4_Count;
	int EIR5_Count;
	int EIR6_Count;
	int EIR7_Count;
	int IR1_Count;
	int IR2_Count;
	int IR3_Count;
	int IR4_Count;
	int IR5_Count;
	int IR6_Count;
	int UV_Count;
	int UVL_Count;
	int UVR_Count;
	int HD_Count;
	int IR_NORM_Count;

	//Tape_BackgroundInfo EIR_Back[EIR_NUM];
	int Tape_BackgroundValid;

	tagTdn_DataBuf()
	{
		memset(this, 0, sizeof(tagTdn_DataBuf));

		BM_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		BM_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		BM_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		LSM_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		LSM_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		LSM_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		RSM_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		RSM_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		RSM_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		LM_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		LM_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		LM_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		RM_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		RM_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		RM_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR0_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR0_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR0_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR1_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR1_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR1_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR2_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR2_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR2_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR3_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR3_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR3_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR4_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR4_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR4_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR5_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR5_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR5_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR6_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR6_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR6_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR7_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR7_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		EIR7_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR1_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR1_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR1_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR2_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR2_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR2_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR3_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR3_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR3_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR4_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR4_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR4_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR5_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR5_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR5_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR6_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR6_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR6_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UV_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UV_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UV_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UVL_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UVL_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UVL_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UVR_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UVR_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		UVR_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		HD_W = new int[5 * WAVELENGTH * sizeof(UINT32)];
		HD_T = new int[5 * WAVELENGTH * sizeof(UINT32)];
		HD_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		IR_NORM_V = new int[5 * WAVELENGTH * sizeof(UINT32)];
		Tape_BackgroundValid = 1;
	}
	~tagTdn_DataBuf()
	{
		delete [] BM_W;
		delete [] BM_T;
		delete [] BM_V;
		delete [] LSM_W;
		delete [] LSM_T;
		delete [] LSM_V;
		delete [] RSM_W;
		delete [] RSM_T;
		delete [] RSM_V;
		delete [] LM_W;
		delete [] LM_T;
		delete [] LM_V;
		delete [] RM_W;
		delete [] RM_T;
		delete [] RM_V;
		delete [] EIR0_W;
		delete [] EIR0_T;
		delete [] EIR0_V;
		delete [] EIR1_W;
		delete [] EIR1_T;
		delete [] EIR1_V;
		delete [] EIR2_W;
		delete [] EIR2_T;
		delete [] EIR2_V;
		delete [] EIR3_W;
		delete [] EIR3_T;
		delete [] EIR3_V;
		delete [] EIR4_W;
		delete [] EIR4_T;
		delete [] EIR4_V;
		delete [] EIR5_W;
		delete [] EIR5_T;
		delete [] EIR5_V;
		delete [] EIR6_W;
		delete [] EIR6_T;
		delete [] EIR6_V;
		delete [] EIR7_W;
		delete [] EIR7_T;
		delete [] EIR7_V;
		delete [] IR1_W;
		delete [] IR1_T;
		delete [] IR1_V;
		delete [] IR2_W;
		delete [] IR2_T;
		delete [] IR2_V;
		delete [] IR3_W;
		delete [] IR3_T;
		delete [] IR3_V;
		delete [] IR4_W;
		delete [] IR4_T;
		delete [] IR4_V;
		delete [] IR5_W;
		delete [] IR5_T;
		delete [] IR5_V;
		delete [] IR6_W;
		delete [] IR6_T;
		delete [] IR6_V;
		delete [] UV_W;
		delete [] UV_T;
		delete [] UV_V;
		delete [] UVL_W;
		delete [] UVL_T;
		delete [] UVL_V;
		delete [] UVR_W;
		delete [] UVR_T;
		delete [] UVR_V;
		delete [] HD_W;
		delete [] HD_T;
		delete [] HD_V;
		delete [] IR_NORM_V;
	}
}Tdn_DataBuf;

/************************************************************************/
/*							算法结果结构体								*/
/************************************************************************/
typedef struct tagM_BMHD_Info
{
	int validstart;//HD开始点
	int validend;//HD结束点
	int validstart_BM;//BM开始点
	int validend_BM;//BM结束点
	int valid_dianJugeShu;//间距个数
	int average;//间距均值
	int result;//大磁结果
}M_BMHD_Info;

typedef struct tagSM_RES
{
	int SMthre;//边磁个数
	int flag;//边磁有无
}SM_RES;

typedef struct tagM_RES
{
	int sumMthre;//中磁和阈值
	int MYes;//中磁有无
}M_RES;

typedef struct tagM_LRSM_Info
{
	SM_RES LSMRes;//左边磁结构体
	SM_RES RSMRes;//右边磁结构体
	M_RES LM_RES;//左中磁结构体
	M_RES RM_RES;//右中磁结构体
}M_LRSM_Info;

typedef struct tagResult_RMB
{
	int Moneystartw;//起始码盘
	int Moneyendw;//结束码盘
	int Lasterror;//刹车标记
	int ERRtype;//错误类型
	int Amount;//面额
	int Direction;//方向
	M_BMHD_Info BMHD_Info;//大磁结构体
	M_LRSM_Info LRSM_Info;//边中磁结构体

}Result_RMB;

/************************************************************************/
/*								自检参数								*/
/************************************************************************/
typedef struct tagselfCheck_Info
{
	int _LOW_DV;//方差下限
	int _MID_DV;//方差中限
	int _HIGH_DV;//方差上限
	int _LOW_AV;//均值下限
	int _HIGH_AV;//均值上限

}selfCheck_Info;

/************************************************************************/
/*								码盘参数								*/
/************************************************************************/
typedef struct tag_CODE_Parameter
{
	int _initialSum;//码盘采样点个数初始值
	int _samplePointMin;//每个码盘采样点下限
	int _samplePointMax;//每个码盘采样点上线

}CODE_Parameter;

/************************************************************************/
/*								大磁参数								*/
/************************************************************************/
typedef struct tag_BMHD_Parameter
{
	int _StartEndOffset;//大磁起始结束偏移
	int _BMnum_ceil;//大磁个数上限
	int _BMnum_floor;//大磁个数下限
	int _BMdeltamaskThre;//大磁差值模板匹配次数
	int _BMmaskThre;//大磁模板匹配次数
	int _HDdeltaPeak;//HD峰值差
	int _HDdeltaPosMin;//HD间距最小值
	int _HDdeltaPosMax;//HD间距最大值

}BMHD_Parameter;

/************************************************************************/
/*						   	   边磁中磁参数								*/
/************************************************************************/
typedef struct tag_LRSM_Parameter
{
	int _SMOFFEST_H;//边磁电压线上偏移
	int _SMOFFEST_L;//边磁电压线下偏移
	int _SMnum_strong;//边磁个数上限
	int _SMnum_weak;//边磁个数下限

	int _MIDMOFFEST_H;//中磁电压线上偏移
	int _MIDMOFFEST_L;//中磁电压线下偏移
	int _deltaMidMthre;//中磁峰谷阈值
	int _MidMthre;//中磁峰谷和阈值
	int _MidMnum_weak;//中磁个数下限

}LRSM_Parameter;


typedef struct tag_BMHD_LRSM__Parameter//主结构体
{
	selfCheck_Info _selfCheckInfo;
	CODE_Parameter _CODEParameter;
	BMHD_Parameter _BMHDParameter;
	LRSM_Parameter _LRSMParameter;

	tag_BMHD_LRSM__Parameter::tag_BMHD_LRSM__Parameter()
	{
		//自检参数
		_selfCheckInfo._LOW_DV			= 100;
		_selfCheckInfo._MID_DV			= 40000;
		_selfCheckInfo._HIGH_DV			= 80000;
		_selfCheckInfo._LOW_AV			= 1600;
		_selfCheckInfo._HIGH_AV			= 2500;

		//码盘参数
		_CODEParameter._initialSum		= 2;
		_CODEParameter._samplePointMin	= 5;
		_CODEParameter._samplePointMax	= 14;

		//大磁参数
		_BMHDParameter._StartEndOffset	= 0;//10;
		_BMHDParameter._BMnum_ceil		= 18;
		_BMHDParameter._BMnum_floor		= 11;
		_BMHDParameter._BMdeltamaskThre	= 7;
		_BMHDParameter._BMmaskThre		= 3;
		_BMHDParameter._HDdeltaPeak		= 200;
		_BMHDParameter._HDdeltaPosMin	= 35;
		_BMHDParameter._HDdeltaPosMax	= 300;

		//边磁参数
		_LRSMParameter._SMOFFEST_H		= 1600;
		_LRSMParameter._SMOFFEST_L		= 1300;
		_LRSMParameter._SMnum_strong	= 3;
		_LRSMParameter._SMnum_weak		= 1;
		//中磁参数
		_LRSMParameter._MIDMOFFEST_H	= 1500;
		_LRSMParameter._MIDMOFFEST_L	= 1300;
		_LRSMParameter._deltaMidMthre	= 1100;
		_LRSMParameter._MidMthre		  = 19000;
		_LRSMParameter._MidMnum_weak	= 2;
	}
}BMHD_LRSM_Parameter;

extern BMHD_LRSM_Parameter BMHD_LRSMParameter;
extern Tdn_DataBuf TdnDataBuf;
extern void InitialParameter(const BMHD_LRSM_Parameter * const _pParam);

#endif