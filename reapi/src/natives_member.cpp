#include "precompiled.h"

// native set_member(_index, any:_member, any:...);
static cell AMX_NATIVE_CALL set_member(AMX *amx, cell *params)
{
	enum args_member_e { arg_count, arg_index, arg_member, arg_value, arg_elem };
	member_t *member = memberlist[params[arg_member]];

	if (member == nullptr) {
		MF_LogError(amx, AMX_ERR_NATIVE, "set_member: unknown member id %i", params[arg_member]);
		return 0;
	}

	edict_t *pEdict = INDEXENT(params[arg_index]);
	if (pEdict == nullptr || pEdict->pvPrivateData == nullptr) {
		MF_LogError(amx, AMX_ERR_NATIVE, "set_member: invalid or uninitialized entity", params[arg_member]);
		return 0;
	}
	
	size_t value = *getAmxAddr(amx, params[arg_value]);
	size_t element = *getAmxAddr(amx, params[arg_elem]);

	switch (member->type)
	{
	case MEMBER_ENTITY:
		return -1;
	case MEMBER_CLASSPTR:
	{
		// native set_member(_index, any:_member, _value, _elem);
		CBaseEntity *pEntity = nullptr;
		edict_t *pEdictValue = INDEXENT(value);

		if (pEdictValue != nullptr) {
			pEntity = CBaseEntity::Instance(pEdictValue);
		}
		set_member<CBaseEntity *>(pEdict, member->offset, pEntity, element);
		return 1;
	}
	case MEMBER_EHANDLE:
	{
		// native set_member(_index, any:_member, _value, _elem);
		EHANDLE ehandle = get_member<EHANDLE>(pEdict, member->offset, element);
		edict_t *pEdictValue = INDEXENT(value);
		ehandle.Set(pEdictValue);
		return 1;
	}
	case MEMBER_EVARS:
		return -1;
	case MEMBER_EDICT:
	{
		// native set_member(_index, any:_member, _value, _elem);
		edict_t *pEdictValue = INDEXENT(value);
		set_member<edict_t *>(pEdict, member->offset, pEdictValue, element);
		return 1;
	}
	case MEMBER_VECTOR:
	{
		// native set_member(_index, any:_member, Float:_value[3], _elem);
		cell *pSource = g_amxxapi.GetAmxAddr(amx, params[arg_value]);
		Vector vecDest;
		vecDest.x = *(vec_t *)&pSource[0];
		vecDest.y = *(vec_t *)&pSource[1];
		vecDest.z = *(vec_t *)&pSource[2];

		set_member<Vector>(pEdict, member->offset, vecDest, element);
		return 1;
	}
	case MEMBER_CHAR_ARRAY:
	{
		// native set_member(_index, any:_member, const source[]);
		int len;
		char *source = g_amxxapi.GetAmxString(amx, params[arg_value], 0, &len);
		char *dest = get_member_direct<char *>(pEdict, member->offset);
		strcpy(dest, source);
		return 1;
	}
	case MEMBER_CHAR_POINTER:
	{
		// native set_member(_index, any:_member, const source[]);
		int len;
		char *source = g_amxxapi.GetAmxString(amx, params[arg_value], 0, &len);
		char *dest = get_member<char *>(pEdict, member->offset);

		if (dest != nullptr) {
			delete [] dest;
		}

		dest = new char[len + 1];
		strcpy(dest, source);

		set_member<char *>(pEdict, member->offset, dest);
		return 1;
	}
	case MEMBER_FLOAT:
	case MEMBER_INTEGER:
	{
		// native set_member(_index, any:_member, any:_value, _elem);
		set_member<int>(pEdict, member->offset, value, element);
		return 1;
	}
	case MEMBER_SHORT:
	{
		// native set_member(_index, any:_member, _value, _elem);
		set_member<short>(pEdict, member->offset, value, element);
		return 1;
	}
	case MEMBER_BYTE:
	{
		// native set_member(_index, any:_member, _value, _elem);
		set_member<byte>(pEdict, member->offset, value, element);
		return 1;
	}
	case MEMBER_BOOL:
	{
		// native set_member(_index, any:_member, bool:_value, _elem);
		set_member<bool>(pEdict, member->offset, value != 0, element);
		return 1;
	}
	case MEMBER_SIGNALS:
	{
		// native set_member(_index, any:_member, _value);
		CUnifiedSignals signal = get_member<CUnifiedSignals>(pEdict, member->offset, element);
		signal.Signal(value);
		return 1;
	}
	case MEMBER_DOUBLE:
	{
		// native set_member(_index, any:_member, any:_value, _elem);
		set_member<double>(pEdict, member->offset, *(float *)&value, element);
		return 1;
	}
	case MEBMER_REBUYSTRUCT:
		return -1;
	}

	return 0;
}

// native any:get_member(_index = 0, any:_member, any:...);
static cell AMX_NATIVE_CALL get_member(AMX *amx, cell *params)
{
	enum args_member_e { arg_count, arg_index, arg_member, arg_elem };
	member_t *member = memberlist[params[arg_member]];

	if (member == nullptr) {
		MF_LogError(amx, AMX_ERR_NATIVE, "Member (%d) is unknown", params[arg_member]);
		return 0;
	}

	edict_t *pEdict = INDEXENT(params[arg_index]);

	if (pEdict == nullptr || pEdict->pvPrivateData == nullptr) {
		return 0;
	}

	size_t element = *g_amxxapi.GetAmxAddr(amx, params[arg_elem]);

	switch (member->type)
	{
	case MEMBER_ENTITY:
		return -1;
	case MEMBER_CLASSPTR:
		// native any:get_member(_index, any:_member, element);
		return get_member<CBaseEntity *>(pEdict, member->offset, element)->entindex();
	case MEMBER_EHANDLE:
	{
		// native any:get_member(_index, any:_member, element);
		EHANDLE ehandle = get_member<EHANDLE>(pEdict, member->offset, element);
		edict_t *pEntity = ehandle.Get();
		if (pEntity != nullptr) {
			return ((CBaseEntity *)ehandle)->entindex();
		}
		return 0;
	}
	case MEMBER_EVARS:
		return -1;
	case MEMBER_EDICT:
	{
		// native any:get_member(_index, any:_member, element);
		edict_t *pEntity = get_member<edict_t *>(pEdict, member->offset, element);
		if (pEntity != nullptr) {
			return indexOfEdict(pEntity);
		}

		return 0;
	}
	case MEMBER_VECTOR:
	{
		// native any:get_member(_index, any:_member, any:output[], element);
		cell *pOutput = g_amxxapi.GetAmxAddr(amx, params[3]);
		Vector vecDest = get_member<Vector>(pEdict, member->offset, *g_amxxapi.GetAmxAddr(amx, params[4]));

		pOutput[0] = *(cell *)&vecDest.x;
		pOutput[1] = *(cell *)&vecDest.y;
		pOutput[2] = *(cell *)&vecDest.z;

		return 1;
	}
	case MEMBER_CHAR_ARRAY:
	{
		// native any:get_member(_index, any:_member, any:output[], maxlength);
		char *dest = get_member_direct<char *>(pEdict, member->offset);
		g_amxxapi.SetAmxString(amx, params[3], dest, *g_amxxapi.GetAmxAddr(amx, params[4]));
		return 1;
	}
	case MEMBER_CHAR_POINTER:
	{
		// native any:get_member(_index, any:_member, any:output[], maxlength);
		char *dest = get_member<char *>(pEdict, member->offset);
		if (dest != nullptr) {
			g_amxxapi.SetAmxString(amx, params[3], dest, *g_amxxapi.GetAmxAddr(amx, params[4]));
			return 1;
		}
		return 0;
	}
	case MEMBER_FLOAT:
	case MEMBER_INTEGER:
	{
		// native any:get_member(_index, any:_member, element);
		return get_member<int>(pEdict, member->offset, element);
	}
	case MEMBER_SHORT:
		// native any:get_member(_index, any:_member, element);
		return get_member<short>(pEdict, member->offset, element);
	case MEMBER_BYTE:
		// native any:get_member(_index, any:_member, element);
		return get_member<byte>(pEdict, member->offset, element);
	case MEMBER_BOOL:
		// native any:get_member(_index, any:_member, element);
		return get_member<bool>(pEdict, member->offset, element);
	case MEMBER_SIGNALS:
	{
		// native any:get_member(_index, any:_member, &_signal = 0, &_state = 0);
		CUnifiedSignals signal = get_member<CUnifiedSignals>(pEdict, member->offset);
		cell *pSignal = g_amxxapi.GetAmxAddr(amx, params[3]);
		cell *pState = g_amxxapi.GetAmxAddr(amx, params[4]);
		
		*pSignal = signal.GetSignal();
		*pState = signal.GetState();

		return 1;
	}
	case MEBMER_REBUYSTRUCT:
		return -1;
	}

	return 0;
}

// native set_member_game(any:_member, any:...);
static cell AMX_NATIVE_CALL set_member_game(AMX *amx, cell *params)
{
	enum args_member_e { arg_count, arg_member, arg_value, arg_elem };
	member_t *member = memberlist[params[arg_member]];

	if (member == nullptr) {
		MF_LogError(amx, AMX_ERR_NATIVE, "Member (%d) is unknown", params[arg_member]);
		return 0;
	}
	
	size_t value = *g_amxxapi.GetAmxAddr(amx, params[arg_value]);
	size_t element = *g_amxxapi.GetAmxAddr(amx, params[arg_elem]);

	switch (member->type)
	{
	case MEMBER_ENTITY:
	case MEMBER_EHANDLE:
	case MEMBER_EVARS:
	case MEMBER_EDICT:
	case MEMBER_VECTOR:
	case MEMBER_CHAR_ARRAY:
	case MEMBER_CHAR_POINTER:
	case MEMBER_BYTE:
	case MEMBER_SIGNALS:
	case MEBMER_REBUYSTRUCT:
		return -1;
	case MEMBER_CLASSPTR:
	{
		// native set_member_game(any:_member, _value, _elem);
		CBaseEntity *pEntity = NULL;
		edict_t *pEdictValue = INDEXENT(value);

		if (pEdictValue != nullptr) {
			pEntity = CBaseEntity::Instance(pEdictValue);
		}
		set_member<CBaseEntity *>((*g_pCSGameRules), member->offset, pEntity, element);
		return 1;
	}
	case MEMBER_FLOAT:
	case MEMBER_INTEGER:
	{
		// native set_member_game(any:_member, any:_value, _elem);
		set_member<int>((*g_pCSGameRules), member->offset, value, element);
		return 1;
	}
	case MEMBER_SHORT:
	{
		// native set_member_game(any:_member, _value, _elem);
		set_member<short>((*g_pCSGameRules), member->offset, value, element);
		return 1;
	}
	case MEMBER_BOOL:
	{
		// native set_member_game(any:_member, bool:_value, _elem);
		set_member<bool>((*g_pCSGameRules), member->offset, value != 0, element);
		return 1;
	}
	case MEMBER_DOUBLE:
	{
		// native set_member_game(any:_member, _value);
		set_member<double>((*g_pCSGameRules), member->offset, value);
		return 1;
	}
	}

	return 0;
}

// native get_member_game(any:_member, any:...);
static cell AMX_NATIVE_CALL get_member_game(AMX *amx, cell *params)
{
	enum args_member_e { arg_count, arg_member, arg_elem };
	member_t *member = memberlist[params[arg_member]];

	if (member == nullptr) {
		MF_LogError(amx, AMX_ERR_NATIVE, "Member (%d) is unknown", params[arg_member]);
		return 0;
	}

	size_t element = *g_amxxapi.GetAmxAddr(amx, params[arg_elem]);

	switch (member->type)
	{
	case MEMBER_ENTITY:
	case MEMBER_EHANDLE:
	case MEMBER_EVARS:
	case MEMBER_EDICT:
	case MEMBER_VECTOR:
	case MEMBER_CHAR_ARRAY:
	case MEMBER_CHAR_POINTER:
	case MEMBER_BYTE:
	case MEMBER_SIGNALS:
	case MEBMER_REBUYSTRUCT:
		return -1;
	case MEMBER_CLASSPTR:
		// native any:get_member_game(any:_member, element);
		return get_member<CBaseEntity *>((*g_pCSGameRules), member->offset, element)->entindex();
	case MEMBER_FLOAT:
	case MEMBER_INTEGER:
	{
		// native any:get_member_game(any:_member, element);
		// members for m_VoiceGameMgr
		if (params[arg_member] >= m_msgPlayerVoiceMask && params[arg_member] <= m_nMaxPlayers) {
			return get_member<int>(&((*g_pCSGameRules)->m_VoiceGameMgr), member->offset);
		}
		(*g_pCSGameRules)->m_iMapVotes[5] = 1337;
		return get_member<int>((*g_pCSGameRules), member->offset, element);
	}
	case MEMBER_SHORT:
		// native any:get_member_game(any:_member);
		return get_member<short>((*g_pCSGameRules), member->offset);
	case MEMBER_BOOL:
		// native any:get_member_game(any:_member);
		return get_member<bool>((*g_pCSGameRules), member->offset);
	case MEMBER_DOUBLE:
	{
		// native get_member_game(any:_member, _value);
		float flRet = (float)get_member<double>(&((*g_pCSGameRules)->m_VoiceGameMgr), member->offset);
		return *(cell *)&flRet;
	}
	}

	return 0;
}

AMX_NATIVE_INFO Member_Natives[] =
{
	{ "set_member", set_member },
	{ "get_member", get_member },
	
	{ "set_member_game", set_member_game },
	{ "get_member_game", get_member_game },

	{ nullptr, nullptr }
};
