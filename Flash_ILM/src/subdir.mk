################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/main.c \
../src/myepwm_board.c 

OBJS += \
./src/main.o \
./src/myepwm_board.o 

C_DEPS += \
./src/main.d \
./src/myepwm_board.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GS RISC-V C Compiler'
	gs-riscv-elf-gcc -march=rv32imafc_xgsdida -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore --gsdi_ext1p2h -mgs_aux -O1 -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -Wfatal-errors -funsigned-char --opt_level=off -O1 -g -ggdb -DFLASH_TARGET -DGS32_PART_NUM=0x035 -DGS32_PIN_COUNT=80 -DGS32F00xx=0x3000 -DUSING_VECTOR_INTERRUPT=1 -I"C:\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp" -I"C:\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/sysctl" -I"C:\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/init" -I"C:\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp/NMSIS_core" -I"C:\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib" -I"C:\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/device_gs32f00xx" -I"C:\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/common" -I"C:/GS32 Studio/packages/GS32-DSPWare/gs32_examples/gs32f035/template/gs32f035/src" -I"C:/GS32 Studio/packages/GS32-DSPWare/gs32_examples/gs32f035/template/gs32f035/inc" -I"C:\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/mathlib" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


