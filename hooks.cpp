#include "skse64/PapyrusVM.h"
#include "skse64/GameRTTI.h"
#include "skse64/GameData.h"
#include "skse64/GameObjects.h"
#include "skse64/PapyrusArgs.h"
#include "skse64/ObScript.h"
#include "hooks.h"


extern bool apv_status = true;

UInt32 g_questID = 0x0D62;
UInt32 g_factionKeywordID = 0x01D8A;


FurnitureActivateEventFunctor::FurnitureActivateEventFunctor(TESObjectREFR * refr, bool flag) : m_refr(refr), m_flag(flag)
{}


bool FurnitureActivateEventFunctor::Copy(Output * dst)
{
	VMClassRegistry * registry = (*g_skyrimVM)->GetClassRegistry();
	dst->Resize(2);
	PackValue(dst->Get(0), &m_refr, registry);
	dst->Get(1)->SetBool(m_flag);
	return true;
}

bool hooks::IsEnabled()
{
	return apv_status;
}

void hooks::SetEnabled(bool bStatus)
{
	apv_status = bStatus;
}

TESQuestEx* GetTargetQuest(const char* espname, UInt32 qstid)
{
	static TESQuestEx* result = nullptr;
	if (!result)
	{
		UInt32 formID = 0;

		const ModInfo* modInfo = DataHandler::GetSingleton()->LookupModByName(espname);
		if (modInfo)
		{
			UInt32 idx = modInfo->GetPartialIndex();
			if (idx != 0xFF)
				formID = (idx << 24) | qstid;
			if (formID != 0)
				result = static_cast<TESQuestEx*>(LookupFormByID(formID));
		}
	}
	return result;
}

UInt64 TESQuestEx::GetAliasHandle(UInt64 index)
{
	static UInt64 result;
	if (result == 0)
	{
		if (this && this->IsRunning())
		{
			BGSBaseAlias* baseAlias;
			if (!this->aliases.GetNthItem(index, baseAlias))
				return 0;

			BGSRefAlias*  refAlias = static_cast<BGSRefAlias*>(baseAlias);
			if (!refAlias)
				return 0;

			IObjectHandlePolicy* policy = (*g_skyrimVM)->GetClassRegistry()->GetHandlePolicy();
			result = (policy) ? policy->Create(refAlias->kTypeID, refAlias) : 0;
		}
	}
	return result;
}

bool TESQuestEx::IsRunning()
{
	return (unk0D8.flags & 1) != 0 && (unk0D8.flags >> 7 & 1) == 0 && unk148 == 0;
}

ObScript_Eval fn_org;
bool hook_fn(TESObjectREFR * thisObj, void * arg1, void * arg2, double& result)
{
	TESFaction* faction = static_cast<TESFaction*>(arg1);
	if (!faction)
		return fn_org(thisObj, arg1, arg2, result);
	if (faction->formID != g_factionKeywordID)
	{
#ifdef _DEBUG
		_MESSAGE("FALSE    g_factionKeywordID-->%08X  faction->formID-->%08X", g_factionKeywordID, faction->formID);
#endif
		return fn_org(thisObj, arg1, arg2, result);
	}
#ifdef _DEBUG
	_MESSAGE("TRUE    g_factionKeywordID-->%08X  faction->formID-->", g_factionKeywordID, faction->formID);
#endif

	if (!thisObj || !thisObj->GetNiNode())
		return false;

	TESFurniture* furniture = DYNAMIC_CAST(thisObj->baseForm, TESForm, TESFurniture);
	if (!furniture)
		return false;

	const static UInt32 FurnitureForce1rdPreson = 0x0A56D7;
	const static UInt32 FurnitureForce3rdPreson = 0x0A56D8;

	bool flag = true;
	if (furniture->keyword.HasKeyword(FurnitureForce1rdPreson) || furniture->keyword.HasKeyword(FurnitureForce3rdPreson))
	{
#ifdef _DEBUG
		_MESSAGE("HasKeyword:");
#endif

		flag = false;
	}
	if (furniture->unkE0.unk0 != 0) // unk78.unk0=Bench Type
	{
#ifdef _DEBUG
		_MESSAGE("Bench Type:");
#endif

		flag = false;
	}

#ifdef _DEBUG
	_MESSAGE("apv_status -- %d", apv_status);
#endif
	if (!flag)
		hooks::SetEnabled(flag);

	TESQuestEx* quest = GetTargetQuest("AutoPV.esp", g_questID);
	UInt64 handle = (quest) ? quest->GetAliasHandle(0) : 0;

	if (handle != 0)
	{
#ifdef _DEBUG
		_MESSAGE("send event");
#endif

		static const BSFixedString eventName = "APV_OnFurnitureActivate";
		FurnitureActivateEventFunctor args(thisObj, flag);
		(*g_skyrimVM)->GetClassRegistry()->QueueEvent(handle, &eventName, &args);
	}
	return true;
}

void hooks::init()
{

	apv_status = true;

	DataHandler* dhnd = DataHandler::GetSingleton();
	if (!dhnd)
		return;

	const ModInfo* modInfo = DataHandler::GetSingleton()->LookupModByName("AutoPV.esp");
	if (!modInfo)
		return;

	SInt32 modIndex = -1;
	modIndex = modInfo->GetPartialIndex();
	if (modIndex == -1)
		return;
	else if (modIndex < 0xFF)
	{
		g_factionKeywordID |= (modIndex << 24);
		g_questID |= (modIndex << 24);
	}
	else if (modInfo->IsLight())
	{
		g_factionKeywordID |= modIndex << 8;
		g_questID |= modIndex << 8;
	}
	else
	{
		_MESSAGE("failed. \"AutoPV.esp\" not detected.");
		return;
	}

	//check keyword id
	if (!LookupFormByID(g_factionKeywordID))
		return;

	for (ObScriptCommand* iter = g_firstObScriptCommand; iter->opcode < kObScript_NumObScriptCommands + kObScript_ScriptOpBase; ++iter)
	{
		if (std::strcmp(iter->longName, "GetIsCrimeFaction") == 0)
		{
			_MESSAGE("Hook GetIsCrimeFaction");
			fn_org = iter->eval;
			iter->eval = hook_fn;
			break;
		}
		
	}
}

