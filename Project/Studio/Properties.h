#pragma once

#include "Explorer.h"

using namespace std;

namespace Studio {
    char* PropertyValueBuf[256];

    bool PropertyInstanceAdorneeEnabled = false;
    PropertyDescriptor* DescriptorForAdornee = NULL;

	void RenderProperties() {
		if (!SelectedObject) {
			return;
		}

        int PropertyI = 0;

        for (auto i : ClassPropertyDescriptorList[SelectedObject->GetType()]) {
            auto PropertyInfo = i.second;

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

            if (PropertyInfo->Type != L_Vector && PropertyInfo->Type != L_CFrame)
                flags |= ImGuiTreeNodeFlags_Leaf;

            bool Open = ImGui::TreeNodeEx(i.first.c_str(), flags);
            
            ImGui::SameLine();

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

                VectorString += format("{:.6g}", CurrentText->x) + ", ";
                VectorString += format("{:.6g}", CurrentText->y) + ", ";
                VectorString += format("{:.6g}", CurrentText->z);

                memcpy(PropertyValueStorage, VectorString.c_str(), 255);
                PropertyValueStorage[255] = 0;

                ImGui::PushID(i.first.c_str());

                bool VectorSet = ImGui::InputText(("###" + i.first).c_str(), PropertyValueStorage, 256, ImGuiInputTextFlags_EnterReturnsTrue);

                ImGui::PopID();

                if (Open) {
                    ImGui::Text("Not here yet");
                }

                if (VectorSet) {
                    vector<double> Lista = {};

                    string MadeNumber = "";

                    string Storage = string(PropertyValueStorage);
                    Storage += ",";

                    for (int i = 0; i < Storage.size(); i++) {
                        char CurrentCharacter = Storage[i];

                        if (CurrentCharacter == ',') {
                            double Num = 0;

                            try { Num = stod(MadeNumber); }
                            catch (exception& e) {}

                            Lista.push_back(Num);

                            MadeNumber = "";
                        }
                        else if ((CurrentCharacter >= '0' && CurrentCharacter <= '9') || CurrentCharacter == '-' || CurrentCharacter == '.') {
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
            else if (PropertyInfo->Type == L_CFrame) {
                LuaCoordinateFrame* CurrentTextC = (LuaCoordinateFrame*)PropertyInfo->GetFunction(SelectedObject);

                glm::vec3 CurrentText = CurrentTextC->Position;

                string VectorString = "";
                VectorString.reserve(256); // Will crash if this isn't there

                VectorString += format("{:.6g}", CurrentText.x) + ", ";
                VectorString += format("{:.6g}", CurrentText.y) + ", ";
                VectorString += format("{:.6g}", CurrentText.z) + "";

                memcpy(PropertyValueStorage, VectorString.c_str(), 255);
                PropertyValueStorage[255] = 0;

                ImGui::PushID(i.first.c_str());

                bool CFrameSet = ImGui::InputText(("###" + i.first).c_str(), PropertyValueStorage, 256, ImGuiInputTextFlags_EnterReturnsTrue);

                ImGui::PopID();

                if (Open) {
                    ImGui::Text("Not here yet");
                }

                if (CFrameSet) {
                    vector<double> Lista = {};

                    string MadeNumber = "";

                    string Storage = string(PropertyValueStorage);
                    Storage += ",";

                    for (int i = 0; i < Storage.size(); i++) {
                        char CurrentCharacter = Storage[i];

                        if (CurrentCharacter == ',') {
                            double Num = 0;

                            try { Num = stod(MadeNumber); }
                            catch (exception& e) {}

                            Lista.push_back(Num);

                            MadeNumber = "";
                        }
                        else if ((CurrentCharacter >= '0' && CurrentCharacter <= '9') || CurrentCharacter == '.') {
                            MadeNumber += CurrentCharacter;
                        }
                    }

                    LuaCoordinateFrame NewVec = { };

                    for (int K = 0; K < Lista.size(); K++)
                        if (K == 0)
                            NewVec.Position.x = Lista[K];
                        else if (K == 1)
                            NewVec.Position.y = Lista[K];
                        else if (K == 2)
                            NewVec.Position.z = Lista[K];



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

                string ExtractedValue = format("{:.6g}", * CurrentText);

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

            if (Open) {
                ImGui::TreePop();
            }

            PropertyI++;
        }
	}

    void InitProperties() {
        for (int I = 0; I < 256; I++) {
            PropertyValueBuf[I] = (char*)malloc(1024);
        }
    }
}