# Load the data from a CSV file and plot it
# Usage: python plotcsv.py <filename.csv>

import sys
import os
import numpy as np
import matplotlib.pyplot as plt

# Read the data from the CSV file
filename = sys.argv[1]

if not os.path.exists(filename):
    print("File not found")
    sys.exit(1)

# filename = "test3.csv"
with open(filename, newline='') as csvfile:
    csvreader = csvfile.readlines()
    headers = csvreader[0].strip().split(',')
    array = np.zeros((len(csvreader)-1, len(headers)))
    for i, row in enumerate(csvreader[1:]):
        parts = row.split(',')
        for j, part in enumerate(parts):
            array[i, j] = float(part)
            # if i<5: print(float(part), end=',')
        # if i<5: print()
        # if i < 5:
        #     print(",".join(parts))
fig, (ax1, ax2) = plt.subplots(2, 1)

x = None
time_header = 'Time (ms)'
if time_header in headers:
    ax1.set_xlabel(time_header)
    ax2.set_xlabel(time_header)
    idx = headers.index(time_header)
    x = array[:, idx]
    array = np.delete(array, idx, 1)
    headers.remove(time_header)

ax1.plot(x, [90] * array.shape[0], label='goal', linestyle='--', color='green')
ax2.plot(x, [0] * array.shape[0], label='zero', linestyle='--', color='green')
# Plot the data
# print(array.shape)
# ax2 = ax1.twinx()
for i, h in enumerate(headers):
    # Define line name and data
    # print(array[:5, i])
    if h.lower() in ['output', 'p', 'i', 'd']:
        ax2.plot(x, array[:, i], label=h)
    elif (any(x in h.lower() for x in ['theory', 'therm'])):
        ax1.plot(x, array[:, i], label=h, linestyle='--')
    else:
        ax1.plot(x, array[:, i], label=h)


# Show the plot
ax1.legend()
ax2.legend()
plt.show()
