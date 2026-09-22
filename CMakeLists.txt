cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Windows: 绕开 ranlib 的「先复制再改名」
#
# 症状（本机实测，2026-09-18）：
#   riscv32-esp-elf-ranlib.exe: unable to copy file 'esp-idf\driver\libdriver.a';
#   reason: Permission denied
# 然后 ninja 直接停掉整个构建。安全软件会在 ranlib 改写刚生成的静态库那一刻
# 占住文件；换一个 .a 又会重来（实测两次分别挂在 libdriver.a / libesp_coex.a）。
#
# 根因拆开看：CMake 的静态库规则是
#   rm -f libX.a  &&  ar qc libX.a <objs>  &&  ranlib libX.a
#   · ar     这一步是「新建」文件            —— 能过
#   · ranlib 这一步是「替换」已存在的文件     —— 被拦的就是它
# 所以让 ar 在建档时就用 s 选项把符号索引一起写掉，ranlib 这一步根本不存在。
#
# ★ 必须写在 project() 之前：这几个变量由 CMake 在启用语言时初始化，
#   写在这里才能保证 IDF 的各个 component 目录都继承到。
# ★ 只影响 Windows；其他平台一行都不改。
# ---------------------------------------------------------------------------
if(WIN32)
    set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcs <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH   "<CMAKE_COMMAND> -E echo_append")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_COMMAND> -E echo_append")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(FoloToy-AI-Passport)
