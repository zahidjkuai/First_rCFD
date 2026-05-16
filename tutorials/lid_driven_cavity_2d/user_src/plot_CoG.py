"""
plot_CoG.py
Plots Centre of Gravity (CoG) trajectories for MIX1 and MIX2.
Run from the folder that contains rCFD_CoG.out
Output: diffusivity_comparison.png
"""

import matplotlib.pyplot as plt
import pandas as pd
import os

FILENAME = "rCFD_CoG.out"

if not os.path.exists(FILENAME):
    print(f"ERROR: {FILENAME} not found in {os.getcwd()}")
    exit(1)

# columns: time  CoG_x1  CoG_y1  CoG_z1  CoG_x2  CoG_y2  CoG_z2
df = pd.read_csv(FILENAME, sep=r'\s+', header=None)

x_small = df[1]
y_small = df[2]
x_large = df[4]
y_large = df[5]

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5.5), sharex=True, sharey=True)

# --- Left: MIX1 small diffusivity ---
ax1.plot(x_small, y_small, color='#1f77b4', linewidth=2, label='Trajectory')
ax1.scatter(x_small.iloc[0],  y_small.iloc[0],  color='green', marker='o',
            s=100, zorder=5, label='Start')
ax1.scatter(x_small.iloc[-1], y_small.iloc[-1], color='red',   marker='X',
            s=100, zorder=5, label='End')
ax1.set_title('MIX1  —  Small Diffusivity (0.0001)', fontsize=13,
              fontweight='bold', pad=12)
ax1.set_xlabel('Centre of Gravity X [m]', fontsize=12)
ax1.set_ylabel('Centre of Gravity Y [m]', fontsize=12)
ax1.grid(True, linestyle='--', alpha=0.5)
ax1.legend(loc='best', fontsize=10)
ax1.set_aspect('equal', adjustable='box')

# --- Right: MIX2 large diffusivity ---
ax2.plot(x_large, y_large, color='#ff7f0e', linewidth=2, label='Trajectory')
ax2.scatter(x_large.iloc[0],  y_large.iloc[0],  color='green', marker='o',
            s=100, zorder=5, label='Start')
ax2.scatter(x_large.iloc[-1], y_large.iloc[-1], color='red',   marker='X',
            s=100, zorder=5, label='End')
ax2.set_title('MIX2  —  Large Diffusivity (0.03)', fontsize=13,
              fontweight='bold', pad=12)
ax2.set_xlabel('Centre of Gravity X [m]', fontsize=12)
ax2.grid(True, linestyle='--', alpha=0.5)
ax2.legend(loc='best', fontsize=10)
ax2.set_aspect('equal', adjustable='box')

plt.suptitle('Centre of Gravity Trajectories', fontsize=14, fontweight='bold', y=1.02)
plt.tight_layout()

OUT = "diffusivity_comparison.png"
plt.savefig(OUT, dpi=300, bbox_inches='tight')
print(f"Saved: {OUT}")
