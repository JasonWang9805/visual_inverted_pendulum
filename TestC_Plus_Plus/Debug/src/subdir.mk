################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/GxSingleCamColorOpencv.cpp 

OBJS += \
./src/GxSingleCamColorOpencv.o 

CPP_DEPS += \
./src/GxSingleCamColorOpencv.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/lgh/Desktop/Visual inverted pendulum/TestC_Plus_Plus/include" -I"/home/lgh/Desktop/Visual inverted pendulum/opencv_lib/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


