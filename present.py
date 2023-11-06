import matplotlib.pyplot as plt
import csv
import ast
import numpy as np

coordinates = []

with open("output.csv",newline="") as file:
        read = csv.reader(file, delimiter=";")
        readList = [row for row in read]

m = len(readList[0])
n = len(readList)
vectors = [ast.literal_eval(e) for row in readList for e in row]
print(len(vectors))

# Generate coordinates and vectors using a loop

target_num_coordinates = n * m

for x in range(-n//2, n//2 + 1):
    for y in range(-m//2, m//2 + 1):
        coordinates.append([x, y])

        # Check if  reached the desired number of coordinates
        if len(coordinates) >= target_num_coordinates:
            break

    if len(coordinates) >= target_num_coordinates:
        break

print(len(coordinates))


# initiate plot
plt.figure(figsize=(18, 12))

# plotting arrow shapes as vectors
for i in range(len(coordinates)):
    dx = vectors[i][0] / 5  # Scale down by 1/5
    dy = vectors[i][1] / 5  # Scale down by 1/5
    plt.arrow(coordinates[i][0], coordinates[i][1], dx, dy,
              head_width=0.1, head_length=0.1, fc='red', ec='red')


# formal stuffs
plt.xlim(-10, 10)
plt.ylim(-7, 7)
plt.xlabel('X-axis')
plt.ylabel('Y-axis')
plt.title('Vector Plot')

#show me
plt.grid()
plt.show()