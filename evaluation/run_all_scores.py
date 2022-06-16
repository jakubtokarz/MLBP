import os
import subprocess
from itertools import takewhile
from tqdm import tqdm

# all_MLBP_instances = os.listdir('..\\inst\\mlbp')
all_MLBPCC_instances = os.listdir('..\\inst\\mlbpcc')
# aa = list(all_MLBPCC_instances)[::-1]
# print(all_MLBPCC_instances)

t = 60  # 1 min timout
opt_count = 0
fea_count = 0
idk_count = 0

filename = "scores_MLBPCC_NF_{}s.txt".format(t)
file = open(filename, "a")

result = "-------------------------------\n"
file.write(result)

for inst in tqdm(all_MLBPCC_instances):
    # input_str = "..\\x64\\Debug\\MLBP.exe ifile ..\\inst\\mlbp\\{} prob MLBP ttime {}".format(inst, t)
    input_str = "..\\x64\\Debug\\MLBP.exe ifile ..\\inst\\mlbpcc\\{} prob MLBPCC ttime {}".format(inst, t)
    error = False

    try:
        res = subprocess.run(input_str, shell=True, capture_output=True, text=True)
        output = res.stdout
        err = res.stderr
        if err:
            if "Advanced basis not built." not in err:
                error = True
                print("Error in " + inst + " -> " + err)
                # break

        try:
            a1 = output[output.index('CPLEX status:') + len('CPLEX status:'):]
            b = a1[:a1.index('\n')].strip()
            status = ''.join(takewhile(lambda x: str.isalpha(x), b))

            if status == 'Optimal':
                opt_count += 1
            elif status == 'Feasible':
                fea_count += 1
            elif status == 'Unknown':
                idk_count += 1
                print(inst + " -> " + status)
                result += inst + " -> " + status + ':\n'
                file.write(inst + " -> " + status + ':\n')
                continue
            else:
                raise ValueError("Status: " + status + " in " + inst)

            a2 = output[output.index('best objective value:') + len('best objective value:'):]
            score = a2[:a2.index('\n')].strip()

            result += inst + " -> {}: {}".format(status, score) + '\n'
            file.write(inst + " -> {}: {}".format(status, score) + '\n')

            if error:
                print("There was an error not related to Unknown status")
                break

        except ValueError:
            print(output)
            result += output + '\n'

    except subprocess.TimeoutExpired:
        break

result += "Optimal: " + str(opt_count) + " | " + "Feasible: " + str(fea_count) + " | " + "Unknown: " + str(idk_count) + '\n'
result += str(opt_count + fea_count) + '/' + str(len(all_MLBPCC_instances)) + '\n'
file.write("Optimal: " + str(opt_count) + " | " + "Feasible: " + str(fea_count) + " | " + "Unknown: " + str(idk_count) + '\n')
file.write(str(opt_count + fea_count) + '/' + str(len(all_MLBPCC_instances)) + '\n')

print(result)
file.close()
