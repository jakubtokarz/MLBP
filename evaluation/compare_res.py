

file_a = "ticks_MLBP_raw_1-new.txt"
file_b = "ticks_NF_new_1-new.txt"
# file_b = "ticks_MLBP_raw_1-new.txt"


n = 45
count_a = 0
count_b = 0
avg_a_win = 0
avg_b_win = 0
avg_total = 0

for i in range(n):
    try:
        line_a = open(file_a, "r").readlines()[i]
        ticks_a = float(line_a[line_a.index("Ticks: ") + len("Ticks: ") : line_a.index("|")].strip())

        line_b = open(file_b, "r").readlines()[i]
        ticks_b = float(line_b[line_b.index("Ticks: ") + len("Ticks: "): line_b.index("|")].strip())

        # print("ticks_a", ticks_a)
        # print("ticks_b", ticks_b)
        diff_percent = ticks_a / ticks_b
        avg_total += diff_percent
        if ticks_a < ticks_b:
            count_a+=1
            # diff_percent =
            avg_a_win += diff_percent
        elif ticks_a > ticks_b:
            count_b+=1
            # diff_percent = ((ticks_b) / ticks_a)
            avg_b_win+=diff_percent
        print("line", i+1, "(a is this fraction of b):", str(round(diff_percent,2)) + " ->", ticks_a,"|", ticks_b)
    except ValueError as a:
        print(a)
        break

assert count_a + count_b == n

avg_a_win /= count_a
avg_b_win /= count_b
avg_total /= n

print("-----")
print(file_a, count_a, "-", "avg_win", str(round(avg_a_win,2)))
print(file_b, count_b, "-", "avg_win", str(round(avg_b_win,2)))
print("average a of b:", round(avg_total,2))

