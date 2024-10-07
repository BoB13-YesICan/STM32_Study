################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../COBSLib/Src/cobs.c 

OBJS += \
./COBSLib/Src/cobs.o 

C_DEPS += \
./COBSLib/Src/cobs.d 


# Each subdirectory must supply rules for building sources it contributes
COBSLib/Src/%.o COBSLib/Src/%.su COBSLib/Src/%.cyclo: ../COBSLib/Src/%.c COBSLib/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/cheoljun99/Desktop/STM32_reference/STM32_Study/STM32F407VGT6_CAN_Sniffer_RTOS/COBSLib/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-COBSLib-2f-Src

clean-COBSLib-2f-Src:
	-$(RM) ./COBSLib/Src/cobs.cyclo ./COBSLib/Src/cobs.d ./COBSLib/Src/cobs.o ./COBSLib/Src/cobs.su

.PHONY: clean-COBSLib-2f-Src

