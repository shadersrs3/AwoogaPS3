# CMakeList.txt : CMake project for awoogaPS3, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-D_CRT_SECURE_NO_WARNINGS)

set(SOURCE_FILES "awoogaPS3.cpp" "Common/Logger.cpp"
		# Crypto
		"Crypto/AES.c" "Crypto/PkgDecryptor.cpp"
		# Core
		"Cell/PowerProcessor.cpp"
		# Scheduler
		# Memory
		"Memory/Memory.cpp"
		# HLE
		"HLE/HLE.cpp"
		# Modules
		"HLE/Modules/sceNpTus.cpp" "HLE/Modules/sceNp.cpp" "HLE/Modules/sceNp2.cpp"
		"HLE/Modules/cellNetctl.cpp" "HLE/Modules/sys_net.cpp"
		"HLE/Modules/cellGame.cpp" "HLE/Modules/cellGameExec.cpp"
		"HLE/Modules/cellGcmSys.cpp" "HLE/Modules/sys_fs.cpp" "HLE/Modules/sysPrxForUser.cpp" "HLE/Modules/cellFiber.cpp"
		"HLE/Modules/cellSysmodule.cpp" "HLE/Modules/cellSysutil.cpp"
		"HLE/Modules/cellSpurs.cpp" "HLE/Modules/cellSync.cpp"
		# Syscalls
		"HLE/Syscalls/sys_timer.cpp" "HLE/Syscalls/sys_time.cpp"
		"HLE/Syscalls/sys_dbg.cpp" "HLE/Syscalls/sys_memory.cpp" "HLE/Syscalls/sys_ppu_thread"
		"HLE/Syscalls/sys_spu.cpp" "HLE/Syscalls/sys_tty.cpp" "HLE/Syscalls/sys_event.cpp"
)

project ("awoogaPS3")

# Add source to this project's executable.
add_executable (awoogaPS3 ${SOURCE_FILES})

include_directories(SYSTEM ${PROJECT_SOURCE_DIR})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET awoogaPS3 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
