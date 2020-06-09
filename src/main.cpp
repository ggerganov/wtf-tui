/*! \file main.cpp
 *  \brief wtf-tui : text-base UI for wtf
 */

#include "imtui/imtui.h"

#include "templates.h"

#include "ImGuiColorTextEdit/TextEditor.h"

#include "yaml-cpp/yaml.h"

#ifdef __EMSCRIPTEN__

#include "imtui/imtui-impl-emscripten.h"

#include <emscripten.h>
#include <emscripten/html5.h>

#else

#define EMSCRIPTEN_KEEPALIVE

#include "imtui/imtui-impl-ncurses.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#endif

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <fstream>
#include <iostream>
#include <chrono>

#ifdef MY_DEBUG
#define my_printf(...) printf(__VA_ARGS__)
#else
#define my_printf(...)
#endif

// global vars
bool g_updated = false;
ImTui::TScreen * g_screen = nullptr;

// platform specific functions
extern bool init();
extern void free();

// helper functions
namespace {

uint64_t t_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count(); // duh ..
}

[[maybe_unused]]
std::map<std::string, std::string> parseCmdArguments(int argc, char ** argv) {
    int last = argc;
    std::map<std::string, std::string> res;
    int k = 0;
    for (int i = 1; i < last; ++i) {
        if (argv[i][0] == '-') {
            res[argv[i]] = "";
            if (i < last - 1 && argv[i + 1][0] != '-') {
                res[argv[i]] = argv[i+1];
            }
        } else {
            res[std::string("P") + std::to_string(k)] = argv[i];
            ++k;
        }
    }

    return res;
}

std::string & rtrim(std::string & str)
{
  auto it1 =  std::find_if( str.rbegin() , str.rend() , [](char ch){ return !std::isspace<char>(ch , std::locale::classic() ) ; } );
  str.erase( it1.base() , str.end() );
  return str;
}

// ref : https://stackoverflow.com/questions/41326112/how-to-merge-node-in-yaml-cpp
YAML::Node merge_nodes(YAML::Node a, YAML::Node b)
{
    if (!b.IsMap()) {
        // If b is not a map, merge result is b, unless b is null
        return b.IsNull() ? a : b;
    }
    if (!a.IsMap()) {
        // If a is not a map, merge result is b
        return b;
    }
    if (!b.size()) {
        // If a is a map, and b is an empty map, return a
        return a;
    }
    // Create a new map 'c' with the same mappings as a, merged with b
    auto c = YAML::Node(YAML::NodeType::Map);
    for (auto n : a) {
        //if (n.first.IsScalar()) {
        //    const std::string & key = n.first.Scalar();
        //    auto t = YAML::Node(b[key]);
        //    if (t) {
        //        c[n.first] = merge_nodes(n.second, t);
        //        continue;
        //    }
        //}
        c[n.first] = n.second;
    }
    // Add the mappings from 'b' not already in 'c'
    for (auto n : b) {
        if (!n.first.IsScalar() || !c[n.first.Scalar()]) {
            c[n.first] = n.second;
        }
    }
    return c;
}

}

namespace Core {

constexpr auto kMaxModules = 64;
constexpr auto kMaxColumns = 512;
constexpr auto kMaxRows = 512;

struct Position {
    int x = 0;
    int y = 0;
    int w = 1;
    int h = 1;
};

struct Module {
    bool isNew = true;
    bool hasError = false;
    int enabled = 0;
    int refreshInterval = 500;
    Position position;

    int uid = -1;
    std::string id = "";
    std::string type = "";
    std::string title = "";
    std::string titleDisplay = "";

    std::string config = "";
    std::string configEditor = "";

    std::string error = "";

    bool update() {
        if (title.empty()) {
            title = id;
        }
        if (type.empty()) {
            type = id;
        }
        if (enabled == 0) {
            enabled = 1;
        }
        titleDisplay = std::to_string(uid) + ": " + title + " [" + type + "]" + "###" + std::to_string(uid);

        try {
            auto nodeConfigEditor = YAML::Load(configEditor);

            YAML::Node nodeConfigUser;
            nodeConfigUser["title"] = title;
            nodeConfigUser["type"] = type;
            nodeConfigUser["enabled"] = enabled == 1 ? true : false;
            nodeConfigUser["position"]["top"] = position.y;
            nodeConfigUser["position"]["left"] = position.x;
            nodeConfigUser["position"]["width"] = position.w;
            nodeConfigUser["position"]["height"] = position.h;

            auto root = merge_nodes(nodeConfigUser, nodeConfigEditor);

            {
                std::stringstream ss;
                ss << "\n" << root;
                config = ss.str();
                configEditor = config;
            }

            hasError = false;
        } catch ([[maybe_unused]] const YAML::Exception& e) {
            //fprintf(stderr, "Error : %s\n", e.what());
            hasError = true;
            error = e.what();

            return false;
        }

        return true;
    }
};

struct State {
    State() {
        modules[0].enabled = 1;

        // default config
        config = R"(
wtf:
  colors:
    background: black
    title: lightgreen
    border:
      focusable: darkslateblue
      focused: orange
      normal: gray
    checked: yellow
    highlight:
      fore: black
      back: gray
    rows:
      even: yellow
      odd: white
  refreshInterval: 1
  openFileUtil: "open"
        )";

#ifndef __EMSCRIPTEN__
        struct passwd *pw = getpwuid(getuid());
        fnameInput = pw->pw_dir;
        fnameInput += "/.config/wtf/config.yml";
        fnameOutput = fnameInput;
#endif
    }

    bool hasError = false;
    std::string errorStr = "";

    int uid = 0;
    std::array<Module, kMaxModules> modules;

    int outputId = 0;

    YAML::Node root;
    std::string config;

    std::array<int, kMaxColumns> cols;
    std::array<int, kMaxRows> rows;

    std::string fnameInput = "./config.yml";
    std::string fnameOutput = fnameInput;

    void clear() {
        for (auto & m : modules) {
            m.enabled = 0;
        }
        uid = 0;
    }

    bool addModule(Module && module) {
        for (auto & m : modules) {
            if (m.enabled > 0) continue;

            m = std::move(module);

            m.uid = uid;
            m.isNew = true;
            m.update();

            ++uid;

            return true;
        }

        return false;
    }

    void doError(std::string && err) {
        hasError = true;
        errorStr = err;
    }

    bool load(std::istream& input) {
        try {
            root = YAML::Load(input);
            {
                std::stringstream ss;
                ss << root;
                config = ss.str();
            }

            cols[0] = 0;
            rows[0] = 0;
            {
                int i = 1;
                for (auto c : root["wtf"]["grid"]["columns"]) {
                    cols[i] = cols[i - 1] + c.as<int>();
                    ++i;
                }
                while (i < kMaxColumns) {
                    cols[i] = cols[i - 1];
                    ++i;
                }
            }

            {
                int i = 1;
                for (auto c : root["wtf"]["grid"]["rows"]) {
                    rows[i] = rows[i - 1] + c.as<int>();
                    ++i;
                }
                while (i < kMaxRows) {
                    rows[i] = rows[i - 1];
                    ++i;
                }
            }

            for (auto m : root["wtf"]["mods"]) {
                Core::Module moduleNew;

                moduleNew.id = m.first.as<std::string>();
                if (m.second["title"]) {
                    moduleNew.title = m.second["title"].as<std::string>();
                }
                if (m.second["type"]) {
                    moduleNew.type = m.second["type"].as<std::string>();
                }
                if (m.second["enabled"]) {
                    if (m.second["enabled"].as<bool>()) {
                        moduleNew.enabled = 1;
                    } else {
                        moduleNew.enabled = 2;
                    }
                }

                // todo : check valid type;

                int top = m.second["position"]["top"].as<int>();
                int left = m.second["position"]["left"].as<int>();
                int height = m.second["position"]["height"].as<int>();
                int width = m.second["position"]["width"].as<int>();

                moduleNew.position.x = cols[left];
                moduleNew.position.y = rows[top];
                moduleNew.position.w = cols[left + width] - cols[left];
                moduleNew.position.h = rows[top + height] - rows[top];

                {
                    std::stringstream ss;
                    ss << m.second;
                    moduleNew.config = ss.str();
                    moduleNew.configEditor = moduleNew.config;
                }

                if (addModule(std::move(moduleNew)) == false) {
                    doError("Failed to add new module");
                }
            }
        } catch (const YAML::Exception& e) {
            fprintf(stderr, "Error : %s\n", e.what());
            hasError = true;
            errorStr = e.what();

            return false;
        } catch (...) {
            hasError = true;
            errorStr = "Failed to parse configuration";

            return false;
        }

        return true;
    }

    bool save(std::ostream& output) {
        auto root = YAML::Load(config);

        {
            int i = 1;
            int d = cols[i] - cols[i - 1];
            while (d > 0) {
                root["wtf"]["grid"]["columns"][i - 1] = d;
                ++i;
                d = cols[i] - cols[i - 1];
            }
        }

        int maxx = 0;
        int maxy = 0;

        for (auto & m : modules) {
            if (m.position.x + m.position.w > maxx) maxx = m.position.x + m.position.w;
            if (m.position.y + m.position.h > maxy) maxy = m.position.y + m.position.h;
        }

        root["wtf"]["grid"]["columns"] = YAML::Load("[]");
        root["wtf"]["grid"]["rows"] = YAML::Load("[]");
        for (int i = 0; i < maxx; ++i) {
            root["wtf"]["grid"]["columns"][i] = 1;
        }
        for (int i = 0; i < maxy; ++i) {
            root["wtf"]["grid"]["rows"][i] = 1;
        }

        root["wtf"]["mods"] = YAML::Node();
        for (auto & m : modules) {
            if (m.enabled == 0) continue;

            m.update();
            root["wtf"]["mods"]["module" + std::to_string(m.uid)] = YAML::Load(m.config);
        }

        output << root;

        return true;
    }
};

}

namespace UI {
enum class ColorScheme : int {
    Default,
    Dark,
    Green,
    COUNT,
};

enum class InputState : int {
    Dashboard = 0,
    Editing,
    EditingRAW,
    Template,
};

enum class ShiftMode : int {
    Move = 0,
    Resize,
    COUNT
};

struct State {
    State() {
        {
            auto palette = TextEditor::GetRetroBluePalette();
            palette[(int)TextEditor::PaletteIndex::Selection]   = 0x80a06020;
            palette[(int)TextEditor::PaletteIndex::Background]  = 0xff000000;
            editor.SetPalette(palette);

            auto lang = TextEditor::LanguageDefinition::WTF();
            editor.SetLanguageDefinition(lang);
        }
    }

    int statusWindowHeight = 1;

    bool showHelpModal = false;
    bool showStatusWindow = true;
    bool showEditWindow = true;
    bool showTemplateWindow = true;
    bool showGuideWindow = false;
    bool changeFocus = true;

    int idEditing = -1;
    int idTemplate = -1;
    int idFocused = -1;
    int idSelectedTemplate = 0;
    int idRestore = -1;
    std::array<int, Core::kMaxModules> restoreIds;

    TextEditor editor;
    InputState inputState = InputState::Dashboard;
    ShiftMode shiftMode = ShiftMode::Move;

    ColorScheme colorScheme = ColorScheme::Default;

    uint64_t lastStatusTime = 0;
    std::string lastStatusStr = "";

    void setStatus(const std::string & status) {
        lastStatusTime = ::t_ms();
        lastStatusStr = status;
    }

    void changeColorScheme(bool inc = true) {
        if (inc) {
            colorScheme = (ColorScheme)(((int) colorScheme + 1) % ((int)ColorScheme::COUNT));
        }

        ImVec4* colors = ImGui::GetStyle().Colors;
        switch (colorScheme) {
            case ColorScheme::Default:
                {
                    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
                    colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
                    colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
                    colors[ImGuiCol_TitleBg]                = ImVec4(0.16f, 0.29f, 0.48f, 1.00f);
                    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.50f, 0.08f, 1.00f);
                    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
                    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                    colors[ImGuiCol_PopupBg]                = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
                    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                }
                break;
            case ColorScheme::Dark:
                {
                    colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
                    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
                    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
                    colors[ImGuiCol_TitleBg]                = ImVec4(1.00f, 0.40f, 0.00f, 0.50f);
                    colors[ImGuiCol_TitleBgActive]          = ImVec4(1.00f, 0.40f, 0.00f, 0.50f);
                    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.69f, 0.25f, 0.00f, 0.50f);
                    colors[ImGuiCol_ChildBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
                    colors[ImGuiCol_PopupBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
                    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                }
                break;
            case ColorScheme::Green:
                {
                    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
                    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
                    colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
                    colors[ImGuiCol_TitleBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
                    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
                    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.50f, 1.00f, 0.50f, 1.00f);
                    colors[ImGuiCol_ChildBg]                = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
                    colors[ImGuiCol_PopupBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
                    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                }
                break;
            default:
                {
                }
        }
    }
};
}

// Core state
Core::State stateCore;

// UI state
UI::State stateUI;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
        int get_output_id() {
            return stateCore.outputId;
        }

    EMSCRIPTEN_KEEPALIVE
        bool reload() {
            stateCore.clear();
            std::ifstream fin(stateCore.fnameInput);
            if (fin.is_open() == false || fin.good() == false) {
                stateUI.setStatus("Error : Failed to open file '" + stateCore.fnameInput + "'");
                return false;
            } else {
                if (stateCore.load(fin)) {
                    stateUI.setStatus("Configuration loaded successfully from '" + stateCore.fnameInput + "'");
                } else {
                    stateUI.setStatus("Error : Failed to read configuration from '" + stateCore.fnameInput + "'");
                    return false;
                }
            }

            return true;
        }

    EMSCRIPTEN_KEEPALIVE
        bool render_frame() {
            my_printf("Render: begin\n");
#ifdef __EMSCRIPTEN__
            ImTui_ImplEmscripten_NewFrame();
#else
            bool isActive = g_updated;
            isActive |= ImTui_ImplNcurses_NewFrame();
#endif
            ImTui_ImplText_NewFrame();
            ImGui::NewFrame();

            // save some space for the status window
            ImGui::GetIO().DisplaySize.y -= stateUI.statusWindowHeight;

            if (stateCore.hasError) {
                stateCore.hasError = false;
                stateUI.setStatus("Error : " + stateCore.errorStr);
            }

            if (stateUI.idEditing >= 0) {
                stateUI.idEditing = stateUI.idFocused;
                stateUI.showEditWindow = true;
                stateUI.showTemplateWindow = false;
                if (stateUI.inputState != UI::InputState::Editing && stateUI.inputState != UI::InputState::EditingRAW) {
                    stateUI.inputState = UI::InputState::Editing;
                }
            } else {
                if (stateUI.idTemplate >= 0) {
                    stateUI.showEditWindow = false;
                    stateUI.showTemplateWindow = true;
                    stateUI.inputState = UI::InputState::Template;
                } else {
                    stateUI.showEditWindow = false;
                    stateUI.showTemplateWindow = false;
                    stateUI.inputState = UI::InputState::Dashboard;
                }
            }

            //
            //// Dashboard window
            //

            int nModules = 0;
            for (int wid = 0; wid < (int) stateCore.modules.size(); ++wid) {
                auto & module = stateCore.modules[wid];
                if (module.enabled == 0) continue;

                ++nModules;

                if (stateUI.idFocused == -1) {
                    stateUI.idFocused = wid;
                }

                if (module.isNew) {
                    ImGui::SetNextWindowPos(ImVec2(module.position.x, module.position.y), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2(module.position.w, module.position.h), ImGuiCond_Always);
                    module.update();
                }

                ImGui::GetStyle().WindowBorderAscii = true;

                if (stateUI.changeFocus && wid == stateUI.idFocused) {
                    if (stateUI.showEditWindow) {
                        stateUI.idEditing = stateUI.idFocused;
                    }
                    if (stateUI.showTemplateWindow) {
                        stateUI.idTemplate = stateUI.idFocused;
                    }
                    ImGui::SetNextWindowFocus();
                }

                // save colors
                ImVec4 cols[ImGuiCol_COUNT];
                for (int i = 0; i < ImGuiCol_COUNT; ++i) cols[i] = ImGui::GetStyle().Colors[i];

                ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = module.enabled == 1 ?
                    ImVec4(0.10f, 0.10f, 0.10f, 1.0f) :
                    ImVec4(0.30f, 0.00f, 0.00f, 0.5f);

                if (stateUI.idFocused == wid) {
                    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = module.enabled == 1 ?
                        ImVec4(0.20f, 0.20f, 0.20f, 1.0f) :
                        ImVec4(0.30f, 0.00f, 0.00f, 1.0f);
                    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive];
                    ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.0f, std::abs(std::sin(4.0f*ImGui::GetTime())), 0.0f, 1.0f);
                }

                ImGui::Begin(module.titleDisplay.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
                bool focused = ImGui::IsWindowFocused();

                if (module.hasError) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), " Error : %s ", module.error.c_str());
                }

                if (stateUI.idEditing == wid) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), " Editing");
                } else if (stateUI.idTemplate == wid) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), " Select template");
                } else if (stateUI.idFocused == wid) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", "");
                } else {
                    ImGui::Text("%s", "");
                }

                if (focused) {
                    if (stateUI.changeFocus == false) {
                        stateUI.idFocused = wid;
                    }
                }

                module.position.x = ImGui::GetWindowPos().x;
                module.position.y = ImGui::GetWindowPos().y;
                module.position.w = ImGui::GetWindowSize().x;
                module.position.h = ImGui::GetWindowSize().y;

                module.isNew = false;

                ImGui::Text("%s", "");
                ImGui::SameLine();
                if (stateUI.idFocused == wid) {
                    ImGui::Text("%s", module.config.c_str());
                    if (stateUI.showEditWindow == false && stateUI.showTemplateWindow == false) {
                        if (ImGui::IsKeyDown(ImGui::GetIO().KeyMap[ImGuiKey_DownArrow]) || ImGui::IsKeyDown('j') || ImGui::IsKeyDown('J')) {
                            if (ImGui::GetIO().KeyShift || ImGui::IsKeyPressed('J', true)) {
                                switch (stateUI.shiftMode) {
                                    case UI::ShiftMode::Move:
                                        {
                                            module.position.y = std::min((int) ImGui::GetIO().DisplaySize.y - 1, module.position.y + 1);
                                        }
                                        break;
                                    case UI::ShiftMode::Resize:
                                        {
                                            module.position.h = std::min((int) ImGui::GetIO().DisplaySize.y - 1, module.position.h + 1);
                                        }
                                        break;
                                    case UI::ShiftMode::COUNT:
                                        break;
                                };
                                module.isNew = true;
                            } else {
                                auto y = ImGui::GetScrollY();
                                ImGui::SetScrollY(std::min(ImGui::GetScrollMaxY(), y + 1.0f));
                            }
                        }

                        if (ImGui::IsKeyDown(ImGui::GetIO().KeyMap[ImGuiKey_UpArrow]) || ImGui::IsKeyDown('k') || ImGui::IsKeyDown('K')) {
                            if (ImGui::GetIO().KeyShift || ImGui::IsKeyPressed('K', true)) {
                                switch (stateUI.shiftMode) {
                                    case UI::ShiftMode::Move:
                                        {
                                            module.position.y = std::max(0, module.position.y - 1);
                                        }
                                        break;
                                    case UI::ShiftMode::Resize:
                                        {
                                            module.position.h = std::max(1, module.position.h - 1);
                                        }
                                        break;
                                    case UI::ShiftMode::COUNT:
                                        break;
                                };
                                module.isNew = true;
                            } else {
                                auto y = ImGui::GetScrollY();
                                ImGui::SetScrollY(std::max(0.0f, y - 1.0f));
                            }
                        }
                    }
                } else {
                    ImGui::TextDisabled("%s", module.config.c_str());
                }

                ImGui::End();

                // restore colors
                for (int i = 0; i < ImGuiCol_COUNT; ++i) ImGui::GetStyle().Colors[i] = cols[i];

                ImGui::GetStyle().WindowBorderAscii = false;
            }

            if (stateUI.idFocused >= 0 && stateCore.modules[stateUI.idFocused].enabled == 0) {
                stateUI.idFocused = -1;
            }

            //
            // handle global input
            //

            stateUI.changeFocus = false;

            if (ImGui::GetIO().WantCaptureKeyboard == false) {
                if (ImGui::IsKeyDown(ImGui::GetIO().KeyMap[ImGuiKey_LeftArrow]) || ImGui::IsKeyDown('h') || ImGui::IsKeyDown('H')) {
                    if (ImGui::GetIO().KeyShift || ImGui::IsKeyPressed('H', true)) {
                        if (stateUI.showEditWindow == false && stateUI.showTemplateWindow == false) {
                            switch (stateUI.shiftMode) {
                                case UI::ShiftMode::Move:
                                    {
                                        stateCore.modules[stateUI.idFocused].position.x = std::max(0, stateCore.modules[stateUI.idFocused].position.x - 1);
                                    }
                                    break;
                                case UI::ShiftMode::Resize:
                                    {
                                        stateCore.modules[stateUI.idFocused].position.w = std::max(0, stateCore.modules[stateUI.idFocused].position.w - 1);
                                    }
                                    break;
                                case UI::ShiftMode::COUNT:
                                    break;
                            };
                            stateCore.modules[stateUI.idFocused].isNew = true;
                        }
                    } else if (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_LeftArrow], true) || ImGui::IsKeyPressed('h', true)) {
                        int cnt = 0;
                        do {
                            if (--stateUI.idFocused < 0) stateUI.idFocused = Core::kMaxModules - 1;
                            if (++cnt == Core::kMaxModules) {
                                stateUI.idFocused = -1;
                                break;
                            }
                        } while (stateCore.modules[stateUI.idFocused].enabled == 0);
                        stateUI.changeFocus = true;
                    }
                }

                if (ImGui::IsKeyDown(ImGui::GetIO().KeyMap[ImGuiKey_RightArrow]) || ImGui::IsKeyDown('l') || ImGui::IsKeyDown('L')) {
                    if (ImGui::GetIO().KeyShift || ImGui::IsKeyPressed('L', true)) {
                        if (stateUI.showEditWindow == false && stateUI.showTemplateWindow == false) {
                            switch (stateUI.shiftMode) {
                                case UI::ShiftMode::Move:
                                    {
                                        stateCore.modules[stateUI.idFocused].position.x = std::min((int) ImGui::GetIO().DisplaySize.x - 1, stateCore.modules[stateUI.idFocused].position.x + 1);
                                    }
                                    break;
                                case UI::ShiftMode::Resize:
                                    {
                                        stateCore.modules[stateUI.idFocused].position.w = std::min((int) ImGui::GetIO().DisplaySize.x - 1, stateCore.modules[stateUI.idFocused].position.w + 1);
                                    }
                                    break;
                                case UI::ShiftMode::COUNT:
                                    break;
                            };
                            stateCore.modules[stateUI.idFocused].isNew = true;
                        }
                    } else if (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_RightArrow], true) || ImGui::IsKeyPressed('l', true)) {
                        int cnt = 0;
                        do {
                            if (++stateUI.idFocused == Core::kMaxModules) stateUI.idFocused = 0;
                            if (++cnt == Core::kMaxModules) {
                                stateUI.idFocused = -1;
                                break;
                            }
                        } while (stateCore.modules[stateUI.idFocused].enabled == 0);
                        stateUI.changeFocus = true;
                    }
                }

                if (ImGui::IsKeyPressed('x', false)) {
                    if (stateUI.idFocused >= 0) {
                        stateCore.modules[stateUI.idFocused].enabled = 3 - stateCore.modules[stateUI.idFocused].enabled;
                        stateCore.modules[stateUI.idFocused].update();
                    }
                }

                if (ImGui::IsKeyPressed('w', false)) {
                    stateUI.shiftMode = (UI::ShiftMode)(((int)(stateUI.shiftMode) + 1) % (int)(UI::ShiftMode::COUNT));
                }
            }

            //
            // handle input in main window
            //

            if (stateUI.showEditWindow == false && stateUI.showTemplateWindow == false) {
                if (ImGui::IsKeyPressed('s', false) || ImGui::IsKeyPressed('S', false)) {
                    std::ofstream fout(stateCore.fnameOutput.c_str());
                    if (fout.is_open() == false || fout.good() == false) {
                        stateUI.setStatus("Error : Failed to save file '" + stateCore.fnameOutput + "'");
                    } else {
                        if (stateCore.save(fout)) {
                            stateUI.setStatus("Configuration successfully saved to '" + stateCore.fnameOutput + "'");
                            stateCore.outputId++;
                        } else {
                            stateUI.setStatus("Error : Failed to save current config to '" + stateCore.fnameOutput + "'");
                        }
                    }
                }

                if (ImGui::IsKeyPressed('o', false) || ImGui::IsKeyPressed('O', false)) {
                    auto save = stateCore;
                    if (reload() == false ){
                        stateCore = save;
                        stateUI.setStatus("Error : Failed to reload configuration from '" + stateCore.fnameInput + "'");
                    }
                }

                if (ImGui::IsKeyPressed('c', false) || ImGui::IsKeyPressed('C', false)) {
                    if (stateUI.idFocused >= 0) {
                        Core::Module moduleNew = stateCore.modules[stateUI.idFocused];

                        moduleNew.id = "module " + std::to_string(stateCore.uid);
                        moduleNew.position.x += 2;
                        moduleNew.position.y += 2;

                        if (stateCore.addModule(std::move(moduleNew)) == false) {
                            stateCore.doError("Failed to add new module");
                        }
                    }
                }

                if (ImGui::IsKeyPressed('n', false) || ImGui::IsKeyPressed('N', false)) {
                    Core::Module moduleNew;

                    moduleNew.id = "module " + std::to_string(stateCore.uid);
                    moduleNew.type = "null";
                    moduleNew.position.x = 10 + 4*(stateCore.uid%4);
                    moduleNew.position.y = 10 + 2*(stateCore.uid%4);
                    moduleNew.position.w = 40;
                    moduleNew.position.h = 20;

                    if (stateCore.addModule(std::move(moduleNew)) == false) {
                        stateCore.doError("Failed to add new module");
                    }
                }

                if (stateUI.idFocused >= 0 && (ImGui::IsKeyPressed('d', false) || ImGui::IsKeyPressed('D', false))) {
                    stateCore.modules[stateUI.idFocused].enabled = 0;
                    stateUI.restoreIds[++stateUI.idRestore] = stateUI.idFocused;
                }

                if (ImGui::IsKeyPressed('r', false) || ImGui::IsKeyPressed('R', false)) {
                    if (stateUI.idRestore >= 0) {
                        stateCore.modules[stateUI.restoreIds[stateUI.idRestore--]].enabled = 1;
                    }
                }

                if (ImGui::IsKeyPressed('Q', false)) {
#ifndef __EMSCRIPTEN__
                    return false;
#endif
                }
            }

            //
            //// Template window
            //

            if (stateUI.showTemplateWindow && ImGui::IsPopupOpen("###Template") == false) {
                auto & m = stateCore.modules[stateUI.idFocused];
                ImGui::SetNextWindowPos(ImVec2(m.position.x + 6, m.position.y + 2), ImGuiCond_Always);
                ImGui::OpenPopup("###Template");
            }

            if (stateUI.idFocused >= 0 && stateCore.modules[stateUI.idFocused].enabled > 0 && stateUI.showTemplateWindow) {
                ImGui::SetNextWindowSize(ImVec2(80, 30), ImGuiCond_Always);
                if (ImGui::BeginPopup("Templates###Template")) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "--=[");
                    ImGui::SameLine();
                    ImGui::Text("Available module templates");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "]=--");

                    ImGui::Text("%s", "");

                    auto & m = stateCore.modules[stateUI.idFocused];

                    for (int i = 0; i < (int) Core::kTemplatesOriginal.size(); ++i) {
                        const auto & t = Core::kTemplatesOriginal[i];
                        if (t.desc.empty()) {
                            ImGui::Text("%s", "");
                        }
                        if (stateUI.idSelectedTemplate == i) {
                            ImGui::Text("[ %-18s ] : %s", t.name.c_str(), t.desc.c_str());
                            ImGui::SetScrollHereY();
                        } else {
                            ImGui::TextDisabled("  %-18s   : %s", t.name.c_str(), t.desc.c_str());
                        }
                    }

                    ImGui::Text("%s", "");

                    if (ImGui::GetIO().WantCaptureKeyboard == false &&
                         (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_DownArrow], true) ||
                          ImGui::IsKeyPressed('j', true) || ImGui::IsKeyPressed('J', true))) {
                        stateUI.idSelectedTemplate = std::min((int) Core::kTemplatesOriginal.size() - 1, stateUI.idSelectedTemplate + 1);
                    }

                    if (ImGui::GetIO().WantCaptureKeyboard == false &&
                         (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_UpArrow], true) ||
                          ImGui::IsKeyPressed('k', true) || ImGui::IsKeyPressed('K', true))) {
                        stateUI.idSelectedTemplate = std::max(0, stateUI.idSelectedTemplate - 1);
                    }

                    if (ImGui::Button("Apply ") ||
                        (ImGui::GetIO().WantCaptureKeyboard == false &&
                         (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_Enter], false) ||
                          ImGui::IsKeyPressed('a', false) || ImGui::IsKeyPressed('A', false)))) {
                        if (Core::kTemplatesOriginal.at(stateUI.idSelectedTemplate).desc.empty() == false) {
                            m.title = Core::kTemplatesOriginal.at(stateUI.idSelectedTemplate).name;
                            m.type = Core::kTemplatesOriginal.at(stateUI.idSelectedTemplate).type;
                            m.config = Core::kTemplatesOriginal.at(stateUI.idSelectedTemplate).config;
                            m.configEditor = m.config;
                            m.update();
                        }
                    }
                    ImGui::SameLine();

                    if (ImGui::Button("Close ") ||
                        (ImGui::GetIO().WantCaptureKeyboard == false &&
                         (ImGui::IsKeyPressed('q', false) || ImGui::IsKeyPressed('Q', false)))) {
                        stateUI.idTemplate = -1;
                    }

                    ImGui::Text("%s", "");

                    if (stateUI.changeFocus) {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            } else {
                if (ImGui::BeginPopup("Templates###Template")) {
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
            }

            if (ImGui::GetIO().WantCaptureKeyboard == false && (
                            stateUI.idTemplate < 0 && (ImGui::IsKeyReleased('t') || ImGui::IsKeyReleased('T')))) {
                stateUI.idTemplate = stateUI.idFocused;
                stateUI.idEditing = -1;
            }

            //
            //// Edit window
            //

            if (stateUI.showEditWindow && ImGui::IsPopupOpen("###Edit") == false) {
                auto & m = stateCore.modules[stateUI.idFocused];
                ImGui::SetNextWindowPos(ImVec2(m.position.x + 6, m.position.y + 2), ImGuiCond_Always);
                stateUI.editor.SetReadOnly(false);
                stateUI.editor.SetText(rtrim(m.configEditor));
                ImGui::OpenPopup("###Edit");
            }

            if (stateUI.idFocused >= 0 && stateCore.modules[stateUI.idFocused].enabled > 0 && stateUI.showEditWindow) {
                ImGui::SetNextWindowSize(ImVec2(64, 30), ImGuiCond_Always);
                if (ImGui::BeginPopup("Edit module###Edit")) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "--=[");
                    ImGui::SameLine();
                    ImGui::Text("Edit module");
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "]=--");

                    ImGui::Text("%s", "");

                    bool modified = false;
                    auto & m = stateCore.modules[stateUI.idFocused];

                    {
                        ImGui::Text("ID      : ");
                        ImGui::SameLine();
                        ImGui::Text("%d", m.uid);
                    }

                    {
                        bool enabled = m.enabled == 1;
                        ImGui::Text("Enabled : ");
                        ImGui::SameLine();
                        if (ImGui::Checkbox("##enabled", &enabled)) {
                            m.enabled = enabled ? 1 : 2;
                            modified = true;
                        }
                    }

                    {
                        constexpr size_t kMaxLen = 64;
                        static char buf[kMaxLen] = "";
                        std::memcpy(buf, m.title.c_str(), std::min(kMaxLen, m.title.size() + 1));
                        ImGui::Text("Title   : ");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(32);
                        if (ImGui::InputText("##title", buf, IM_ARRAYSIZE(buf))) {
                            m.title = std::string(buf);
                            modified = true;
                        }
                    }

                    {
                        constexpr size_t kMaxLen = 64;
                        static char buf[kMaxLen] = "";
                        std::memcpy(buf, m.type.c_str(), std::min(kMaxLen, m.type.size() + 1));
                        ImGui::Text("Type    : ");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(32);
                        if (ImGui::InputText("##type", buf, IM_ARRAYSIZE(buf))) {
                            m.type = std::string(buf);
                            modified = true;
                        }
                    }

                    ImGui::Text("%s", "");

                    {
                        ImGui::Text("%s", "");
                        ImGui::SameLine();
                        if (stateUI.editor.IsFocused() == false) {
                            stateUI.inputState = UI::InputState::Editing;

                            constexpr size_t kMaxLen = 4;
                            static char buf[kMaxLen] = "";

                            auto savePos = ImGui::GetCursorScreenPos();
                            ImGui::InputText("##editor_dummy", buf, IM_ARRAYSIZE(buf));
                            if (ImGui::IsItemActivated()) {
                                stateUI.editor.SetFocus();
                                stateUI.inputState = UI::InputState::EditingRAW;
                            }
                            ImGui::SetCursorScreenPos(savePos);
                        } else {
                            stateUI.inputState = UI::InputState::EditingRAW;
                        }
                        stateUI.editor.Render("TextEditor", ImVec2(ImGui::GetContentRegionAvailWidth(), 20));
                        if (stateUI.editor.IsFocused() == false && stateUI.inputState == UI::InputState::EditingRAW && m.configEditor != stateUI.editor.GetText()) {
                            m.configEditor = stateUI.editor.GetText();
                            modified = true;
                        }
                    }

                    if (m.hasError) {
                        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), " Error : %s ", m.error.c_str());
                    }

                    ImGui::Text("%s", "");

                    if (ImGui::Button("Close ") ||
                        (ImGui::GetIO().WantCaptureKeyboard == false &&
                         (ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_Enter], false) ||
                          ImGui::IsKeyPressed('q', false) || ImGui::IsKeyPressed('Q', false)))) {
                        stateUI.idEditing = -1;
                    }

                    ImGui::Text("%s", "");

                    if (modified) {
                        m.update();
                    }

                    if (stateUI.inputState != UI::InputState::EditingRAW && stateUI.editor.GetText() != m.configEditor) {
                        stateUI.editor.SetText(m.configEditor);
                    }

                    if (stateUI.changeFocus) {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
            } else {
                if (ImGui::BeginPopup("Edit module###Edit")) {
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
            }

            if (stateUI.idEditing < 0 && (ImGui::IsMouseDown(1) || ImGui::IsKeyReleased('e') || ImGui::IsKeyReleased('E'))) {
                stateUI.idEditing = stateUI.idFocused;
                stateUI.idTemplate = -1;
            }

            ImGui::GetIO().DisplaySize.y += stateUI.statusWindowHeight;

            //
            //// Status window
            //

            if (stateUI.showStatusWindow) {
                {
                    auto wSize = ImGui::GetIO().DisplaySize;
                    ImGui::SetNextWindowPos(ImVec2(0, wSize.y - stateUI.statusWindowHeight), ImGuiCond_Always);
                    ImGui::SetNextWindowSize(ImVec2(wSize.x, stateUI.statusWindowHeight), ImGuiCond_Always);
                }
                ImGui::Begin("###StatusWindow", nullptr,
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoTitleBar);

                if (::t_ms() - stateUI.lastStatusTime <= 3000) {
                    ImGui::Text("%s (%d ms)", stateUI.lastStatusStr.c_str(), 3000 - int(::t_ms() - stateUI.lastStatusTime));
                    ImGui::SameLine();
                }

                switch (stateUI.inputState) {
                    case UI::InputState::Dashboard:
                        {
                            if (stateUI.idFocused >= 0) {
                                switch (stateUI.shiftMode) {
                                    case UI::ShiftMode::Move:
                                        {
                                            ImGui::Text("| shift: move (w) |");
                                        }
                                        break;
                                    case UI::ShiftMode::Resize:
                                        {
                                            ImGui::Text("| shift: resize (w) |");
                                        }
                                        break;
                                    case UI::ShiftMode::COUNT:
                                        break;
                                };
                                ImGui::SameLine();
                                ImGui::Text("c: clone | d: delete | x: toggle | e: edit |");
                                ImGui::SameLine();
                            }
                            ImGui::Text("n: create | r: restore | s: save | ?: help |");
                        }
                        break;
                    case UI::InputState::Editing:
                        {
                            if (ImGui::GetIO().WantCaptureKeyboard) {
                                ImGui::Text("| tab: focus next parameter |");
                            } else {
                                ImGui::Text("| q: close | t: template | x: toggle |");
                            }
                        }
                        break;
                    case UI::InputState::EditingRAW:
                        {
                            ImGui::Text("| tab: save current config |");
                        }
                        break;
                    case UI::InputState::Template:
                        {
                            ImGui::Text("| q: close | e: edit | enter: apply |");
                        }
                        break;
                };
                ImGui::End();
            }

            if (stateUI.showHelpModal == false &&
                stateUI.showEditWindow == false &&
                stateUI.showTemplateWindow == false) {
                if (ImGui::IsKeyPressed('?', false)) {
                    ImGui::OpenPopup("###Help");
                }

                if (ImGui::IsKeyPressed('g', false)) {
                    stateUI.showGuideWindow = !stateUI.showGuideWindow;
                }
            }

            //
            //// Help window
            //

            if (ImGui::BeginPopupModal("wtf-tui v0.1###Help", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text(" ");
                ImGui::Text("Text-based UI for wtf : https://github.com/wtfutil/wtf  ");
                ImGui::Text(" ");
                ImGui::Text("    ?           - toggle Help window    ");
                ImGui::Text("    c           - clone module");
                ImGui::Text("    n           - create new module");
                ImGui::Text("    d           - delete focused module");
                ImGui::Text("    r           - restore last deleted module");
                ImGui::Text("    x           - toggle module enabled");
                ImGui::Text("    e           - edit module parameters");
                ImGui::Text("    t           - module template selection");
                ImGui::Text("    w           - change shift mode");
                ImGui::Text("    s           - save configuration");
                ImGui::Text("    o           - reload configuration");
                ImGui::Text("    shift       - move / resize module");
                ImGui::Text("    Q           - quit    ");
                ImGui::Text(" ");

                if (stateUI.showHelpModal) {
                    if (ImGui::Button("Close ") || ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_Enter], false) ||  ImGui::IsKeyPressed('?', false) || ImGui::IsMouseDown(0)) {
                        ImGui::CloseCurrentPopup();
                        stateUI.showHelpModal = false;
                    }
                } else {
                    stateUI.showHelpModal = true;
                }

                ImGui::Text(" ");

                ImGui::EndPopup();
            }

            if (nModules == 0) {
                ImGui::SetNextWindowPos(ImVec2(2, 1), ImGuiCond_Always);
                if (ImGui::Begin("Example: Simple overlay", nullptr,
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration |
                                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav)) {
                    ImGui::Text("Press '?' for available shortcuts ");
                    ImGui::Text("Press 'g' for a tutorial ");
                }
                ImGui::End();
            }

            if (stateUI.showGuideWindow) {
                ImGui::SetNextWindowPos(ImVec2(4, 5), ImGuiCond_Once);
                if (ImGui::Begin("Guide", &stateUI.showGuideWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav)) {
                    ImGui::Text("%s", "");
#ifdef __EMSCRIPTEN__
                    ImGui::Text("---------------------------------------------");
                    ImGui::Text("Emscripten port of wtf-tui");
                    ImGui::Text("This demo is not suitable for mobile devices!");
                    ImGui::Text("---------------------------------------------");
#endif
                    ImGui::Text("%s", "");
                    ImGui::Text("This program can be used to create YAML configuration files for the WTF terminal dashboard.  ");
                    ImGui::Text("You can choose from a variety of module types and arrange and configure them easily in your terminal.  ");
                    ImGui::Text("The position, size and parameters of the modules can be adjusted via the text-based user interface.  ");
                    ImGui::Text("%s", "");
                    ImGui::Text("This window can be moved with the mouse. ");
                    ImGui::Text("Here are a few tips to get you started: ");
                    ImGui::Text("%s", "");
                    ImGui::Text(" - Create a module by pressing 'n' ");
                    ImGui::Text(" - Position the module with shift + arrows or using the mouse ");
                    ImGui::Text(" - Resize the module by press 'w' and again use shift + arrows ");
                    ImGui::Text(" - Select a wtf module type for the current module by pressing 't' ");
                    ImGui::Text(" - Navigate the list of available templates and select one by pressing 'enter'  ");
                    ImGui::Text(" - Close the template selection popup by pressing 'q' ");
                    ImGui::Text(" - Edit the parameters of the module by pressing 'e' ");
                    ImGui::Text(" - Navigate along the parameters with tab or the mouse ");
                    ImGui::Text(" - Close the edit window by pressing 'q' ");
                    ImGui::Text(" - You can clone the currently selected module by pressing 'c' ");
                    ImGui::Text(" - To save the current wtf configuration press 's' ");
#ifdef __EMSCRIPTEN__
                    ImGui::Text(" - Drag and drop WTF configuration files in this window to edit them  ");
                    ImGui::Text(" - Choose from available WTF sample configuration from the bottom of the page  ");
#endif
                    ImGui::Text("%s", "");
                    ImGui::Text("To close this guide window press 'g'.");
                    ImGui::Text("Enjoy!");
                    ImGui::Text("%s", "");
                }
                ImGui::End();
            }

            ImGui::Render();

            ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), g_screen);

#ifndef __EMSCRIPTEN__
            ImTui_ImplNcurses_DrawScreen(isActive);
#endif

            my_printf("Render: end\n");
            return true;
        }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char ** argv) {
#ifndef __EMSCRIPTEN__
    auto argm = parseCmdArguments(argc, argv);
    if (argm.find("--help") != argm.end() || argm.find("-h") != argm.end()) {
        printf("Usage: wtf-tui [config.yml] [-h] [-nm] [-i fname.yml] [-o fname.yml]\n");
        printf("    -h, --help              : print this help\n");
        printf("    -nm, --nomouse          : disable mouse\n");
        printf("    -i, --input fname.yml   : read wtf config from fname.yml\n");
        printf("    -o, --output fname.yml  : save wtf config to fname.yml\n");
        return -1;
    }

    int mouseSupport = (argm.find("--nomouse") != argm.end() || argm.find("-nm") != argm.end()) ? 0 : 1;

    if (argm.find("P0") != argm.end()) {
        stateCore.fnameInput = argm.at("P0");
        stateCore.fnameOutput = stateCore.fnameInput;
    }

    if (argm.find("-i") != argm.end()) {
        stateCore.fnameInput = argm.at("-i");
        stateCore.fnameOutput = stateCore.fnameInput;
    }

    if (argm.find("--input") != argm.end()) {
        stateCore.fnameInput = argm.at("--input");
        stateCore.fnameOutput = stateCore.fnameInput;
    }

    if (argm.find("-o") != argm.end()) {
        stateCore.fnameOutput = argm.at("-o");
    }

    if (argm.find("--output") != argm.end()) {
        stateCore.fnameOutput = argm.at("--output");
    }
#endif

    reload();

    if (init() == false) {
        fprintf(stderr, "Failed to initialize. Aborting\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;

#ifdef __EMSCRIPTEN__
    g_screen = ImTui_ImplEmscripten_Init(true);
#else
    g_screen = ImTui_ImplNcurses_Init(mouseSupport != 0, 60.0, 60.0);
#endif
    ImTui_ImplText_Init();

    stateUI.changeColorScheme(false);

#ifndef __EMSCRIPTEN__
    while (true) {
        if (render_frame() == false) break;
    }

    ImTui_ImplText_Shutdown();
    ImTui_ImplNcurses_Shutdown();
    free();
#endif

    return 0;
}
