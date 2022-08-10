set -vex

#FLAG="--config=linux --define=terminal=linux_console_ncuses"
FLAG="--config=linux --define=terminal=linux_console"

MODE="-c dbg"
#MODE="-c dbg --copt=-O0 --copt=-g"
#MODE="-c opt"

bazel build $MODE //exploratron/cli:game $FLAG
reset
bazel-bin/exploratron/cli/game
# tui