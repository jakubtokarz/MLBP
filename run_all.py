import os
import subprocess
from itertools import takewhile

all_MLBP_instances = os.listdir('inst\\mlbp')

opt_count = 0
fea_count = 0
idk_count = 0
for inst in all_MLBP_instances:
    input_str = ".\\x64\\Debug\\MLBP.exe ifile inst\\mlbp\\%s prob MLBP ttime 60" % inst

    try:
        res = subprocess.run(input_str, shell=True, capture_output=True, text=True) #, timeout=1
        output = res.stdout
        err = res.stderr
        # if err:
        #     print("Error in " + inst + " -> " + err)
        #     # break

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
                continue
            else:
                raise ValueError("Status: " + status + " in " + inst)

            a2 = output[output.index('best objective value:') + len('best objective value:'):]
            score = a2[:a2.index('\n')].strip()

            print(inst + " -> {}: {}".format(status, score))
        except ValueError:
            print(output)

    except subprocess.TimeoutExpired:
        break


print("Optimal: " + str(opt_count) + " | " + "Feasible: " + str(fea_count) + " | " + "Unknown: " + str(idk_count))
print(str(opt_count + fea_count) + '/' + str(len(all_MLBP_instances)))