################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
Example_2802xCpuTimer.obj: ../Example_2802xCpuTimer.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-c2000_18.1.2.LTS/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-c2000_18.1.2.LTS/include" --include_path="C:/ti/C2000Ware_1_00_04_00_Software/device_support/f2802x/common/include" --include_path="C:/ti/C2000Ware_1_00_04_00_Software/device_support/f2802x/headers/include" --include_path="C:/ti/C2000Ware_1_00_04_00_Software/device_support/f2802x" --include_path="C:/ti/C2000Ware_1_00_04_00_Software/libraries/math/IQmath/c28/include" --define="_DEBUG" --define="LARGE_MODEL" --quiet --verbose_diagnostics --diag_warning=225 --diag_suppress=10063 --preproc_with_compile --preproc_dependency="Example_2802xCpuTimer.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

user.obj: ../user.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C2000 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-c2000_18.1.2.LTS/bin/cl2000" -v28 -ml -mt -g --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-c2000_18.1.2.LTS/include" --include_path="C:/ti/C2000Ware_1_00_04_00_Software/device_support/f2802x/common/include" --include_path="C:/ti/C2000Ware_1_00_04_00_Software/device_support/f2802x/headers/include" --include_path="C:/ti/C2000Ware_1_00_04_00_Software/device_support/f2802x" --include_path="C:/ti/C2000Ware_1_00_04_00_Software/libraries/math/IQmath/c28/include" --define="_DEBUG" --define="LARGE_MODEL" --quiet --verbose_diagnostics --diag_warning=225 --diag_suppress=10063 --preproc_with_compile --preproc_dependency="user.d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


