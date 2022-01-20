# Visual Inverted Pendulum
 
This is the demo project mainly about the visual inverted pendulum demonstration project. It contains identify the swing angle of the swing rod by industrial camera and display the swing error in real time.

This repository contains:

1. OpenCV library
2. Daheng SDK

# Table of Contens

 - [Goal](#goal)
 - [Dependencies](#dependencies)
   - [Hardware](#hardware)
   - [Software](#software)
 - [Instruction](#instruction)
   - [Images_processing](#images_processing)
   - [Angle_recognition](#angle_recognition)
 - [Epitome](#epitome)

# Goal

* Use the the SDK of Daheng Camera to display images in real time on the computer.<br>
* Use the OpenCV library to images processing for display the swing erro of rod.

# Dependencies

## Hardware

* Computer
* Daheng Camera(MER-060-200GC-P)
* Straight-bar

## Software

* Linux 
* Ros
* OpenCV

# Instruction

## Images_processing

The original image is transformed into **grayscale image** and processed by **Gaussian filtering** and **binarization**.

Morphological operation on it:

 1. **Closed operation**
 2. **Open operation**

**FindContours** : Image contour search and rendering.

## Angle_recognition

The mainly use **Canny**, **Findcounters** and **RotatedRect**.


# Epitome

![image](https://user-images.githubusercontent.com/95466083/150332608-6b8d8321-ee0c-4fe5-a350-42f76f4a7573.png)
