set -vex

#FLAG="--config=linux --define=terminal=linux_console_ncuses"
FLAG="--config=linux --define=terminal=linux_console"

MODE="-c dbg"
#MODE="-c opt"

bazel build $MODE //exploratron/cli:game $FLAG
bazel-bin/exploratron/cli/game
