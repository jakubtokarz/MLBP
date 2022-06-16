import matplotlib.pyplot as plt

file_a = "f_ticks_MLBP_all.txt"
file_b = "f_ticks_MLBP_NF_all.txt"
n = 250

# file_a = "f_ticks_MLBPCC_all_t60.txt"
# file_b = "f_ticks_MLBPCC_NF_all_t60.txt"
# n = 1100

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

        if "Unknown" in line_a and "Unknown" in line_b:
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


    if a_not_unknown > 0 and avg_a_ticks > 0 and avg_b_ticks > 0:
        avg_a_ticks /= a_not_unknown
        res_a_ticks.append(avg_a_ticks)
    if b_not_unknown > 0 and avg_a_ticks > 0 and avg_b_ticks > 0:
        avg_b_ticks /= b_not_unknown
        res_b_ticks.append(avg_b_ticks)

    if a_not_unknown > 0 and avg_a_bnb > 0 and avg_b_bnb > 0:
        avg_a_bnb /= a_not_unknown
        res_a_bnb.append(avg_a_bnb)
    if b_not_unknown > 0 and avg_a_bnb > 0 and avg_b_bnb > 0:
        avg_b_bnb /= b_not_unknown
        res_b_bnb.append(avg_b_bnb)

    print(curr + " ->", avg_a_ticks, "|", avg_b_ticks, "| bnb ->", avg_a_bnb, "|", avg_b_bnb)


significant = 134 if n == 1100 else 47

plt.plot(res_a_ticks[:significant], label="Model 1")
plt.plot(res_b_ticks[:significant], label="Model 2")
plt.legend()
plt.yscale('symlog')
plt.xlabel('Instance')
plt.ylabel('Number of Ticks')
if n == 1100:
    plt.xlim(-10, 140)
else:
    plt.xlim(-2, 50)
plt.show()

plt.plot(res_a_bnb[:significant], label="Model 1")
plt.plot(res_b_bnb[:significant], label="Model 2")
plt.legend()
plt.yscale('symlog')
plt.xlabel('Instance')
plt.ylabel('Number of BnB nodes')
if n == 1100:
    plt.xlim(-10, 140)
else:
    plt.xlim(-2, 50)
plt.show()
