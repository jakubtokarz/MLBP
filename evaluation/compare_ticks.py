

file_b = "ticks_MLBPCC_MLBP_Ilo_eq.txt"
file_a = "ticks_MLBPCC_MLBP_Ilo_eq_y.txt"
# file_b = "ticks_MLBP_raw_1-new.txt"


n = 17
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
            count_a += 1
            avg_a_win += diff_percent
        elif ticks_a > ticks_b:
            count_b += 1
            avg_b_win += diff_percent
        print(line_a[:line_a.index("->")] + "-> (a is this fraction of b):", str(round(diff_percent,5)) + " ->", ticks_a,"|", ticks_b)
        # print(diff_percent)
        # print("a", count_a)
        # print("b", count_b)
        # break
    except ValueError as a:
        print(a)
        break

assert count_a + count_b == n

if avg_a_win != 0: avg_a_win /= count_a
if avg_b_win != 0: avg_b_win /= count_b
if avg_total != 0: avg_total /= n

print("-----")
print(file_a, count_a, "-", "avg_win", str(round(avg_a_win,5)))
print(file_b, count_b, "-", "avg_win", str(round(avg_b_win,5)))
print("average a of b:", round(avg_total,15))

