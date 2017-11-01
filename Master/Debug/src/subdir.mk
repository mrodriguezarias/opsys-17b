################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Master.c \
../src/connection.c \
../src/funcionesMaster.c \
../src/manejadores.c 

OBJS += \
./src/Master.o \
./src/connection.o \
./src/funcionesMaster.o \
./src/manejadores.o 

C_DEPS += \
./src/Master.d \
./src/connection.d \
./src/funcionesMaster.d \
./src/manejadores.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=gnu11 -I"/home/utnso/git/tp-2017-2c-YATPOS/Shared" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


