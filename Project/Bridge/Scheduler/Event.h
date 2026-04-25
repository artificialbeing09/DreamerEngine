#pragma once

#include "Lua.h"

using namespace std;

namespace Scheduler::Event {
    enum FiredEventParameterType {
        TInstance,
        TNumber,
        TString,
        TBoolean,
        TInteger,
        TVector
    };

    struct FiredEventParameter {
        FiredEventParameterType Type;
        LuaVector v;
        shared_ptr<Instance> o;
        double n = 0.0;
        long i = 0;
        string s;
    };

    template <typename T>
    FiredEventParameter GetEventParamFromT(T Value) {
        FiredEventParameter Object;

        if (typeid(T).hash_code() == typeid(LuaVector).hash_code()) {
            Object.Type = TVector;
            Object.v = *(LuaVector*)&Value;
        }
        else if (typeid(T).hash_code() == typeid(shared_ptr<Instance>).hash_code()) {
            Object.Type = TInstance;
            Object.o = *(shared_ptr<Instance>*) & Value;
        }
        else if (typeid(T).hash_code() == typeid(string).hash_code()) {
            Object.Type = TString;
            Object.s = *(string*)&Value;
        }
        else if (typeid(T).hash_code() == typeid(double).hash_code()) {
            Object.Type = TNumber;
            Object.n = *(double*)&Value;
        }
        else if (typeid(T).hash_code() == typeid(long).hash_code()) {
            Object.Type = TInteger;
            Object.i = *(long*)&Value;
        }
        else if (typeid(T).hash_code() == typeid(int).hash_code()) {
            Object.Type = TInteger;
            Object.i = *(int*)&Value;
        }
        else if (typeid(T).hash_code() == typeid(bool).hash_code()) {
            Object.Type = TBoolean;
            Object.i = *(bool*)&Value;
        }
        else {
            string what = typeid(T).name();

            what = "Incompatible type: " + what;

            throw std::runtime_error(what);
        }

        return Object;
    }

    void lua_pusheventparam(lua_State* L, FiredEventParameter Param) {
        if (Param.Type == TInstance)
            lua_pushinstance(L, Param.o);
        else if (Param.Type == TInteger)
            lua_pushinteger(L, Param.i);
        else if (Param.Type == TNumber)
            lua_pushinteger(L, Param.n);
        else if (Param.Type == TString)
            lua_pushstring(L, Param.s.c_str());
        else if (Param.Type == TBoolean)
            lua_pushboolean(L, (int)Param.i);
        else if (Param.Type == TVector)
            lua_pushvector(L, Param.v);
        else
            throw std::runtime_error("Invalid event parameter type.");
    }

    int FireListenerID(int ListenerID, vector<FiredEventParameter> Parameters = {}) {
        auto Scheduler = Scheduler::GetScheduler();

        auto gL = Scheduler::Lua::GetGlobalState();

        if (LuaEvent::ListeningFunctions[ListenerID].size() == 0) {
            return 0;
        }

        // Note: whatever you do to this code, or any code with global state for that matter,
        //       be extremely careful because it can lead to the program corrupting and crashing
        //       with crash logs that don't make sense and that'll just be a waste of time.
        //       Thanks -- lightersmash

        for (auto f : (LuaEvent::ListeningFunctions[ListenerID])) {
            lua_rawgeti(gL, LUA_REGISTRYINDEX, f.function_reference);

            lua_State* nL = Scheduler::GenerateNewThread(gL, -1, true);;

            for (auto o : Parameters) {
                lua_pusheventparam(nL, o);
            }

            Scheduler::AddThreadToSchedulerLoop(nL, 0, "none", int(Parameters.size()));
        }

        return 0;
    }

    void FireListenerInstance(Instance* Object, const char* EventName, vector<FiredEventParameter> Parameters = {}) {
        if (Object == NULL) {
            return;
        }

        string Type = Object->GetType();

        EventDescriptor* LOL = ClassEventDescriptorList[Type][EventName];

        if (!LOL) {
            return;
        }

        int ListeningID = *(LOL->GetFunction(Object));

        if (ListeningID != 0) {
            FireListenerID(ListeningID, Parameters);
        }
    }
}

void ChildAddedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child) {
    Scheduler::Event::FireListenerInstance(Object.get(), "ChildAdded", { Scheduler::Event::GetEventParamFromT(Child) });
}

void ChildRemovedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child) {
    Scheduler::Event::FireListenerInstance(Object.get(), "ChildRemoved", { Scheduler::Event::GetEventParamFromT(Child) });
}

void DescendantAddedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child) {
    Scheduler::Event::FireListenerInstance(Object.get(), "DescendantAdded", { Scheduler::Event::GetEventParamFromT(Child) });
}

void DescendantRemovedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child) {
    Scheduler::Event::FireListenerInstance(Object.get(), "DescendantRemoved", { Scheduler::Event::GetEventParamFromT(Child) });
}

void DestroyingEventFirer(shared_ptr<Instance> Object) {
    Scheduler::Event::FireListenerInstance(Object.get(), "Destroying");
}

void ChangedEventFirer(shared_ptr<Instance> Object, string PropertyName) {
    Scheduler::Event::FireListenerInstance(Object.get(), "Changed", { Scheduler::Event::GetEventParamFromT(PropertyName) });
}