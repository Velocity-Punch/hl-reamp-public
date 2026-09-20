#ifdef _WIN32
#pragma once
#endif
#include "cbase.h"
#include "ai_basenpc.h"
#include "player.h"
#include "datacache/imdlcache.h"

extern ConVar npc_create_equipment;

void SpawnTheNPC(bool bWeapon,const char* classname)
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache(true);

	// Try to create entity
	CAI_BaseNPC* baseNPC = dynamic_cast<CAI_BaseNPC*>(CreateEntityByName(classname));
	if (baseNPC)
	{
		if (bWeapon)
			baseNPC->KeyValue("additionalequipment", npc_create_equipment.GetString());
		baseNPC->Precache();
		DispatchSpawn(baseNPC);

		// Now attempt to drop into the world
		QAngle angles;
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors(&forward);
		VectorAngles(forward, angles);
		angles.x = 0;
		angles.z = 0;
		AI_TraceLine(pPlayer->EyePosition(),
			pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
			pPlayer, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction != 1.0)
		{
			if (baseNPC->CapabilitiesGet() & bits_CAP_MOVE_FLY)
			{
				Vector pos = tr.endpos - forward * 36;
				baseNPC->Teleport(&pos, &angles, NULL);
			}
			else
			{
				// Raise the end position a little up off the floor, place the npc and drop him down
				tr.endpos.z += 12;
				baseNPC->Teleport(&tr.endpos, &angles, NULL);
				UTIL_DropToFloor(baseNPC, MASK_NPCSOLID);
			}

			// Now check that this is a valid location for the new npc to be
			Vector	vUpBit = baseNPC->GetAbsOrigin();
			vUpBit.z += 1;

			AI_TraceHull(baseNPC->GetAbsOrigin(), vUpBit, baseNPC->GetHullMins(), baseNPC->GetHullMaxs(),
				MASK_NPCSOLID, baseNPC, COLLISION_GROUP_NONE, &tr);
			if (tr.startsolid || (tr.fraction < 1.0))
			{
				baseNPC->SUB_Remove();
				DevMsg("Can't create %s.  Bad Position!\n", classname);
				NDebugOverlay::Box(baseNPC->GetAbsOrigin(), baseNPC->GetHullMins(), baseNPC->GetHullMaxs(), 255, 0, 0, 0, 0);
			}
		}
		else
		{
			baseNPC->Teleport(NULL, &angles, NULL);
		}

		baseNPC->Activate();
	}
	CBaseEntity::SetAllowPrecache(allowPrecache);
}
void CC_SpawnNPC(const CCommand& args)
{
	SpawnTheNPC(false, args[1]);
}
void CC_SpawnNPCWithWeapon(const CCommand& args)
{
	SpawnTheNPC(true, args[1]);
}
void CC_SpawnElderVort(const CCommand& args)
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache(true);

	// Try to create entity
	CAI_BaseNPC* baseNPC = dynamic_cast<CAI_BaseNPC*>(CreateEntityByName("npc_vortigaunt"));
	if (baseNPC)
	{
		baseNPC->KeyValue("SpawnClass", args[1]);
		baseNPC->Precache();
		DispatchSpawn(baseNPC);

		// Now attempt to drop into the world
		QAngle angles;
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors(&forward);
		VectorAngles(forward, angles);
		angles.x = 0;
		angles.z = 0;
		AI_TraceLine(pPlayer->EyePosition(),
			pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
			pPlayer, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction != 1.0)
		{
			if (baseNPC->CapabilitiesGet() & bits_CAP_MOVE_FLY)
			{
				Vector pos = tr.endpos - forward * 36;
				baseNPC->Teleport(&pos, &angles, NULL);
			}
			else
			{
				// Raise the end position a little up off the floor, place the npc and drop him down
				tr.endpos.z += 12;
				baseNPC->Teleport(&tr.endpos, &angles, NULL);
				UTIL_DropToFloor(baseNPC, MASK_NPCSOLID);
			}

			// Now check that this is a valid location for the new npc to be
			Vector	vUpBit = baseNPC->GetAbsOrigin();
			vUpBit.z += 1;

			AI_TraceHull(baseNPC->GetAbsOrigin(), vUpBit, baseNPC->GetHullMins(), baseNPC->GetHullMaxs(),
				MASK_NPCSOLID, baseNPC, COLLISION_GROUP_NONE, &tr);
			if (tr.startsolid || (tr.fraction < 1.0))
			{
				baseNPC->SUB_Remove();
				DevMsg("Can't create %s.  Bad Position!\n", "npc_vortigaunt");
				NDebugOverlay::Box(baseNPC->GetAbsOrigin(), baseNPC->GetHullMins(), baseNPC->GetHullMaxs(), 255, 0, 0, 0, 0);
			}
		}
		else
		{
			baseNPC->Teleport(NULL, &angles, NULL);
		}

		baseNPC->Activate();
	}
	CBaseEntity::SetAllowPrecache(allowPrecache);
}
void CC_SpawnImplion(const CCommand& args)
{
	MDLCACHE_CRITICAL_SECTION();

	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache(true);

	// Try to create entity
	CAI_BaseNPC* baseNPC = dynamic_cast<CAI_BaseNPC*>(CreateEntityByName("npc_antlion"));
	if (baseNPC)
	{
		baseNPC->AddSpawnFlags(1 << 18);
		baseNPC->Precache();
		DispatchSpawn(baseNPC);

		// Now attempt to drop into the world
		QAngle angles;
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors(&forward);
		VectorAngles(forward, angles);
		angles.x = 0;
		angles.z = 0;
		AI_TraceLine(pPlayer->EyePosition(),
			pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_NPCSOLID,
			pPlayer, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction != 1.0)
		{
			if (baseNPC->CapabilitiesGet() & bits_CAP_MOVE_FLY)
			{
				Vector pos = tr.endpos - forward * 36;
				baseNPC->Teleport(&pos, &angles, NULL);
			}
			else
			{
				// Raise the end position a little up off the floor, place the npc and drop him down
				tr.endpos.z += 12;
				baseNPC->Teleport(&tr.endpos, &angles, NULL);
				UTIL_DropToFloor(baseNPC, MASK_NPCSOLID);
			}

			// Now check that this is a valid location for the new npc to be
			Vector	vUpBit = baseNPC->GetAbsOrigin();
			vUpBit.z += 1;

			AI_TraceHull(baseNPC->GetAbsOrigin(), vUpBit, baseNPC->GetHullMins(), baseNPC->GetHullMaxs(),
				MASK_NPCSOLID, baseNPC, COLLISION_GROUP_NONE, &tr);
			if (tr.startsolid || (tr.fraction < 1.0))
			{
				baseNPC->SUB_Remove();
				DevMsg("Can't create %s.  Bad Position!\n", "npc_vortigaunt");
				NDebugOverlay::Box(baseNPC->GetAbsOrigin(), baseNPC->GetHullMins(), baseNPC->GetHullMaxs(), 255, 0, 0, 0, 0);
			}
		}
		else
		{
			baseNPC->Teleport(NULL, &angles, NULL);
		}

		baseNPC->Activate();
	}
	CBaseEntity::SetAllowPrecache(allowPrecache);
}
void CC_SpawnEntity(const CCommand& args)
{
	CBaseEntity* pEntity = (CBaseEntity*)CreateEntityByName(args[1]);
	if (pEntity)
	{
		pEntity->Precache();

		// Pass in any additional parameters.
		for (int i = 2; i + 1 < args.ArgC(); i += 2)
		{
			const char* pKeyName = args[i];
			const char* pValue = args[i + 1];
			pEntity->KeyValue(pKeyName, pValue);
		}

		DispatchSpawn(pEntity);
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		// Now attempt to drop into the world
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors(&forward);
		UTIL_TraceLine(pPlayer->EyePosition(),
			pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_SOLID,
			pPlayer, COLLISION_GROUP_NONE, &tr);
		if (tr.fraction != 1.0)
		{
			// Raise the end position a little up off the floor, place the npc and drop him down
			tr.endpos.z += 12;
			pEntity->Teleport(&tr.endpos, NULL, NULL);
			UTIL_DropToFloor(pEntity, MASK_SOLID);
		}

		pEntity->Activate();
	}

}

void CC_SpawnResistance(const CCommand& args)
{
	CBaseEntity* pEntity = (CBaseEntity*)CreateEntityByName("hlr_environmental_resistance");
	if (pEntity)
	{
		pEntity->Precache();
		pEntity->KeyValue("DamageType", args[1]);
		pEntity->KeyValue("BlockPoints", "30");
		DispatchSpawn(pEntity);
		CBasePlayer* pPlayer = UTIL_GetCommandClient();
		// Now attempt to drop into the world
		trace_t tr;
		Vector forward;
		pPlayer->EyeVectors(&forward);
		UTIL_TraceLine(pPlayer->EyePosition(),
			pPlayer->EyePosition() + forward * MAX_TRACE_LENGTH, MASK_SOLID,
			pPlayer, COLLISION_GROUP_NONE, &tr);
		if (tr.fraction != 1.0)
		{
			// Raise the end position a little up off the floor, place the npc and drop him down
			tr.endpos.z += 16;
			pEntity->Teleport(&tr.endpos, NULL, NULL);
		}

		pEntity->Activate();
	}
}

static ConCommand SpawnNPC("SpawnNPC", CC_SpawnNPC, "",FCVAR_HIDDEN);
static ConCommand SpawnNPCWithWeapon("SpawnNPCWithWeapon", CC_SpawnNPCWithWeapon, "", FCVAR_HIDDEN);
static ConCommand SpawnElderVort("SpawnElderVort", CC_SpawnElderVort, "", FCVAR_HIDDEN);
static ConCommand SpawnImplion("SpawnImplion", CC_SpawnImplion, "", FCVAR_HIDDEN);

static ConCommand SpawnEntity("SpawnEntity", CC_SpawnEntity, "", FCVAR_HIDDEN);
static ConCommand SpawnResistance("SpawnResistance", CC_SpawnResistance, "", FCVAR_HIDDEN);