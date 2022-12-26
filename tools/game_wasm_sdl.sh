set -vex

FLAG="--config=wasm --define=terminal=sdl_wasm_console"

MODE="--jobs=1"
#MODE="-c dbg"
#MODE="-c opt"

#bazel --output_base=/tmp/bazel clean --expunge
# --output_base=/home/achoum/tmp/bazel 
bazel build $MODE --verbose_failures $FLAG //exploratron/web:pack_game_sdl --keep_going
#bazel --output_base=/home/achoum/tmp/bazel build $MODE --verbose_failures $FLAG //exploratron/core/utils:terminal_sdl_wasm


# --output_base=/mnt/c/tmp/bazel

# Start a local http server for the game.
rm -fr compiled_web_game
cp -R bazel-bin/exploratron/web/compiled_web_game compiled_web_game
( cd compiled_web_game && ./start_server.sh )
