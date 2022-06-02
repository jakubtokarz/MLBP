import matplotlib.pyplot as plt

file_a = "ticks_MLBP_all.txt"

n = 250
avg = 0
curr = ""
res = []

for i in range(n//5):
    for j in range(5):
        line_a = open(file_a, "r").readlines()[i*5+j]
        ticks_a = float(line_a[line_a.index("Ticks: ") + len("Ticks: "): line_a.index("|")].strip())
        curr = line_a[:line_a.index("__")]
        avg += ticks_a
        # print(i,j)
    avg /= 5
    res.append(avg)
    print(curr + " ->", avg)

plt.plot(res)
plt.yscale('log')
plt.show()
