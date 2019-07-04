project(solid-octo-palm-tree)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++11")

set(source_files
    global.c
    graph_op.cc
    panel.c
    main.c
    shapes_ext.c
    ../graph_algorithm/graph.cpp
)

add_executable(octo ${source_files})

if(APPLE)
    set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -framework OpenGL -framework OpenAL -framework IOKit -framework CoreVideo -framework Cocoa")
    target_link_libraries(octo ${CMAKE_SOURCE_DIR}/libraylib_osx.a)
endif()
