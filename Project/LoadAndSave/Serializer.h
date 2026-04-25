#pragma once

#include "../World/World.h"

using namespace std;

namespace Serializer {
	string SerializeObject(shared_ptr<Instance> Object);

	string SerializeObject(shared_ptr<Instance> Object) {
        string ObjectInfo = "";

        ObjectInfo += "\"" + Object->GetType() + "\"\"" + to_string((uint64_t)Object.get()) + "\"";

        if (Object->GetType() == "Part") {
            shared_ptr<Part> ObjectNew = dynamic_pointer_cast<Part>(Object);
            uint16_t Textures[6] = {
                ObjectNew->Primitive->Texture0,
                ObjectNew->Primitive->Texture1,
                ObjectNew->Primitive->Texture2,
                ObjectNew->Primitive->Texture3,
                ObjectNew->Primitive->Texture4,
                ObjectNew->Primitive->Texture5
            };

            for (int i = 0; i < 6; i++) {
                ObjectInfo += "\"" + Texture::GetIDByTexture(Textures[i]) + "\"";
            }
        }

        ObjectInfo += "o";

        for (auto i : ClassPropertyDescriptorList[Object->GetType()]) {
            if (i.first == "Parent" || i.first == "Type") {
                continue;
            }

            ObjectInfo += "\"" + i.first + "\"";
            
            auto PropertyInfo = i.second;

            if (PropertyInfo->Type == L_String) {

                string* CurrentText = (string*)i.second->GetFunction(Object);

                string TestedString = *CurrentText;
                string NewString = "";

                char LastCharacter = '\0';

                for (int I = 0; I < TestedString.size(); I++) {
                    char CurrentCharacter = TestedString[I];

                    if (CurrentCharacter == '\"' && LastCharacter != '\\') {
                        NewString += '\\';
                    }

                    NewString += CurrentCharacter;

                    LastCharacter = CurrentCharacter;
                }

                ObjectInfo += "\"" + TestedString + "\"";
            }
            else if (PropertyInfo->Type == L_Object) {
                shared_ptr<Instance>* CurrentText = (shared_ptr<Instance>*)i.second->GetFunction(Object);

                ObjectInfo += "\"" + to_string((uint64_t)CurrentText->get()) + "\"";
            }
            else if (PropertyInfo->Type == L_Vector) {
                LuaVector* CurrentText = (LuaVector*)i.second->GetFunction(Object);

                string VectorString = "";

                VectorString += to_string(CurrentText->x) + ",";
                VectorString += to_string(CurrentText->y) + ",";
                VectorString += to_string(CurrentText->z) + ",";
                VectorString += to_string(CurrentText->a) + ";";

                ObjectInfo += "\"" + VectorString + "\"";
            }
            else if (PropertyInfo->Type == L_Int) {
                int64_t* CurrentText = (int64_t*)i.second->GetFunction(Object);

                ObjectInfo += "\"" + to_string(*CurrentText) + "\"";
            }
            else if (PropertyInfo->Type == L_Number) {
                double* CurrentText = (double*)i.second->GetFunction(Object);

                ObjectInfo += "\"" + to_string(*CurrentText) + "\"";
            }

            ObjectInfo += "p";
        }

        ObjectInfo += "c";

        for (shared_ptr<Instance> Obj : Object->GetChildren()) {
            ObjectInfo += SerializeObject(Obj);
        }

        ObjectInfo += "e";

        return ObjectInfo;
	}

	string Serialize(shared_ptr<GameWorld> WorldToSerialize) {
        string Saved = "";

		// Scene

        Saved += SerializeObject(Services::GetService<Scene>("Scene"));

        // UIScene

        Saved += SerializeObject(Services::GetService<UIScene>("UIScene"));

        return Saved;
	}

    void DeserializeIntoObject(shared_ptr<Instance> World, string SerializedGame, bool ClearWorld = false) {
        auto SceneObj = Services::GetService<Scene>("Scene");
        auto UISceneObj = Services::GetService<UIScene>("UIScene");

        if (ClearWorld) {
            for (shared_ptr<Instance> Inst : SceneObj->GetChildren()) {
                Inst->Destroy();
            }

            for (shared_ptr<Instance> Inst : UISceneObj->GetChildren()) {
                Inst->Destroy();
            }
        }

        vector<shared_ptr<Instance>> InstanceStack = { World };

        string DecodedString = "";
        bool InString = false;

        DecodedString.reserve(1000);

        char LastCharacterStorage = 0;

        vector<string> Stored = {};

        map<string, shared_ptr<Instance>> UUIDMap = {};

        shared_ptr<Instance> CurrentInstance = NULL;

        for (int I = 0; I < SerializedGame.size(); I++) {
            char LastCharacter = LastCharacterStorage;

            auto Character = SerializedGame[I];

            LastCharacterStorage = Character;

            bool QuotationActivated = false;
            if (Character == '"' && LastCharacter != '\\') {
                InString = !InString;
                QuotationActivated = true;
            }

            if (QuotationActivated && !InString) { // String ended
                Stored.push_back(DecodedString);
            }
            else if (!QuotationActivated && !InString) { // Special code
                cout << Character << endl;

                for (string Txt : Stored) {
                    cout << Txt << endl;
                }

                cout << "-" << endl;

                if (Character == 'o') {
                    if (Stored[0] == "Scene") {
                        CurrentInstance = SceneObj;
                    }
                    else if (Stored[0] == "UIScene") {
                        CurrentInstance = UISceneObj;
                    }
                    else {
                        CurrentInstance = CreateInstanceOfType(Stored[0]);

                        UUIDMap[Stored[1]] = CurrentInstance;

                        CurrentInstance->SetParent(InstanceStack[InstanceStack.size() - 1]);

                        if (CurrentInstance->GetType() == "Part") {
                            // add stuff later
                        }
                    }
                }
                else if (Character == 'p') {
                    auto Properties = ClassPropertyDescriptorList[CurrentInstance->GetType()];

                    auto PropertyInfo = Properties[Stored[0]];
                    auto PropertyValue = Stored[1];

                    cout << "Set Property: " << Stored[0] << endl;

                    if (PropertyInfo->Type == L_String) {
                        PropertyInfo->SetFunction(CurrentInstance, &PropertyValue); 
                        // Yes, this is safe because PropertyValue's pointer isn't going to be stored anywhere after the SetFunction call completes.
                    }
                    else if (PropertyInfo->Type == L_Object) {
                        PropertyInfo->SetFunction(CurrentInstance, &UUIDMap[PropertyValue]); 
                        // Incomplete (UUIDMap isn't full of all of the Instances once created).
                    }
                    else if (PropertyInfo->Type == L_Vector) {
                        string DecodedNumberStr = "";

                        LuaVector Value = { 0, 0, 0, 0 };

                        vector<double> ValueVector = { };

                        for (int J = 0; J < PropertyValue.size(); J++) {
                            char FoundCharacter = PropertyValue[J];

                            if (FoundCharacter == ',' || FoundCharacter == ';') {
                                ValueVector.push_back(stod(DecodedNumberStr));
                                DecodedNumberStr = "";
                            }
                            else {
                                DecodedNumberStr += FoundCharacter;
                            }
                        }

                        Value = { ValueVector[0], ValueVector[1], ValueVector[2], ValueVector[3] };

                        PropertyInfo->SetFunction(CurrentInstance, &Value);
                    }
                    else if (PropertyInfo->Type == L_Int) {
                        long long Value = stoll(PropertyValue);
                        PropertyInfo->SetFunction(CurrentInstance, &Value);
                    }
                    else if (PropertyInfo->Type == L_Number) {
                        double Value = stod(PropertyValue);
                        PropertyInfo->SetFunction(CurrentInstance, &Value);
                    }
                }
                else if (Character == 'c') {
                    InstanceStack.push_back(CurrentInstance);
                }
                else if (Character == 'e') {
                    InstanceStack.pop_back();
                }
                else {
                    cout << "That's not right." << endl;
                }

                Stored = {};
            }

            DecodedString += Character;

            if (QuotationActivated && InString) { // String started
                DecodedString = "";
            }
        }
    }
}