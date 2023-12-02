import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Read the CSV file
df = pd.read_csv('output_field_vel.csv', header=None) 

# Convert data from csv to NumPy array
data = np.flipud(df.to_numpy() )

 # Create a heatmap
plt.imshow(data, cmap='viridis', interpolation='nearest') 
plt.colorbar()  
plt.show()