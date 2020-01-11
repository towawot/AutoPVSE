#pragma once

namespace camera
{
	void ForceFirstPersonModify(bool zoom = true);
	void ForceThirdPersonModify(bool zoom = true);
}

namespace papyrus
{
	void RegisterFuncs(VMClassRegistry* vm);
}
