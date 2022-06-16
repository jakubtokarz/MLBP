
file_b = "f_ticks_MLBPCC_NF_all_t60.txt"
file_a = "f_ticks_MLBPCC_all_t60.txt"
# file_b

n = 1100
count_ticks_a = 0
count_ticks_b = 0
count_bnb_a = 0
count_bnb_b = 0
bnb_same = 0
avg_a_win = 0
avg_b_win = 0
avg_total = 0
unknown_count = 0


def extract_tick_bnb(lines):
    ticks = float(lines[lines.index("Ticks: ") + len("Ticks: "): lines.index("|")].strip())

    feasible = "Feasible" in lines
    temp_str = lines[lines.index("BnB nodes: ") + len("BnB nodes: "):]
    end = temp_str.index('|') if feasible else len(temp_str)
    bnb = int(temp_str[:end].strip())

    return ticks, bnb


for i in range(n):
    try:
        line_a = open(file_a, "r").readlines()[i]
        line_b = open(file_b, "r").readlines()[i]
        if "Unknown" in line_a and "Unknown" in line_b:
            unknown_count += 1
        elif "Unknown" in line_a:
            count_ticks_b += 1
            count_bnb_b += 1
            continue
        elif "Unknown" in line_b:
            count_ticks_a += 1
            count_bnb_a += 1
            continue
        else:
            ticks_a, bnb_a = extract_tick_bnb(line_a)
            ticks_b, bnb_b = extract_tick_bnb(line_b)

            diff_percent = ticks_a / ticks_b
            avg_total += diff_percent
            if ticks_a < ticks_b:
                count_ticks_a += 1
                avg_a_win += diff_percent
            elif ticks_a > ticks_b:
                count_ticks_b += 1
                avg_b_win += diff_percent

            if bnb_a < bnb_b:
                count_bnb_a += 1
            elif bnb_b < bnb_a:
                count_bnb_b += 1
            else:
                bnb_same += 1

            print(line_a[:line_a.index("->")] + "-> (a is this fraction of b):", str(round(diff_percent, 5)) + " ->", ticks_a, "|", ticks_b, "|||",
                  bnb_a, "|", bnb_b)

    except ValueError as a:
        print(a)
        break

assert count_ticks_a + count_ticks_b + unknown_count == n == count_bnb_a + count_bnb_b + bnb_same + unknown_count

if avg_a_win != 0:
    avg_a_win /= count_ticks_a
if avg_b_win != 0:
    avg_b_win /= count_ticks_b
if avg_total != 0:
    avg_total /= n

print("-----")
print("Both Unknown:", unknown_count)
print(file_a, count_ticks_a, "-", "avg_win", str(round(avg_a_win, 5)))
print(file_b, count_ticks_b, "-", "avg_win", str(round(avg_b_win, 5)))
print("average a of b:", round(avg_total, 15))
print("---")  # Keep bnb table comp???
print("Same:", bnb_same + unknown_count)
print(file_a, count_bnb_a)
print(file_b, count_bnb_b)

