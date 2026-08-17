#pragma once

#include "../Bridge/Vector.h"

using namespace std;

#define NULLInstance shared_ptr<Instance>()

class Instance;

int lua_pushinstance(lua_State* L, shared_ptr<Instance> Value) {
	void* mem = lua_newuserdata(L, sizeof(std::shared_ptr<Instance>));
	auto udata = new (mem) std::shared_ptr<Instance>(Value);

	luaL_getmetatable(L, "Instance");
	lua_setmetatable(L, -2);

	return 1;
}

shared_ptr<Instance> luaL_checkinstance(lua_State* L, int index) {
	if (lua_isnoneornil(L, index))
		return NULL;

	return *(shared_ptr<Instance>*)luaL_checkudata(L, index, "Instance");
}

void ChildAddedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child);
void ChildRemovedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child);
void DescendantAddedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child);
void DescendantRemovedEventFirer(shared_ptr<Instance> Object, shared_ptr<Instance> Child);
void DestroyingEventFirer(shared_ptr<Instance> Object);

static int NumberOfDeletedInstances = 0;

class Instance : public std::enable_shared_from_this<Instance> { // Class to define the structure for all game objects
protected: // Standard: don't use private for any classes
	string Name = "";

	string Type = "";

	weak_ptr<Instance> Parent;

	deque<shared_ptr<Instance>> Children = { }; // TODO: make this thread safe (if it isn't already)
public:
	int ParentChildrenIndex = 0;

	inline string GetName() { return Name; }

	inline void SetName(string NewName) { Name = NewName; }

	inline string GetType() { return Type; }

	inline void SetType(string Type) { /* Nothing */ }

	inline bool HasChildren() { return Children.size() > 0; }

	inline shared_ptr<Instance> GetParent() { return Parent.lock(); }

	inline shared_ptr<Instance> GetShared() { return shared_from_this(); }

	inline bool IsDescendantOf(shared_ptr<Instance> OtherObject) {
		shared_ptr<Instance> Cursor = shared_from_this();
		
		while (Cursor != NULL) {
			Cursor = Cursor->Parent.lock();

			if (Cursor.get() == OtherObject.get()) {
				return true;
			}
		}

		return false;
	}

	void SetParent(shared_ptr<Instance> NewParent) {
		auto LockedNewParent = NewParent;
		auto LockedParent = Parent.lock();

		if (LockedNewParent && NewParent->IsDescendantOf(shared_from_this())) {
			return; // No recursive parenting :)
		}

		if (LockedNewParent.get() == LockedParent.get()) {
			return;
		}

		if (LockedParent) {
			auto& C = LockedParent->Children;
			if (ParentChildrenIndex >= C.size() || C[ParentChildrenIndex].get() != this) { 
				// This code shouldn't ever happen but we're being safe

				for (int I = 0; I < C.size(); I++) {
					if (C[I].get() == this) {
						ParentChildrenIndex = I;
						break;
					}
				}

				cout << "Warning: ParentChildrenIndex was set to an invalid value. (This means unexpected behavior is happening)" << endl;
			}

			std::swap(LockedParent->Children[ParentChildrenIndex], LockedParent->Children.back());
			if (LockedParent->Children[ParentChildrenIndex])
				LockedParent->Children[ParentChildrenIndex]->ParentChildrenIndex = ParentChildrenIndex;
			LockedParent->Children.pop_back();

			OnChildChanged(shared_from_this());
		}
		
		if (LockedNewParent) {
			auto S = shared_from_this();

			if (S) {
				ParentChildrenIndex = LockedNewParent->Children.size();
				LockedNewParent->Children.push_back(S);
			}
			else {
				cout << "Error: Self doesn't exist (BAD BAD BAD BAD)" << endl;

				return;
			}
		}

		Parent = NewParent;

		OnParentChanged(LockedNewParent);
	}

	void Destroy() {
		DestroyingEventFirer(shared_from_this());

		SetParent(NULLInstance);

		// No possible way of deleting completely until its destructed.
	}

	inline const deque<shared_ptr<Instance>> GetChildren() {
		return Children;
	}

	inline const vector<shared_ptr<Instance>> GetDescendants() {
		vector<shared_ptr<Instance>> Descendants = {};

		for (auto Inst : Children) {
			Descendants.push_back(Inst);

			for (auto Inst : Inst->GetDescendants()) {
				Descendants.push_back(Inst);
			}
		}

		return Descendants;
	}

	int DestroyLua(lua_State* L) {
		shared_ptr<Instance> obj = luaL_checkinstance(L, 1);

		obj->OnParentChanged(NULL);

		obj->Destroy();

		return 0;
	}

	int ClearAllChildrenLua(lua_State* L) {
		auto obj = luaL_checkinstance(L, 1);

		for (auto I : obj->GetChildren()) {
			I->Destroy();
		}

		return 0;
	}

	int IsDescendantOfLua(lua_State* L) {
		auto obj = luaL_checkinstance(L, 1);
		auto obj2 = luaL_checkinstance(L, 2);

		lua_pushboolean(L, obj->IsDescendantOf(obj2));

		return 1;
	}

	int GetDescendantsLua(lua_State* L) {
		auto obj = luaL_checkinstance(L, 1);

		lua_newtable(L);

		int i = 0;

		for (auto Inst : obj->GetDescendants()) {
			lua_pushinstance(L, Inst);

			lua_rawseti(L, -2, ++i);        // table[i+1] = value (Lua uses 1-based arrays)
		}

		return 1;
	}

	int GetChildrenLua(lua_State* L) {
		auto obj = luaL_checkinstance(L, 1);

		lua_newtable(L);

		int i = 0;

		for (auto Inst : obj->GetChildren()) {
			lua_pushinstance(L, Inst);

			lua_rawseti(L, -2, ++i);        // table[i+1] = value (Lua uses 1-based arrays)
		}

		return 1;
	}

	virtual void OnChildChanged(shared_ptr<Instance> NewChild) { }
	virtual void OnParentChanged(shared_ptr<Instance> NewParent) { }

	virtual ~Instance() {
		//cout << "Deconstructor called!" << endl;

		for (auto I : GetChildren())
			I.reset();

		NumberOfDeletedInstances++;
	}

	int Changed = 0;
	int Destroying = 0;
	int ChildAdded = 0;
	int ChildRemoved = 0;
	int DescendantAdded = 0;
	int DescendantRemoved = 0;

	Instance() {
		Type = "Instance"; // Standard
		Name = Type; // Can be changed if needed
	}
};


enum PropertyLuaTypes {
	L_String,
	L_Object,
	L_Number,
	L_Int,
	L_Boolean,
	L_Vector,
	L_CFrame
};

struct PropertyDescriptor {
	PropertyLuaTypes Type = L_Int;
	std::function<void* (shared_ptr<Instance>)> GetFunction = NULL;
	std::function<void(shared_ptr<Instance>, void*)> SetFunction = NULL;
};

struct LuaNamecallDescriptor {
	lua_CFunction Function = NULL;
};

struct EventDescriptor {
	std::function<int* (void*)> GetFunction = NULL;
};

map<string, std::function<shared_ptr<Instance>()>> ClassCreators = {};
map<string, map<string, PropertyDescriptor*>> ClassPropertyDescriptorList = {};
map<string, map<string, LuaNamecallDescriptor*>> ClassLuaNamecallDescriptorList = {};
map<string, map<string, EventDescriptor*>> ClassEventDescriptorList = {};

bool EmplaceNewClassCreator(string ClassName, std::function<shared_ptr<Instance>()> Function) {
	ClassCreators.emplace(ClassName, Function);

	return 1;
}

shared_ptr<Instance> CreateInstanceOfType(string Type) {
	if (ClassCreators[Type] == NULL) {
		throw std::runtime_error("Attempt to create instance of non-existant type.");
	}

	shared_ptr<Instance> Created = ClassCreators[Type]();
	
	return Created;
}

bool EmplaceNewPropertyDescriptor(string ClassName, string PropertyName, std::function<void(shared_ptr<Instance>, void*)> SetFunction, std::function<void* (shared_ptr<Instance>)> GetFunction, PropertyLuaTypes Type) {
	PropertyDescriptor* NewProp = new PropertyDescriptor;

	NewProp->Type = Type;
	NewProp->SetFunction = SetFunction;
	NewProp->GetFunction = GetFunction;

	ClassPropertyDescriptorList[ClassName].emplace(PropertyName, NewProp);

	return 1;
}

bool EmplaceNewLuaNamecallDescriptor(string ClassName, string NamecallName, lua_CFunction Function) {
	LuaNamecallDescriptor* NewProp = new LuaNamecallDescriptor;

	NewProp->Function = Function;

	ClassLuaNamecallDescriptorList[ClassName].emplace(NamecallName, NewProp);

	return 1;
}

bool InheritPropertiesFunc(string ClassNameTo, string ClassNameFrom) {
	if (ClassNameTo == ClassNameFrom) {
		return 1;
	}

	for (auto P : ClassPropertyDescriptorList[ClassNameFrom]) {
		ClassPropertyDescriptorList[ClassNameTo].emplace(P.first, P.second);
	}

	for (auto P : ClassLuaNamecallDescriptorList[ClassNameFrom]) {
		ClassLuaNamecallDescriptorList[ClassNameTo].emplace(P.first, P.second);
	}

	for (auto P : ClassEventDescriptorList[ClassNameFrom]) {
		ClassEventDescriptorList[ClassNameTo].emplace(P.first, P.second);
	}

	return 1;
}

bool EmplaceNewEventDescriptor(string ClassName, string EventName, std::function<int* (void*)> GetFunction) {
	EventDescriptor* Descriptor = new EventDescriptor;

	Descriptor->GetFunction = GetFunction;

	ClassEventDescriptorList[ClassName].emplace(EventName, Descriptor);

	return true;
}

void ChangedEventFirer(shared_ptr<Instance> Object, string PropertyName);

#define CreateClassDescriptor(Class, ClassName, InheritedClassName) bool INTERNALCLASSMACRO##Class = EmplaceNewClassCreator(ClassName, []() {return dynamic_pointer_cast<Instance>(make_shared<Class>());}) && InheritPropertiesFunc(ClassName, InheritedClassName)
#define CreatePropertyDescriptor(Class, ClassName, PropertyName, Type, LuaType, SetFunction, GetFunction) EmplaceNewPropertyDescriptor(ClassName, PropertyName, [](shared_ptr<Instance> t, void* v) { auto o = (Class*)t.get(); auto f = SetFunction; ChangedEventFirer(t, PropertyName); (o->*f)(*(Type*)v); }, [](shared_ptr<Instance> t) { auto o = (Class*)t.get(); auto f = GetFunction; auto r = (o->*f)(); void* m = malloc(sizeof(r)); if (m != nullptr) { memcpy(m, &r, sizeof(r)); } return m; }, LuaType)

#define CreateLuaNamecallDescriptor(Class, ClassName, NamecallName, Function) EmplaceNewLuaNamecallDescriptor(ClassName, NamecallName, [](lua_State* L) { auto o = (Class*)0x1111; auto f = Function; return (o->*f)(L); })

#define CreateLuaEventDescriptor(Class, ClassName, EventName, GetThing) EmplaceNewEventDescriptor(ClassName, EventName, [](void* t){ return &(((Class*)t)->GetThing); })

auto propInstanceName = CreatePropertyDescriptor(Instance, "Instance", "Name", string, L_String, &Instance::SetName, &Instance::GetName);
auto propInstanceParent = CreatePropertyDescriptor(Instance, "Instance", "Parent", shared_ptr<Instance>, L_Object, &Instance::SetParent, &Instance::GetParent);
auto propInstanceType = CreatePropertyDescriptor(Instance, "Instance", "Type", string, L_String, &Instance::SetType, &Instance::GetType);

auto callInstanceGetChildren = CreateLuaNamecallDescriptor(Instance, "Instance", "GetChildren", &Instance::GetChildrenLua);
auto callInstanceGetDescendants = CreateLuaNamecallDescriptor(Instance, "Instance", "GetDescendants", &Instance::GetDescendantsLua);
auto callInstanceIsAncestorOf = CreateLuaNamecallDescriptor(Instance, "Instance", "IsDescendantOf", &Instance::IsDescendantOfLua);
auto callInstanceDestroy = CreateLuaNamecallDescriptor(Instance, "Instance", "Destroy", &Instance::DestroyLua);
auto callInstanceClearAllChildren = CreateLuaNamecallDescriptor(Instance, "Instance", "ClearAllChildren", &Instance::ClearAllChildrenLua);

auto eventInstanceChanged = CreateLuaEventDescriptor(Instance, "Instance", "Changed", Instance::Changed);
auto eventInstanceDestroying = CreateLuaEventDescriptor(Instance, "Instance", "Destroying", Instance::Destroying);
auto eventInstanceChildAdded = CreateLuaEventDescriptor(Instance, "Instance", "ChildAdded", Instance::ChildAdded);
auto eventInstanceChildRemoved = CreateLuaEventDescriptor(Instance, "Instance", "ChildRemoved", Instance::ChildRemoved);
auto eventInstanceDescendantAdded = CreateLuaEventDescriptor(Instance, "Instance", "DescendantAdded", Instance::DescendantAdded);
auto eventInstanceDescendantRemoved = CreateLuaEventDescriptor(Instance, "Instance", "DescendantRemoved", Instance::DescendantRemoved);

CreateClassDescriptor(Instance, "Instance", "Instance");