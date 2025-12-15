#pragma once

#include "../../../Main/Utils.h"

namespace ObjParser {
    struct ObjTriangle {
        glm::vec3 Vertex[3];
        glm::vec3 TextureCoord[3];
        glm::vec3 Normal[3];
    };

    vector<string> SplitString(string Str, char del) {
        vector<string> res;
        stringstream ss(Str);
        string t;
        while (getline(ss, t, del) && t.c_str())
            res.push_back(t);
        return res;
    }

    vector<ObjTriangle> ParseObj(string Raw) {
        map<string, bool> RecognizedCommands = {};

        RecognizedCommands["v"] = true;
        RecognizedCommands["vn"] = true;
        RecognizedCommands["f"] = true;
        RecognizedCommands["vt"] = true;

        vector<string> Lines;

        for (string t : SplitString(Raw, '\n'))
            if (t.size() > 2 && RecognizedCommands[t.substr(0, t.find_first_of(' '))])
                Lines.push_back(t);

        vector<glm::vec3> Vertexes;
        vector<glm::vec3> Normals;
        vector<glm::vec3> TextureCoords;
        vector<ObjTriangle> Triangles;

        for (string Line : Lines) {
            string Command = Line.substr(0, Line.find_first_of(' '));

            if (Command == "v" || Command == "vn" || Command == "vt") {
                glm::vec3 pos(0, 0, 0);

                int i = -1;
                for (string t : SplitString(Line, ' '))
                    if (t.empty() || t == "\r" || ++i == 0)
                        continue;
                    else
                        pos[i - 1] = stof(t);

                if (Command == "v")
                    Vertexes.push_back(pos);
                else if (Command == "vn")
                    Normals.push_back(pos);
                else if (Command == "vt")
                    TextureCoords.push_back(pos);
            }

            if (Command == "f") {
                ObjTriangle tri;

                int vIndex = 0;
                for (string vtn : SplitString(Line, ' ')) {
                    if (vIndex >= 3 || vtn == "f" || vtn.empty()) continue;

                    int j = -1;
                    for (string num : SplitString(vtn, '/'))
                        if (num.empty())
                            continue;
                        else if (++j == 0)
                            tri.Vertex[vIndex] = Vertexes[stoi(num) - 1];
                        else if (j == 1)
                            tri.TextureCoord[vIndex] = TextureCoords[stoi(num) - 1];
                        else if (j == 2)
                            tri.Normal[vIndex] = Normals[stoi(num) - 1];
                        else
                            break;

                    vIndex++;
                }

                Triangles.push_back(tri);
            }
        }

        return Triangles;
    }

    vector<ObjTriangle> NormalizeMeshToUnitCube(vector<ObjTriangle> objs) {
        glm::vec3 minV(FLT_MAX), maxV(-FLT_MAX);

        for (auto& o : objs) {
            for (auto& v : o.Vertex) {
                minV = glm::min(minV, v);
                maxV = glm::max(maxV, v);
            }
        }

        glm::vec3 size = maxV - minV;
        float maxExtent = std::max({ size.x, size.y, size.z });
        float scale = 1.0f / maxExtent;

        for (auto& o : objs) {
            for (auto& v : o.Vertex) {
                v = (v - minV) * scale;
            }
        }

        return objs;
    }

    vector<float> ParsedObjToInterleaved(vector<ObjTriangle> Parsed) {
        vector<float> sphereThing = {};

        for (const ObjTriangle& tri : Parsed) {
            for (int j = 0; j < 3; j++) {
                sphereThing.push_back(tri.Vertex[j].x);
                sphereThing.push_back(tri.Vertex[j].y);
                sphereThing.push_back(tri.Vertex[j].z);
                sphereThing.push_back(tri.Normal[j].x);
                sphereThing.push_back(tri.Normal[j].y);
                sphereThing.push_back(tri.Normal[j].z);
                sphereThing.push_back(tri.TextureCoord[j].x);
                sphereThing.push_back(tri.TextureCoord[j].y);
            }
        }

        return sphereThing;
    }

    // ParseObj -> Normalize to Cube -> Convert to interleaved from string
    vector<float> DefaultParseObj(string Str) {
        return ParsedObjToInterleaved(NormalizeMeshToUnitCube(ParseObj(Str)));
    }
}