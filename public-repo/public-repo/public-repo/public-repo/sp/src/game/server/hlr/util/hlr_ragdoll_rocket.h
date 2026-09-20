#pragma once
#ifndef RAGDOLL_ROCKET_H
#define RAGDOLL_ROCKET_H

#include "cbase.h"
#include "baseanimating.h"
#include "physics_prop_ragdoll.h"



class CRagdollRocket : public CBaseAnimating
{
	DECLARE_CLASS(CRagdollRocket, CBaseAnimating)

public:
	void Spawn();
	void Precache();

	bool VPhysicsCreate();

	void PropelThink();
	void Explode();

	void InitPropulsion();

	CRagdollProp* pParent;
private:
	float m_fExplodeTime;


};

void RocketRagdoll(CAI_BaseNPC* npc, const CTakeDamageInfo& info);

#endif