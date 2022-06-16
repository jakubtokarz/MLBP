from matplotlib import pyplot as plt

file_a = "f_scores_MLBPCC_60s.txt"
file_b = "f_scores_MLBPCC_NF_60s.txt"


n = 1100
same_count = 0
better_status_a = 0
better_status_b = 0
fea_count_a = 0
fea_count_b = 0

a_graph = []
b_graph = []

for i in range(n):
    try:
        line_a = open(file_a, "r").readlines()[i]
        status_a = line_a[line_a.index("->") + len("->") : line_a.index(":")].strip()
        score_a = -1
        if status_a != "Unknown":
            score_a = int(line_a[line_a.index(":") + 1:].strip())
            a_graph.append(1)
        else:
            a_graph.append(0)

        line_b = open(file_b, "r").readlines()[i]
        status_b = line_b[line_b.index("->") + len("->"): line_b.index(":")].strip()
        score_b = -1
        if status_b != "Unknown":
            score_b = int(line_b[line_b.index(":") + 1:].strip())
            b_graph.append(1)
        else:
            b_graph.append(0)

        if status_a == status_b:
            if status_a == "Optimal":
                # if score_a != score_b: ##Add back in for MLBPCC
                #     print("Both Optimal but different??")
                #     # break
                # else:
                same_count += 1
            elif status_a == "Feasible":
                if score_a < score_b:
                    fea_count_a += 1
                elif score_b < score_a:
                    fea_count_b += 1
                else:
                    same_count += 1
            else:
                same_count += 1
        elif status_a == "Optimal":
            if status_b == "Feasible":
                if score_a > score_b:
                    print("a optimal but worse score??")
                    print(status_a, score_a)
                    print(status_b, score_b)
                    break
            better_status_a += 1
        elif status_b == "Optimal":
            if status_a == "Feasible":
                if score_b > score_a:
                    print("b optimal but worse score??")
                    break
            better_status_b += 1
        elif status_a == "Feasible" and status_b == "Unknown":
            better_status_a += 1
        elif status_b == "Feasible" and status_a == "Unknown":
            better_status_b += 1

        print(line_a[:line_a.index("->")] + "-> a:", line_a[line_a.index("->" ) +2:].strip(), "| b:", line_b[line_b.index("->" ) +2:].strip())

    except ValueError as a:
        print(a)
        break

assert same_count + better_status_a + better_status_b + fea_count_a + fea_count_b == n

print("-----")
print(file_a + " = a |", file_b + " = b")
print("same_count:", same_count)
print("better_status_a:", better_status_a, "|", "better_status_b:", better_status_b)
print("Both Feasible: a better -", fea_count_a, "| b better -", fea_count_b)

# plt.plot(a_graph, label=file_a)
# # plt.plot(b_graph, label=file_b)
# plt.legend()
# # plt.yscale('log')
# plt.show()
#
#
# plt.plot(b_graph, label=file_b)
# plt.legend()
# plt.show()