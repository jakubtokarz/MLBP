import os
import subprocess
from itertools import takewhile
from tqdm import tqdm

n_trials = 10

MLBP_instances = []
n = [20,30]
m = [3]
inst = [0,1,2,3,4]

for i in n:
    for j in m:
        for k in inst:
            MLBP_instances.append("n00{}_m0{}__00{}.inst".format(i,j,k))

t = 0
opt_count = 0
fea_count = 0
idk_count = 0
result = ""

for inst in tqdm(MLBP_instances):
    input_str = ".\\x64\\Debug\\MLBP.exe ifile inst\\mlbp\\{} prob MLBP ttime {}".format(inst, t)

    n_avg = 0
    for i in range(n_trials):
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
                    n_avg += float(time) ## ???
                else:
                    raise ValueError("Status: " + status + " in " + inst)
            except ValueError:
                print(output)
                result += output + '\n'

        except subprocess.TimeoutExpired:
            break

    # print(inst + " -> Time: {}".format(n_avg / n_trials))
    result += inst + " -> Time: {}".format(n_avg / n_trials) + '\n'
print(result)
# print("Optimal: " + str(opt_count) + " | " + "Feasible: " + str(fea_count) + " | " + "Unknown: " + str(idk_count))
# print(str(opt_count + fea_count) + '/' + str(len(MLBP_instances)))
# result += "Optimal: " + str(opt_count) + " | " + "Feasible: " + str(fea_count) + " | " + "Unknown: " + str(idk_count) + '\n'
# result += str(opt_count + fea_count) + '/' + str(len(MLBP_instances)) + '\n'

file = open("test_{}.txt".format(t), "w")
file.write(result)
file.close()