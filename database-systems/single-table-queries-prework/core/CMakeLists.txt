add_library(core
            STATIC
            tomekdb_tuple.cpp
            tomekdb_sortiterator.cpp
            tomekdb_scaniterator.cpp
            tomekdb_limititerator.cpp
            tomekdb_selectioniterator.cpp
            tomekdb_projectioniterator.cpp
           )

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})