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
    cmake --preset=vcpkg-debug
    cmake --build {{ debug_target_dir }}

# build project (release)
build-release:
    cmake --preset=vcpkg-release
    cmake --build {{ release_target_dir }}

# build and run project (debug)
run: build
    ./{{ debug_target_dir }}/bin/{{ executable }}

# build and run project (release)
run-release: build-release
    ./{{ release_target_dir }}/bin/{{ executable }}
