
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

struct Velocity
{
    float vx;
    float vy;
};

namespace ParticleLife
{
    inline float randomFloat(const float& max)
    {
        return static_cast<float>((rand()) / static_cast<float>(RAND_MAX/max));
    }

    void addEntities(std::vector<ImVec2>& pos_component, int n, float x_max, float y_max)
    {
        pos_component.reserve(pos_component.size() + n);
        for (int i = 0; i < n; ++i)
            pos_component.push_back({randomFloat(x_max), randomFloat(y_max)});
    }

    void rule(std::vector<ImVec2>& group1_pos_component, std::vector<Velocity>& group1_velocity_component, std::vector<ImVec2>& group2_pos_component, float g, const float& radius)
    {
        for (std::size_t i = 0; i < group1_pos_component.size(); ++i)
        {
            auto& a_pos = group1_pos_component[i];
            Velocity a_velocity = group1_velocity_component[i];
            float fx = 0;
            float fy = 0;

            for (std::size_t j = 0; j < group2_pos_component.size(); ++j)
            {
                const auto& b = group2_pos_component[j];
                const auto dx = a_pos.x - b.x;
                const auto dy = a_pos.y - b.y;
                const auto d = std::sqrt(dx*dx + dy*dy);

                float F = 0.0f;
                if (d > 12.0f && d < radius)
                {
                    F = (g / d);
                    fx += dx * F;
                    fy += dy * F;
                }
            }
            a_velocity.vx = (a_velocity.vx + fx) * (1.0 - 0.2);
            a_velocity.vy = (a_velocity.vy + fy) * (1.0 - 0.2);
            if (a_pos.x < 0.0f && a_velocity.vx < 0) a_velocity.vx *= -1.0;
            if (a_pos.x > 1390.0f && a_velocity.vx > 0) a_velocity.vx *= -1.0;
            if (a_pos.y < 0.0f && a_velocity.vy < 0) a_velocity.vy *= -1.0;
            if (a_pos.y > 1190.0f && a_velocity.vy > 0) a_velocity.vy *= -1.0;
            a_pos.x += a_velocity.vx;
            a_pos.y += a_velocity.vy;
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

    std::vector<ImVec2> white_pos_component;
    std::vector<ImVec2> blue_pos_component;
    std::vector<ImVec2> red_pos_component;
    std::vector<ImVec2> green_pos_component;

    ParticleLife::addEntities(white_pos_component, 1000, 1600.0f, 1200.0f);
    ParticleLife::addEntities(blue_pos_component, 1000, 1600.0f, 1200.0f);
    ParticleLife::addEntities(red_pos_component, 1000, 1600.0f, 1200.0f);
    ParticleLife::addEntities(green_pos_component, 1000, 1600.0f, 1200.0f);

    std::vector<Velocity> white_velocity_component(white_pos_component.size(), {0, 0});
    std::vector<Velocity> blue_velocity_component(blue_pos_component.size(), {0, 0});
    std::vector<Velocity> red_velocity_component(red_pos_component.size(), {0, 0});
    std::vector<Velocity> green_velocity_component(green_pos_component.size(), {0, 0});

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
            if (white_pos_component.size() > 0)
            {
                ParticleLife::rule(white_pos_component, white_velocity_component, white_pos_component, fwhite_white, white_radius);
                ParticleLife::rule(white_pos_component, white_velocity_component, blue_pos_component, fwhite_blue, white_radius);
                ParticleLife::rule(white_pos_component, white_velocity_component, red_pos_component, fwhite_red, white_radius);
                ParticleLife::rule(white_pos_component, white_velocity_component, green_pos_component, fwhite_green, white_radius);
            }

            // BLUE
            if (blue_pos_component.size() > 0)
            {
                ParticleLife::rule(blue_pos_component, blue_velocity_component, white_pos_component, fblue_white, blue_radius);
                ParticleLife::rule(blue_pos_component, blue_velocity_component, blue_pos_component, fblue_blue, blue_radius);
                ParticleLife::rule(blue_pos_component, blue_velocity_component, red_pos_component, fblue_red, blue_radius);
                ParticleLife::rule(blue_pos_component, blue_velocity_component, green_pos_component, fblue_green, blue_radius);
            }

            // RED
            if (red_pos_component.size() > 0)
            {
                ParticleLife::rule(red_pos_component, red_velocity_component, white_pos_component, fred_white, red_radius);
                ParticleLife::rule(red_pos_component, red_velocity_component, blue_pos_component, fred_blue, red_radius);
                ParticleLife::rule(red_pos_component, red_velocity_component, red_pos_component, fred_red, red_radius);
                ParticleLife::rule(red_pos_component, red_velocity_component, green_pos_component, fred_green, red_radius);
            }

            // GREEN
            if (green_pos_component.size() > 0)
            {
                ParticleLife::rule(green_pos_component, green_velocity_component, white_pos_component, fgreen_white, green_radius);
                ParticleLife::rule(green_pos_component, green_velocity_component, blue_pos_component, fgreen_blue, green_radius);
                ParticleLife::rule(green_pos_component, green_velocity_component, red_pos_component, fgreen_red, green_radius);
                ParticleLife::rule(green_pos_component, green_velocity_component, green_pos_component, fgreen_green, green_radius);
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
                for (const auto& p : white_pos_component)
                    draw_list->AddCircleFilled(ImVec2(p.x, p.y), 1.8f, IM_COL32_WHITE);
                for (const auto& p : blue_pos_component)
                    draw_list->AddCircleFilled(ImVec2(p.x, p.y), 1.8f, IM_COL32(0,0,255,255));
                for (const auto& p : red_pos_component)
                    draw_list->AddCircleFilled(ImVec2(p.x, p.y), 1.8f, IM_COL32(255,0,0,255));
                for (const auto& p : green_pos_component)
                    draw_list->AddCircleFilled(ImVec2(p.x, p.y), 1.8f, IM_COL32(0,255,0,255));

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

