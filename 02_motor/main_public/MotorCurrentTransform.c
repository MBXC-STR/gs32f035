/****************************************************************
�ļ����ܣ���������ѹ������任
�ļ��汾��
���¸��£�
	
****************************************************************/
#include "MotorCurrentTransform.h"
#include "MotorInclude.h"
#include "MotorEncoder.h"

ALPHABETA_STRUCT		gUAlphBeta;	    //���������������ѹ
MT_STRUCT_Q24           gUMTQ24; 
extern PMSM_FLUX_WEAK_STRUCT   gPmFluxWeak;
int para0;
//extern MT_STRUCT_Q24           gIMTQ24_obs;
//extern MT_STRUCT_Q24           gIMTQ12_obs;
/*******************************************************************
    ���ڲ����˱�ôֵϵͳ��Ҫ�����е�����任�����뱣֤��ֵ��ȵı任��
���ɣ���ôֵϵͳ�£���ֵȷ����ų��任��
********************************************************************/
/*******************************************************************
Date Type Q24(��֤��ֵ���������任)(������ЧֵΪ1���Ҳ�������任��ֵΪ1)
	Alph= U * (1/2)^0.5 
	Beta= (3^0.5/2) * (U + 2*V)
	UVW�������Է�ֵ��ʾ�ģ�ALPH BETA��M T�����������Чֵ���
********************************************************************/
void inline UVWToAlphBetaAxes(UVW_STRUCT_Q24 * uvw, ALPHABETA_STRUCT * AlphBeta)
{
	AlphBeta->Alph = ((llong)uvw->U * 23170L)>>15;	

	AlphBeta->Beta = ((llong)((long)uvw->V - (long)uvw->W) * 13377L)>>15;
}

/*******************************************************************
Date Type Q12 ��q�ᳬd��90�ȣ�
	d= cos(theta)*alph + sin(theta)*beta;
	q= -sin(theta)*alph + cos(theta)*beta;
********************************************************************/
void AlphBetaToDQ(ALPHABETA_STRUCT * AlphBeta, int angle, MT_STRUCT_Q24 * MT)
{
	int m_sin,m_cos;

	m_sin  = qsin(angle);
	m_cos  = qsin(16384 - angle);
	MT->M = ( ((llong)m_cos * (llong)(AlphBeta->Alph)) + 
	          ((llong)m_sin * (llong)(AlphBeta->Beta)) )>>15;
	MT->T = (-((llong)m_sin * (llong)(AlphBeta->Alph)) + 
	          ((llong)m_cos * (llong)(AlphBeta->Beta)) )>>15;
}

/*******************************************************************
Date Type Q12
	A= (d*d + q*q)^0.5
	q= user_atan(q/d)
********************************************************************/
void DQToAmpTheta(MT_STRUCT * MT,AMPTHETA_STRUCT * AmpTheta)
{
	long m_Input;

	m_Input = (((long)MT->M * (long)MT->M) + ((long)MT->T * (long)MT->T));
	AmpTheta->Amp = (Uint)qsqrt(m_Input);

	AmpTheta->Theta = user_atan(MT->M,MT->T);
}

/*************************************************************
	�����任����
*************************************************************/
void ChangeCurrent(void)
{
  //  Ulong   m_Long;    
    int temp1,temp2;		// wg
	Ulong   tmpAmp;

    // ��ȡ�������˲ʱֵ, �������ת��Ϊ���������������µĵ���
	UVWToAlphBetaAxes((UVW_STRUCT_Q24*)&gIUVWQ24,(ALPHABETA_STRUCT*)&gIAlphBeta);
    gIAlphBetaQ12.Alph = gIAlphBeta.Alph>>12;
    gIAlphBetaQ12.Beta = gIAlphBeta.Beta>>12;
	// ���������������µ���ת��ΪDQ�µĵ���
	AlphBetaToDQ((ALPHABETA_STRUCT*)&gIAlphBeta,gPhase.IMPhase, &gIMTQ24);
    
    gIMTQ12.M = gIMTQ24.M >> 12;     //ȡ���˲����˲�����Ӱ��SVC����������
    gIMTQ12.T = gIMTQ24.T >> 12;

	gLineCur.ImTotal += gIMTQ12.M;
    gLineCur.ItTotal += gIMTQ12.T;
    gLineCur.CurCnt ++;
	//�����ߵ���
    /*tmpAmp = (long)gIMTQ12.M * gIMTQ12.M;
    tmpAmp += (long)gIMTQ12.T * gIMTQ12.T;
    gIAmpTheta.Amp = (Uint)qsqrt(tmpAmp);*/
    //...................................�����ߵ���

    //gIAmpTheta.Theta = user_atan(gIMTSetQ12.M, gIMTSetQ12.T);	//����MT��н�
    gIAmpTheta.Theta = user_atan(gIMTQ12.M, gIMTQ12.T);	//����MT��н�
    gIAmpTheta.ThetaOld = gIAmpTheta.Theta + gPhase.IMPhaseOld;	// wg
   // temp = gOutVolt.VoltPhaseApply - gIAmpTheta.Theta;
/*	temp1 = gOutVolt.VoltPhaseApply1 - gIAmpTheta.ThetaOld;		// wg
    temp2 = gOutVolt.VoltPhaseApply2 - gIAmpTheta.ThetaOld;		// wg
    temp3 = ((Uint)temp1 >> 1)+ ((Uint)temp2 >> 1);

    gIAmpTheta.PowerAngle = Filter8(temp3, gIAmpTheta.PowerAngle);		// wg
*/
	temp1 = gOutVolt.VoltPhaseApply1 - gIAmpTheta.ThetaOld;
    temp2 = temp1 + ((gOutVolt.VoltPhaseApply2 - gOutVolt.VoltPhaseApply1)>>1);
    temp2 = __IQsat(temp2,32767,-32767);
    gIAmpTheta.PowerAngle = Filter8(temp2, gIAmpTheta.PowerAngle);
	
    if(gMainCmd.Command.bit.ControlMode == IDC_SVC_CTL)
	{
	    CalUVWVoltSet(gPhase.IMPhase);
	    gPmsmRotorPosEst.Ud = gVoltUVW.UdQ;
	    gPmsmRotorPosEst.Uq = gVoltUVW.UqQ;											// ����SVC�µĵ�ѹ��ʾ
	}
}

