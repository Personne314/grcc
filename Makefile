default_target:
	make -s -C build/debug default_target

cmake_force:
	make -s -C build/debug cmake_force

edit_cache:
	make -s -C build/debug edit_cache

rebuild_cache:
	make -s -C build/debug rebuild_cache

all:
	make -s -C build/debug all

clean:
	make -s -C build/debug clean

preinstall:
	make -s -C build/debug preinstall

depend:
	make -s -C build/debug depend

copy_resources:
	make -s -C build/debug copy_resources

grcc:
	make -s -C build/debug grcc

build_tests:
	make -s -C build/debug build_tests

run_tests:
	make -s -C build/debug run_tests

test_grammar:
	make -s -C build/debug test_grammar

test_regex:
	make -s -C build/debug test_regex

build_root_makefile:
	make -s -C build/debug build_root_makefile

run:
	make -s -C build/debug run

help:
	make -s -C build/debug help

cmake_check_build_system:
	make -s -C build/debug cmake_check_build_system

Release:
	@cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	@make -s -C build/release build_root_makefile
	@echo "[Release Mode]"

Debug:
	@cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
	@make -s -C build/debug build_root_makefile
	@echo "[Debug Mode]"

update:
	@cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
	@make -s -C build/debug build_root_makefile
	@echo "[Debug Mode]"

Info:
	@echo "[Debug Mode]"

