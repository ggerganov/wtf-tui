set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(imgui-color-text-edit
    ImGuiColorTextEdit/TextEditor.cpp
    )

target_include_directories(imgui-color-text-edit INTERFACE
    .
    )

target_link_libraries(imgui-color-text-edit PRIVATE
    imtui-core
    )
