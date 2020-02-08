/*	-------------------------------------------------------------------------------------------------------
	� 1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */

#pragma once

#include "CvLuaMethodWrapper.h"

template<class Derived, class InstanceType, typename EnumType>
class CvLuaScopedIndirectInstance : public CvLuaMethodWrapper<Derived, InstanceType>
{
public:
	static void Push(lua_State* L, InstanceType* pkType);
	static void Push(lua_State* L, FObjectHandle<InstanceType> handle)
	{
		Push(L, handle.pointer());
	}
	static InstanceType* GetInstance(lua_State* L, int idx = 1, bool bErrorOnFail = true);

	//! Used by CvLuaMethodWrapper to know where first argument is.
	static const int GetStartingArgIndex();

protected:
	static void DefaultHandleMissingInstance(lua_State* L);
};



//------------------------------------------------------------------------------
// template members
//------------------------------------------------------------------------------
template<class Derived, class InstanceType, typename EnumType>
void CvLuaScopedIndirectInstance<Derived, InstanceType, EnumType>::Push(lua_State* L, EnumType eID)
{
	//Pushing an instance involves more than just actually pushing an enum into the
	//Lua stack.  There are some caching optimizations that are done as well as some
	//checks.
	//The first step is to load or create a global table <Typename> to store all member
	//methods and all pushed instances.  This conserves memory and offers faster pushing
	//speed.
	//If <Typename>.__instances[eID] is not nil, return that value.
	//otherwise push a new instance and assign it to __instances.

	//NOTE: Raw gets and sets are used as an optimization over using lua_[get,set]field
	lua_getglobal(L, Derived::GetTypeName());
	if(lua_isnil(L, -1))
	{
		//Typename wasn't found, time to build it.
		lua_pop(L, 1);
		lua_newtable(L);

		//Create weak __instances table.
		lua_pushstring(L, "__instances");
		lua_newtable(L);

		//Create __instances.mt
		lua_newtable(L);
		lua_pushstring(L, "__mode");
		lua_pushstring(L, "v");
		lua_rawset(L, -3);				// mt.__mode = "v";
		lua_setmetatable(L, -2);

		lua_rawset(L, -3);				//type.__instances = t;


		lua_pushvalue(L, -1);
		lua_setglobal(L, Derived::GetTypeName());

		Derived::PushMethods(L, lua_gettop(L));
	}
	const int type_index = lua_gettop(L);

	lua_pushstring(L, "__instances");
	lua_rawget(L, -2);

	const int instances_index = lua_gettop(L);

	lua_pushinteger(L, static_cast<int>(eID));
	lua_rawget(L, -2);					//retrieve type.__instances[pkType]

	if(lua_isnil(L, -1))
	{
		lua_pop(L, 1);

		//Push new instance
		lua_createtable(L, 0, 1);
		lua_pushinteger(L, static_cast<int>(eID));
		lua_setfield(L, -2, "__instance");

		lua_createtable(L, 0, 1);			// create mt
		lua_pushstring(L, "__index");
		lua_pushvalue(L, type_index);
		lua_rawset(L, -3);					// mt.__index = Type
		lua_setmetatable(L, -2);

		//Assign it in instances
		lua_pushinteger(L, static_cast<int>(eID));
		lua_pushvalue(L, -2);
		lua_rawset(L, instances_index);				//__instances[pkType] = t;
	}

	//VERIFY(instances_index > type_index);
	lua_remove(L, instances_index);
	lua_remove(L, type_index);
}
//------------------------------------------------------------------------------
template<class Derived, class InstanceType, typename EnumType>
InstanceType* CvLuaScopedIndirectInstance<Derived, InstanceType, EnumType>::GetInstance(lua_State* L, int idx, bool bErrorOnFail)
{
	const int stack_size = lua_gettop(L);
	bool bFail = true;

	InstanceType* pkInstance = NULL;
	if(lua_type(L, idx) == LUA_TTABLE)
	{
		lua_getfield(L, idx, "__instance");
		if(lua_type(L, -1) == LUA_TNUMBER)
		{
			EnumType eValue = static_cast<EnumType>(lua_tointeger(L, -1));
			pkInstance = static_cast<Derived*>(this)->ResolveInstance(eValue);
			if(pkInstance != NULL)
			{
				bFail = false;
			}
		}
	}

	lua_settop(L, stack_size);

	if(bFail && bErrorOnFail)
	{
		if(idx == 1)
			luaL_error(L, "Not a valid instance.  Either the instance is NULL or you used '.' instead of ':'.");
		Derived::HandleMissingInstance(L);
	}
	return pkInstance;
}
//------------------------------------------------------------------------------
template<class Derived, class InstanceType, typename EnumType>
const int CvLuaScopedIndirectInstance<Derived, InstanceType, EnumType>::GetStartingArgIndex()
{
	return 2;
}
//------------------------------------------------------------------------------
template<class Derived, class InstanceType, typename EnumType>
void CvLuaScopedIndirectInstance<Derived, InstanceType, EnumType>::DefaultHandleMissingInstance(lua_State* L)
{
	luaL_error(L, "Instance does not exist.");
}
