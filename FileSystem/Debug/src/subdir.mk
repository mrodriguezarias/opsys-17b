################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/FileSystem.c \
../src/console.c \
../src/dirtree.c \
../src/filetable.c \
../src/nodelist.c \
../src/server.c 

OBJS += \
./src/FileSystem.o \
./src/console.o \
./src/dirtree.o \
./src/filetable.o \
./src/nodelist.o \
./src/server.o 

C_DEPS += \
./src/FileSystem.d \
./src/console.d \
./src/dirtree.d \
./src/filetable.d \
./src/nodelist.d \
./src/server.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=gnu11 -I"/home/utnso/git/tp-2017-2c-YATPOS/Shared" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


