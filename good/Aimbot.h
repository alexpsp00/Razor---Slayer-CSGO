#include "HitBox.h"

namespace CmdStuff{
	bool Aimbot(CUserCmd*, CBaseEntity*, CBaseCombatWeapon*, byte* bSendPacket);
	bool IsVisible(Vector&, Vector&, CBaseEntity*, CBaseEntity*);
	bool CanAutowall(Vector&, Vector&, CBaseEntity*, CBaseEntity*);
	bool GetHitBoxSpot(CUserCmd*, CBaseEntity*, CBaseEntity*, Vector&, Vector&, bool, int);
	CBaseEntity* GetTarget(CUserCmd*, CBaseEntity*, Vector&, Vector&, int WeaponID);
	int TranslateBone[128];
	int NextShot;
}

bool CmdStuff::CanAutowall(Vector& StartPos, Vector& EndPos, CBaseEntity* localPly, CBaseEntity* pEnt) {
	return false;
}

bool CmdStuff::IsVisible(Vector& StartPos, Vector& EndPos, CBaseEntity* localPly, CBaseEntity* pEnt) {
	trace_t tr;
	Ray_t ray;
	Vector vStart, vEnd, vEndPos[3];
	vStart = StartPos;
	vEnd = EndPos;
	CTraceFilterSkipTwoEntities filter = CTraceFilterSkipTwoEntities(localPly, pEnt);

	ray.Init(vStart, vEnd);
	Interfaces->Trace->TraceRay(ray, MASK_NPCWORLDSTATIC | CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_HITBOX | CONTENTS_GRATE, &filter, &tr);

	if (tr.fraction == 1.f || tr.m_pEnt == pEnt) return true;

	if (!Vars.Aimbot.Autowall) return false;

	vEndPos[0] = tr.endpos;

	ray.Init(vEnd, vStart);
	Interfaces->Trace->TraceRay(ray, MASK_NPCWORLDSTATIC | CONTENTS_SOLID | CONTENTS_MOVEABLE | CONTENTS_MONSTER | CONTENTS_WINDOW | CONTENTS_DEBRIS | CONTENTS_HITBOX | CONTENTS_GRATE, &filter, &tr);

	vEndPos[1] = tr.endpos;

	VectorSubtract(vEndPos[0], vEndPos[1], vEndPos[2]);

	float flLength = vEndPos[2].Length();

	if (tr.fraction != 0)
		if (flLength < 14)
			return true;

	return false;
}



bool CmdStuff::GetHitBoxSpot(CUserCmd* cmd, CBaseEntity* pEnt, CBaseEntity* localPly, Vector& EyePos, Vector& HitBox, bool MultiSpot, int iBone) {
	matrix3x4 bMatrix[128];
	if (!pEnt->GetRenderable()->SetupBones(bMatrix, 128, 0x100, 0.f)) return false;

	const model_t* pModel = pEnt->GetRenderable()->GetModel();
	if (!pModel) return false;

	studiohdr_t* studiohdr = Interfaces->ModelInfo->GetStudioModel(pModel);
	if (!studiohdr) return false;

	sizeof(studiohdr2_t);

	mstudiohitboxset_t* pHitBoxSet = studiohdr->pHitboxSet(pEnt->m_nHitboxSet());
	if (!pHitBoxSet) return false;

	Vector bbmin = Vector(-.3f, -1.5f, -2.640f);
	Vector bbmax = Vector(-1.f, 1.f, 2.016f);

	float scalecenter = Vars.Aimbot.PointScale;

	Vector SpotArray[30];

	static bool bOnce = false;
	if (!bOnce) {
		bOnce = true;
		TranslateBone[0] = 5;
	}

	SpotArray[0] = (bbmax + bbmin) * .5f;
	SpotArray[0].z = fabs(bbmin.z) + bbmax.z;
	if (MultiSpot) {
		for (int i = 1; i < Vars.Aimbot.MultiSpot * 6; i++) {
			SpotArray[i] = SpotArray[i - 1];
			SpotArray[i].z *= scalecenter;
			if (i % 3 == 1) {
				SpotArray[i].x = bbmin.x + 2.f;
				SpotArray[i].y = bbmin.y + 2.f;
			}
			else if (i % 3 == 2) {
				SpotArray[i].x = bbmax.x - 2.f;
				SpotArray[i].y = bbmax.y - 2.f;
			}
			
		}
	}

	if (iBone == 0) {
		float pitch = pEnt->m_angEyeAngles().x;
		if ((pitch > 45.f) && (pitch < 91.f))
		{
			SpotArray[0].x = bbmin.x * .65f;
			SpotArray[0].y = bbmax.y * .65f;
			SpotArray[0].z = (bbmin.z + bbmax.z) * .5f;
		}
		else if ((pitch >= -91.f) && (pitch <= -70.f))
		{
			SpotArray[0].x = bbmin.x * .65f;
			SpotArray[0].y = bbmax.y * .65f;
			SpotArray[0].z -= 1.f;
		}
	} else SpotArray[0] += SpotArray[0] * .5f;

	Vector BonePosition = Vector(bMatrix[TranslateBone[iBone]][0][3], bMatrix[TranslateBone[iBone]][1][3], bMatrix[TranslateBone[iBone]][2][3]);

	if (pEnt->GetChockedPackets() > 5 && Vars.Aimbot.Accuracy.LagFix) { // epic
		BonePosition -= pEnt->AbsOrigin();
		BonePosition += pEnt->m_vecNetworkOrigin();
	}

	for (int i = 0; i < ((Vars.Aimbot.MultiSpot) ? Vars.Aimbot.MultiSpot * 6 : 1); i++) {
		if (i)
			SpotArray[i].z -= fabs(bbmin.z);
		Vector temp = SpotArray[i] + BonePosition;
		if (!IsVisible(EyePos, temp, localPly, pEnt)) {
			if (!MultiSpot) return false;
			else continue;
		}
		HitBox = temp;
		return true;
	}

	return false;
}

CBaseEntity* CmdStuff::GetTarget(CUserCmd* cmd, CBaseEntity* localPly, Vector& HitBox, Vector& EyePos, int WeaponID) {
	CBaseEntity* pTarget = 0;
	bool NextShotValid = false;

	int iBone = 0;
	if ((WeaponID == 9 && Vars.Aimbot.AWPBody) || Vars.Aimbot.Bodyaim)
		iBone = 5;

	std::vector<int> NextShotVector;

	bool HitScan = Vars.Aimbot.Hitscan;

	for (int i = 1; i < 65; i++) {
		if (i == Interfaces->Engine->GetLocalPlayer()) continue;
		CBaseEntity* pEnt = Interfaces->EntList->GetClientEntity(i);
		if (!pEnt) continue;
		if (pEnt->GetNetworkable()->IsDormant()) continue;
		if (pEnt->m_lifeState() != 0) continue;
		if (pEnt->m_iHealth() < 1) continue;
		if (pEnt->m_iTeamNum() == localPly->m_iTeamNum()) continue;
		if (pEnt->m_bGunGameImmunity()) continue;
		if (Vars.Aimbot.Accuracy.LagFix == 2 && pEnt->GetChockedPackets() == 0 /*&& pEnt->m_vecVelocity().Length() > menuwidth.f*/) continue;
		if (WeaponID != 3 && HitScan && !Vars.Aimbot.Bodyaim) {
			bool quit = true;
			for (int iHitscanBone = 0; iHitscanBone < 10; iHitscanBone++) {
				if (GetHitBoxSpot(cmd, pEnt, localPly, EyePos, HitBox, Vars.Aimbot.MultiSpot, iHitscanBone)) {
					quit = false;
					break;
				}
			}
			if (quit) continue;
		}
		else if (!GetHitBoxSpot(cmd, pEnt, localPly, EyePos, HitBox, Vars.Aimbot.MultiSpot, iBone)) continue;

		if (i == NextShot)
			NextShotValid = true;
		else
			NextShotVector.push_back(i);
	}

	int iIndex = -1;

	if (!NextShotVector.empty() && NextShotVector.size() != 0) {
		iIndex = NextShotVector.at(rand() % NextShotVector.size());
	}

	if (iIndex == -1 && NextShotValid)
		iIndex = NextShot;

	NextShot = iIndex;

	if (iIndex != -1) {
		pTarget = Interfaces->EntList->GetClientEntity(iIndex);
		if (WeaponID != 3 && HitScan && !Vars.Aimbot.Bodyaim) {
			for (int iHitscanBone = 0; iHitscanBone < 10; iHitscanBone++) {
				if (GetHitBoxSpot(cmd, pTarget, localPly, EyePos, HitBox, Vars.Aimbot.MultiSpot, iHitscanBone)) break;
			}
		} else GetHitBoxSpot(cmd, pTarget, localPly, EyePos, HitBox, Vars.Aimbot.MultiSpot, iBone);
	}

	return pTarget;
}

bool CmdStuff::Aimbot(CUserCmd* cmd, CBaseEntity* localPly, CBaseCombatWeapon* pWeapon, byte* bSendPacket) {
	if (!Vars.Aimbot.Enabled) return false;
	if (!pWeapon->CanFire(localPly)) return false;
	if (!Vars.Aimbot.AutoShoot && !(cmd->buttons & IN_ATTACK)) return false;

	Vector HitBox;
	Vector EyePos = localPly->AbsOrigin() + localPly->m_vecViewOffset();
	CBaseEntity* pTarget = GetTarget(cmd, localPly, HitBox, EyePos, pWeapon->m_iItemDefinitionIndex());
	if (!pTarget) return false;

	CalcAngle(EyePos, HitBox, cmd->viewangles);
	if(Vars.Aimbot.Accuracy.NoRecoil) cmd->viewangles -= localPly->m_aimPunchAngle() * 2.f;
	AngleNormalize(cmd->viewangles);

	if (!Vars.Aimbot.Silent)
		Interfaces->Engine->SetViewAngles(cmd->viewangles);

	if (Vars.Aimbot.Silent == 2)
		*bSendPacket = 0;
	else
		*bSendPacket = 1;

	if(pWeapon->m_iItemDefinitionIndex() == 64)
		cmd->buttons |= IN_ATTACK2;
	else
		cmd->buttons |= IN_ATTACK;

	FakelagTick = -1;

	if (pTarget->GetChockedPackets() > 5) {
		/*float correct = 0.f;
		int lerpticks = TIME_TO_TICKS(Interfaces->Globals->interpolation_amount);
		correct = Interfaces->Engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) + TICKS_TO_TIME(lerpticks);
		int fix = pTarget->GetChockedPackets() + 1 + TIME_TO_TICKS(fabs(correct));*/
		cmd->tick_count += pTarget->GetChockedPackets() + 1;
	}

	Resolver::AddShot(pTarget, Interfaces->Engine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING) * 1.5f + Interfaces->Engine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING) * 1.25f);

	return true;
}