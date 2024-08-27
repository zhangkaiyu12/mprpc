cmake_minimum_required(VERSION 3.0)
project(mprpc)


#配置建议选项
set(CMAKE_CXX_STANDARD 11) # 设置C++标准为C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 确保使用指定的C++标准
#set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)  # 可调试可执行文件

#生成debug版本进行调试
set(CMAKE_BUILD_TYPE "Debug")

#配置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置库文件输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#配置头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
#项目库文件搜索路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

#加载子目录
add_subdirectory(src)
add_subdirectory(example)