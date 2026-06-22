################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../03_function/f_CANlinkEx.c \
../03_function/f_canlink.c \
../03_function/f_comm.c \
../03_function/f_comm_DP.c \
../03_function/f_common.c \
../03_function/f_dspcan.c \
../03_function/f_eeprom.c \
../03_function/f_error.c \
../03_function/f_fcDeal.c \
../03_function/f_frqSrc.c \
../03_function/f_frqSrc_pid.c \
../03_function/f_funcCode.c \
../03_function/f_funcCode_disp.c \
../03_function/f_interface.c \
../03_function/f_invPara.c \
../03_function/f_io.c \
../03_function/f_main.c \
../03_function/f_menu.c \
../03_function/f_modbus.c \
../03_function/f_motorFc.c \
../03_function/f_osc.c \
../03_function/f_p2p.c \
../03_function/f_plc.c \
../03_function/f_profibus.c \
../03_function/f_runSrc.c \
../03_function/f_runSrc_accDecFrq.c \
../03_function/f_table_eeprom2Fc.c \
../03_function/f_torqueCtrl.c \
../03_function/f_ui.c \
../03_function/f_vf.c 

OBJS += \
./03_function/f_CANlinkEx.o \
./03_function/f_canlink.o \
./03_function/f_comm.o \
./03_function/f_comm_DP.o \
./03_function/f_common.o \
./03_function/f_dspcan.o \
./03_function/f_eeprom.o \
./03_function/f_error.o \
./03_function/f_fcDeal.o \
./03_function/f_frqSrc.o \
./03_function/f_frqSrc_pid.o \
./03_function/f_funcCode.o \
./03_function/f_funcCode_disp.o \
./03_function/f_interface.o \
./03_function/f_invPara.o \
./03_function/f_io.o \
./03_function/f_main.o \
./03_function/f_menu.o \
./03_function/f_modbus.o \
./03_function/f_motorFc.o \
./03_function/f_osc.o \
./03_function/f_p2p.o \
./03_function/f_plc.o \
./03_function/f_profibus.o \
./03_function/f_runSrc.o \
./03_function/f_runSrc_accDecFrq.o \
./03_function/f_table_eeprom2Fc.o \
./03_function/f_torqueCtrl.o \
./03_function/f_ui.o \
./03_function/f_vf.o 

C_DEPS += \
./03_function/f_CANlinkEx.d \
./03_function/f_canlink.d \
./03_function/f_comm.d \
./03_function/f_comm_DP.d \
./03_function/f_common.d \
./03_function/f_dspcan.d \
./03_function/f_eeprom.d \
./03_function/f_error.d \
./03_function/f_fcDeal.d \
./03_function/f_frqSrc.d \
./03_function/f_frqSrc_pid.d \
./03_function/f_funcCode.d \
./03_function/f_funcCode_disp.d \
./03_function/f_interface.d \
./03_function/f_invPara.d \
./03_function/f_io.d \
./03_function/f_main.d \
./03_function/f_menu.d \
./03_function/f_modbus.d \
./03_function/f_motorFc.d \
./03_function/f_osc.d \
./03_function/f_p2p.d \
./03_function/f_plc.d \
./03_function/f_profibus.d \
./03_function/f_runSrc.d \
./03_function/f_runSrc_accDecFrq.d \
./03_function/f_table_eeprom2Fc.d \
./03_function/f_torqueCtrl.d \
./03_function/f_ui.d \
./03_function/f_vf.d 


# Each subdirectory must supply rules for building sources it contributes
03_function/%.o: ../03_function/%.c 03_function/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GS RISC-V C Compiler'
	gs-riscv-elf-gcc -march=rv32imafc_xgsdida -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore --gsdi_ext1p2h -mgs_aux -O1 -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -funsigned-char --opt_level=off -O1 -g -ggdb -DFLASH_TARGET -DGS32_PART_NUM=0x035 -DGS32_PIN_COUNT=80 -DGS32F00xx=0x3000 -DUSING_VECTOR_INTERRUPT=1 -DTARGET_GS32=1 -DDSP2803X -DMATH_TYPE=IQ_MATH -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/sysctl" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/init" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp/NMSIS_core" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/device_gs32f00xx" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/common" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/src" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/inc" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/mathlib" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/function_header" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/LinT_HD01_DEF_header" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/motor_header" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


