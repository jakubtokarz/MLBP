
file_a = "f_scores_MLBP_600s.txt"
file_b = "f_scores_MLBP_NF_600s.txt"
n = 275

# file_a = "f_scores_MLBPCC_60s.txt"
# file_b = "f_scores_MLBPCC_NF_60s.txt"
# n = 1100

same_count = 0
better_status_a = 0
better_status_b = 0
fea_count_a = 0
fea_count_b = 0

def extract_score_and_status(file):
    lines = open(file, "r").readlines()[i]
    status = lines[lines.index("->") + len("->"): lines.index(":")].strip()
    score = -1
    if status != "Unknown":
        score = int(lines[lines.index(":") + 1:].strip())

    return status, score


for i in range(n):
    try:
        line_a = open(file_a, "r").readlines()[i]
        curr_inst = line_a[:line_a.index("->")]

        status_a, score_a = extract_score_and_status(file_a)
        status_b, score_b = extract_score_and_status(file_b)

        if status_a == status_b:
            if status_a == "Optimal":
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

        print(curr_inst + "-> a:", status_a, score_a, "| b:", status_b, score_b,)

    except ValueError as a:
        print(a)
        break

assert same_count + better_status_a + better_status_b + fea_count_a + fea_count_b == n

print("-----")
print(file_a + " = a |", file_b + " = b")
print("same_count:", same_count)
print("better_status_a:", better_status_a, "|", "better_status_b:", better_status_b)
print("Both Feasible: a better -", fea_count_a, "| b better -", fea_count_b)
