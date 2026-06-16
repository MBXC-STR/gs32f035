#include "PmFluxWeakCsr2.h"

UVW_VOLT_STRUCT gVoltUVW;
void PMCsr2()
{
	long data,data1,DeltM;
//*******************************************************
	if((gPmCsr2.PhaseOut > gPmCsr2.PhaseMax)
				||(gPmCsr2.PhaseOut < gPmCsr2.PhaseMin)
				||(gPmCsr2.Im < gPmCsr2.ImMin))
	{
		gPmCsr2.OutFlag1 = 1;
	}
	else
	{
		gPmCsr2.OutFlag1 = 0;
	}
//********************************************************
	data = gPmCsr2.DeltM * gPmCsr2.KiM;
	if(gPmCsr2.FreqSyn < 0)
	{
		data = -data;
	}
	if(gPmCsr2.OutFlag1 != 0)
	{
		data1 = data * 2;
		gPmCsr2.UTTotal = gPmCsr2.UTTotal + data1;//ut����
	}

	data1 = -gPmCsr2.DeltM;
	if(data1 < 0)
	{
		data1 = 0;
	}
	data1 = data1 + 256L;
	data = (long long)data * (long long)data1>>8;
	if(gPmCsr2.OutFlag * data <= 0)
	{
		gPmCsr2.UTTotal += data;
	}

	gPmCsr2.UTTotal = __IQsat(gPmCsr2.UTTotal,1966080000L,-1966080000L);

	if(gPmCsr2.FreqSyn > 0)
	{
		data = __IQsat(gPmCsr2.DeltT,gPmCsr2.DeltTMax,-gPmCsr2.DeltTMin);
	}
	else
	{
		data = __IQsat(gPmCsr2.DeltT,gPmCsr2.DeltTMin,-gPmCsr2.DeltTMax);
	}
	data = gPmCsr2.Kp * data>>12;
	data = __IQsat(data,4000,-4000);
	gPmCsr2.UTOut =  data + (gPmCsr2.UTTotal>>12) + gPmCsr2.UTComp;

	data = __IQsat(gPmCsr2.DeltT,gPmCsr2.DeltTMin,-gPmCsr2.DeltTMin);
	data = data * gPmCsr2.Ki;
	if(gPmCsr2.FreqSyn > 0)
	{
		data = -data;
	}
	if(gPmCsr2.OutFlag1 == 1)
	{
		if((gPmCsr2.FreqSyn * gPmCsr2.DeltT)> 0)
		{
			if((gPmCsr2.UMOut < 0)||(gPmCsr2.OutFlag == 0))//if(gPmCsr2.It > 0)
			{
				data = -data>>3;//UM����
			}
			else
			{
				data = 0;
			}
		}
	}
	gPmCsr2.UMTotal = gPmCsr2.UMTotal + data;

	gPmCsr2.UMTotal = __IQsat(gPmCsr2.UMTotal,1966080000L,-1966080000L);
	data = gPmCsr2.DeltM * gPmCsr2.KFreq>>10;
	data = gPmCsr2.Kp * data>>12;
	gPmCsr2.UMOut = data + (gPmCsr2.UMTotal>>12) + gPmCsr2.UMComp;


	data = gPmCsr2.UMOut * gPmCsr2.UMOut + gPmCsr2.UTOut * gPmCsr2.UTOut;
	gPmCsr2.VoltOut  = qsqrt(data);
	if(gPmCsr2.VoltOut >= gPmCsr2.VoltMax)
	{
		gPmCsr2.OutFlag = 1;
		if(gPmCsr2.FreqSyn < 0)
		{
			gPmCsr2.OutFlag = -1;
		}
	}
	else
	{
		gPmCsr2.OutFlag = 0;
	}
	DeltM = 0;
	if(gPmCsr2.VoltOut >= gPmCsr2.VoltMax)
	{
		DeltM = (gPmCsr2.VoltOut - gPmCsr2.VoltMax);
	}
	if(DeltM > gPmCsr2.DeleteVMax)
	{
		DeltM = gPmCsr2.DeleteVMax;
	}
	data = DeltM * gPmCsr2.UMOut>>4;
	gPmCsr2.UMTotal = gPmCsr2.UMTotal - data;

	data = DeltM * gPmCsr2.UTOut>>4;
	gPmCsr2.UTTotal = gPmCsr2.UTTotal - data;


	gPmCsr2.VoltOut = __IQsat(gPmCsr2.VoltOut,gPmCsr2.MaxOutVoltPer*2L,100);
	gPmCsr2.PhaseOut = user_atan(gPmCsr2.UTOut,gPmCsr2.UMOut) * 10430.378;
}


