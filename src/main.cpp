
#include <iostream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

#include <array>
#include <vector>
#include "ParticleObject.h"
#include <math.h>

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

namespace ParticleLife
{
    inline float randomFloat(const float& max)
    {
        return static_cast<float>((rand()) / static_cast<float>(RAND_MAX/max));
    }

    void addPoints(std::vector<ParticleObject>& particles, int n, float x_max, float y_max, ImU32 color)
    {
        particles.reserve(particles.size() + n);
        for (int i = 0; i < n; ++i)
            particles.push_back({randomFloat(x_max), randomFloat(y_max), 0.0f, 0.0f, color});
    }

    void rule(std::vector<ParticleObject>& group1, std::vector<ParticleObject>& group2, float g, const float& radius)
    {
        for (std::size_t i = 0; i < group1.size(); ++i)
        {
            auto& a = group1[i];
            float fx = 0;
            float fy = 0;

            for (std::size_t j = 0; j < group2.size(); ++j)
            {
                const auto& b = group2[j];
                const auto dx = a.x - b.x;
                const auto dy = a.y - b.y;
                const auto d = std::sqrt(dx*dx + dy*dy);

                float F = 0.0f;
                if (d > 12.0f && d < radius)
                {
                    F = (g / d);
                    fx += dx * F;
                    fy += dy * F;
                }
            }
            a.vx = (a.vx + fx) * (1.0 - 0.2);
            a.vy = (a.vy + fy) * (1.0 - 0.2);
            if (a.x < 0.0f && a.vx < 0) a.vx *= -1.0;
            if (a.x > 1390.0f && a.vx > 0) a.vx *= -1.0;
            if (a.y < 0.0f && a.vy < 0) a.vy *= -1.0;
            if (a.y > 1190.0f && a.vy > 0) a.vy *= -1.0;
            a.x += a.vx;
            a.y += a.vy;
        }
    }

    void move(std::vector<ParticleObject>& particles)
    {
        for (auto& p : particles)
        {
            p.x += p.vx;
            p.y += p.vy;
        }
    }
}


int main(int, char**)
{
    ParticleObject what;
    std::cout << sizeof(what) << std::endl;
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

#define DISPLAY_WIDTH  1800
#define DISPLAY_HEIGHT 1200
#define SETTINGS_WIDTH 401.0f
#define WORLD_WIDTH    1400.0f
    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(DISPLAY_WIDTH, DISPLAY_HEIGHT, "Particle Life", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    std::array<std::vector<ParticleObject>, 4> particle_groups;
    particle_groups.fill(std::vector<ParticleObject>());
    #define WHITE_PARTICLES particle_groups[0]
    #define BLUE_PARTICLES  particle_groups[1]
    #define RED_PARTICLES  particle_groups[2]
    #define GREEN_PARTICLES particle_groups[3]

    ParticleLife::addPoints(WHITE_PARTICLES, 1000, 1600.0f, 1200.0f, IM_COL32_WHITE);
    ParticleLife::addPoints(BLUE_PARTICLES, 1000, 1600.0f, 1200.0f, IM_COL32(0,0,255,255));
    ParticleLife::addPoints(RED_PARTICLES, 1000, 1600.0f, 1200.0f, IM_COL32(255,0,0,255));
    ParticleLife::addPoints(GREEN_PARTICLES, 1000, 1600.0f, 1200.0f, IM_COL32(0,255,0,255));

    float f32_minus_one = -1.0f, f32_one = 1.0f;
    float fmin_radius = 50.0f, fmax_radius = WORLD_WIDTH;

    float white_radius = 455.0;
    float blue_radius = 112.0;
    float red_radius = 80.0;
    float green_radius = 150.0;

    float fwhite_white = -0.1501;
    float fwhite_blue = -0.372f;
    float fwhite_red = -0.432f;
    float fwhite_green = -0.00344f;

    float fblue_white = -0.00015f;
    float fblue_blue = 0.0f;
    float fblue_red = -0.00051;
    float fblue_green = -0.00035;

    float fred_white = 0.1381f;
    float fred_blue = 0.321f;
    float fred_red = -0.9f;
    float fred_green = -0.2304f;

    float fgreen_white = -0.4151f;
    float fgreen_blue = 0.0006f;
    float fgreen_red = -0.4142f;
    float fgreen_green = -0.251f;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Game Loop start
       {
            {
                ImGui::SetNextWindowPos(ImVec2(DISPLAY_WIDTH - SETTINGS_WIDTH, 0));
                ImGui::SetNextWindowSize(ImVec2(SETTINGS_WIDTH, DISPLAY_HEIGHT));
                ImGui::Begin("Settings", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

                if (ImGui::CollapsingHeader("White", NULL, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::DragScalar("White Radius",     ImGuiDataType_Float,  &white_radius, 1.0f,  &fmin_radius, &fmax_radius, "%f");
                    ImGui::NewLine();
                    ImGui::DragScalar("White->White",     ImGuiDataType_Float,  &fwhite_white, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("White->Blue",     ImGuiDataType_Float,  &fwhite_blue, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("White->Red",     ImGuiDataType_Float,  &fwhite_red, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("White->Green",     ImGuiDataType_Float,  &fwhite_green, 0.001f,  &f32_minus_one, &f32_one, "%f");
                }
                if (ImGui::CollapsingHeader("Blue", NULL, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::DragScalar("Blue Radius",     ImGuiDataType_Float,  &blue_radius, 1.0f,  &fmin_radius, &fmax_radius, "%f");
                    ImGui::NewLine();
                    ImGui::DragScalar("Blue->White",     ImGuiDataType_Float,  &fblue_white, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Blue->Blue",     ImGuiDataType_Float,  &fblue_blue, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Blue->Red",     ImGuiDataType_Float,  &fblue_red, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Blue->Green",     ImGuiDataType_Float,  &fblue_green, 0.001f,  &f32_minus_one, &f32_one, "%f");
                }
                if (ImGui::CollapsingHeader("Red", NULL, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::DragScalar("Red Radius",     ImGuiDataType_Float,  &red_radius, 1.0f,  &fmin_radius, &fmax_radius, "%f");
                    ImGui::NewLine();
                    ImGui::DragScalar("Red->White",     ImGuiDataType_Float,  &fred_white, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Red->Blue",     ImGuiDataType_Float,  &fred_blue, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Red->Red",     ImGuiDataType_Float,  &fred_red, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Red->Green",     ImGuiDataType_Float,  &fred_green, 0.001f,  &f32_minus_one, &f32_one, "%f");
                }
                if (ImGui::CollapsingHeader("Green", NULL, ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::DragScalar("Green Radius",     ImGuiDataType_Float,  &green_radius, 1.0f,  &fmin_radius, &fmax_radius, "%f");
                    ImGui::NewLine();
                    ImGui::DragScalar("Green->White",     ImGuiDataType_Float,  &fgreen_white, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Green->Blue",     ImGuiDataType_Float,  &fgreen_blue, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Green->Red",     ImGuiDataType_Float,  &fgreen_red, 0.001f,  &f32_minus_one, &f32_one, "%f");
                    ImGui::DragScalar("Green->Green",     ImGuiDataType_Float,  &fgreen_green, 0.001f,  &f32_minus_one, &f32_one, "%f");
                }

                ImGui::End();
            }
            // WHITE
            if (WHITE_PARTICLES.size() > 0)
            {
                ParticleLife::rule(WHITE_PARTICLES, WHITE_PARTICLES, fwhite_white, white_radius);
                ParticleLife::rule(WHITE_PARTICLES, BLUE_PARTICLES, fwhite_blue, white_radius);
                ParticleLife::rule(WHITE_PARTICLES, RED_PARTICLES, fwhite_red, white_radius);
                ParticleLife::rule(WHITE_PARTICLES, GREEN_PARTICLES, fwhite_green, white_radius);
            }

            // BLUE
            if (BLUE_PARTICLES.size() > 0)
            {
                ParticleLife::rule(BLUE_PARTICLES, WHITE_PARTICLES, fblue_white, blue_radius);
                ParticleLife::rule(BLUE_PARTICLES, BLUE_PARTICLES, fblue_blue, blue_radius);
                ParticleLife::rule(BLUE_PARTICLES, RED_PARTICLES, fblue_red, blue_radius);
                ParticleLife::rule(BLUE_PARTICLES, GREEN_PARTICLES, fblue_green, blue_radius);
            }

            // RED
            if (RED_PARTICLES.size() > 0)
            {
                ParticleLife::rule(RED_PARTICLES, WHITE_PARTICLES, fred_white, red_radius);
                ParticleLife::rule(RED_PARTICLES, BLUE_PARTICLES, fred_blue, red_radius);
                ParticleLife::rule(RED_PARTICLES, RED_PARTICLES, fred_red, red_radius);
                ParticleLife::rule(RED_PARTICLES, GREEN_PARTICLES, fred_green, red_radius);
            }

            // GREEN
            if (GREEN_PARTICLES.size() > 0)
            {
                ParticleLife::rule(GREEN_PARTICLES, WHITE_PARTICLES, fgreen_white, green_radius);
                ParticleLife::rule(GREEN_PARTICLES, BLUE_PARTICLES, fgreen_blue, green_radius);
                ParticleLife::rule(GREEN_PARTICLES, RED_PARTICLES, fgreen_red, green_radius);
                ParticleLife::rule(GREEN_PARTICLES, GREEN_PARTICLES, fgreen_green, green_radius);
            }

            // move particles only after all forces have been recalculated
            // Commented out as this 'more accurate' way produces lses interesting patterns
            //for (auto& group : particle_groups)
            //    ParticleLife::move(group);

            {
                ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
                ImGui::SetNextWindowSize(ImVec2(WORLD_WIDTH, DISPLAY_HEIGHT));
                ImGui::Begin("Canvas", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // iterating through groups and rendering each particle objectj
                for (const auto& group : particle_groups)
                {
                    for (const auto& p : group)
                    {
                        draw_list->AddCircleFilled(ImVec2(p.x, p.y), 1.8f, p.color);
                    }
                }

                ImGui::End();
            }
        }
        // Game Loop End

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

