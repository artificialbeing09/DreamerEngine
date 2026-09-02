#pragma once

#include "Configuration.h"

using namespace std;

namespace Studio {
    shared_ptr<Instance> SelectedObject = NULL;

    void RenderDescendants(shared_ptr<Instance> Start, int LeftOffset);

    void RenderDescendants(shared_ptr<Instance> Start, int LeftOffset) {
        auto Children = Start->GetChildren();

        for (auto Inst : Children) {
            string Name = Inst->GetName();

            ImGui::PushID((int)Inst.get());

            ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_OpenOnArrow;

            if (Inst->GetChildren().size() == 0)
                flag |= ImGuiTreeNodeFlags_Leaf;

            if (Inst.get() == SelectedObject.get())
                flag |= ImGuiTreeNodeFlags_Selected;

            bool Open = ImGui::TreeNodeEx(Name.c_str(), flag);
            bool Clicked = ImGui::IsItemClicked();

            if (Clicked)
                if (Inst.get() == SelectedObject.get())
                    SelectedObject = NULL;
                else
                    SelectedObject = Inst;

            if (Open) {
                RenderDescendants(Inst, LeftOffset + 1);

                ImGui::TreePop();
            }

            ImGui::PopID();
        }
    }

    void RenderExplorer() {
        RenderDescendants(GetGameWorld(), 0);
    }
}