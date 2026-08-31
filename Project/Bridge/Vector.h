#pragma once

#include "../Graphics/Internal/Graphics.h"
#include "Lua/lua.hpp"
#include <functional>
#include <map>
#include <string>
#include <stdexcept>

using namespace std;

struct LuaVector {
    double x = 0;
    double y = 0;
    double z = 0;
    double a = 0;
};

static int lua_pushvector(lua_State* L, LuaVector v) {
    LuaVector* udata = (LuaVector*)lua_newuserdata(L, sizeof(LuaVector));
    *udata = v;

    luaL_getmetatable(L, "Vector");
    lua_setmetatable(L, -2);

    return 1;
}

LuaVector luaL_checkvector(lua_State* L, int index) {
    return *(LuaVector*)luaL_checkudata(L, index, "Vector");
}

bool lua_isvector(lua_State* L, int index) {
    return luaL_testudata(L, index, "Vector");
}

namespace Vector {
    static int l_Vector_new(lua_State* L) {
        double X = lua_tonumber(L, 1);
        double Y = lua_tonumber(L, 2);
        double Z = lua_tonumber(L, 3);
        double A = lua_tonumber(L, 4);

        lua_pushvector(L, { X, Y, Z, A });
        return 1;
    }

    static int l_Vector_index(lua_State* L) {
        LuaVector obj = luaL_checkvector(L, 1);
        const char* ckey = luaL_checkstring(L, 2);
        string key = ckey;
        string lkey = Utils::StrToLower(key);

        if (lkey == "x") {
            lua_pushnumber(L, obj.x);
        }
        else if (lkey == "y") {
            lua_pushnumber(L, obj.y);
        }
        else if (lkey == "z") {
            lua_pushnumber(L, obj.z);
        }
        else if (lkey == "a") {
            lua_pushnumber(L, obj.a);
        }
        else {
            lua_pushnil(L);
        }

        return 1;
    }

    static int l_Vector_eq(lua_State* L) {
        LuaVector obj = luaL_checkvector(L, 1);
        LuaVector obj2 = luaL_checkvector(L, 2);

        lua_pushboolean(L, memcmp(&obj, &obj2, sizeof(obj)) == 0);

        return 1;
    }

    static int l_Vector_add(lua_State* L) {
        LuaVector obj = luaL_checkvector(L, 1);
        LuaVector obj2 = luaL_checkvector(L, 2);

        lua_pushvector(L, { obj.x + obj2.x, obj.y + obj2.y, obj.z + obj2.z, obj.a + obj2.a });

        return 1;
    }

    static int l_Vector_sub(lua_State* L) {
        LuaVector obj = luaL_checkvector(L, 1);
        LuaVector obj2 = luaL_checkvector(L, 2);

        lua_pushvector(L, { obj.x - obj2.x, obj.y - obj2.y, obj.z - obj2.z, obj.a - obj2.a });

        return 1;
    }

    static int l_Vector_mul(lua_State* L) {
        LuaVector obj = luaL_checkvector(L, 1);

        if (lua_type(L, 2) == LUA_TNUMBER) {
            double obj2 = luaL_checknumber(L, 2);

            lua_pushvector(L, { obj.x * obj2, obj.y * obj2, obj.z * obj2, obj.a * obj2 });

            return 1;
        }

        LuaVector obj2 = luaL_checkvector(L, 2);

        lua_pushvector(L, { obj.x * obj2.x, obj.y * obj2.y, obj.z * obj2.z, obj.a * obj2.a });

        return 1;
    }

    static int l_Vector_div(lua_State* L) {
        LuaVector obj = luaL_checkvector(L, 1);

        if (lua_type(L, 2) == LUA_TNUMBER) {
            double obj2 = luaL_checknumber(L, 2);

            lua_pushvector(L, { obj.x / obj2, obj.y / obj2, obj.z / obj2, obj.a / obj2 });

            return 1;
        }

        LuaVector obj2 = luaL_checkvector(L, 2);

        lua_pushvector(L, { obj.x / obj2.x, obj.y / obj2.y, obj.z / obj2.z, obj.a / obj2.a });

        return 1;
    }

    static int l_Vector_unm(lua_State* L) {
        LuaVector obj = luaL_checkvector(L, 1);

        lua_pushvector(L, { -obj.x, -obj.y, -obj.z, -obj.a });

        return 1;
    }

    static int l_Vector_tostring(lua_State* L) {
        LuaVector obj = luaL_checkvector(L, 1);

        string S = to_string(obj.x) + ", " + to_string(obj.y) + ", " + to_string(obj.z) + to_string(obj.a);

        lua_pushstring(L, S.c_str());

        return 1;
    }

    int luaopen_vector(lua_State* L) {
        luaL_newmetatable(L, "Vector");

        lua_pushstring(L, "__index");
        lua_pushcfunction(L, l_Vector_index);
        lua_settable(L, -3);

        lua_pushstring(L, "__eq");
        lua_pushcfunction(L, l_Vector_eq);
        lua_settable(L, -3);

        lua_pushstring(L, "__add");
        lua_pushcfunction(L, l_Vector_add);
        lua_settable(L, -3);

        lua_pushstring(L, "__sub");
        lua_pushcfunction(L, l_Vector_sub);
        lua_settable(L, -3);

        lua_pushstring(L, "__mul");
        lua_pushcfunction(L, l_Vector_mul);
        lua_settable(L, -3);

        lua_pushstring(L, "__div");
        lua_pushcfunction(L, l_Vector_div);
        lua_settable(L, -3);

        lua_pushstring(L, "__unm");
        lua_pushcfunction(L, l_Vector_unm);
        lua_settable(L, -3);

        lua_pushstring(L, "__tostring");
        lua_pushcfunction(L, l_Vector_tostring);
        lua_settable(L, -3);

        lua_pop(L, 1);

        lua_register(L, "Vector", l_Vector_new);
        lua_register(L, "vector", l_Vector_new);
        lua_register(L, "Vector3", l_Vector_new);
        lua_register(L, "vector3", l_Vector_new);

        return 1;
    }
}

struct LuaCoordinateFrame {
    glm::vec3 Position = glm::vec3(0.0, 0.0, 0.0);
    glm::mat4 Rotation = glm::mat4(1.0f);
};


static int lua_pushcoordinateframe(lua_State* L, LuaCoordinateFrame v) {
    LuaCoordinateFrame* udata = (LuaCoordinateFrame*)lua_newuserdata(L, sizeof(LuaCoordinateFrame));
    *udata = v;

    luaL_getmetatable(L, "CoordinateFrame");
    lua_setmetatable(L, -2);

    return 1;
}

LuaCoordinateFrame luaL_checkcoordinateframe(lua_State* L, int index) {
    return *(LuaCoordinateFrame*)luaL_checkudata(L, index, "CoordinateFrame");
}

bool lua_iscoordinateframe(lua_State* L, int index) {
    return luaL_testudata(L, index, "CoordinateFrame");
}

namespace CoordinateFrame {
    static int lookAt(lua_State* L) {
        LuaVector camera = luaL_checkvector(L, 1);
        LuaVector target = luaL_checkvector(L, 2);

        glm::vec3 forward = glm::normalize(glm::vec3(target.x, target.y, target.z) - glm::vec3(camera.x, camera.y, camera.z));
        glm::vec3 worldUp = glm::vec3(0, 1, 0); // idt i need to make this a parameter

        glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
        glm::vec3 up = glm::cross(right, forward);

        glm::mat4 rotation(1.0f);

        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(up, 0.0f);
        rotation[2] = glm::vec4(-forward, 0.0f);

        LuaCoordinateFrame NewObj;
        NewObj.Position = glm::vec3(camera.x, camera.y, camera.z);
        NewObj.Rotation = rotation;

        lua_pushcoordinateframe(L, NewObj);

        return 1;
    }

    static int l_Vector_new(lua_State* L) {
        LuaCoordinateFrame Created;

        if (lua_isvector(L, 1) && lua_isvector(L, 2) && lua_gettop(L) == 2) {
            return lookAt(L);
        }
        else if (lua_isvector(L, 1) && lua_gettop(L) == 1) {
            auto V = luaL_checkvector(L, 1);

            Created.Position = glm::vec3(V.x, V.y, V.z);
        }
        else if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_gettop(L) == 3) {
            double X = lua_tonumber(L, 1);
            double Y = lua_tonumber(L, 2);
            double Z = lua_tonumber(L, 3);

            Created.Position.x = X;
            Created.Position.y = Y;
            Created.Position.z = Z;
        }
        else {
            luaL_error(L, "Invalid parameters passed to CFrame.new");

            return 0;
        }
        lua_pushcoordinateframe(L, Created);
        return 1;
    }

    static int l_Vector_index(lua_State* L) {
        LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 1);
        const char* ckey = luaL_checkstring(L, 2);
        string key = ckey;
        string lkey = Utils::StrToLower(key);

        if (lkey == "x") {
            lua_pushnumber(L, obj.Position.x);
        }
        else if (lkey == "y") {
            lua_pushnumber(L, obj.Position.y);
        }
        else if (lkey == "z") {
            lua_pushnumber(L, obj.Position.z);
        }
        else if (lkey == "position" || lkey == "pos") {
            lua_pushvector(L, LuaVector(obj.Position.x, obj.Position.y, obj.Position.z));
        }
        else if (lkey == "rotation") {
            LuaCoordinateFrame newt = obj;
            newt.Position = glm::vec3(0.0f, 0.0f, 0.0f);

            lua_pushcoordinateframe(L, newt);
        }
        else if (lkey == "rightvector" || lkey == "right" || lkey == "xvector") {
            glm::vec3 up = glm::vec3(obj.Rotation[0]);

            lua_pushvector(L, LuaVector(up.x, up.y, up.z));
        }
        else if (lkey == "upvector" || lkey == "up" || lkey == "yvector") {
            glm::vec3 up = glm::vec3(obj.Rotation[1]);

            lua_pushvector(L, LuaVector(up.x, up.y, up.z));
        }
        else if (lkey == "frontvector" || lkey == "front" || lkey == "zvector") {
            glm::vec3 up = glm::vec3(obj.Rotation[2]);

            lua_pushvector(L, LuaVector(up.x, up.y, up.z));
        }
        else if (lkey == "inverse") {
            obj.Position = -obj.Position;
            obj.Rotation = glm::inverse(obj.Rotation);

            lua_pushcoordinateframe(L, obj);
        }
        else if (lkey == "toworldspace") {
            lua_pushcfunction(L, [](lua_State* L) {
                LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 2);
                LuaCoordinateFrame obj2 = luaL_checkcoordinateframe(L, 3);

                obj.Position += obj2.Position;
                obj.Rotation *= obj2.Rotation;

                lua_pushcoordinateframe(L, obj);

                return 1;
                });
        }
        else if (lkey == "toobjectspace") {
            lua_pushcfunction(L, [](lua_State* L) {
                LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 2);
                LuaCoordinateFrame obj2 = luaL_checkcoordinateframe(L, 3);

                obj.Position -= obj2.Position;
                obj.Rotation *= glm::inverse(obj2.Rotation);

                lua_pushcoordinateframe(L, obj);

                return 1;
                });
        }
        else if (lkey == "toangles" || lkey == "toeulerangles" || lkey == "angles") {
            auto euler = glm::eulerAngles(glm::quat_cast(obj.Rotation));

            lua_pushvector(L, LuaVector(euler.x, euler.y, euler.z));
        }
        else {
            return luaL_error(L, "Invalid property of CoordinateFrame");
        }

        return 1;
    }

    static int l_Vector_eq(lua_State* L) {
        LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 1);
        LuaCoordinateFrame obj2 = luaL_checkcoordinateframe(L, 2);

        lua_pushboolean(L, memcmp(&obj, &obj2, sizeof(obj)) == 0);

        return 1;
    }

    static int l_Vector_add(lua_State* L) {
        LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 1);
        LuaCoordinateFrame obj2 = luaL_checkcoordinateframe(L, 2);

        obj.Position += obj2.Position;
        obj.Rotation *= obj2.Rotation;

        lua_pushcoordinateframe(L, obj);

        return 1;
    }

    static int l_Vector_sub(lua_State* L) {
        LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 1);
        LuaCoordinateFrame obj2 = luaL_checkcoordinateframe(L, 2);

        obj.Position -= obj2.Position;
        obj.Rotation *= glm::inverse(obj2.Rotation);

        lua_pushcoordinateframe(L, obj);

        return 1;
    }

    static int l_Vector_mul(lua_State* L) {
        LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 1);

        if (lua_type(L, 2) == LUA_TNUMBER) {
            double obj2 = luaL_checknumber(L, 2);

            obj.Position *= obj2;
            obj.Rotation *= obj2;

            lua_pushcoordinateframe(L, obj);

            return 1;
        }

        LuaCoordinateFrame obj2 = luaL_checkcoordinateframe(L, 2);


        obj.Position *= obj2.Position;

        obj.Rotation *= obj2.Rotation;
        obj.Rotation *= obj2.Rotation;

        lua_pushcoordinateframe(L, obj);

        return 1;
    }

    static int l_Vector_div(lua_State* L) {
        LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 1);

        if (lua_type(L, 2) == LUA_TNUMBER) {
            double obj2 = luaL_checknumber(L, 2);

            obj.Position /= obj2;
            obj.Rotation *= ( 1.0 / obj2 );

            lua_pushcoordinateframe(L, obj);

            return 1;
        }

        LuaCoordinateFrame obj2 = luaL_checkcoordinateframe(L, 2);


        obj.Position /= obj2.Position;

        obj.Rotation *= glm::inverse(obj2.Rotation);
        obj.Rotation *= glm::inverse(obj2.Rotation);

        lua_pushcoordinateframe(L, obj);

        return 1;
    }

    static int l_Vector_unm(lua_State* L) {
        LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 1);

        obj.Position = -obj.Position;
        obj.Rotation = glm::inverse(obj.Rotation);

        lua_pushcoordinateframe(L, obj);

        return 1;
    }

    static int l_Vector_tostring(lua_State* L) {
        LuaCoordinateFrame obj = luaL_checkcoordinateframe(L, 1);

        string S = to_string(obj.Position.x) + ", " + to_string(obj.Position.y) + ", " + to_string(obj.Position.z);

        lua_pushstring(L, S.c_str());

        return 1;
    }

    static int fromAngles(lua_State* L) {
        double X = lua_tonumber(L, 1);
        double Y = lua_tonumber(L, 2);
        double Z = lua_tonumber(L, 3);

        LuaCoordinateFrame obj;
        obj.Position = glm::vec3(0.0, 0.0, 0.0);
        obj.Rotation = Gl.DirectionFromEuler(X, Y, Z);

        lua_pushcoordinateframe(L, obj);

        return 1;
    }

    static int fromComponents(lua_State* L) {
        double X = luaL_checknumber(L, 1);
        double Y = luaL_checknumber(L, 2);
        double Z = luaL_checknumber(L, 3);

        double R00 = luaL_checknumber(L, 4);
        double R01 = luaL_checknumber(L, 5);
        double R02 = luaL_checknumber(L, 6);

        double R10 = luaL_checknumber(L, 7);
        double R11 = luaL_checknumber(L, 8);
        double R12 = luaL_checknumber(L, 9);

        double R20 = luaL_checknumber(L, 10);
        double R21 = luaL_checknumber(L, 11);
        double R22 = luaL_checknumber(L, 12);

        LuaCoordinateFrame obj;
        obj.Position = glm::vec3(X, Y, Z);
        obj.Rotation = glm::mat3(R00, R01, R02, R10, R11, R12, R20, R21, R22);

        lua_pushcoordinateframe(L, obj);

        return 1;
    }

    static const luaL_Reg cframelib[] = {
        {"new",   l_Vector_new},
        {"lookAt",   lookAt},
        {"fromAngles",   fromAngles},
        {"fromComponents", fromComponents},
        {NULL, NULL}
    };

    int realopen_cframe(lua_State* L) {
        luaL_newlib(L, cframelib);

        return 1;
    }

    int luaopen_vector(lua_State* L) {
        luaL_requiref(L, "CFrame", realopen_cframe, 1);
        lua_pop(L, 1);

        luaL_newmetatable(L, "CoordinateFrame");

        lua_pushstring(L, "__index");
        lua_pushcfunction(L, l_Vector_index);
        lua_settable(L, -3);

        lua_pushstring(L, "__eq");
        lua_pushcfunction(L, l_Vector_eq);
        lua_settable(L, -3);

        lua_pushstring(L, "__add");
        lua_pushcfunction(L, l_Vector_add);
        lua_settable(L, -3);

        lua_pushstring(L, "__sub");
        lua_pushcfunction(L, l_Vector_sub);
        lua_settable(L, -3);

        lua_pushstring(L, "__mul");
        lua_pushcfunction(L, l_Vector_mul);
        lua_settable(L, -3);

        lua_pushstring(L, "__div");
        lua_pushcfunction(L, l_Vector_div);
        lua_settable(L, -3);

        lua_pushstring(L, "__unm");
        lua_pushcfunction(L, l_Vector_unm);
        lua_settable(L, -3);

        lua_pushstring(L, "__tostring");
        lua_pushcfunction(L, l_Vector_tostring);
        lua_settable(L, -3);

        lua_pop(L, 1);

        return 1;
    }
}