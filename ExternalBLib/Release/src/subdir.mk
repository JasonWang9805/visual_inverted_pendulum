################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/dealData.cpp \
../src/memoryOperateBase.cpp \
../src/rqueue.cpp \
../src/saveData.cpp 

OBJS += \
./src/dealData.o \
./src/memoryOperateBase.o \
./src/rqueue.o \
./src/saveData.o 

CPP_DEPS += \
./src/dealData.d \
./src/memoryOperateBase.d \
./src/rqueue.d \
./src/saveData.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++  -fpermissive -I"/home/lgh/Desktop/Imavision/ExternalBLib/include" -include"/home/lgh/Desktop/Imavision/ExternalBLib/include/dealData.h" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


