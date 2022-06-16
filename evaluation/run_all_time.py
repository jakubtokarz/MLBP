import subprocess
from itertools import takewhile
from tqdm import tqdm
import os

# all_MLBPCC_instances = os.listdir('..\\inst\\mlbpcc')
all_MLBP_instances = os.listdir('..\\inst\\mlbp')

t = 0
filename = "ticks_MLBP_all.txt".format()
file = open(filename, "a")

result = "-------------------------------\n"
file.write(result)


for inst in tqdm(all_MLBP_instances):
    input_str = "..\\x64\\Debug\\MLBP.exe ifile ..\\inst\\mlbp\\{} prob MLBP ttime {}".format(inst, t)
    # input_str = "..\\x64\\Debug\\MLBP.exe ifile ..\\inst\\mlbpcc\\{} prob MLBPCC ttime {}".format(inst, t)

    ticks = -1
    time = -1
    bnb_nodes = -1
    try:
        res = subprocess.run(input_str, shell=True, capture_output=True, text=True)
        output = res.stdout
        err = res.stderr
        if err:
            print("Error in " + inst + " -> " + err)

        try:
            a1 = output[output.index('CPLEX status:') + len('CPLEX status:'):]
            a2 = a1[:a1.index('\n')].strip()
            status = ''.join(takewhile(lambda x: str.isalpha(x), a2))

            if status == 'Optimal' or status == 'Feasible':
                a3 = output[output.index('CPU time:') + len('CPU time:'):]
                time = a3[:a3.index('\n')].strip()

                a4 = output[output.index('Total (root+branch&cut)') + len('Total (root+branch&cut)'):]
                a5 = a4[a4.index('(') + 1: a4.index(' ticks)')]
                ticks = float(a5)

                a6 = output[output.index('Branch-and-Bound nodes:') + len('Branch-and-Bound nodes:'):]
                a7 = a6[:a6.index('\n')].strip()
                bnb_nodes = int(''.join(takewhile(lambda x: str.isnumeric(x), list(a7))))

                if status == 'Feasible':
                    result += inst + " -> Ticks: {} | Time: {} | BnB nodes: {} | Status: {}".format(ticks, time, bnb_nodes, status) + '\n'
                    file.write(inst + " -> Ticks: {} | Time: {} | BnB nodes: {} | Status: {}".format(ticks, time, bnb_nodes, status) + '\n')
                else:
                    result += inst + " -> Ticks: {} | Time: {} | BnB nodes: {}".format(ticks, time, bnb_nodes) + '\n'
                    file.write(inst + " -> Ticks: {} | Time: {} | BnB nodes: {}".format(ticks, time, bnb_nodes) + '\n')
            else:
                result += inst + " -> Status: {}".format(status) + '\n'
                file.write(inst + " -> Status: {}".format(status) + '\n')

        except ValueError as a:
            print(output)
            print(a)
            result += str(a) + '\n'

    except subprocess.TimeoutExpired:
        break

print(result)
file.close()
