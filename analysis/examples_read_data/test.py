#!/usr/bin/python3

# VMM Analysis
# --------------------------------------------------------------------
# This script is a simple example, showing how to read the data from a
# ROOT tree, generated with vmm-sdat. In addition, some cuts are
# applied to the data using pandas. In this specific example, this
# means that only the position of the clusters is plotted, if the
# ADC value of the cluster is larger than a specific value.
# --------------------------------------------------------------------
# Lucian Scharenberg
# lucian.scharenberg@cern.ch
# 18 November 2019 and 03 March 2022


# --------------------------------------------------------------------
# PACKAGE HANDLING
# --------------------------------------------------------------------

import uproot3 as uproot
import pandas as pd
import matplotlib.pyplot as plt
import sys


# --------------------------------------------------------------------
# DATA HANDLING
# --------------------------------------------------------------------

# Get the tree in the ROOT file
tree = uproot.open("./ps_runs.root")['PKUP']

# Now get the branches of interest
PulseIntensity = tree.array('PulseIntensity')
psTime = tree.array('psTime')


# Create a pandas data frame, which is used to apply the cuts
data = {'PulseIntensity': PulseIntensity,
        'psTime': psTime}
df = pd.DataFrame(data)

# Get only the events, which 'survive' the cut
events = df.query('psTime >= 0 and psTime <= 1730817717000000000')



# --------------------------------------------------------------------
# PLOT THE DATA
# --------------------------------------------------------------------

# Create the plot of the positions
plt.hist(events['PulseIntensity'], bins=1000, range=(0,1e13))

# Some labels
plt.xlabel('Position 0 (Strip Numbers)')
plt.xlabel('Position 1 (Strip Numbers)')

# Save the plot and show it
plt.savefig('tree-cutter.png', dpi = 500)
plt.show()
