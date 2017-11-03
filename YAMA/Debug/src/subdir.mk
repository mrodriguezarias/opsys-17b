################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/YAMA.c \
../src/client.c \
../src/funcionesYAMA.c \
../src/requests.c \
../src/server.c \
../src/struct.c 

OBJS += \
./src/YAMA.o \
./src/client.o \
./src/funcionesYAMA.o \
./src/requests.o \
./src/server.o \
./src/struct.o 

C_DEPS += \
./src/YAMA.d \
./src/client.d \
./src/funcionesYAMA.d \
./src/requests.d \
./src/server.d \
./src/struct.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=gnu11 -I"/home/utnso/workspace/tp-2017-2c-YATPOS/Shared" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


