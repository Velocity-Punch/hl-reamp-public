#include "cbase.h"
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "hl2_player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "weapon_rpg.h"
#include "gamemovement.h"
#include "game.h"
#include "vstdlib/random.h"
#include "particle_parse.h"
#include "hl2_gamerules.h"
#include "movevars_shared.h"
#include "gamestats.h"
#include "triggers.h"
#include "particle_parse.h"
#include "particle_system.h"

#include "tier0/memdbgon.h"

#define JUMPPAD_MODEL "models/props_combine/combine_mine01.mdl"


ConVar debug_launchpad_dfo("debug_launchpad_dfo", "0", FCVAR_GAMEDLL);
ConVar debug_launchpad_trajectory("debug_launchpad_trajectory", "0", FCVAR_GAMEDLL);


class CLaunchpad : public CBaseEntity //set up the class
{
	DECLARE_CLASS(CLaunchpad, CBaseEntity);
public:
	void Spawn(void); //everything below is for setting up functions and variables
	void Precache(void);
	void Reenable(void);
	void AllowJump(void);
	void SetForce(inputdata_t &inputdata);
	void Enable(inputdata_t &inputdata);
	void Disable(inputdata_t &inputdata);
	void TouchThink(CBaseEntity *pOther);
	void PlayAnim(void);

	bool KeyValue(const char* szKeyName, const char* szValue);
	void Think();
	void DebugDrawLines();
	COutputEvent m_OnLaunch;
	void DistanceContext(void);
	float m_flgravitymod;
	virtual bool IsEnabled(void);
	bool m_bIsEnabled;
	bool m_bBlockAirControl;
	bool m_bBlockDistanceFromOrigin;
	//nsigned int PhysicsSolidMaskForEntity() const;
	float m_flpushforce;
	float m_fMaxThink;
	float m_fThinkLength;

	string_t m_iszSound;

	const char* szTargetEnt;
	char strEnt[64];
	Vector signalPoint;
	DECLARE_DATADESC();
};
LINK_ENTITY_TO_CLASS(hlr_launchpad, CLaunchpad);
LINK_ENTITY_TO_CLASS(hlr_jumppad, CLaunchpad);
BEGIN_DATADESC(CLaunchpad)
DEFINE_KEYFIELD(m_flpushforce, FIELD_FLOAT, "pushforce"),
DEFINE_KEYFIELD(m_iszSound, FIELD_SOUNDNAME, "puntsound"),
DEFINE_KEYFIELD(m_bIsEnabled, FIELD_BOOLEAN, "isenabled"),
DEFINE_KEYFIELD(m_flgravitymod, FIELD_FLOAT, "gravitymod"),
DEFINE_KEYFIELD(m_bBlockAirControl, FIELD_BOOLEAN, "blockaircontrol"),
DEFINE_KEYFIELD(m_bBlockDistanceFromOrigin, FIELD_BOOLEAN, "blocktype"),
DEFINE_KEYFIELD(m_fThinkLength, FIELD_FLOAT, "maxblocklength"),
DEFINE_OUTPUT(m_OnLaunch, "OnLaunch"),
// Function Pointers
DEFINE_THINKFUNC(Reenable),
DEFINE_THINKFUNC(PlayAnim),
DEFINE_THINKFUNC(Think),
DEFINE_THINKFUNC(DistanceContext),
DEFINE_THINKFUNC(AllowJump),
DEFINE_THINKFUNC(TouchThink),

DEFINE_INPUTFUNC(FIELD_VOID,"Enable",Enable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable",Disable),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetForce", SetForce),

DEFINE_FIELD(m_iszSound,FIELD_STRING),
DEFINE_FIELD(signalPoint,FIELD_VECTOR),
DEFINE_FIELD(szTargetEnt,FIELD_STRING),

END_DATADESC()
void CLaunchpad::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, -Vector(20.0f, 20.0f, 20.0f), Vector(20.0f, 20.0f, 20.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER | FSOLID_USE_TRIGGER_BOUNDS);

	RegisterThinkContext("DistanceContext");
	RegisterThinkContext("ResetContext");
	RegisterThinkContext("AllowJumpContext");
	RegisterThinkContext("AnimContext");
	RegisterThinkContext("DebugContext");

	//SetModel(JUMPPAD_MODEL);
	SetTouch(&CLaunchpad::TouchThink);
	SetContextThink(&CLaunchpad::Think, gpGlobals->curtime, "DebugContext");
}
void CLaunchpad::Precache(void)
{
	char *szSoundFile = (char *)STRING(m_iszSound);
	if (m_iszSound != NULL_STRING && strlen(szSoundFile) > 1)
	{
		if (*szSoundFile != '!')
		{
			PrecacheScriptSound(szSoundFile);
		}
	}
	PrecacheModel(JUMPPAD_MODEL);
	//PrecacheScriptSound("Weapon_Mortar.Single");
}

// HACKHACK - I can't be fucked to recompile every map to use the new entity keyvalues, just check for the old ones and copy the values into the new ones
bool CLaunchpad::KeyValue(const char* szKeyName, const char* szValue)
{
	if (FStrEq(szKeyName, "playerSpeed"))
	{
		m_flpushforce = atof(szValue);
	}
	else if (FStrEq(szKeyName, "launchTarget") || FStrEq(szKeyName,"target"))
	{
		V_strcpy_safe(strEnt, szValue);
		szTargetEnt = strEnt;
	}
	else
		return BaseClass::KeyValue(szKeyName, szValue);

	return true;
}
void CLaunchpad::Think()
{
	if (debug_launchpad_trajectory.GetBool())
		DebugDrawLines();
	SetNextThink(gpGlobals->curtime + 0.01f, "DebugContext");
}

void CLaunchpad::SetForce(inputdata_t &inputdata)
{
	m_flpushforce = inputdata.value.Float();
}
void CLaunchpad::Enable(inputdata_t &inputdata)
{
	m_bIsEnabled = true;
}
void CLaunchpad::Disable(inputdata_t &inputdata)
{
	m_bIsEnabled = false;
}
bool CLaunchpad::IsEnabled(void)
{
	return m_bIsEnabled;
}

Vector VecCheckThrow(CBaseEntity *pEdict, const Vector &vecSpot1, Vector vecSpot2, float flSpeed, float flTolerance, float flgravitymod)
{
	flSpeed = MAX(1.0f, flSpeed);

	float svgravity = GetCurrentGravity();

	float flGravity = svgravity * flgravitymod;
	//flGravity *= flgravitymod;

	Vector vecGrenadeVel = (vecSpot2 - vecSpot1);

	// throw at a constant time	
	float time = vecGrenadeVel.Length() / flSpeed;
	vecGrenadeVel = vecGrenadeVel * (1.0 / time);

	// adjust upward toss to compensate for gravity loss
	vecGrenadeVel.z += flGravity * time * 0.5;

	Vector vecApex = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5;
	vecApex.z += 0.5 * flGravity * (time * 0.5) * (time * 0.5);


	trace_t tr;
	UTIL_TraceLine(vecSpot1, vecApex, MASK_SOLID, pEdict, COLLISION_GROUP_NONE, &tr);


	UTIL_TraceLine(vecApex, vecSpot2, MASK_SOLID_BRUSHONLY, pEdict, COLLISION_GROUP_NONE, &tr);
	if (tr.fraction != 1.0)
	{
		bool bFail = true;

		// Didn't make it all the way there, but check if we're within our tolerance range
		if (flTolerance > 0.0f)
		{
			float flNearness = (tr.endpos - vecSpot2).LengthSqr();
			if (flNearness < Square(flTolerance))
			{


				bFail = false;
			}
		}

		if (bFail)
		{

			return vec3_origin;
		}
	}

	return vecGrenadeVel;
}

void CLaunchpad::DebugDrawLines()
{
	CBaseEntity* signalEntity = gEntList.FindEntityByName(NULL, szTargetEnt); //point to the target entity assigned in hammer

	if (!signalEntity)
		return;

	Vector vecStart = GetAbsOrigin();
	Vector vecEnd = signalEntity->GetAbsOrigin();

	Vector vecDir = VecCheckThrow(this, vecStart, vecEnd, m_flpushforce, (10.0f * 12.0f), m_flgravitymod);
	Vector vecGrav = Vector(0, 0, -GetCurrentGravity());

	float totaltime = (vecEnd - vecStart).Length() / m_flpushforce;

	for (int i = 0; i <= 16; i++)
	{
		float perc = float(i) / 16.f;
		float t = perc * totaltime;
		float perc2 = float(i + 1) / 16.f;
		float t2 = perc2 * totaltime;

		Vector vec1 = vecStart + vecDir * t + 0.5f * vecGrav * t * t;
		Vector vec2 = vecStart + vecDir * t2 + 0.5f * vecGrav * t2 * t2;

		DebugDrawLine(vec1, vec2, 255, 200, 0, false, 0.1f);
	}


}

void CLaunchpad::TouchThink(CBaseEntity *pOther) //something touched me
{
	if (!pOther) //if null
		return; //stop
	if (!IsEnabled()) //if i'm disabled
		return; //stop
	if (pOther->IsWorld()) //if i touched a world object
		return; //stop

	SetTouch(NULL); //disable temporarily
	SetContextThink(&CLaunchpad::Reenable, gpGlobals->curtime + 0.1f, "ResetContext");//schedule re-enable

	char *szSoundFile = (char *)STRING(m_iszSound); //emit sound chosen in hammer

	DevMsg("Launchpad looking up entity %s\n", szTargetEnt);
	CBaseEntity *signalEntity = gEntList.FindEntityByName(NULL, szTargetEnt); //point to the target entity assigned in hammer

	if (signalEntity) //if target exists
	{
		signalPoint = signalEntity->GetAbsOrigin(); //get the absolute origin of the target, save it as a vector
		DevMsg("launchpad target found! using launchpad function\n");
	}
	else //if not
	{
		DevMsg("launchpad target not found! using legacy jumppad function!\n"); //put an error in console
	}

	if (pOther->IsSolid()) //if what i touched is solid
	{
		m_OnLaunch.FireOutput(pOther, this);

		if (!signalEntity) //if no target entity is found, we're doing the legacy launch
		{
			QAngle angSrc = this->GetAbsAngles(); //get angles from hammer
			Vector vecDir, vecdnspd, veccurvel;
			float fldnspd, flpunchforce;
			AngleVectors(angSrc, &vecDir); //transform angles into a vector
			vecdnspd = pOther->GetAbsVelocity(); //get velocity vector
			fldnspd = vecdnspd[2]; //isolate y value of said vector
			if (fldnspd > 0.0f) //if it's greater than 0
				fldnspd = 0.0f; //set it to 0
			flpunchforce = (m_flpushforce + abs(fldnspd)); //calculate punch force by adding abs of downward velocity to defined push force
			if (pOther->IsSolid()) //was the thing that touched me solid?
			{
				veccurvel = pOther->GetAbsVelocity(); //whats its current velocity
				veccurvel[2] += flpunchforce; //add the punch force to the y value of the current velocity
				pOther->SetAbsVelocity(veccurvel); //apply the new velocity
				//pOther->VelocityPunch(vecDir * flpunchforce); //do punch
				SetTouch(NULL); //disable temporarily
				EmitSound(szSoundFile);//emit sound
				if (pOther->IsPlayer())
				{
					SetContextThink(&CLaunchpad::AllowJump, gpGlobals->curtime + 0.5f, "AllowJumpContext"); //allow me to be used again in half a second
					SetContextThink(&CLaunchpad::PlayAnim, gpGlobals->curtime + 0.1f, "AnimContext"); //play the third person jump animation to avoid a running loop
					extern IGameMovement *g_pGameMovement;
					CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
					gm->m_iJumpCount = 1;
					gm->m_bLaunchpadTimedBlock = true;
					
				}

			}
		}
		else //target is found, we're doing the targeted launch
		{
			Vector vecToss = VecCheckThrow(this, GetAbsOrigin(), signalPoint, m_flpushforce, (10.0f*12.0f), m_flgravitymod); //calculate the trajectory
			Vector vecToTarget = (signalPoint - GetAbsOrigin());
			VectorNormalize(vecToTarget);
			float flVelocity = VectorNormalize(vecToss);

			Vector vecStall = Vector(0, 0, 0); //stall the entity 
			EmitSound(szSoundFile);
			pOther->SetAbsVelocity(vecStall);

			if (pOther->IsPlayer()) //if it's the player 
			{
				CBasePlayer* pPlayer = ToBasePlayer(pOther);
				CHL2_Player* phl2player = dynamic_cast<CHL2_Player*>(pPlayer);
				extern IGameMovement *g_pGameMovement; //call the game movement code 
				CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement); //create a pointer for the game movement code
				gm->m_iJumpCount = 1; //set the jump count to 1 
				Vector vecVel = gm->GetMoveData()->m_vecVelocity;
				gm->GetMoveData()->m_vecVelocity += -vecVel;
				gm->m_bLaunchpadTimedBlock = true;
				//pOther->VelocityPunch(vecToss * flVelocity); //launch the player
				pOther->SetAbsVelocity(vecToss * flVelocity);
				SetContextThink(&CLaunchpad::AllowJump, gpGlobals->curtime + 0.5f, "AllowJumpContext");
				SetContextThink(&CLaunchpad::PlayAnim, gpGlobals->curtime + 0.1f, "AnimContext");
				if (m_bBlockAirControl)
				{
					gm->m_bBlockingAirControl = true;

					if (m_bBlockDistanceFromOrigin)
					{
						gm->m_bBlockDistanceFromOrigin = true;
						SetContextThink(&CLaunchpad::DistanceContext, gpGlobals->curtime, "DistanceContext");
						m_fMaxThink = gpGlobals->curtime + m_fThinkLength;
					}
				}
				phl2player->SetAnimation(PLAYER_JUMP);
				
			}
			else
			{
				QAngle angDir;
				VectorAngles(vecToTarget, angDir); //get the angle from start to target
				if (pOther->GetMoveType() && pOther->GetMoveType() == MOVETYPE_FLY) //if it isn't affected by gravity
				{
					pOther->SetMoveType(MOVETYPE_FLYGRAVITY); //make it affected by gravity 
				}
				if (pOther->IsNPC()) //if it's an npc
				{
					pOther->SetGroundEntity(NULL); //register it as in the air
				}
				pOther->SetGravity(1.0f); //set the gravity to 100%
				pOther->SetAbsOrigin(GetAbsOrigin()); //set the absolute origin to that of the launchpad
				
				pOther->SetAbsVelocity(vecToss * flVelocity); //launch the object

				if (!pOther->IsNPC())
					pOther->SetAbsAngles(angDir); //point it towards the target
			}

		}
	}
}
void CLaunchpad::AllowJump(void)
{
	extern IGameMovement* g_pGameMovement;
	CGameMovement* gm = dynamic_cast<CGameMovement*>(g_pGameMovement);
	gm->m_bLaunchpadTimedBlock = false;

}
void CLaunchpad::Reenable(void)
{
	SetTouch(&CLaunchpad::TouchThink);//re-enable
}
void CLaunchpad::PlayAnim(void)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();
	CHL2_Player* phl2player = dynamic_cast<CHL2_Player*>(pPlayer);
	phl2player->SetAnimation(PLAYER_JUMP);
}
void CLaunchpad::DistanceContext(void)
{
	extern IGameMovement *g_pGameMovement; //call the game movement code 
	CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
	if (gpGlobals->curtime >= m_fMaxThink)
	{
		SetThink(NULL);
		gm->m_fDistanceFromOrigin = 100;
		return;
	}
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	Vector playerPos = pPlayer->GetAbsOrigin();
	Vector vecSrc = GetAbsOrigin();
	Vector vecEnd = signalPoint;
	vecEnd[2] = vecSrc[2];
	playerPos[2] = vecSrc[2];
	float disttotarget = (vecEnd - vecSrc).Length2D();
	float disttoplayer = (playerPos - vecSrc).Length2D();
	float ratio = disttoplayer / disttotarget;
	if (debug_launchpad_dfo.GetBool())
	{
		DebugDrawLine(vecSrc, playerPos, 0, 128, 255, false, 1.0f);
		DebugDrawLine(vecSrc, vecEnd, 255, 0, 0, false, 1.0f);
		Msg("dist from src to tgt: %f\n", disttotarget);
		Msg("dist from player to tgt: %f\n", disttoplayer);
		DevMsg("dist ratio: %f\n", ratio);
	}
	 //create a pointer for the game movement code
	gm->m_fDistanceFromOrigin = ratio;

	SetNextThink(gpGlobals->curtime + 0.01f, "DistanceContext");
}


class CTriggerLaunch : public CBaseTrigger
{
	DECLARE_CLASS(CTriggerLaunch, CBaseTrigger);
public:
	void Spawn(void);
	void Enable(void);
	void Touch(CBaseEntity* pOther);
	void Reenable(void);

	void LegacyLaunch(CBaseEntity* pOther);
	void TargetedLaunch(CBaseEntity* pOther);


	float m_flPushForce;

	const char* szTargetEnt;
	Vector signalPoint;

	COutputEvent m_OnLaunch;

	DECLARE_DATADESC()
};

LINK_ENTITY_TO_CLASS(trigger_launch, CTriggerLaunch);
BEGIN_DATADESC(CTriggerLaunch)
	DEFINE_KEYFIELD(m_flPushForce,FIELD_FLOAT,"PushForce"),
	DEFINE_OUTPUT(m_OnLaunch, "OnLaunch"),
	DEFINE_KEYFIELD(szTargetEnt, FIELD_STRING, "target"),
	DEFINE_FUNCTION(Touch),
	DEFINE_THINKFUNC(Reenable),
END_DATADESC()

void CTriggerLaunch::Spawn(void)
{
	BaseClass::Spawn();
	InitTrigger();

}

void CTriggerLaunch::Touch(CBaseEntity* pOther)
{
	CBaseEntity* targetEnt = gEntList.FindEntityByName(NULL, szTargetEnt);

	if (!targetEnt)
	{
		LegacyLaunch(pOther);
	}
	else
	{
		signalPoint = targetEnt->GetAbsOrigin();
		TargetedLaunch(pOther);
	}
}

void CTriggerLaunch::LegacyLaunch(CBaseEntity* pOther)
{
	if (!pOther) //if null
		return; //stop
	if (pOther->IsWorld()) //if i touched a world object
		return; //stop

	if (!pOther->IsPlayer())
		return;

	QAngle angSrc = this->GetAbsAngles(); //get angles from hammer
	Vector vecDir;
	Vector vecdnspd;
	Vector veccurvel;
	float fldnspd;
	float flpunchforce;
	AngleVectors(angSrc, &vecDir); //transform angles into a vector
	vecdnspd = pOther->GetAbsVelocity(); //get velocity vector
	fldnspd = vecdnspd[2]; //isolate y value of said vector
	if (fldnspd > 0.0f)
		fldnspd = 0.0f;
	flpunchforce = (m_flPushForce + abs(fldnspd)); //calculate punch force by adding abs of downward velocity to defined push force
	if (pOther->IsSolid()) //was the thing that touched me solid?
	{
		veccurvel = pOther->GetAbsVelocity(); //whats its current velocity
		veccurvel[2] += flpunchforce; //add the punch force to the y value of the current velocity
		pOther->SetAbsVelocity(veccurvel); //apply the new velocity
		//pOther->VelocityPunch(vecDir * flpunchforce); //do punch
		SetNextThink(gpGlobals->curtime + 0.1f);//after 0.1 seconds

		if (pOther->IsPlayer())
		{
			extern IGameMovement* g_pGameMovement;
			CGameMovement* gm = dynamic_cast<CGameMovement*>(g_pGameMovement);
			gm->m_iJumpCount = 1;
			gm->m_bLaunchpadTimedBlock = true;
		}
		SetTouch(NULL);
		SetThink(&CTriggerLaunch::Reenable);
		SetNextThink(gpGlobals->curtime + 0.25f);
	}

}

void CTriggerLaunch::TargetedLaunch(CBaseEntity* pOther)
{
	if (!pOther) //if null
		return; //stop
	if (pOther->IsWorld()) //if i touched a world object
		return; //stop
	if (pOther->GetSolid() == SOLID_BSP)
		return;

	if (!pOther->IsPlayer())
		return;

	Vector vecToss = VecCheckThrow(this, pOther->GetAbsOrigin(), signalPoint, m_flPushForce, (10.0f * 12.0f), 1.0f); //calculate the trajectory
	Vector vecToTarget = (signalPoint - pOther->GetAbsOrigin());
	VectorNormalize(vecToTarget);
	float flVelocity = VectorNormalize(vecToss);

	Vector vecStall = Vector(0, 0, 0); //stall the entity 
	pOther->SetAbsVelocity(vecStall);

	if (pOther->IsPlayer()) //if it's the player 
	{
		CBasePlayer* pPlayer = ToBasePlayer(pOther);
		CHL2_Player* phl2player = dynamic_cast<CHL2_Player*>(pPlayer);
		extern IGameMovement* g_pGameMovement; //call the game movement code 
		CGameMovement* gm = dynamic_cast<CGameMovement*>(g_pGameMovement); //create a pointer for the game movement code
		gm->m_iJumpCount = 1; //set the jump count to 1 
		Vector vecVel = gm->GetMoveData()->m_vecVelocity;
		gm->GetMoveData()->m_vecVelocity += -vecVel;
		gm->m_bLaunchpadTimedBlock = true;
		//pOther->VelocityPunch(vecToss * flVelocity); //launch the player
		pOther->SetAbsVelocity(vecToss * flVelocity);
		phl2player->SetAnimation(PLAYER_JUMP);

	}
	else
	{
		QAngle angDir;
		VectorAngles(vecToTarget, angDir); //get the angle from start to target
		if (pOther->GetMoveType() && pOther->GetMoveType() == MOVETYPE_FLY) //if it isn't affected by gravity
		{
			pOther->SetMoveType(MOVETYPE_FLYGRAVITY); //make it affected by gravity 
		}
		if (pOther->IsNPC()) //if it's an npc
		{
			pOther->SetGroundEntity(NULL); //register it as in the air
		}
		pOther->SetGravity(1.0f); //set the gravity to 100%
		pOther->SetAbsOrigin(GetAbsOrigin()); //set the absolute origin to that of the launchpad

		pOther->SetAbsVelocity(vecToss * flVelocity); //launch the object

		if (!pOther->IsNPC())
			pOther->SetAbsAngles(angDir); //point it towards the target
	}

	SetThink(&CTriggerLaunch::Reenable);
	SetNextThink(gpGlobals->curtime + 0.25f);
}

void CTriggerLaunch::Reenable(void)
{
	extern IGameMovement* g_pGameMovement;
	CGameMovement* gm = dynamic_cast<CGameMovement*>(g_pGameMovement);
	gm->m_bLaunchpadTimedBlock = false;
	SetTouch(&CTriggerLaunch::Touch);
}


#define LAUNCHPAD_MODEL "models/props/lab/telepad.mdl"
class CSandboxLaunchpad : public CBaseAnimating
{
	DECLARE_CLASS(CSandboxLaunchpad, CBaseAnimating);
public:
	void Spawn();
	void Precache();
	void CreatePad();
private:
	CHandle<CParticleSystem> m_hSpitEffect;
	CHandle<CLaunchpad> m_hLaunchpad;
};

LINK_ENTITY_TO_CLASS(sandbox_launchpad,CSandboxLaunchpad)


void CSandboxLaunchpad::Spawn()
{
	Precache();
	SetModel(LAUNCHPAD_MODEL);
	CreatePad();
	SetSolid(SOLID_BBOX);
}

void CSandboxLaunchpad::Precache()
{
	PrecacheModel(LAUNCHPAD_MODEL);
	UTIL_PrecacheOther("hlr_launchpad");
}

void CSandboxLaunchpad::CreatePad()
{
	m_hLaunchpad = (CLaunchpad*)CreateEntityByName("hlr_launchpad");
	if (m_hLaunchpad)
	{
		m_hLaunchpad->SetParent(this);
		m_hLaunchpad->SetLocalOrigin(vec3_origin + Vector(0, 0, 16));
		m_hLaunchpad->KeyValue("playerSpeed", "1000");
		m_hLaunchpad->KeyValue("isenabled", "1");
		m_hLaunchpad->KeyValue("puntsound", "Weapon_Mortar.Single");
		m_hLaunchpad->KeyValue("gravitymod", "1");
		DispatchSpawn(m_hLaunchpad);
	}
	m_hSpitEffect = (CParticleSystem*)CreateEntityByName("info_particle_system");
	if (m_hSpitEffect)
	{
		m_hSpitEffect->KeyValue("effect_name", "fountain");
		m_hSpitEffect->KeyValue("start_active", "1");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffect->Activate();
	}
}