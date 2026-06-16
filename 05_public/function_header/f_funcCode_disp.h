/*
 * f_funcCode_disp.h
 *
 *  Created on: 2016-1-12
 *      Author: 10024a02
 */
#ifndef F_FUNCCODE_DISP_H_
#define F_FUNCCODE_DISP_H_

#define DISP_FRQ_RUN            0   //  0, 运行频率
#define DISP_FRQ_AIM            1   //  1, 设定频率
#define DISP_OUT_CURRENT        4   //  4, 输出电流
#define DISP_OUT_POWER          5   //  5, 输出功率
#define DISP_FRQ_RUN_FDB        19  //  19,反馈速度
#define DISP_FRQ_COMM           28  //  28,通讯设定值
#define DISP_P2P_COMM_SEND      63  //  63,点对点通讯数据发送
#define DISP_P2P_COMM_REV       64  //  64,点对点通讯数据接收
#define DISP_FRQ_FDB            29  //  29,反馈速度 
#define DISP_FRQ_X              30  //  30, 主频率X显示
#define DISP_FRQ_Y              31  //  31, 辅助频率Y显示
#define DISP_LOAD_SPEED         14  //  运行时负载速度显示的bit位置

#define DISP_DI_STATUS_SPECIAL1 41  // DI输入状态直观显示，DI1-DI19,VDI1
#define DISP_DO_STATUS_SPECIAL1 42  // DO输入状态直观显示，R1-R10,DO1,R11,R12,VDO1-VDO5
#define DISP_DI_FUNC_SPECIAL1   43  // DI功能状态直观显示1，功能01-功能40
#define DISP_DI_FUNC_SPECIAL2   44  // DI功能状态直观显示2，功能41-功能80

#define UF_VIEW_ATTRIBUTE  0x3140

//extern union FUNC_ATTRIBUTE const dispAttributeU0[];
//extern const Uint16 stopDispIndex[];
//extern Uint16 const commDispIndex[];

extern Uint16 const commDispIndex[];
extern const Uint16 stopDispIndex[];
extern union FUNC_ATTRIBUTE const dispAttributeU1[];
extern union FUNC_ATTRIBUTE const dispAttributeU0[];

#endif /* F_FUNCCODE_DISP_H_ */
