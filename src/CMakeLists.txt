set(TARGET wtfutil-tui)

if (EMSCRIPTEN)
    set (CMAKE_CXX_FLAGS "-s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0")
endif()

if (EMSCRIPTEN)
    add_executable(${TARGET}
        main.cpp
        impl-emscripten.cpp
        )

    target_include_directories(${TARGET} PRIVATE
        ..
        )

    target_link_libraries(${TARGET} PRIVATE
        yaml-cpp
        imtui-core
        imtui-emscripten
        imgui-color-text-edit
        )

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/index-tmpl.html ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}/index.html @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/style.css ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}/style.css @ONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/samples.js ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}/samples.js @ONLY)
else()
    add_executable(${TARGET}
        main.cpp
        impl-ncurses.cpp
        )

    target_include_directories(${TARGET} PRIVATE
        ..
        )

    target_link_libraries(${TARGET} PRIVATE
        yaml-cpp
        imtui-ncurses
        imgui-color-text-edit
        )

    install(TARGETS wtfutil-tui RUNTIME DESTINATION bin)
endif()
