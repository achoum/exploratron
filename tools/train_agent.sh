set -vex

FLAG="--config=linux --define=terminal=linux_console"

#MODE="-c dbg"
MODE="-c opt"

#bazel run $MODE //exploratron/cli:evaluate_main $FLAG
#bazel run $MODE //exploratron/cli:evaluate_main $FLAG -- --controller_key=Keyboard
bazel run $MODE //controller/genetic:train_main $FLAG
