::bazel clean --expunge

::set FLAG=--config=windows_clang --compiler=clang-cl --define=terminal=windows_console
set FLAG=--config=windows_vs --define=terminal=windows_console

:: Other items to add to FLAG
:: set BAZEL_LLVM=C:\Program Files\LLVM
:: set BAZEL_VC=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC
::
:: Window build options:
:: --repo_env=CC=clang
:: --repo_env=CC=clang -- --output=a
:: --compiler=clang-cl
:: --compiler=mingw-gcc
:: --compiler=cygwin-gcc

set MODE=-c dbg
::set MODE=-c opt

::bazel run %MODE% //exploratron/cli:evaluate_main %FLAG%
bazel run %MODE% //exploratron/cli:evaluate_main %FLAG% -- --controller_key=Keyboard
::bazel run %MODE% //controller/genetic:train_main %FLAG%
