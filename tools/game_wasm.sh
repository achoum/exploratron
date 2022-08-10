set -vex

FLAG="--config=wasm --define=terminal=wasm_console"

#MODE="-c dbg"
MODE="-c opt"

bazel --output_base=/mnt/c/tmp/bazel build $MODE //exploratron/web:pack_game $FLAG

# Start a local http server for the game.
rm -fr compiled_web_game
cp -R bazel-bin/exploratron/web/compiled_web_game compiled_web_game
( cd compiled_web_game && ./start_server.sh )
