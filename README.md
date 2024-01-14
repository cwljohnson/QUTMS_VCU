# QUTMS_VCU

This repo contains the source code for QUT Motorsport's Vehicle Control Unit (VCU), and was developed by Calvin Johnson. This was a multi-purpose board, able to act as the key vehicle controller, whilst also as a generic sensor board, digitializing analogue sensor outputs and broadcasting them over CAN, and also able to switch 12V loads.

The VCU utilized a STM32F207 based microcontroller.

## Contents

This repository contains three main STM32 projects:

- software_v3: RTOS based rewrite of the firmware, latest version of the code, ran on QEV3 and QEV4 in 2023.
- software: Original non-RTOS based firmware project, run on QEV3 in 2022.
- TestFirmware: Project containing firmware to test actuation of VCU hardware.
