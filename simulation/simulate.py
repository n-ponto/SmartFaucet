from math import ceil
import struct
import time
import serial
from serial import SerialException
from serial.tools import list_ports
from threading import Thread

HOT_WATER = 120
COLD_WATER = 55
MAX_FAUCET = 255
current_hot_water = COLD_WATER
prev_faucet_temp = COLD_WATER
hot_water_on = False

# Connect to serial
valid_port = None
ports = list_ports.comports()
for port, desc, _ in ports:
    if "Arduino" in desc:
        print(f"Found Arduino on port {valid_port}: {desc}")
        valid_port = port
        break

port = serial.Serial(port=port, baudrate=9600, timeout=1)
port.set_buffer_size(rx_size=12800, tx_size=12800)

# Create thread to communicate with serial
run_thread = True

new_temp_weight = 0.05  # Weight of new temp in moving average


def get_water_temp(faucet_setting):
    global prev_faucet_temp
    assert (0 <= faucet_setting and faucet_setting <= 255), f"Faucet setting out of range: {faucet_setting}"
    if hot_water_on:
        cold_percent = faucet_setting / 2 / MAX_FAUCET
        hot_percent = 1.0 - cold_percent
        new_temp = cold_percent * COLD_WATER + hot_percent * current_hot_water
    else:
        new_temp = COLD_WATER
    # Moving average of new and old temp to simulate delay
    tempurature = prev_faucet_temp * (1 - new_temp_weight) + new_temp * new_temp_weight
    return tempurature


def communicate():
    global run_thread, prev_faucet_temp
    current_faucet_temp = COLD_WATER
    faucet_setting = 0
    port.write(struct.pack("B", int(current_faucet_temp)))
    while run_thread:
        if port.in_waiting > 0:
            faucet_setting = int.from_bytes(port.read(), 'big')
        current_faucet_temp = get_water_temp(faucet_setting)
        if int(current_faucet_temp) != int(prev_faucet_temp):
            print(".", end="")
            port.write(struct.pack("B", int(current_faucet_temp)))
            # print(f"Sent: {current_faucet_temp}")
        print(f"\rFaucet: {faucet_setting:<5} Temp: {current_faucet_temp:7.2f} Hot: {current_hot_water:<5}", end="")
        prev_faucet_temp = current_faucet_temp
        time.sleep(0.1)


# Start thread
thread = Thread(target=communicate)
thread.start()

increase_seconds = 10


def slow_change_temp():
    global current_hot_water
    current_hot_water = COLD_WATER
    update_period = 0.5  # Period in seconds between updates
    update_steps = ceil(increase_seconds / update_period)
    change_amount = (HOT_WATER - COLD_WATER) / update_steps
    current_step = 0
    while run_thread and current_step <= update_steps:
        current_hot_water += change_amount
        current_step += 1
        time.sleep(0.5)
    print("Temp increase complete")


# Main loop
try:
    while True:
        user_input = input("Enter command:").lower()
        if user_input == "exit":
            break
        elif user_input == "on":
            hot_water_on = True
            current_hot_water = HOT_WATER
        elif user_input == "off":
            hot_water_on = False
        elif user_input == "increase":
            hot_water_on = True
            increase_thread = Thread(target=slow_change_temp)
            increase_thread.start()
        else:
            print("Invalid command")
finally:
    print('Exiting...')
    run_thread = False
    thread.join()
    port.close()
    exit(0)
