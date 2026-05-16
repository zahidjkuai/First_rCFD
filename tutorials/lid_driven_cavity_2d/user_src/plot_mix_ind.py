"""
plot_mix_ind.py
Plots Mixing Index vs Time for MIX1 and MIX2.
Run from the folder that contains rCFD_mix_ind.out
Output: mixing_index_comparison.png
"""

import matplotlib.pyplot as plt
import pandas as pd
import os

FILENAME = "rCFD_mix_ind.out"

if not os.path.exists(FILENAME):
    print(f"ERROR: {FILENAME} not found in {os.getcwd()}")
    exit(1)

# columns: time  mix_ind1  mix_ind2
df = pd.read_csv(FILENAME, sep=r'\s+', header=None)

time      = df[0]
mix_small = df[1]
mix_large = df[2]

fig, ax = plt.subplots(figsize=(9, 5.5))

ax.plot(time, mix_small, color='#1f77b4', linewidth=2.5,
        label='MIX1  —  Small Diffusivity (0.0001)')
ax.plot(time, mix_large, color='#ff7f0e', linewidth=2.5,
        label='MIX2  —  Large Diffusivity (0.03)')

ax.set_title('Mixing Index Comparison over Time', fontsize=14,
             fontweight='bold', pad=12)
ax.set_xlabel('Time [s]', fontsize=12)
ax.set_ylabel('Mixing Index  [ - ]', fontsize=12)
ax.set_ylim(-0.05, 1.05)
ax.grid(True, linestyle='--', alpha=0.5)
ax.legend(loc='best', fontsize=11)

plt.tight_layout()

OUT = "mixing_index_comparison.png"
plt.savefig(OUT, dpi=300, bbox_inches='tight')
print(f"Saved: {OUT}")
