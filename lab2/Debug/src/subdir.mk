################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
<<<<<<< HEAD
../src/lab2_2.c 

OBJS += \
./src/lab2_2.o 

C_DEPS += \
./src/lab2_2.d 
=======
../src/lab2_1.c 

OBJS += \
./src/lab2_1.o 

C_DEPS += \
./src/lab2_1.d 
>>>>>>> 450e28b25748dc27740348fc5a3c04a3cf567cec


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


