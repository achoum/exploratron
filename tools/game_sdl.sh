set -vex

FLAG="--config=linux --define=terminal=sdl_console"

MODE=
#MODE="-c dbg"
#MODE="-c dbg --copt=-O0 --copt=-g"
#MODE="-c opt"

bazel build $MODE //exploratron/cli:game $FLAG
reset
bazel-bin/exploratron/cli/game

# tui