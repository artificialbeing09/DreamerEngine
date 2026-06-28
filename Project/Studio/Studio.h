#pragma once

#define GL_SILENCE_DEPRECATION
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "Testing/Test.h"
#include "Graphics/Graphics.h"

#include "TextEditor.h"
#include "LoadAndSave/Serializer.h"

#include "pfd.h"

using namespace std;

namespace Plugin {
    void LoadPlugins() {
        LoadAndSave::CreateFolderIfDoesNotExist("./Plugins");

        for (const auto& dirEntry : filesystem::directory_iterator("./Plugins")) {
            bool IsDirectory = filesystem::is_directory(dirEntry);

            string Path = dirEntry.path().generic_string();
            string EntryName = Path.substr(Path.find_first_of("/Plugins") + 9);

            if (IsDirectory) {
                Scheduler::Lua::RunScript(LoadAndSave::GetScriptByModuleName(EntryName + "/main.lua", 1), EntryName, 1);
            }
            else {
                Scheduler::Lua::RunScript(LoadAndSave::GetScriptByModuleName(EntryName, 1), EntryName, 1);
            }
        }
    }
}

namespace Studio {
    int LoadedItem = 0;
    int TotalItems = 5;

    bool Enabled = false;
    bool BarInfoEnabled = false;

    GLFWwindow* window = NULL;
    float main_scale = 1.0f;
    const char* glsl_version = "#version 130";

    map<void*, bool> ExpandedChildrenLists = {}; 
    shared_ptr<Instance> SelectedObject = NULL;
    // Note to self: use deque whenever possible.

    bool PropertyInstanceAdorneeEnabled = false;
    PropertyDescriptor* DescriptorForAdornee = NULL;

    void RenderDescendants(shared_ptr<Instance> Start, int LeftOffset);

    void RenderDescendants(shared_ptr<Instance> Start, int LeftOffset) {
        auto Children = Start->GetChildren();

        for (auto Inst : Children) {
            bool Set = ExpandedChildrenLists[Inst.get()];
            
            ImGui::SetCursorPosX(LeftOffset * ImGui::GetFrameHeight());

            if (Inst->HasChildren()) {
                if (ImGui::ArrowButton(to_string((long long)Inst.get()).c_str(), Set ? ImGuiDir_Left : ImGuiDir_Right))
                    ExpandedChildrenLists[Inst.get()] = !Set;

                ImGui::SameLine();
            }
            else {
                ImVec2 size(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
                ImGui::Dummy(size);

                ImGui::SameLine();
            }

            bool Pushed = false;

            if (Inst.get() == SelectedObject.get()) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.5f, 0.0f, 1.0f));
                Pushed = true;
            }

            ImGui::PushID((int)Inst.get());

            if (ImGui::Button((Inst->GetName()).c_str())) {
                if (PropertyInstanceAdorneeEnabled) {
                    DescriptorForAdornee->SetFunction(SelectedObject, &Inst);
                    PropertyInstanceAdorneeEnabled = false;
                }
                else {
                    if (Inst.get() == SelectedObject.get())
                        SelectedObject = NULL;
                    else
                        SelectedObject = Inst;
                }
            }

            ImGui::PopID();

            if (Pushed) {
                ImGui::PopStyleColor(1);
            }

            if (ExpandedChildrenLists[Inst.get()] == true)
                RenderDescendants(Inst, LeftOffset + 1);
        }
    }

    enum FileEntryType {
        SerializedObject,
        Script,
        Folder
    };

    struct FileEntry;
    struct FileEntry {
        FileEntryType T = FileEntryType::Script;
        string Name = "";
        string Path = "";
        deque<FileEntry*> List = {};
    };

    FileEntry FilesList;

    void UpdateFileEntryList(FileEntry* Parent, string BasePath);

    void UpdateFileEntryList(FileEntry* Parent = &FilesList, string BasePath = ".\\Game") {
        if (Parent == &FilesList) {
            FilesList.List = {};
        }

        for (const auto& dirEntry : filesystem::directory_iterator(BasePath)) {
            bool IsDirectory = filesystem::is_directory(dirEntry);

            string Path = dirEntry.path().generic_string();
            string EntryName = Path.substr(Path.find_last_of("/") + 1);

            FileEntry* New = new FileEntry;
            New->Name = EntryName;
            New->Path = Path;
            New->List = {};

            if (IsDirectory) {
                New->T = FileEntryType::Folder;

                UpdateFileEntryList(New, Path);
            }
            else {
                if (EntryName.substr(EntryName.find_last_of('.')) == ".map") {
                    New->T = FileEntryType::SerializedObject;
                }
                else {
                    New->T = FileEntryType::Script;
                }
            }

            Parent->List.push_back(New);
        }
    }

    map<string, bool> ExpandedFilesLists;

    string CurrentScriptSelected = "";
    string CurrentMapSelected = "";

    string IDEText = "";

    TextEditor editor;
    bool EditorTextChangedLock = false;
    bool EditorTextChangedThroughSetText = false;

    void RenderFilesList(FileEntry* Start, int LeftOffset);

    void RenderFilesList(FileEntry* Start, int LeftOffset) {
        for (auto Inst : Start->List) {
            bool Set = ExpandedFilesLists[Inst->Path];

            ImGui::SetCursorPosX(LeftOffset * ImGui::GetFrameHeight());

            if (Inst->T == FileEntryType::Folder) {
                if (ImGui::ArrowButton(Inst->Path.c_str(), Set ? ImGuiDir_Left : ImGuiDir_Right))
                    ExpandedFilesLists[Inst->Path] = !Set;

                ImGui::SameLine();
            }
            else {
                ImVec2 size(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
                ImGui::Dummy(size);

                ImGui::SameLine();
            }

            ImGui::PushID((int)Inst);

            if (ImGui::Button(Inst->Name.c_str())) {
                if (Inst->T == FileEntryType::SerializedObject) {
                    string SerializedText = Utils::ReadFile(Inst->Path.c_str());

                    CurrentMapSelected = Inst->Path;

                    Serializer::DeserializeIntoObject(GetGameWorld(), SerializedText, true);
                }
                else if (Inst->T == FileEntryType::Script) {
                    if (EditorTextChangedLock && CurrentScriptSelected.size() > 0) {
                        auto m = pfd::message("Unsaved Script", "Do you want to save changes to " + CurrentScriptSelected.substr(CurrentScriptSelected.find_last_of('/') + 1),
                            pfd::choice::yes_no, pfd::icon::warning);

                        while (!m.ready(200000)) {}

                        switch (m.result())
                        {
                        case pfd::button::yes: Utils::WriteFile(CurrentScriptSelected.c_str(), editor.GetText().c_str()); break;
                        case pfd::button::no: break;
                        default: break; // Should not happen
                        }
                    }

                    CurrentScriptSelected = Inst->Path;

                    IDEText = Utils::ReadFile(CurrentScriptSelected.c_str());

                    editor.SetText(IDEText);

                    EditorTextChangedLock = false;
                    EditorTextChangedThroughSetText = true;
                }
            }

            ImGui::PopID();

            if (ExpandedFilesLists[Inst->Path] == true)
                RenderFilesList(Inst, LeftOffset + 1);
        }
    }

    void TaskbarInfoRender() {
        string Info = "[";

        Info += to_string(0) + " ms]";

        glfwSetWindowTitle(Gl.window, Info.c_str());
    }

    void InitGUIElements() {
        auto lang = TextEditor::LanguageDefinition::Lua();
        editor.SetLanguageDefinition(lang);
        editor.SetText(IDEText);

        UpdateFileEntryList();
    }

    bool Running = false;

    void Run() {
        Scheduler::Lua::RunScript(LoadAndSave::GetScriptByModuleName("main.lua", 0), "main.lua", 0);
        Running = true;
    }

    void Stop() {
        Scheduler::ExitAllThreadsWithIdentity(0);
        Running = false;
    }

    int Counter = 0;

    char* PropertyValueBuf[256];

    void GUIRender() {
        if (BarInfoEnabled) {
            TaskbarInfoRender();
        }

        if (!Enabled)
            return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Counter++;


        ImGuiID dockspace_id = ImGui::GetID("My Dockspace");
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        // Create settings
        if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
        {
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);
            ImGuiID dock_id_left = 0;
            ImGuiID dock_id_main = dockspace_id;
            ImGui::DockBuilderSplitNode(dock_id_main, ImGuiDir_Left, 0.20f, &dock_id_left, &dock_id_main);
            ImGuiID dock_id_left_top = 0;
            ImGuiID dock_id_left_bottom = 0;
            ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.50f, &dock_id_left_top, &dock_id_left_bottom);
            ImGui::DockBuilderDockWindow("Scene", dock_id_main);
            ImGui::DockBuilderDockWindow("Properties", dock_id_left_top);
            ImGui::DockBuilderDockWindow("Explorer", dock_id_left_bottom);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        // Submit dockspace
        ImGui::DockSpaceOverViewport(dockspace_id, viewport, ImGuiDockNodeFlags_PassthruCentralNode);

        
        {
            ImGui::Begin("Scene");

            ImVec2 windowsize = ImGui::GetWindowSize();

            ImVec2 pos = ImGui::GetCursorScreenPos();

            ImGui::GetWindowDrawList()->AddImage(
                (void*)Graphics::Engine3D::FBOtextureMap, pos,
                ImVec2(pos.x + windowsize.x / 2, pos.y + windowsize.y / 2), ImVec2(0, 1), ImVec2(1, 0));

            ImGui::End();

            ImGui::Begin("Explorer"); 

            RenderDescendants(GetGameWorld(), 0);

            ImGui::End();

            ImGui::Begin("Properties"); 

            if (SelectedObject != NULL) {
                int PropertyI = 0;

                for (auto i : ClassPropertyDescriptorList[SelectedObject->GetType()]) {
                    ImGui::Text(i.first.c_str());

                    ImGui::SameLine();

                    auto PropertyInfo = i.second;

                    char* PropertyValueStorage = PropertyValueBuf[PropertyI];

                    if (PropertyInfo->Type == L_String) {
                        string* CurrentText = (string*)PropertyInfo->GetFunction(SelectedObject);

                        string ExtractedValue = *CurrentText;

                        memcpy(PropertyValueStorage, ExtractedValue.c_str(), 255);
                        PropertyValueStorage[255] = 0;

                        if (ImGui::InputText(("###" + i.first).c_str(), PropertyValueStorage, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                            string* NewString = new string;
                            *NewString = PropertyValueStorage;

                            PropertyInfo->SetFunction(SelectedObject, NewString);
                        }

                    }
                    else if (PropertyInfo->Type == L_Object) {
                        shared_ptr<Instance>* CurrentText = (shared_ptr<Instance>*)PropertyInfo->GetFunction(SelectedObject);
                        
                        bool ObjectEnabled = PropertyInstanceAdorneeEnabled && DescriptorForAdornee == PropertyInfo;

                        if (ObjectEnabled) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
                        }

                        string Name = "null";

                        if ((*CurrentText) != NULL) {
                            Name = (*CurrentText)->GetName();
                        }

                        if (ImGui::Button(Name.c_str())) {
                            if (ObjectEnabled) {
                                PropertyInstanceAdorneeEnabled = false;
                            }
                            else {
                                PropertyInstanceAdorneeEnabled = true;
                                DescriptorForAdornee = PropertyInfo;
                            }
                        }

                        if (ObjectEnabled) {
                            ImGui::PopStyleColor();
                        }
                    }
                    else if (PropertyInfo->Type == L_Vector) {
                        LuaVector* CurrentText = (LuaVector*)PropertyInfo->GetFunction(SelectedObject);

                        string VectorString = "";
                        VectorString.reserve(256); // Will crash if this isn't there

                        VectorString += to_string(CurrentText->x) + ", ";
                        VectorString += to_string(CurrentText->y) + ", ";
                        VectorString += to_string(CurrentText->z) + ", ";
                        VectorString += to_string(CurrentText->a) + "";

                        memcpy(PropertyValueStorage, VectorString.c_str(), 255);
                        PropertyValueStorage[255] = 0;

                        if (ImGui::InputText(("###" + i.first).c_str(), PropertyValueStorage, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                            vector<double> Lista = {};

                            string MadeNumber = "";

                            for (int i = 0; i < strlen(PropertyValueStorage); i++) {
                                char CurrentCharacter = PropertyValueStorage[i];

                                if (CurrentCharacter == ',') {
                                    double Num = 0;

                                    try { Num = stod(MadeNumber); } catch (exception& e) {}

                                    Lista.push_back(Num);

                                    MadeNumber = "";
                                }
                                else if ((CurrentCharacter >= '0' && CurrentCharacter <= '9') || CurrentCharacter == '.') {
                                    MadeNumber += CurrentCharacter;
                                }
                            }

                            LuaVector NewVec = { };

                            for (int K = 0; K < Lista.size(); K++)
                                if (K == 0)
                                    NewVec.x = Lista[K];
                                else if (K == 1)
                                    NewVec.y = Lista[K];
                                else if (K == 2)
                                    NewVec.z = Lista[K];
                                else if (K == 3)
                                    NewVec.a = Lista[K];

                            

                            PropertyInfo->SetFunction(SelectedObject, &NewVec);
                        }
                    }
                    else if (PropertyInfo->Type == L_Int) {
                        int64_t* CurrentText = (int64_t*)PropertyInfo->GetFunction(SelectedObject);

                        string ExtractedValue = to_string(*CurrentText);

                        memcpy(PropertyValueStorage, ExtractedValue.c_str(), 255);
                        PropertyValueStorage[255] = 0;

                        if (ImGui::InputText(("###" + i.first).c_str(), PropertyValueStorage, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                            int64_t NewValue = stoll(PropertyValueStorage);

                            PropertyInfo->SetFunction(SelectedObject, &NewValue);
                        }
                    }
                    else if (PropertyInfo->Type == L_Number) {
                        double* CurrentText = (double*)PropertyInfo->GetFunction(SelectedObject);

                        string ExtractedValue = to_string(*CurrentText);

                        memcpy(PropertyValueStorage, ExtractedValue.c_str(), 255);
                        PropertyValueStorage[255] = 0;

                        if (ImGui::InputText(("###" + i.first).c_str(), PropertyValueStorage, 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
                            double NewValue = stod(PropertyValueStorage);

                            PropertyInfo->SetFunction(SelectedObject, &NewValue);
                        }
                    }
                    else if (PropertyInfo->Type == L_Boolean) {
                        bool* CurrentText = (bool*)PropertyInfo->GetFunction(SelectedObject);

                        bool Value = *CurrentText;

                        if (ImGui::Checkbox(("###" + i.first).c_str(), &Value)) {
                            PropertyInfo->SetFunction(SelectedObject, &Value);
                        }
                    }

                    PropertyI++;
                }
            }
            
            ImGui::End();

            ImGui::Begin("Files");

            if (Counter % 60 == 0) {
                UpdateFileEntryList();
            }

            RenderFilesList(&FilesList, 0);

            ImGui::End();

            string WindowTitle = "IDE (" + CurrentScriptSelected.substr(CurrentScriptSelected.find_last_of('/') + 1) + ")";

            if (editor.IsTextChanged()) {
                if (!EditorTextChangedThroughSetText) {
                    EditorTextChangedLock = true;
                }
                else {
                    EditorTextChangedThroughSetText = false;
                }
            }

            if (EditorTextChangedLock) {
                WindowTitle += "*";
            }

            ImGui::Begin((WindowTitle + "###id").c_str());

            if (ImGui::Button("Save")) {
                if (CurrentScriptSelected.size() == 0) {
                    auto f = pfd::save_file("Choose file to save",
                        filesystem::current_path().string() + "\\Game\\script.lua",
                        { "Lua Script (.lua)", "*.lua" },
                        pfd::opt::force_overwrite);

                    CurrentScriptSelected = f.result();
                }

                Utils::WriteFile(CurrentScriptSelected.c_str(), editor.GetText().c_str());

                EditorTextChangedLock = false;
            }

            if (ImGui::Button("Run as console")) {
                Scheduler::Lua::RunScript(editor.GetText());
            }

            editor.Render("Script");

            ImGui::End();

            ImGui::Begin("Controls");

            if (!Running) {
                if (ImGui::Button("Run")) {
                    Run();
                }
            }
            else {
                if (ImGui::Button("Stop")) {
                    Stop();
                }
            }

            if (ImGui::Button("Clean up")) {
                Serializer::DeserializeIntoObject(GetGameWorld(), "", true);
            }

            ImGui::Separator();

            if (ImGui::Button("Save Current Map")) {
                if (CurrentMapSelected.size() == 0) {
                    auto f = pfd::save_file("Choose file to save",
                        filesystem::current_path().string() + "\\Game\\main.map",
                        { "Concise Game Map File (.map)", "*.map" },
                        pfd::opt::force_overwrite);

                    CurrentMapSelected = f.result();
                }

                string Serialized = Serializer::Serialize(GetGameWorld());

                Utils::WriteFile(CurrentMapSelected.c_str(), Serialized.c_str());
            }

            if (ImGui::Button("Save Current Map As...")) {
                auto f = pfd::save_file("Choose file to save",
                    filesystem::current_path().string() + "\\Game\\main.map",
                    { "Concise Game Map File (.map)", "*.map" },
                    pfd::opt::force_overwrite);

                CurrentMapSelected = f.result();
                
                string Serialized = Serializer::Serialize(GetGameWorld());

                Utils::WriteFile(CurrentMapSelected.c_str(), Serialized.c_str());
            }

            if (ImGui::Button("Load Map From File")) {
                auto f = pfd::open_file("Choose file to open",
                    filesystem::current_path().string() + "\\Game",
                    { "Concise Game Map File (.map)", "*.map" },
                    pfd::opt::none);

                if (f.result().size() > 0) {
                    string Path = f.result()[0];
                    string FoundFile = Utils::ReadFile(Path.c_str());

                    Serializer::DeserializeIntoObject(GetGameWorld(), FoundFile, true);
                }
                else {
                    cout << "No File" << endl;
                }
            }

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    int InitGUIWindow(GLFWwindow* customWindow = NULL) {
        if (!Enabled)
            return 0;

        main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
        window = customWindow;

        if (window == nullptr)
            return 1;

        glfwMakeContextCurrent(window);
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();
        
        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        InitGUIElements();

        for (int I = 0; I < 256; I++) {
            PropertyValueBuf[I] = (char*)malloc(1024);
        }
        
        return 0;
    }

    void ConfigurationWindow() {
        struct EngineConfiguration_t {
            Texture::TextureMemoryUseMode TextureMode = Texture::TextureMemoryUseMode::Low;
            int ShadowResolution = 10;
            bool EnableStudio = false;
            bool EnableConsole = false;
            bool YieldOnRequires = false;
            bool TaskbarInfo = false;
            char Zero = 0;
        };

        EngineConfiguration_t EngineConfiguration;

        FILE* ConfigFile = fopen("concisesettings.ini", "rb+");

        if (!ConfigFile) {
            cout << "Config file failed to open." << endl;
        }
        else {
            fread(&EngineConfiguration, sizeof(EngineConfiguration), 1, ConfigFile);

            fclose(ConfigFile);
        }

        

        if (EngineConfiguration.ShadowResolution == 0) {
            EngineConfiguration_t New;

            EngineConfiguration = New;
        }

        if (!glfwInit())
            return;

        float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
        GLFWwindow* window = glfwCreateWindow((int)(600 * main_scale), (int)(600 * main_scale), "Concise Pre-configuration", nullptr, nullptr);
        if (window == nullptr)
            return;
        glfwMakeContextCurrent(window);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        vector<const char*> PerformanceModes = {
            "Highest",
            "UltraHigh",
            "High",
            "Medium",
            "Low",
            "UltraLow",
            "UltraUltraLow",
            "Windows95",
            "Headless"
        };

        while (!glfwWindowShouldClose(window))
        {
            this_thread::sleep_for(chrono::milliseconds(1000 / 30));

            glfwPollEvents();
            if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
            {
                ImGui_ImplGlfw_Sleep(10);
                continue;
            }

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

            {
                static float f = 0.0f;
                static int counter = 0;

                ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);

                ImGui::Checkbox("Studio", &EngineConfiguration.EnableStudio);
                ImGui::Checkbox("Console", &EngineConfiguration.EnableConsole);
                ImGui::Checkbox("Yield on requires", &EngineConfiguration.YieldOnRequires);
                ImGui::Checkbox("Debug info in title", &EngineConfiguration.TaskbarInfo);

                ImGui::SliderInt("Shadow Resolution", &EngineConfiguration.ShadowResolution, 1, 20, "%d00", ImGuiSliderFlags_AlwaysClamp);

                ImGui::BeginListBox("Performance Mode");

                for (int i = 0; i < PerformanceModes.size(); i++) {
                    const bool isSelected = ((Texture::TextureMemoryUseMode)i == EngineConfiguration.TextureMode);

                    if (ImGui::Selectable(PerformanceModes[i], isSelected)) { EngineConfiguration.TextureMode = (Texture::TextureMemoryUseMode)i; }

                    if (isSelected) { ImGui::SetItemDefaultFocus(); }
                }

                ImGui::EndListBox();

                if (ImGui::Button("Play"))
                    glfwSetWindowShouldClose(window, true);


                ImGui::SameLine();

                if (ImGui::Button("Close"))
                    exit(0);

                ImGui::End();
            }

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        glfwDestroyWindow(window);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        Texture::PerformanceMode = EngineConfiguration.TextureMode;
        Graphics::Engine3D::SHADOW_SIZE = EngineConfiguration.ShadowResolution * 100;
        Studio::Enabled = EngineConfiguration.EnableStudio;
        Studio::BarInfoEnabled = EngineConfiguration.TaskbarInfo;
        Scheduler::YieldOnRequires = EngineConfiguration.YieldOnRequires;

        if (!EngineConfiguration.EnableConsole) {
#ifdef WIN32
            FreeConsole();
#endif
        }

        ConfigFile = fopen("concisesettings.ini", "wb");

        fwrite(&EngineConfiguration, sizeof(EngineConfiguration), 1, ConfigFile);

        fclose(ConfigFile);

        return;
    }
}