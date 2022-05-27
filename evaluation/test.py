import subprocess
from itertools import takewhile
from tqdm import tqdm

# n_trials = 5

MLBP_instances = []
MLBPCC_instances = []

n = ['05', '10']
m = [3,4]
p = [25,50]
inst = [0,1,2,3,4]

for i in n:
    for j in m:
        for l in p:
            for k in inst:
                # MLBP_instances.append("n00{}_m0{}__00{}.inst".format(i,j,k))
                MLBPCC_instances.append("n00{}_m0{}_p0{}__00{}.inst".format(i, j, l, k))

# MLBP_instances.remove("n0040_m03__001.inst")
# MLBP_instances.append("n0030_m03__000.inst") #for reference
# print(MLBPCC_instances)

t = 0
filename = "ticks_MLBPCC_MLBP.txt".format()
file = open(filename, "a")

result="-------------------------------\n"
file.write(result)
# result = "n_trials = " + str(n_trials) + '\n'


for inst in tqdm(MLBPCC_instances):
    # input_str = "..\\x64\\Debug\\MLBP.exe ifile ..\\inst\\mlbp\\{} prob MLBP ttime {}".format(inst, t)
    input_str = "..\\x64\\Debug\\MLBP.exe ifile ..\\inst\\mlbpcc\\{} prob MLBPCC ttime {}".format(inst, t)

    # n_avg = 0
    # for i in range(n_trials):
    ticks = -1
    time = -1
    try:
        res = subprocess.run(input_str, shell=True, capture_output=True, text=True) #, timeout=1
        output = res.stdout
        err = res.stderr
        if err:
            print("Error in " + inst + " -> " + err)
        #     # break
        try:
            a1 = output[output.index('CPLEX status:') + len('CPLEX status:'):]
            b = a1[:a1.index('\n')].strip()
            status = ''.join(takewhile(lambda x: str.isalpha(x), b))

            if status == 'Optimal':
                a3 = output[output.index('CPU time:') + len('CPU time:'):]
                time = a3[:a3.index('\n')].strip()
                # n_avg += float(time) ## ???
                a4 = output[output.index('Total (root+branch&cut)') + len('Total (root+branch&cut)'):]
                a5 = a4[a4.index('(')+1 : a4.index(' ticks)')]
                ticks = float(a5)
                # print(a5)
            else:
                raise ValueError("Status: " + status + " in " + inst)
        except ValueError as a:
            print(output)
            result += output + '\n'

    except subprocess.TimeoutExpired:
        break

    # print(inst + " -> Time: {}".format(n_avg / n_trials))
    result += inst + " -> Ticks: {} | Time: {}".format(ticks, time) + '\n'
    file.write(inst + " -> Ticks: {} | Time: {}".format(ticks, time) + '\n')
print(result)
# print("Optimal: " + str(opt_count) + " | " + "Feasible: " + str(fea_count) + " | " + "Unknown: " + str(idk_count))
# print(str(opt_count + fea_count) + '/' + str(len(MLBP_instances)))
# result += "Optimal: " + str(opt_count) + " | " + "Feasible: " + str(fea_count) + " | " + "Unknown: " + str(idk_count) + '\n'
# result += str(opt_count + fea_count) + '/' + str(len(MLBP_instances)) + '\n'

# file = open("test_MLBP_sym_{}.txt".format(t), "a")
# file.write(result)
file.close()