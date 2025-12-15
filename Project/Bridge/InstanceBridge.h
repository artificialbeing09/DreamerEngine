#pragma once

#include "../World/Instance.h"

using namespace std;

enum enumIndexMode {
	Get,
	Set
};

namespace LuaEvent {
	struct FunctionListener {
		lua_State* base;
		int function_reference;
	};

	int EventTop = 1;

	static unordered_map<int, vector<FunctionListener>> ListeningFunctions = {};

	struct LuaEvent {
		shared_ptr<Instance> Object = NULL;
		const char* Name = "";
	};

	int lua_pushevent(lua_State* L, LuaEvent Value) {
		LuaEvent* mem = (LuaEvent*)lua_newuserdata(L, sizeof(LuaEvent));
		auto udata = new (mem) std::shared_ptr<Instance>(Value.Object);
		mem->Name = Value.Name;

		luaL_getmetatable(L, "Event");
		lua_setmetatable(L, -2);

		return 1;
	}

	LuaEvent luaL_checkevent(lua_State* L, int index) {
		if (lua_type(L, index) == LUA_TNIL)
			return { 0 };

		return *(LuaEvent*)luaL_checkudata(L, index, "Event");
	}

	struct EventListener {
		int Index1 = 0;
		FunctionListener Index2;
	};

	int lua_pushListener(lua_State* L, EventListener Value) {
		EventListener* udata = (EventListener*)lua_newuserdata(L, sizeof(EventListener));
		*udata = Value;

		luaL_getmetatable(L, "EventListener");
		lua_setmetatable(L, -2);

		return 1;
	}

	EventListener luaL_checkListener(lua_State* L, int index) {
		if (lua_type(L, index) == LUA_TNIL)
			return { 0 };

		return *(EventListener*)luaL_checkudata(L, index, "EventListener");
	}

	int EventConnect(lua_State* L) {
		LuaEvent Event = luaL_checkevent(L, 1);
		luaL_checktype(L, 2, LUA_TFUNCTION);
		string ClassName = Event.Object->GetType();
		string EventName = Event.Name;

		EventDescriptor* LOL = ClassEventDescriptorList[ClassName][EventName];

		int* ListeningIDPtr = LOL->GetFunction(Event.Object.get());

		if ((*ListeningIDPtr) == 0) {
			*ListeningIDPtr = EventTop++;
		}

		int ListeningID = *ListeningIDPtr;

		lua_pushvalue(L, 2);
		int func_ref = luaL_ref(L, LUA_REGISTRYINDEX);

		FunctionListener CNC = { L, func_ref };

		EventListener ListenerIndexes;
		ListenerIndexes.Index1 = ListeningID;
		ListenerIndexes.Index2 = CNC;

		lua_pushListener(L, ListenerIndexes);

		ListeningFunctions[ListeningID].push_back(CNC);

		return 1;
	}

	static int l_Event_index(lua_State* L) {
		const char* ckey = luaL_checkstring(L, 2);
		string key = ckey;
		string lkey = Utils::StrToLower(key);

		if (lkey == "connect") {
			lua_pushcfunction(L, EventConnect);
		}
		else {
			lua_pushnil(L);
		}

		return 1;
	}

	int EventListenerDisconnect(lua_State* L) {
		EventListener Event = luaL_checkListener(L, 1);

		auto& List = ListeningFunctions[Event.Index1];
		
		for (int i = 0; i < List.size(); i++) {
			FunctionListener o = List[i];

			if (o.base == Event.Index2.base && o.function_reference == Event.Index2.function_reference) {
				List[i] = std::move(List.back());
				List.pop_back();

				break;
			}
		}

		return 0;
	}

	static int l_EventListener_index(lua_State* L) {
		const char* ckey = luaL_checkstring(L, 2);
		string key = ckey;
		string lkey = Utils::StrToLower(key);

		if (lkey == "disconnect") {
			lua_pushcfunction(L, EventListenerDisconnect);
		}
		else {
			lua_pushnil(L);
		}

		return 1;
	}

	int luaopen_event(lua_State* L) {
		luaL_newmetatable(L, "Event");

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, l_Event_index);
		lua_settable(L, -3);

		lua_pop(L, 1);

		luaL_newmetatable(L, "EventListener");

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, l_EventListener_index);
		lua_settable(L, -3);

		lua_pop(L, 1);

		return 1;
	}
}

namespace InstanceBridge {
	int InstancePropertyIndex(shared_ptr<Instance> Object, string IndexName, enumIndexMode IndexMode, lua_State* L = NULL) {
		string Type = Object->GetType();
		PropertyDescriptor* Descriptor = ClassPropertyDescriptorList[Type][IndexName];

		if (Descriptor == NULL) {
			luaL_error(L, "Attempt to index non-existant property of instance.");
			return 0;
		}

		if (IndexMode == Get) {
			if (Descriptor->Type == L_String) {
				string* Value = (string*)Descriptor->GetFunction(Object);

				lua_pushstring((lua_State*)L, Value->c_str());
			}
			else if (Descriptor->Type == PropertyLuaTypes::L_Number) {
				double* Value = (double*)Descriptor->GetFunction(Object);

				lua_pushnumber((lua_State*)L, *Value);
			}
			else if (Descriptor->Type == PropertyLuaTypes::L_Int) {
				int64_t* Value = (int64_t*)Descriptor->GetFunction(Object);

				lua_pushinteger((lua_State*)L, *Value);
			}
			else if (Descriptor->Type == PropertyLuaTypes::L_Object) {
				shared_ptr<Instance>* Value = (shared_ptr<Instance>*)Descriptor->GetFunction(Object);

				if (*Value == NULL) {
					lua_pushnil(L);
				}
				else {
					lua_pushinstance(L, *Value);
				}
			}
			else if (Descriptor->Type == PropertyLuaTypes::L_Vector) {
				LuaVector* Value = (LuaVector*)Descriptor->GetFunction(Object);

				lua_pushvector(L, *Value);
			}
		}
		else if (IndexMode == Set) {
			if (Descriptor->Type == L_String) {
				string Value = luaL_checkstring(L, 3);
				Descriptor->SetFunction(Object, &Value);
			}
			else if (Descriptor->Type == PropertyLuaTypes::L_Number) {
				double Value = luaL_checknumber(L, 3);
				Descriptor->SetFunction(Object, &Value);
			}
			else if (Descriptor->Type == PropertyLuaTypes::L_Int) {
				int64_t Value = luaL_checkinteger(L, 3);
				Descriptor->SetFunction(Object, &Value);
			}
			else if (Descriptor->Type == PropertyLuaTypes::L_Object) {
				shared_ptr<Instance> Value = luaL_checkinstance(L, 3);
				Descriptor->SetFunction(Object, &Value);
			}
			else if (Descriptor->Type == PropertyLuaTypes::L_Vector) {
				LuaVector Value = luaL_checkvector(L, 3);
				Descriptor->SetFunction(Object, &Value);
			}
		}

		return 1;
	}

	lua_CFunction InstanceNamecallIndex(Instance* Object, string IndexName) {
		string Type = Object->GetType();

		LuaNamecallDescriptor* Descriptor = ClassLuaNamecallDescriptorList[Type][IndexName];

		if (Descriptor)
			return Descriptor->Function;

		// Later will add different types of namecalls

		return 0;
	}

	static int l_Instance_new(lua_State* L) {
		string InstanceType = luaL_checkstring(L, 1);
		auto Created = CreateInstanceOfType(InstanceType);

		lua_pushinstance(L, Created);

		return 1;
	}

	static int l_Instance_index(lua_State* L) {
		auto obj = luaL_checkinstance(L, 1);
		const char* key = luaL_checkstring(L, 2);

		if (ClassEventDescriptorList[obj->GetType()][key] != NULL) {
			LuaEvent::LuaEvent Event;
			Event.Object = obj;
			Event.Name = key;

			LuaEvent::lua_pushevent(L, Event);

			return 1;
		}

		auto Func = InstanceNamecallIndex(obj.get(), key);

		if (Func) {
			lua_pushcfunction(L, Func);

			return 1;
		}

		for (auto Inst : obj->GetChildren()) {
			if (Inst->GetName() == key) {
				lua_pushinstance(L, Inst);
				return 1;
			}
		}

		InstancePropertyIndex(obj, key, Get, L);

		return 1;
	}

	static int l_Instance_newindex(lua_State* L) {
		auto obj = luaL_checkinstance(L, 1);
		const char* key = luaL_checkstring(L, 2);
		InstancePropertyIndex(obj, key, Set, L);

		return 1;
	}

	static int l_Instance_eq(lua_State* L) {
		auto obj = luaL_checkinstance(L, 1);
		auto obj2 = luaL_checkinstance(L, 2);

		lua_pushboolean(L, obj.get() == obj2.get());

		return 1;
	}

	static int l_Instance_tostring(lua_State* L) {
		auto obj = luaL_checkinstance(L, 1);

		string WTF = obj->GetName();

		lua_pushstring(L, WTF.c_str());

		return 1;
	}

	static int l_Instance_gc(lua_State* L) {
		auto udata = static_cast<std::shared_ptr<Instance>*>(
			luaL_checkudata(L, 1, "Instance")
			);
		udata->~shared_ptr<Instance>();

		//cout << "Reset shared ptr" << endl;

		return 0;
	}

	int luaopen_Instance(lua_State* L) {
		luaL_newmetatable(L, "Instance");

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, l_Instance_index);
		lua_settable(L, -3);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, l_Instance_newindex);
		lua_settable(L, -3);

		lua_pushstring(L, "__eq");
		lua_pushcfunction(L, l_Instance_eq);
		lua_settable(L, -3);

		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, l_Instance_tostring);
		lua_settable(L, -3);

		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, l_Instance_gc);
		lua_settable(L, -3);

		lua_pop(L, 1);

		lua_register(L, "Instance", l_Instance_new);

		return 1;
	}

	int lua_initInstance(lua_State* L) {
		Vector::luaopen_vector(L);
		LuaEvent::luaopen_event(L);
		return luaopen_Instance(L);
	}
}