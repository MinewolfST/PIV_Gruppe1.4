import matplotlib.pyplot as plt
import csv
import ast
import numpy as np

coordinates = []


# Reading File
suffix = input("output suffix: ")
filename = "output" + suffix + ".csv"

with open(filename,newline="") as file:
        read = csv.reader(file, delimiter=";")
        readList = [row for row in read]

n = len(readList[0])
m = len(readList)
print ((n,m))
vectors = [ast.literal_eval(e) for row in readList for e in row]
print(len(vectors))


# Generate coordinates for vectors

target_num_coordinates = n * m

for y in range(0, m):
    for x in range(0, n):
        coordinates.append([x, y])
        # Check if  reached the desired number of coordinates
        if len(coordinates) >= target_num_coordinates:
            break

    if len(coordinates) >= target_num_coordinates:
        break

print(len(coordinates))


# initiate plot
plt.figure(figsize=(n, m))

# plotting arrow shapes as vectors
for i in range(len(coordinates)):
    dx = vectors[i][0] / 5  # Scale down by 1/5
    dy = vectors[i][1] / 5  # Scale down by 1/5
    plt.arrow(coordinates[i][0], coordinates[i][1], dx, dy,
              head_width=0.1, head_length=0.1, fc='red', ec='red')


# formal stuffs
plt.xlim(-1, n+1)
plt.ylim(-1,m+1)
plt.xlabel('X-axis')
plt.ylabel('Y-axis')
plt.title('Vector Plot')

#show me
plt.grid()
plt.show()