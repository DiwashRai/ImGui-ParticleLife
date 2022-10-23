
add_library(IMGUI STATIC)

target_sources(IMGUI PRIVATE
    imconfig.h
    imgui.h
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_internal.h
    imgui_tables.cpp
    imgui_widgets.cpp
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h

    ../backends/imgui_impl_glfw.h
    ../backends/imgui_impl_glfw.cpp
    ../backends/imgui_impl_opengl3.h
    ../backends/imgui_impl_opengl3.cpp
    ../backends/imgui_impl_opengl3_loader.h
    )

target_include_directories(IMGUI PUBLIC
    ${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/backends
    )

