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

ax1.plot([90] * array.shape[0], label='goal', linestyle='--', color='green')
ax2.plot([0] * array.shape[0], label='zero', linestyle='--', color='green')
# Plot the data
# print(array.shape)
# ax2 = ax1.twinx()
for i, h in enumerate(headers):
    # Define line name and data
    # print(array[:5, i])
    if h.lower() in ['output', 'p', 'i', 'd']:
        ax2.plot(array[:, i], label=h)
    else:
        ax1.plot(array[:, i], label=h)

# Show the plot
ax1.legend()
ax2.legend()
plt.show()
