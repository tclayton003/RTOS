# OKCubeSat (OBC System)


## Naming Convention Update:
- The nasty looking BSP and SDK dir names will likely be changed to just BSP and SDK once we verify that changing those will not break any imports or dependencies in the provided code (e.g. the SDK itself)
- They look bad, but I want to make sure the very basic test programs work before changing file naming


## Overview
OKCubeSat is an initiative of Oklahoma State University. It is a multi-year project, worked on mostly by Senior CEAT students and the OKCubeSat Club. 

Our mission is to verify NASA's Low Earth Orbit (LEO) debris models. This would allow NASA to begin researching removal methods and help future missions better prepare for LEO missions.

This repo specifically is the software for the On-Board Computer (OBC), including the Operating System (OS), Board Support Package (BSP), Command and Management Software Development Kit (SDK), and mission specific programs.

<em>If you are new to this project, please scroll to the <strong>Onboarding</strong> section at the bottom. That section is intended to bring you up to speed on Satellite Software so you can contribute.</em>

## File Structure
### Summary
- App/: Mission Code (Where you write code)
	- Tasks (battery monitor, comms, attitude control, etc.)
	- System logic
	- Calls BSP functions
	- Uses FreeRTOS APIs
- BSP/: Hardware Abstraction
	- Talks to MCU and peripherals
	- Sets clocks, interrupts, UART, ADC, sensors
	- Exposes these through C functions
- FreeRTOS-Kernel/: CPU Scheduler and Synchronization
	- From the official repo, but with only the parts we need.
	- This is where the process management logic lives, <em>but you should not need to touch it.</em>
- FreeRTOSConfig.h: RTOS Config
	- Tells FreeRTOS how to behave
	- Stack sizes
	- Priorities
	- Enabled features
- SDK/: Telemetry/Command Helpers
	- Prebuilt code from GomSpace for common needed functions
- License.md: FreeRTOS License
	- From the official FreeRTOS-Kernel repo and we retain it in our repo for legal transparency

### Tree Diagram
```
/
│
├─ App/                                                     (Mission Code)
│   ├─ main.c
│   └─ Tasks/
│       └─ (your task files go here later)
│
├─ gs-sw-nanomind-a3200-board-support-package-2.6.3/        (BSP for interacting with hardware)
│   ├─ Drivers/
│   ├─ Include/
│   ├─ Startup/
│   └─ (vendor hardware code & init)
│
├─ FreeRTOS-Kernel/                                        (Official kernel with unneeded elements removed)
│   ├─ event_groups.c
│   ├─ list.c
│   ├─ queue.c
│   ├─ stream_buffer.c
│   ├─ tasks.c
│   ├─ timers.c
│   │
│   ├─ include/
│   │   └─ (all FreeRTOS headers)
│   │
│   └─ portable/
│       └─ GCC/
│           └─ AVR32_UC3/                                   (Code for our specific board)
│               ├─ exception.S
│               ├─ port.c
│               └─ portmacro.h
│
├─ FreeRTOSConfig.h          								(RTOS configuration for NanoMind)
│
├─ SDK/
│   └─ (command & management SDK if you keep it)
│
├─ Makefile  or  CMakeLists.txt
│
├─ LICENSE.md
└─ README.md
```

## Development
- Code is written in App/
- <strong>App/main.c is the entry of the whole program</strong>
	- It initializes the hardware and drivers, creates the tasks, then starts the scheduler
	- The FreeRTOS scheduler initialize function should never return, so it will run forever
- <strong>App/Tasks is where all of the individual tasks live</strong>
	- You code functionality here
	- These are then turned into OS tasks by main.c when the program starts


## Deployment
- Use toolchain provided by GomSpace to compile program
- (Finish later)


## Hardware Specs
(Fill in later)


# Onboarding
Welcome to OKCubeSat! Satellite development can be overwhelming at first, but we would like to provide a clear picture of what it will look like so you can dive in ASAP!

### OS
- (FreeRTOS vs User OS)
- (Reason for choosing FreeRTOS)
- (Explanation that everything that was deleted from the downloaded official repo was generic code provided by FreeRTOS so it was compatible with any system, but we only need the stuff for our specific system)

### BSP
- (GomSpace Provided)
- (What it is)

### Tasks and Development
- (This is where you code)
- (Idea behind tasks)

### Data Flow Diagram???
- (possible visual flow diagram to show data and logic? would help understanding how pieces fit together)
