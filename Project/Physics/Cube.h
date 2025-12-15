#pragma once

/*

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
    glm::vec3 PositionA, glm::vec3 RotationA, glm::vec3 SizeA,
    glm::vec3 PositionB, glm::vec3 RotationB, glm::vec3 SizeB,
    glm::vec3& MTVAxis) {

    glm::mat4 RotatedA = RotationX(RotationA.x) * RotationY(RotationA.y) * RotationZ(RotationA.z);
    glm::mat4 RotatedB = RotationX(RotationB.x) * RotationY(RotationB.y) * RotationZ(RotationB.z);

    glm::vec3 HalfSizeA = SizeA / 2.0f;
    glm::vec3 HalfSizeB = SizeB / 2.0f;

    glm::vec3 AxesA[3] = {
        RotatedA * glm::vec4(1.0, 0.0, 0.0, 0.0),
        RotatedA * glm::vec4(0.0, 1.0, 0.0, 0.0),
        RotatedA * glm::vec4(0.0, 0.0, 1.0, 0.0)
    };

    glm::vec3 AxesB[3] = {
        RotatedB * glm::vec4(1.0, 0.0, 0.0, 0.0),
        RotatedB * glm::vec4(0.0, 1.0, 0.0, 0.0),
        RotatedB * glm::vec4(0.0, 0.0, 1.0, 0.0)
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

struct PhysicsCube {
    bool Anchored = false;
    glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 size = glm::vec3(1.0, 1.0, 1.0);
    glm::vec3 rotation = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 velocity = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 rotationvelocity = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 color = glm::vec3(1.0, 1.0, 1.0);
    float mass = 1.0f;
};

void SimulateCubes(vector<PhysicsCube>& Cubes) {
    // Apply Velocity

    for (PhysicsCube& Cube : Cubes) {
        if (Cube.Anchored)
            continue;

        if (Cube.velocity.y > -1.0f) {
            Cube.velocity += glm::vec3(0.0f, -0.01f, 0.0f);
        }

        Cube.position += Cube.velocity;
        Cube.rotation += Cube.rotationvelocity;
    }

    // Check Collisions

    for (int I = 0; I < Cubes.size(); I++) {
        PhysicsCube* CubeARPtr = &Cubes[I];
        for (int J = I + 1; J < Cubes.size(); J++) {
            PhysicsCube* CubeAPtr = CubeARPtr;
            PhysicsCube* CubeBPtr = &Cubes[J];

            if (CubeAPtr->Anchored && CubeBPtr->Anchored)
                continue;

            if (CubeAPtr->Anchored) {
                PhysicsCube* OldCubeB = CubeBPtr;

                CubeBPtr = CubeAPtr;
                CubeAPtr = OldCubeB;
            }

            PhysicsCube CubeA = *CubeAPtr;
            PhysicsCube CubeB = *CubeBPtr;

            glm::vec3 mtv;
            if (CubesIntersect(CubeA.position, CubeA.rotation, CubeA.size, CubeB.position, CubeB.rotation, CubeB.size, mtv)) {
                glm::vec3 normal = glm::normalize(mtv);

                CubeA.velocity *= 0.98f;

                float aDot = glm::dot(CubeA.velocity, normal);
                if (aDot < 0.0f)
                    CubeA.velocity -= normal * aDot;

                if (!CubeB.Anchored) {
                    CubeA.position += mtv * 0.5f;
                    CubeB.position -= mtv * 0.5f;

                    float bDot = glm::dot(CubeB.velocity, -normal);
                    if (bDot < 0.0f)
                        CubeB.velocity -= -normal * bDot;
                }
                else {
                    CubeA.position += mtv;
                }

                *CubeAPtr = CubeA;
                *CubeBPtr = CubeB;
            }
        }
    }
}


*/