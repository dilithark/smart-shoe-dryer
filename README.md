# smart-shoe-dryer
System Overview
This Arduino sketch controls a time-dependent automated system that integrates user input, thermal control, motion actuation, and visual feedback. It allows an operator to input a specific operational duration (in minutes) via a keypad, after which it activates a motor/Peltier thermal assembly, adjusts servo positions, and displays a live countdown on an I2C LCD.

Key Hardware Components
Microcontroller: Arduino (implied by standard pin structures and libraries).

User Input: 4x4 Matrix Keypad connected via analog/digital pins (A0–A3 for rows, A4, A5, 2, 3 for columns).

Display: 16x2 I2C LCD (at address 0x27) for real-time prompts and countdown tracking.

Actuators:

Motor / Peltier Modules: Driven via an H-bridge motor driver interface (pins 6, 7, 12, 13). Powering the motor simultaneously activates the attached Peltier thermoelectric modules.

Servos (x2): Controlled via the VarSpeedServo library (pins 8 and 9) to handle mechanical positioning.

Status Indicators: Two distinct LEDs—a Run LED (pin 10) indicating active operation, and a Done LED (pin 11) indicating completion.

Operational Workflow
1. Initialization & Setup
The system configures all motor control lines, LED indicators, and servo attachments.

Both servos are initialized to their default home position (0°).

The I2C LCD boots up and displays the initial prompt: Enter Time (min).

2. Input Mode (Standby)
The system waits for numeric input from the keypad:

Number Keys (0–9): Appends digits to build the runtime value string, updating the LCD dynamically.

Clear Key (*): Resets the current input string if a mistake is made.

Confirm Key (#): Converts the entered string into an integer, calculates total milliseconds (minutes × 60,000), and initiates the process via startMotor().

3. Running Mode
Once triggered, the system executes the following actions simultaneously:

Thermal & Motion: Activates the H-bridge outputs to run the motor and Peltier modules.

Actuation: Moves both servos to an active operating position (90° at a specified speed).

Visual Status: Turns ON the Run LED (ledRun), turns OFF the Done LED, and switches the LCD to display a live countdown (Left: Xs).

4. Completion & Reset
Once the elapsed time matches or exceeds the selected duration, stopMotor() is invoked.

Power to the motor and Peltier modules is cut off.

Servos return to their home position (0°).

The Run LED turns off, the Done LED turns on, and the LCD displays Finished before resetting to the initial prompt after a short delay.
