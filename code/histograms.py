import os
import matplotlib.pyplot as plt
from matplotlib.widgets import Button

FILES = [
    ("results/t250.dat",   250_000),
    ("results/t500.dat",   500_000),
    ("results/t1000.dat", 1_000_000),
    ("results/t10000.dat", 10_000_000),
    ("results/tnicelow.dat", 1_000_000),
    ("results/tnicehigh.dat", 1_000_000),
    ("results/tcpuloop.dat", 1_000_000),
    ("results/ttriple.dat", 1_000_000),
]

def load_data(filename):
    data = []
    with open(filename, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            try:
                data.append(float(line))
            except ValueError:
                pass
    return data

datasets = []
for filename, expected in FILES:
    if not os.path.exists(filename):
        continue
    data = load_data(filename)
    if not data:
        continue

    datasets.append({
        "name": os.path.basename(filename),
        "data": [x / 1000.0 for x in data],   # ns -> µs
        "expected": expected / 1000.0,
        "mean": sum(data) / len(data) / 1000.0,
    })

if not datasets:
    raise SystemExit("No data found")

index = 0
fig, ax = plt.subplots(figsize=(10, 6))
plt.subplots_adjust(bottom=0.2)

def draw():
    ax.clear()
    ds = datasets[index]

    ax.hist(
        ds["data"],
        bins=40,
        edgecolor="black",
        linewidth=0.8,
        alpha=0.85
    )

    ax.axvline(ds["expected"], linestyle="--", linewidth=2,
               label=f"Expected = {ds['expected']:.2f} µs")
    ax.axvline(ds["mean"], linestyle=":", linewidth=2,
               label=f"Mean = {ds['mean']:.2f} µs")

    ax.set_title(f"Histogram of measured intervals - {ds['name']} ({index + 1}/{len(datasets)})")
    ax.set_xlabel("Measured interval (µs)")
    ax.set_ylabel("Frequency")
    ax.grid(True, linestyle="--", linewidth=0.5, alpha=0.6)

    xmin = min(ds["data"])
    xmax = max(ds["data"])
    margin = max((xmax - xmin) * 0.1, 1e-6)
    ax.set_xlim(xmin - margin, xmax + margin)

    ax.legend()
    fig.canvas.draw_idle()

def next_plot(event):
    global index
    index = (index + 1) % len(datasets)
    draw()

def prev_plot(event):
    global index
    index = (index - 1) % len(datasets)
    draw()

# Button axes
ax_prev = plt.axes([0.2, 0.05, 0.2, 0.075])
ax_next = plt.axes([0.6, 0.05, 0.2, 0.075])

# Keep references to the buttons
btn_prev = Button(ax_prev, "Previous")
btn_next = Button(ax_next, "Next")

btn_prev.on_clicked(prev_plot)
btn_next.on_clicked(next_plot)

draw()
plt.show()