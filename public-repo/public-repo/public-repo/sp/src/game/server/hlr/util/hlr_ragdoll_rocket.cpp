#include "cbase.h"
#include "baseanimating.h"
#include "ai_basenpc.h"
#include "physconstraint.h"
#include "physics_prop_ragdoll.h"
#include "explode.h"
#include "smoke_trail.h"
#include "hlr_projectile.h"

#include "hlr_ragdoll_rocket.h"

#include "tier0/memdbgon.h"


#define PROPANE_TANK_MODEL "models/props_junk/propane_tank001a.mdl"

ConVar rocket_death_force("rocket_death_force", "1000");
LINK_ENTITY_TO_CLASS(ragdoll_rocket, CRagdollRocket);



void CRagdollRocket::Spawn()
{
	Precache();
	SetMoveType(MOVETYPE_VPHYSICS);
	SetModel(PROPANE_TANK_MODEL);
	SetSolid(SOLID_VPHYSICS);
	VPhysicsCreate();
}

void CRagdollRocket::Precache()
{
	PrecacheModel(PROPANE_TANK_MODEL);
}

bool CRagdollRocket::VPhysicsCreate()
{
	return VPhysicsInitNormal(SOLID_VPHYSICS, FSOLID_NOT_SOLID, false);
}

void CRagdollRocket::PropelThink()
{
	if (VPhysicsGetObject())
	{
		Vector vecUp;
		AngleVectors(GetAbsAngles(), NULL, NULL, &vecUp);
		Vector vecAbsUp = Vector(0, 0, 1);
		Vector vecAng = ((vecUp + vecAbsUp) * 0.5f).Normalized();
		VPhysicsGetObject()->ApplyForceCenter(vecAng * rocket_death_force.GetFloat());
	}

	if (gpGlobals->curtime > m_fExplodeTime)
	{
		SetThink(NULL);
		Explode();
	}

	SetNextThink(gpGlobals->curtime + 0.01f);
}
void CRagdollRocket::Explode()
{
	ExplosionCreate(GetAbsOrigin(), GetAbsAngles(), GetOwnerEntity(), GetDamage(), 128.f, SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NODLIGHTS | SF_ENVEXPLOSION_NOSMOKE, 0.0f);

	for (int i = 0; i < 5; i++)
	{
		CHLRFireball* pBall = (CHLRFireball*)CBaseEntity::Create("hlr_fireball", GetAbsOrigin(), GetAbsAngles(), this);
		if (pBall)
		{
			Vector vecRand = Vector(RandomFloat(-1.f, 1.f), RandomFloat(-1.f, 1.f), 1.f);
			VectorNormalize(vecRand);
			pBall->SetAbsVelocity(vecRand * RandomFloat(200.f, 400.f));
		}
	}
	UTIL_Remove(pParent);
	UTIL_Remove(this);
}

void CRagdollRocket::InitPropulsion()
{
	SetThink(&CRagdollRocket::PropelThink);
	SetNextThink(gpGlobals->curtime);
	m_fExplodeTime = gpGlobals->curtime + 2.f;
	CFireTrail* pTrail = CFireTrail::CreateFireTrail();
	if (pTrail)
	{
		pTrail->FollowEntity(this, "");
		pTrail->SetLocalOrigin(vec3_origin);
		pTrail->SetLifetime(2.f);
	}
}

void RocketRagdoll(CAI_BaseNPC* npc, const CTakeDamageInfo& info)
{
	if (!npc)
		return;

	Vector vecBack, vecOffset;
	QAngle angBack;
	int iBone = -1;
	iBone = npc->GetHitboxBone(0);
	npc->GetBonePosition(0, vecBack, angBack);
	AngleVectors(angBack, &vecOffset);

	Vector vecSpawn = vecBack + (vecOffset * -8);

	CRagdollRocket* pRocket = (CRagdollRocket*)CBaseEntity::Create("ragdoll_rocket", vecSpawn, vec3_angle);
	pRocket->Spawn();

	CBaseEntity* pEnt = CreateServerRagdoll(npc, iBone, info, COLLISION_GROUP_NPC);

	CRagdollProp* pRag = (CRagdollProp*)pEnt;

	if (pRag)
	{
		constraint_ragdollparams_t constraint;
		constraint.Defaults();

		physenv->CreateRagdollConstraint(pRag->VPhysicsGetObject(), pRocket->VPhysicsGetObject(), pRag->GetRagdoll()->pGroup, constraint);
		pRocket->pParent = pRag;
		float angdrag = 150.f;
		const float* drag = &angdrag;
		pRag->VPhysicsGetObject()->SetDamping(0, drag);
		pRocket->InitPropulsion();
		UTIL_Remove(npc);
	}
}