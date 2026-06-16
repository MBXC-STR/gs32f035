################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../02_motor/main_public/MotorCarrier.c \
../02_motor/main_public/MotorConstant.c \
../02_motor/main_public/MotorCurrentTransform.c \
../02_motor/main_public/MotorDataExchange.c \
../02_motor/main_public/MotorDebug.c \
../02_motor/main_public/MotorFlyingStart.c \
../02_motor/main_public/MotorInfoCollect.c \
../02_motor/main_public/MotorInvProtect.c \
../02_motor/main_public/MotorMain.c \
../02_motor/main_public/MotorParChange.c \
../02_motor/main_public/MotorPublicCal.c \
../02_motor/main_public/MotorVar.c 

OBJS += \
./02_motor/main_public/MotorCarrier.o \
./02_motor/main_public/MotorConstant.o \
./02_motor/main_public/MotorCurrentTransform.o \
./02_motor/main_public/MotorDataExchange.o \
./02_motor/main_public/MotorDebug.o \
./02_motor/main_public/MotorFlyingStart.o \
./02_motor/main_public/MotorInfoCollect.o \
./02_motor/main_public/MotorInvProtect.o \
./02_motor/main_public/MotorMain.o \
./02_motor/main_public/MotorParChange.o \
./02_motor/main_public/MotorPublicCal.o \
./02_motor/main_public/MotorVar.o 

C_DEPS += \
./02_motor/main_public/MotorCarrier.d \
./02_motor/main_public/MotorConstant.d \
./02_motor/main_public/MotorCurrentTransform.d \
./02_motor/main_public/MotorDataExchange.d \
./02_motor/main_public/MotorDebug.d \
./02_motor/main_public/MotorFlyingStart.d \
./02_motor/main_public/MotorInfoCollect.d \
./02_motor/main_public/MotorInvProtect.d \
./02_motor/main_public/MotorMain.d \
./02_motor/main_public/MotorParChange.d \
./02_motor/main_public/MotorPublicCal.d \
./02_motor/main_public/MotorVar.d 


# Each subdirectory must supply rules for building sources it contributes
02_motor/main_public/%.o: ../02_motor/main_public/%.c 02_motor/main_public/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GS RISC-V C Compiler'
	gs-riscv-elf-gcc -march=rv32imafc_xgsdida -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore --gsdi_ext1p2h -mgs_aux -O1 -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -Wfatal-errors -funsigned-char --opt_level=off -O1 -g -ggdb -DFLASH_TARGET -DGS32_PART_NUM=0x035 -DGS32_PIN_COUNT=80 -DGS32F00xx=0x3000 -DUSING_VECTOR_INTERRUPT=1 -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/sysctl" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/init" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp/NMSIS_core" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/device_gs32f00xx" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/common" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/src" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/inc" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/mathlib" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/28034_header" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/function_header" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/LinT_HD01_DEF_header" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/motor_header" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


