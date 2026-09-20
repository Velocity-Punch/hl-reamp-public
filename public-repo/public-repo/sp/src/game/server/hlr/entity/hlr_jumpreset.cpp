#include "cbase.h"
#include "basecombatweapon.h"
#include "basecombatcharacter.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "gamemovement.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "particle_parse.h"
#include "sprite.h"
#include "hl2_gamerules.h"
#include "movevars_shared.h"
#include "gamestats.h"

#include "tier0/memdbgon.h"


class CJumpReset : public CBaseAnimating
{
	DECLARE_CLASS(CJumpReset, CBaseAnimating);
public:
	void Spawn(void);
	void Precache(void);
	void Reenable(void);
	void TouchThink(CBaseEntity *pOther);
	void RadiusThink(void);
	DECLARE_DATADESC();
private:
	Vector m_vecOrigin;
};
LINK_ENTITY_TO_CLASS(hlr_jumpreset, CJumpReset);
BEGIN_DATADESC(CJumpReset)
// Function Pointers
DEFINE_THINKFUNC(Reenable),
DEFINE_THINKFUNC(RadiusThink),
DEFINE_FUNCTION(TouchThink),
END_DATADESC()

#define ORB_MODEL "models/spitball_large.mdl"

void CJumpReset::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, -Vector(64.0f, 64.0f, 64.0f), Vector(64.0f, 64.0f, 64.0f));
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	SetModel(ORB_MODEL);
	SetRenderColor(255, 215, 0);
	SetModelScale(2.0f);
	m_vecOrigin = GetAbsOrigin();
	SetTouch(&CJumpReset::TouchThink);
	SetThink(&CJumpReset::RadiusThink);
	SetNextThink(gpGlobals->curtime + 0.1f);
	DispatchParticleEffect("jumpreset_core", GetAbsOrigin(), GetAbsAngles(), this);
}
void CJumpReset::Precache(void)
{
	PrecacheModel(ORB_MODEL);
	PrecacheScriptSound("Jump_Reset.Single");
	PrecacheParticleSystem("jumpreset_core");
}
void CJumpReset::TouchThink(CBaseEntity *pOther) //something touched me
{
	if (pOther->IsPlayer())
	{
		SetAbsVelocity(vec3_origin);
		SetMoveType(MOVETYPE_NONE);
		extern IGameMovement *g_pGameMovement;
		CGameMovement *gm = dynamic_cast<CGameMovement *>(g_pGameMovement);
		gm->m_iJumpCount = 0;
		SetTouch(NULL);
		EmitSound("Jump_Reset.Single");
		//AddEffects(EF_NODRAW);
		SetThink(&CJumpReset::Reenable);
		SetNextThink(gpGlobals->curtime + 15.0f);
		StopParticleEffects(this);
	}
}
void CJumpReset::Reenable(void)
{
	SetTouch(&CJumpReset::TouchThink);
	UTIL_SetOrigin(this, m_vecOrigin);
	RemoveEffects(EF_NODRAW);
	DispatchParticleEffect("jumpreset_core", GetAbsOrigin(), GetAbsAngles(), this);
	SetThink(&CJumpReset::RadiusThink);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CJumpReset::RadiusThink(void)
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

	Vector dir = pPlayer->WorldSpaceCenter() - GetAbsOrigin();
	float dist = dir.Length();
	Vector outdir = dir.Normalized();
	if (dist < 256.f)
	{
		SetMoveType(MOVETYPE_FLY);
		SetAbsVelocity(outdir * 1000);
	}

	SetNextThink(gpGlobals->curtime + 0.1f);
}