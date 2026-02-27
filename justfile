set windows-shell := ["powershell"]

alias b := build
alias br := build-release
alias r := run
alias rr := run-release

# Папки

debug_target_dir := "build/debug"
release_target_dir := "build/release"

# Экзешники

executable := "desolation_engine"

# build project (debug)
build:
    cmake -B {{ debug_target_dir }} \
        -G Ninja \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DCMAKE_BUILD_TYPE=Debug
    cmake --build {{ debug_target_dir }}


build-release:
    cmake -B {{ release_target_dir }} \
        -G Ninja \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build {{ release_target_dir }}
    cmake --install {{ release_target_dir }}


# build and run project (debug)
run: build
    ./{{ debug_target_dir }}/bin/{{ executable }}


# build and run project (release)
run-release: build-release
    ./{{ release_target_dir }}/bin/{{ executable }}
