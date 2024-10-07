################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cQueue/Src/cQueue.c 

OBJS += \
./cQueue/Src/cQueue.o 

C_DEPS += \
./cQueue/Src/cQueue.d 


# Each subdirectory must supply rules for building sources it contributes
cQueue/Src/%.o cQueue/Src/%.su cQueue/Src/%.cyclo: ../cQueue/Src/%.c cQueue/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/cheoljun99/Desktop/STM32_reference/STM32_Study/STM32F407VGT6_CAN_Sniffer_NonRTOS/COBSLib/Inc" -I"C:/Users/cheoljun99/Desktop/STM32_reference/STM32_Study/STM32F407VGT6_CAN_Sniffer_NonRTOS/cQueue/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-cQueue-2f-Src

clean-cQueue-2f-Src:
	-$(RM) ./cQueue/Src/cQueue.cyclo ./cQueue/Src/cQueue.d ./cQueue/Src/cQueue.o ./cQueue/Src/cQueue.su

.PHONY: clean-cQueue-2f-Src

