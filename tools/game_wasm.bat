set FLAG=--config=wasm --define=terminal=wasm_console --symlink_prefix=wasm_bazel/

::set MODE=-c dbg
set MODE=-c opt

:: Does not work. Something about file collision.
bazel build %MODE% //exploratron/cli:game_wasm %FLAG%

if %errorlevel% neq 0 exit /b %errorlevel%
echo "====================================="


