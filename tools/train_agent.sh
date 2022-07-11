set -vex
reset

FLAG="--config=linux --define=terminal=linux_console"

#MODE="-c dbg"
MODE="-c opt"

#bazel run $MODE //exploratron/cli:evaluate_main $FLAG
#bazel run $MODE //exploratron/cli:evaluate_main $FLAG -- --controller_key=Keyboard
#bazel run $MODE //exploratron/controller/genetic:train_main $FLAG
bazel run $MODE //exploratron/controller/external_optimizer:train_main $FLAG -- \
  --training_log_base=/mnt/d/projects/exploratron/training_logs/evo/rng_wall_
