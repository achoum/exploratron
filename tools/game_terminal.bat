set FLAG=--config=windows_clang --compiler=clang-cl --define=terminal=windows_console
::set FLAG=--config=windows_vs --define=terminal=windows_console

set MODE=-c dbg
::set MODE=-c opt

bazel build %MODE% //exploratron/cli:game %FLAG%

if %errorlevel% neq 0 exit /b %errorlevel%
echo "====================================="

bazel-bin\exploratron\cli\game "--map=robot_city_2.tmx"
