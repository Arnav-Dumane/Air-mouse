# Wired PC Mouse Controller with Clicks
# This script connects to the Air Mouse via a direct USB serial connection.

import serial
from pynput.mouse import Controller, Button

# --- PC-SIDE SENSITIVITY MULTIPLIER ---
# > 1.0 = Faster movement (e.g., 1.5 is 50% faster)
# < 1.0 = Slower movement (e.g., 0.8 is 20% slower)
#   1.0 = No change
sensitivity_multiplier = 1.0

# --- ARDUINO CONNECTION SETTINGS ---
# 1. Find your Arduino's COM port in the Arduino IDE (Tools > Port).
# 2. CHANGE THE PORT NUMBER BELOW to match.
ARDUINO_PORT = 'COM10' 
'' # <-- IMPORTANT: CHANGE THIS!
BAUD_RATE = 115200    # Must match the speed in the Arduino sketch

mouse = Controller()

try:
  ser = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=1)

  print(f"Connecting to Arduino on {ARDUINO_PORT}...")
  # Wait for the Arduino to send its "Calibration Complete!" message
  while "Calibration Complete!" not in str(ser.readline(), 'utf-8'):
      pass
  print("Arduino connected and calibrated. Wired mouse with clicks is now active!")
  print("Press Ctrl+C in this window to quit.")

  while True:
    line = ser.readline()
    if not line:
        continue

    try:
        dump = str(line, 'utf-8').strip()
        data = dump.split(',')

        if data[0] == "DATAL" and len(data) >= 4:
            click_info = int(data[3])

            if click_info == 1:
                mouse.click(Button.left)
                print("Left Click") # For debugging
                continue # Skip movement on the same frame as a click

            elif click_info == 2:
                mouse.click(Button.right)
                print("Right Click") # For debugging
                continue # Skip movement

            # If no click, handle movement
            roll_movement = int(data[1])
            pitch_movement = int(data[2])

            # --- AXIS MAPPING & CORRECTION ---
            # This maps the physical sensor movement to the screen's axes.
            move_x = -pitch_movement
            move_y = -roll_movement
            
            # Apply the sensitivity multiplier
            final_move_x = int(move_x * sensitivity_multiplier)
            final_move_y = int(move_y * sensitivity_multiplier)

            mouse.move(final_move_x, final_move_y)
            
    except (ValueError, IndexError):
        # This catches any incomplete data from the serial port and ignores it.
        pass

except serial.SerialException:
  print(f"\nError: Could not connect to Arduino on {ARDUINO_PORT}.")
  print("Please check that the correct COM port is selected and the device is plugged in.")
  input("Press Enter to exit.")
except KeyboardInterrupt:
  print("\nProgram terminated by user.")
except Exception as e:
  print(f"An unexpected error occurred: {e}")
  input("Press Enter to exit.")

