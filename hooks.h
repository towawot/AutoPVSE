#pragma once

#include "skse64/GameObjects.h"

extern bool apv_status;

class FurnitureActivateEventFunctor : public IFunctionArguments
{
public:
	FurnitureActivateEventFunctor(TESObjectREFR* refr, bool flag);
	virtual bool Copy(Output* dst);
private:
	TESObjectREFR* m_refr;
	bool m_flag;
};

class TESQuestEx : public TESQuest
{
public:
	UInt64 GetAliasHandle(UInt64 index);
	bool IsRunning(void);
};
TESQuestEx* GetTargetQuest(const char* espname, UInt32 qstid);

namespace hooks
{
	void init();
	bool IsEnabled(void);
	void SetEnabled(bool bStatus);
};

