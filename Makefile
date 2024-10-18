default_target:
	make -s -C build/release default_target

cmake_force:
	make -s -C build/release cmake_force

edit_cache:
	make -s -C build/release edit_cache

rebuild_cache:
	make -s -C build/release rebuild_cache

all:
	make -s -C build/release all

clean:
	make -s -C build/release clean

preinstall:
	make -s -C build/release preinstall

depend:
	make -s -C build/release depend

copy_resources:
	make -s -C build/release copy_resources

grcc:
	make -s -C build/release grcc

build_root_makefile:
	make -s -C build/release build_root_makefile

run:
	make -s -C build/release run

help:
	make -s -C build/release help

cmake_check_build_system:
	make -s -C build/release cmake_check_build_system

Release:
	@cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	@make -s -C build/release build_root_makefile
	@echo "[Release Mode]"

Debug:
	@cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
	@make -s -C build/debug build_root_makefile
	@echo "[Debug Mode]"

update:
	@cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
	@make -s -C build/release build_root_makefile
	@echo "[Release Mode]"

Info:
	@echo "[Release Mode]"

