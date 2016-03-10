import matplotlib.pyplot as plt

f = open("refed_count_count.txt", "r")
line = f.readline()

while line:
    line = line.split(",")
    y = line[0]
    x = line[1]
    plt.figure(1)
    plt.plot(x, y, "r.")
    line = f.readline()

plt.xlabel("refed_count")
plt.ylabel("refed_count_count")
plt.show()
