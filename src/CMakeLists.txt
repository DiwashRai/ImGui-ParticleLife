
add_executable(
    ParticleLife
    main.cpp
)
add_executable(
    ECSParticleLife
    ecs.cpp
)

target_include_directories(ParticleLife PRIVATE
    ../imgui
    ../backends
    )
target_include_directories(ECSParticleLife PRIVATE
    ../imgui
    ../backends
    )

target_link_libraries(ParticleLife PRIVATE
    IMGUI
    glfw
    GL
    )
target_link_libraries(ECSParticleLife PRIVATE
    IMGUI
    glfw
    GL
    )

