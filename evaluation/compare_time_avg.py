import matplotlib.pyplot as plt

# file_a = "f_ticks_MLBPCC_all_t60.txt"
# file_b = "f_ticks_MLBPCC_NF_all_t60.txt"
# n = 1100

file_a = "f_ticks_MLBP_all.txt"
file_b = "f_ticks_MLBP_NF_all.txt"
n = 250


curr = ""
global avg_a_ticks
global avg_b_ticks
global avg_a_bnb
global avg_b_bnb
res_a_ticks = []
res_b_ticks = []
res_a_bnb = []
res_b_bnb = []

def extract_tick_bnb(lines, bool_a):
    ticks = float(lines[lines.index("Ticks: ") + len("Ticks: "): lines.index("|")].strip())

    feasible = "Feasible" in lines
    temp_str = lines[lines.index("BnB nodes: ") + len("BnB nodes: "):]
    end = temp_str.index('|') if feasible else len(temp_str)
    bnb_nodes = int(temp_str[:end].strip())

    if bool_a:
        global avg_a_ticks
        global avg_a_bnb
        avg_a_ticks += ticks
        avg_a_bnb += bnb_nodes
    else:
        global avg_b_ticks
        global avg_b_bnb
        avg_b_ticks += ticks
        avg_b_bnb += bnb_nodes

    return ticks, bnb_nodes


for i in range(n//5):
    avg_a_ticks = 0
    avg_b_ticks = 0
    avg_a_bnb = 0
    avg_b_bnb = 0
    curr = None
    a_not_unknown = 5
    b_not_unknown = 5
    for j in range(5):
        line_a = open(file_a, "r").readlines()[i*5+j]
        line_b = open(file_b, "r").readlines()[i * 5 + j]

        curr_inst_a = line_a[:line_a.index("__")]
        curr_inst_b = line_b[:line_b.index("__")]

        assert curr_inst_a == curr_inst_b
        curr = curr_inst_a

        if ("Unknown" in line_a and "Unknown" in line_b):
            a_not_unknown -= 1
            b_not_unknown -= 1
            continue
        elif "Unknown" in line_a:
            a_not_unknown -= 1
            continue
        elif "Unknown" in line_b:
            b_not_unknown -= 1
            continue
        else:
            ticks_a, bnb_nodes_a = extract_tick_bnb(line_a, True)
            ticks_b, bnb_nodes_b = extract_tick_bnb(line_b, False)

    # if a_not_unknown > 0 and avg_a_ticks > 0 and avg_b_ticks > 0:
    #     avg_a_ticks /= a_not_unknown
    #     avg_a_bnb /= a_not_unknown
    #     res_a_ticks.append(avg_a_ticks)
    #     res_a_bnb.append(avg_a_bnb)
    # else:
    #     res_a_ticks.append(None)
    #     res_a_bnb.append(None)
    # if b_not_unknown > 0 and avg_a_ticks > 0 and avg_b_ticks > 0:
    #     avg_b_ticks /= b_not_unknown
    #     avg_b_bnb /= b_not_unknown
    #     res_b_ticks.append(avg_b_ticks)
    #     res_b_bnb.append(avg_b_bnb)
    # else:
    #     res_b_ticks.append(None)
    #     res_b_bnb.append(None)

    if a_not_unknown > 0 and avg_a_ticks > 0 and avg_b_ticks > 0:
        avg_a_ticks /= a_not_unknown
        # avg_a_bnb /= a_not_unknown
        res_a_ticks.append(avg_a_ticks)
        # res_a_bnb.append(avg_a_bnb)
    # else:
    #     res_a_ticks.append(None)
    #     res_a_bnb.append(None)
    if b_not_unknown > 0 and avg_a_ticks > 0 and avg_b_ticks > 0:
        avg_b_ticks /= b_not_unknown
        # avg_b_bnb /= b_not_unknown
        res_b_ticks.append(avg_b_ticks)
        # res_b_bnb.append(avg_b_bnb)

    if a_not_unknown > 0 and avg_a_bnb > 0 and avg_b_bnb > 0:# and avg_a_ticks > 0 and avg_b_ticks > 0:
        avg_a_bnb /= a_not_unknown
        res_a_bnb.append(avg_a_bnb)
    # else:
    #     res_a_ticks.append(None)
    #     res_a_bnb.append(None)
    if b_not_unknown > 0 and avg_a_bnb > 0 and avg_b_bnb > 0:# and avg_a_ticks > 0 and avg_b_ticks > 0:
        avg_b_bnb /= b_not_unknown
        res_b_bnb.append(avg_b_bnb)
    # else:
    #     res_b_ticks.append(None)
    #     res_b_bnb.append(None)

    print(curr + " ->", avg_a_ticks, "|", avg_b_ticks, "| bnb ->", avg_a_bnb, "|", avg_b_bnb)

# print(len(res_a))
# print(len(res_b))

# print("res_a_ticks", res_a_ticks)
# print("res_b_ticks", res_b_ticks)
# print("res_a_bnb", res_a_bnb)
# print("res_b_bnb", res_b_bnb)

# file_a[file_a.index("_")+1:file_a.index("all")-1]
plt.plot(res_a_ticks[:47], label="Model 1")
plt.plot(res_b_ticks[:47], label="Model 2")
plt.legend()
plt.yscale('symlog')
plt.xlabel('Instance')
plt.ylabel('Number of Ticks')
# plt.xlim(-2, 50)
plt.show()

print(len(res_a_ticks))
print(len(res_b_ticks))

print(len(res_a_bnb))
print(len(res_b_bnb))

plt.plot(res_a_bnb[:47], label="Model 1")
plt.plot(res_b_bnb[:47], label="Model 2")
plt.legend()
plt.yscale('symlog')
plt.xlabel('Instance')
plt.ylabel('Number of BnB nodes')
# plt.xlim(-2, 50)
plt.show()
