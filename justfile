# --- defaults ----
default_mode := "debug"
cc := "clang"
cxx := "clang++"

configure mode=default_mode:
  cmake -S . -B build-{{ mode }} \
    -DCMAKE_BUILD_TYPE={{ if mode == "release" { "Release" } \
                          else if mode == "relwithdebinfo" { "RelWithDebInfo" } \
                          else if mode == "minsizerel" { "MinSizeRel" } \
                          else { "Debug" } }} \
    -DCMAKE_C_COMPILER={{ cc }} \
    -DCMAKE_CXX_COMPILER={{ cxx }} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  ln -sf build-{{ mode }}/compile_commands.json compile_commands.json

build mode=default_mode: (configure mode)
  cmake --build build-{{ mode }} --parallel

run mode=default_mode: (build mode)
  ./build-{{ mode }}/bin/Panarch

clean mode=default_mode:
  rm -rf build-{{ mode }}
