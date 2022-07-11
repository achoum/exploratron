from pgpelib import PGPE
import numpy as np
import sys

sys.stdout.write(f"#solution_length?\n")
sys.stdout.flush()
solution_length = int(input())
sys.stdout.write(f"#solution_length={solution_length}\n")
sys.stdout.flush()

sys.stdout.write(f"#popsize?\n")
sys.stdout.flush()
popsize = int(input())
sys.stdout.write(f"#popsize={popsize}\n")
sys.stdout.flush()

pgpe = PGPE(
    solution_length=solution_length,   # A solution vector has the length of 5
    popsize=popsize,          # Our population size is 20

    optimizer='clipup',
    # optimizer_config=dict(
    #    max_speed=...,
    #    momentum=0.9
    # ),

    # optimizer='adam',
    # optimizer_config=dict(
    #    beta1=0.9,
    #    beta2=0.999,
    #    epsilon=1e-8
    # ),
)

while True:
    solutions = pgpe.ask()

    sys.stdout.write(f"#new_candidate\n")
    sys.stdout.flush()

    raw_solutions = ",".join([" ".join([str(y) for y in x])
                             for x in solutions])
    sys.stdout.write(raw_solutions + "\n")
    sys.stdout.flush()

    sys.stdout.write(f"#fitnesses?\n")
    sys.stdout.flush()

    raw_fitnesses = input()
    fitnesses = np.array([float(v.strip())
                         for v in raw_fitnesses.strip().split(" ")])

    # sys.stdout.write(f"#fitnesses={fitnesses} ({type(fitnesses)})\n")
    # sys.stdout.flush()

    pgpe.tell(fitnesses)

    sys.stdout.write(f"#center\n")
    sys.stdout.flush()

    raw_center = " ".join([str(x) for x in pgpe.center])
    sys.stdout.write(raw_center + "\n")
    sys.stdout.flush()

# After 1000 generations, we print the center solution.
# print(pgpe.center)
