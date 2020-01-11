#include "skse64/PluginAPI.h"
#include "skse64/GameRTTI.h"
#include "skse64/PapyrusVM.h"
#include"skse64/GameObjects.h"
#include "skse64/GameCamera.h"
#include "skse64/PapyrusNativeFunctions.h"
#include "hooks.h"
#include "papyrus.h"

//ThirdPersonState
//float					basePosX;			// 68
//float					basePosY;			// 6C
//float					basePosZ;			// 70
//float					dstPosY;			// 74
//float					curPosY;			// 78

void camera::ForceFirstPersonModify(bool zoom)
{
	PlayerCamera* camera = PlayerCamera::GetSingleton();
	if (!camera || !camera->cameraState)
		return;
	if (camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
		return;

	ThirdPersonState* state = static_cast<ThirdPersonState*>(camera->cameraStates[PlayerCamera::kCameraState_ThirdPerson2]);
	if (!state)
		return;

	if (state->unkDA == 1)
	{
		state->curPosY = state->dstPosY;
		return;
	}

	state->unk84 = state->curPosY;
	state->dstPosY = -0.2f;
	state->unkDA = 1;

	if (!zoom)
		state->curPosY = -0.2f;

}

void camera::ForceThirdPersonModify(bool zoom)
{
	PlayerCamera* camera = PlayerCamera::GetSingleton();
	if (!camera || !camera->cameraState)
		return;
	if (camera->cameraState->stateId >= PlayerCamera::kCameraState_ThirdPerson2)
		return;

	ThirdPersonState* state = static_cast<ThirdPersonState*>(camera->cameraStates[PlayerCamera::kCameraState_ThirdPerson2]);
	if (!state)
		return;

	state->basePosX = state->fOverShoulderPosX;
	state->basePosY = state->fOverShoulderCombatAddY;
	state->basePosZ = state->fOverShoulderPosZ;

	if (zoom)
	{
		state->curPosY = -0.2f;
		state->dstPosY = state->unk84;
	}
	else
	{
		state->curPosY = state->dstPosY;
	}

	CALL_MEMBER_FN(camera, SetCameraState)(state);
}

namespace papyrus
{
	void DebugTrace(StaticFunctionTag* base, BSFixedString s) { _MESSAGE("%s", s); }
	void ForceFirstPersonSmooth(StaticFunctionTag* base) { camera::ForceFirstPersonModify(true); }
	void ForceFirstPersonInstant(StaticFunctionTag* base) { camera::ForceFirstPersonModify(false); }
	void ForceThirdPersonSmooth(StaticFunctionTag* base) { camera::ForceThirdPersonModify(true); }
	void ForceThirdPersonInstant(StaticFunctionTag* base) { camera::ForceThirdPersonModify(false); }

	bool IsEnabled(StaticFunctionTag* base) { return hooks::IsEnabled(); }
	void SetEnabled(StaticFunctionTag* base, bool status) { hooks::SetEnabled(status); }

	float GetCastTime(StaticFunctionTag*, TESForm* pForm)
	{
		if (!pForm)
			return -1.0;

		if (pForm->formType == kFormType_Spell)
		{
			SpellItem* item = DYNAMIC_CAST(pForm, TESForm, SpellItem);
			return (item) ? item->data.castTime : -1.0;
		}
		else if (pForm->formType == kFormType_ScrollItem)
		{
			ScrollItem* item = DYNAMIC_CAST(pForm, TESForm, ScrollItem);
			return (item) ? item->data.castTime : -1.0;
		}
		else if (pForm->formType == kFormType_Enchantment)
		{
			EnchantmentItem* item = DYNAMIC_CAST(pForm, TESForm, EnchantmentItem);
			return (item) ? item->data.castTime : -1.0;
		}
		return -1.0;
	}

	SInt32 GetDeliveryType(StaticFunctionTag*, TESForm* pForm)
	{
		if (!pForm)
			return -1;

		if (pForm->formType == kFormType_Spell)
		{
			MagicItem::EffectItem* pEI = nullptr;
			SpellItem* item = DYNAMIC_CAST(pForm, TESForm, SpellItem);
			if (item)
				item->effectItemList.GetNthItem(0, pEI);
			return (pEI) ? pEI->mgef->properties.deliveryType : -1;
		}
		else if (pForm->formType == kFormType_ScrollItem)
		{
			MagicItem::EffectItem* pEI = nullptr;
			ScrollItem* item = DYNAMIC_CAST(pForm, TESForm, ScrollItem);
			if (item)
				item->effectItemList.GetNthItem(0, pEI);
			return (pEI) ? pEI->mgef->properties.deliveryType : -1;
		}
		else if (pForm->formType == kFormType_Enchantment)
		{
			EnchantmentItem* item = DYNAMIC_CAST(pForm, TESForm, EnchantmentItem);
			return (item) ? item->data.deliveryType : -1;
		}
		return -1;
	}

}

void papyrus::RegisterFuncs(VMClassRegistry* registry)
{
#ifdef _DEBUG
	_MESSAGE("papyrus::RegisterFuncs");
#endif
	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, void, BSFixedString>("DebugTrace", "AutoPV", papyrus::DebugTrace, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, void>("ForceFirstPersonSmooth", "AutoPV", papyrus::ForceFirstPersonSmooth, registry));
	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, void>("ForceFirstPersonInstant", "AutoPV", papyrus::ForceFirstPersonInstant, registry));
	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, void>("ForceThirdPersonSmooth", "AutoPV", papyrus::ForceThirdPersonSmooth, registry));
	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, void>("ForceThirdPersonInstant", "AutoPV", papyrus::ForceThirdPersonInstant, registry));
	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, float, TESForm*>("GetCastTime", "AutoPV", papyrus::GetCastTime, registry));
	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, SInt32, TESForm*>("GetDeliveryType", "AutoPV", papyrus::GetDeliveryType, registry));
	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, bool>("IsEnabled", "AutoPV", papyrus::IsEnabled, registry));
	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, void, bool>("SetEnabled", "AutoPV", papyrus::SetEnabled, registry));

}
