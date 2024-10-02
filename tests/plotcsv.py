# Load the data from a CSV file and plot it
# Usage: python plotcsv.py <filename.csv>

import sys
import os
import numpy as np
import matplotlib.pyplot as plt


def arr_from_csv(filename):
    with open(filename, newline='') as csvfile:
        csvreader = csvfile.readlines()
        headers = csvreader[0].strip().split(',')
        array = np.zeros((len(csvreader)-1, len(headers)))
        for i, row in enumerate(csvreader[1:]):
            parts = row.split(',')
            for j, part in enumerate(parts):
                array[i, j] = float(part)
    return headers, array


def plot_drill_results():
    headers, array = arr_from_csv('csv/drill_results.csv')
    fig, (ax, ax_kp, ax_kd) = plt.subplots(3, 1)
    for i, h in enumerate(headers):
        if 'kp' in h.lower():
            ax_kp.plot(array[:, i], label=h)
        elif 'kd' in h.lower():
            ax_kd.plot(array[:, i], label=h)
        else:
            ax.plot(array[:, i], label=h)

    ax.legend()
    ax_kp.legend()
    ax_kd.legend()
    fig.set_size_inches(18, 12)
    plt.savefig(f"plots/drill_results.png")
    plt.close()


def plot_csv(filename: str):

    if not os.path.exists(filename):
        print(f"File not found {filename}")
        sys.exit(1)

        # Remove dir and extension from filename
    basename = os.path.splitext(os.path.basename(filename))[0]
    parts = basename.split('_')
    goal = None
    if len(parts) == 6:
        runSecs, warmUpMs, goal, kp, ki, kd = parts
        # title = f"kp: {parts[0]} ki: {parts[1]} kd: {parts[2]}"
        title = f"warmUpMs: {warmUpMs} goal: {goal} kp: {kp} ki: {ki} kd: {kd}"
    else:
        title = basename

    if basename.lower() == "drill_results":
        plot_drill_results()
        return

    headers, array = arr_from_csv(filename)
    fig, (ax1, ax2) = plt.subplots(2, 1)

    plt.suptitle(title)

    x = None
    time_header = 'Time (ms)'
    if time_header in headers:
        ax1.set_xlabel(time_header)
        ax2.set_xlabel(time_header)
        idx = headers.index(time_header)
        x = array[:, idx]
        array = np.delete(array, idx, 1)
        headers.remove(time_header)

    if x is not None:
        if goal is not None:
            ax1.plot(x, [int(goal)] * array.shape[0], label='goal', linestyle='--', color='green')
        ax2.plot(x, [0] * array.shape[0], label='zero', linestyle='--', color='green')

    # Plot the data
    for i, h in enumerate(headers):
        # Define line name and data
        # print(array[:5, i])
        if h.lower() in ['output', 'p', 'i', 'd']:
            ax2.plot(x, array[:, i], label=h) if x is not None else ax2.plot(array[:, i], label=h)
        elif (any(x in h.lower() for x in ['theory'])):
            ax1.plot(x, array[:, i], label=h,
                     linestyle='--') if x is not None else ax1.plot(array[:, i], label=h, linestyle='--')
        elif 'faucet' in h.lower():
            ax3 = ax1.twinx()
            ax3.plot(x, array[:, i], label=h, color='red')
        else:
            ax1.plot(x, array[:, i], label=h) if x is not None else ax1.plot(array[:, i], label=h)

    # Show the plot
    ax1.legend()
    ax2.legend()
    fig.set_size_inches(18, 12)

    plt.savefig(f"plots/{basename}.png")
    # plt.show()
    plt.close()


# Check if dir exsists, if not create
if not os.path.exists('plots'):
    os.makedirs('plots')

# Plot a single file
if len(sys.argv) > 1:
    plot_csv(sys.argv[1])
    sys.exit(0)
else:
    print("No file specified, plotting all files in csv directory")
    # Plot all files in csv directory
    for file in os.listdir('csv'):
        if file.endswith(".csv"):
            plot_csv(f'csv/{file}')
            # break
