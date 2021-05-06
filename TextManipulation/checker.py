from os import listdir
from os.path import isfile, join, splitext
import time, subprocess, shlex

IN_FOLDER  = "./tests/in"
OUT_FOLDER = "./tests/out"
REFS       = "./tests/ref"

SCORE = 0
serial_times = []
run_times    = []

def execute_command(command, timeout=None):
    cmd = shlex.split(command)
    subprocess.call(cmd, timeout=timeout)

def setup():
    try:
        subprocess.check_call("make")
        return True
    except:
        print("Homework does not compile")
        return False
    finally:
        print()

def get_serial_times():
    print("~~~~~~~~~~~~ ESTABLISHING BASE TIME ~~~~~~~~~~~~")
    files = [f for f in listdir(IN_FOLDER) if isfile(join(IN_FOLDER, f))]

    for f in files:
        start_time = time.time()
        execute_command("./serial " + IN_FOLDER + "/" + f)
        serial_times.append(time.time() - start_time) 
        print("Test " + f + " took " + str(serial_times[-1]) + " seconds")
        execute_command("mv ./result.out ./tests/ref/" + splitext(f)[0] + ".ref" )
        
    print()

def run_tests():
    print("~~~~~~~~~~~~ RUNNING TESTS ~~~~~~~~~~~~")
    files = [f for f in listdir(IN_FOLDER) if isfile(join(IN_FOLDER, f))]

    for f in files:
        start_time = time.time()
        timeout = int(serial_times[len(run_times)])
        timeout = max(1, 2 * timeout)
        command = "mpirun -np 5 ./main " + IN_FOLDER + "/" + f
        try:
            execute_command(command, timeout=timeout)
            run_times.append(time.time() - start_time) 
            print("Test " + f + " took " + str(run_times[-1]) + " seconds")
            execute_command("mv " + IN_FOLDER + "/" + f + ".out " + OUT_FOLDER)
        except:
            print("Test " + f + " took too long")
    print()

def cleanup():
    files = [f for f in listdir(OUT_FOLDER) if isfile(join(OUT_FOLDER, f))]
    for f in files:
        execute_command("rm " + OUT_FOLDER + "/" + f)
    
    files = [f for f in listdir(REFS) if isfile(join(REFS, f))]
    for f in files:
        execute_command("rm " + REFS + "/" + f)

def check_if_multiple_read():
    files = [f for f in listdir(IN_FOLDER) if isfile(join(IN_FOLDER, f))]
    command = "mpirun -np 5 ./main " + IN_FOLDER + "/" + files[-1] + \
                " & sleep 0.5 && lsof tests/in/input5.txt | wc -l"
    p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    output, _ = p.communicate()
    execute_command("rm " + IN_FOLDER + "/" + files[-1] + ".out ")
    if int(output) != 5:
        return False
    return True

def compare_files(f1, f2):
    try:
        command = "diff -B " + f1 + " " + f2
        subprocess.check_call(shlex.split(command), stdout=subprocess.DEVNULL)
        return True
    except:
        return False

def compute_score():
    global SCORE

    print("~~~~~~~~~~~~ COMPARING ~~~~~~~~~~~~")
    outs = [f for f in listdir(OUT_FOLDER) if isfile(join(OUT_FOLDER, f))]
    refs = [f for f in listdir(REFS) if isfile(join(REFS, f))]
    for o, r in zip(outs, refs):
        if compare_files(OUT_FOLDER + "/" + o, REFS + "/" + r):
            SCORE += 2
            print("Test " + o + " is correct\t" + "SCORE = " + str(SCORE))
        else:
            print("Test " + o + " failed\t" + "SCORE = " + str(SCORE))
    
    print("\n~~~~~~~~~~~~ DOES IT READ IN PARALLEL? ~~~~~~~~~~~~")
    if not check_if_multiple_read():
        SCORE -= 2
        print("IT DOES NOT")
    else:
        print("IT DOES")
    print("SCORE = " + str(SCORE))

    print("\n~~~~~~~~~~~~ DOES IT SCALE? ~~~~~~~~~~~~")
    scalability = True
    for i, j in zip(serial_times, run_times):
        if i > 2 and i < j:
            print(str(i) + " vs " + str(j))
            scalability = False
            break

    if scalability:
        print("IT DOES :)")
    else:
        print("IT DOES NOT :(")
        SCORE -= 2
    
    print("SCORE = " + str(SCORE))

if __name__ == "__main__":    
    if not setup():
        print("Final grade = ", SCORE)
        exit()
    
    get_serial_times()
    run_tests()
    compute_score()
    #cleanup()