################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
C:/Users/Jianrong\ Lin/str/GS32\ Studio/packages/GS32-DSPWare/gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32.S \
C:/Users/Jianrong\ Lin/str/GS32\ Studio/packages/GS32-DSPWare/gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32_s.S \
C:/Users/Jianrong\ Lin/str/GS32\ Studio/packages/GS32-DSPWare/gs32_driver/driverlib/dsp/startup_dsp/startup_gs32_dsp.S 

OBJS += \
./gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32.o \
./gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32_s.o \
./gs32_driver/driverlib/dsp/startup_dsp/startup_gs32_dsp.o 

S_UPPER_DEPS += \
./gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32.d \
./gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32_s.d \
./gs32_driver/driverlib/dsp/startup_dsp/startup_gs32_dsp.d 


# Each subdirectory must supply rules for building sources it contributes
gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32.o: C:/Users/Jianrong\ Lin/str/GS32\ Studio/packages/GS32-DSPWare/gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32.S gs32_driver/driverlib/dsp/startup_dsp/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GS RISC-V Assembler'
	gs-riscv-elf-gcc -march=rv32imafc_xgsdida -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore --gsdi_ext1p2h -mgs_aux -O1 -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -funsigned-char --opt_level=off -O1 -g -ggdb -x assembler-with-cpp -DFLASH_TARGET -DGS32_PART_NUM=0x035 -DGS32_PIN_COUNT=80 -DGS32F00xx=0x3000 -DUSING_VECTOR_INTERRUPT=1 -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp/NMSIS_core" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/inc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32_s.o: C:/Users/Jianrong\ Lin/str/GS32\ Studio/packages/GS32-DSPWare/gs32_driver/driverlib/dsp/startup_dsp/intexc_gs32_s.S gs32_driver/driverlib/dsp/startup_dsp/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GS RISC-V Assembler'
	gs-riscv-elf-gcc -march=rv32imafc_xgsdida -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore --gsdi_ext1p2h -mgs_aux -O1 -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -funsigned-char --opt_level=off -O1 -g -ggdb -x assembler-with-cpp -DFLASH_TARGET -DGS32_PART_NUM=0x035 -DGS32_PIN_COUNT=80 -DGS32F00xx=0x3000 -DUSING_VECTOR_INTERRUPT=1 -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp/NMSIS_core" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/inc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

gs32_driver/driverlib/dsp/startup_dsp/startup_gs32_dsp.o: C:/Users/Jianrong\ Lin/str/GS32\ Studio/packages/GS32-DSPWare/gs32_driver/driverlib/dsp/startup_dsp/startup_gs32_dsp.S gs32_driver/driverlib/dsp/startup_dsp/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GS RISC-V Assembler'
	gs-riscv-elf-gcc -march=rv32imafc_xgsdida -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore --gsdi_ext1p2h -mgs_aux -O1 -fmessage-length=0 -ffunction-sections -fdata-sections -fsingle-precision-constant -funsigned-char --opt_level=off -O1 -g -ggdb -x assembler-with-cpp -DFLASH_TARGET -DGS32_PART_NUM=0x035 -DGS32_PIN_COUNT=80 -DGS32F00xx=0x3000 -DUSING_VECTOR_INTERRUPT=1 -I"C:\Users\Jianrong Lin\str\GS32 Studio\\packages\GS32-DSPWare\gs32_driver/driverlib/dsp/NMSIS_core" -I"C:/Users/Jianrong Lin/str/work/gs32g035-str/inc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


