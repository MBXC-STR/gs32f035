################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Jianrong\ Lin/str/GS32\ Studio/packages/GS32-DSPWare/gs32_driver/driverlib/dsp/system_dsp.c 

OBJS += \
./gs32_driver/driverlib/dsp/system_dsp.o 

C_DEPS += \
./gs32_driver/driverlib/dsp/system_dsp.d 


# Each subdirectory must supply rules for building sources it contributes
gs32_driver/driverlib/dsp/system_dsp.o: C:/Users/Jianrong\ Lin/str/GS32\ Studio/packages/GS32-DSPWare/gs32_driver/driverlib/dsp/system_dsp.c gs32_driver/driverlib/dsp/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GS RISC-V C Compiler'
	gs-riscv-elf-gcc -march=rv32imafc_xgsdida -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore --gsdi_ext1p2h -mgs_aux -O1 -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -funsigned-char --opt_level=off -O1 -g -ggdb -DFLASH_TARGET -DGS32_PART_NUM=0x035 -DGS32_PIN_COUNT=80 -DGS32F00xx=0x3000 -DUSING_VECTOR_INTERRUPT=1 -DTARGET_GS32=1 -DDSP2803X -DMATH_TYPE=IQ_MATH -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/sysctl" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/init" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp/NMSIS_core" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/device_gs32f00xx" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/common" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/src" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/inc" -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/mathlib" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/function_header" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/LinT_HD01_DEF_header" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/05_public/motor_header" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


