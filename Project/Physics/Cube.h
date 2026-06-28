#pragma once

#include "../Graphics/Graphics.h"

using namespace std;

namespace Physics {
    inline glm::mat4 RotationX(float angle) {
        float s = sin(angle);
        float c = cos(angle);
        return glm::mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, c, s, 0.0,
            0.0, -s, c, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
    }

    inline glm::mat4 RotationY(float angle) {
        float s = sin(angle);
        float c = cos(angle);
        return glm::mat4(
            c, 0.0, -s, 0.0,
            0.0, 1.0, 0.0, 0.0,
            s, 0.0, c, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
    }

    inline glm::mat4 RotationZ(float angle) {
        float s = sin(angle);
        float c = cos(angle);
        return glm::mat4(
            c, s, 0.0, 0.0,
            -s, c, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        );
    }

    inline bool CubesIntersect(
        glm::vec3 PositionA, glm::mat4 RotationA, glm::vec3 SizeA,
        glm::vec3 PositionB, glm::mat4 RotationB, glm::vec3 SizeB,
        glm::vec3& MTVAxis) {

        glm::mat4 RotatedA = glm::mat4(RotationA);
        glm::mat4 RotatedB = glm::mat4(RotationB);

        glm::vec3 HalfSizeA = SizeA / 2.0f;
        glm::vec3 HalfSizeB = SizeB / 2.0f;

        glm::vec3 AxesA[3] = {
            RotatedA * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
            RotatedA * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
            RotatedA * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)
        };

        glm::vec3 AxesB[3] = {
            RotatedB * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
            RotatedB * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
            RotatedB * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)
        };

        int AllAxesI = 0;

        glm::vec3 AllAxes[15];

        for (glm::vec3 AxisB : AxesB) {
            AllAxes[AllAxesI] = AxisB;
            AllAxesI++;
        }

        for (glm::vec3 AxisA : AxesA) {
            AllAxes[AllAxesI] = AxisA;
            AllAxesI++;

            for (glm::vec3 AxisB : AxesB) {
                glm::vec3 Axis = glm::cross(AxisA, AxisB);

                if (glm::length(Axis) > 1e-6f) { // idt this actually matters at all
                    AllAxes[AllAxesI] = glm::normalize(Axis);
                    AllAxesI++;
                }
            }
        }

        float MinOverlap = FLT_MAX;
        glm::vec3 DistanceBetweenCenters = PositionB - PositionA;

        for (int I = 0; I < AllAxesI; I++) {
            glm::vec3 Axis = AllAxes[I];

            float ProjectionA =
                HalfSizeA.x * abs(glm::dot(AxesA[0], Axis)) +
                HalfSizeA.y * abs(glm::dot(AxesA[1], Axis)) +
                HalfSizeA.z * abs(glm::dot(AxesA[2], Axis));

            float ProjectionB =
                HalfSizeB.x * abs(glm::dot(AxesB[0], Axis)) +
                HalfSizeB.y * abs(glm::dot(AxesB[1], Axis)) +
                HalfSizeB.z * abs(glm::dot(AxesB[2], Axis));

            float CenterDistance = abs(glm::dot(DistanceBetweenCenters, glm::normalize(Axis)));

            float Overlap = (ProjectionA + ProjectionB) - CenterDistance;

            if (Overlap < 0)
                return false; // Separating axis found

            if (Overlap < MinOverlap) {
                MinOverlap = Overlap;

                float dir = glm::dot(-DistanceBetweenCenters, Axis) < 0 ? -1.0f : 1.0f;
                MTVAxis = Axis * dir * MinOverlap;
            }
        }

        return true;
    }

    void SimulateCubes() {
        vector<RenderObjectStore_t*> PhysicsObjects;

        PhysicsObjects.reserve(10000);

        for (auto & Val : Graphics::Engine3D::RenderObjects) {
            string Name = Val.first;
            deque<RenderObjectStore_t>& List = Val.second;

            for (int i = 0; i < List.size(); i++) {
                PhysicsObjects.push_back(&List[i]);
            }
        }

        // Apply Velocity

        for (RenderObjectStore_t* Cube : PhysicsObjects) {
            RenderObjectStore_t RegularCube = *Cube;

            if (RegularCube.Physics.Anchored)
                continue;

            if (RegularCube.Physics.Velocity.y > -1.0f) {
                RegularCube.Physics.Velocity += glm::vec3(0.0f, -0.01f, 0.0f);
            }

            RegularCube.Object.Position += RegularCube.Physics.Velocity;
            
            //RegularCube.Object.Rotation += RegularCube.Physics.RotationVelocity;
            
            *Cube = RegularCube;
        }

        // Check Collisions

        for (int I = 0; I < PhysicsObjects.size(); I++) {
            RenderObjectStore_t* CubeA = PhysicsObjects[I];
            RenderObjectStore_t CubeAR = *PhysicsObjects[I];

            glm::mat4 R = CubeAR.Object.Rotation;

            CubeAR.Physics.InvInertiaTensorWorld =
                R * glm::mat4(CubeAR.Physics.InvInertiaTensorLocal) * glm::transpose(R);

            for (int J = I + 1; J < PhysicsObjects.size(); J++) {
                RenderObjectStore_t* CubeB = PhysicsObjects[J];
                RenderObjectStore_t CubeBR = *PhysicsObjects[J];

                if (CubeAR.Physics.Anchored && CubeBR.Physics.Anchored)
                    continue;

                if (CubeAR.Physics.Anchored) {
                    swap(CubeA, CubeB);
                    swap(CubeAR, CubeBR);
                }

                glm::vec3 mtv;
                if (CubesIntersect(
                    CubeAR.Object.Position,
                    CubeAR.Object.Rotation,
                    CubeAR.Object.Size,
                    CubeBR.Object.Position,
                    CubeBR.Object.Rotation,
                    CubeBR.Object.Size, mtv)) {

                    glm::mat4 R2 = CubeBR.Object.Rotation;

                    CubeBR.Physics.InvInertiaTensorWorld =
                        R2 * glm::mat4(CubeBR.Physics.InvInertiaTensorLocal) * glm::transpose(R2);

                    glm::vec3 normal = glm::normalize(mtv);

                    glm::vec3 contactPoint = (CubeAR.Object.Position + CubeBR.Object.Position) * 0.5f;//CubeAR.Object.Position + normal * 0.5f * glm::length(mtv); // jeez

                    glm::vec3 ra = contactPoint - CubeAR.Object.Position;
                    glm::vec3 rb = contactPoint - CubeBR.Object.Position;

                    glm::vec3 velA = CubeAR.Physics.Velocity + glm::cross(CubeAR.Physics.RotationVelocity, ra);
                    glm::vec3 velB = CubeBR.Physics.Velocity + glm::cross(CubeBR.Physics.RotationVelocity, rb);

                    glm::vec3 rv = velB - velA;

                    // ensure normal points from A → B
                    if (glm::dot(CubeBR.Object.Position - CubeAR.Object.Position, normal) < 0.0f)
                        normal = -normal;

                    // --- inverse masses ---
                    float invMassA = CubeAR.Physics.Anchored ? 0.0f : 1.0f / CubeAR.Physics.Mass;
                    float invMassB = CubeBR.Physics.Anchored ? 0.0f : 1.0f / CubeBR.Physics.Mass;

                    // --- relative velocity ---
                    float velAlongNormal = glm::dot(rv, normal);

                    float denom =
                        invMassA + invMassB +
                        glm::dot(normal,
                            glm::cross(CubeAR.Physics.InvInertiaTensorWorld * glm::cross(ra, normal), ra) +
                            glm::cross(CubeBR.Physics.InvInertiaTensorWorld * glm::cross(rb, normal), rb)
                        );

                    // don't resolve if separating
                    if (velAlongNormal < 0.0f)
                    {
                        float restitution = 0.5; // (fabs(velAlongNormal) < 0.5f) ? 0.0f : 0.5f;

                        float j = -(1.0f + restitution) * velAlongNormal;
                        j /= denom;

                        glm::vec3 impulse = j * normal;

                        if (!CubeAR.Physics.Anchored) {
                            CubeAR.Physics.Velocity -= invMassA * impulse;
                            //CubeAR.Physics.RotationVelocity -= CubeAR.Physics.InvInertiaTensorWorld * glm::cross(ra, impulse);
                        }

                        if (!CubeBR.Physics.Anchored) {
                            CubeBR.Physics.Velocity += invMassB * impulse;
                            //CubeBR.Physics.RotationVelocity += CubeBR.Physics.InvInertiaTensorWorld * glm::cross(rb, impulse);
                        }

                    }
                    else {
                        // optional: prevent tiny re-penetration
                        if (!CubeAR.Physics.Anchored)
                            CubeAR.Physics.Velocity -= normal * glm::dot(CubeAR.Physics.Velocity, normal);
                    }

                    // --- positional correction (anti-jitter) ---
                    const float percent = 0.8f;
                    const float slop = 0.01f;

                    float penetration = glm::length(mtv);

                    if (penetration > slop)
                    {
                        glm::vec3 correction = (penetration - slop)
                            / (invMassA + invMassB)
                            * percent * normal;

                        if (!CubeAR.Physics.Anchored)
                            CubeAR.Object.Position -= invMassA * correction;

                        if (!CubeBR.Physics.Anchored)
                            CubeBR.Object.Position += invMassB * correction;
                    }

                    *CubeA = CubeAR;
                    *CubeB = CubeBR;
                }
            }
        }
    }
}