#include "cbase.h"
#include "triggers.h"
#include "player.h"

//you know the drill
#include "tier0/memdbgon.h"

class CPointViewDrag : public CPointEntity
{
	DECLARE_CLASS(CPointViewDrag, CPointEntity);
	DECLARE_DATADESC();
public:
	void Spawn();
	void DragView();
	void StartDrag();
	void EndDrag();
	void InputStartDrag(inputdata_t& data);
	void InputStopDrag(inputdata_t& data);
	void InputSetDragTarget(inputdata_t& data);

private:
	float m_flMaxTime;
	float m_flDragScale;
	Vector m_vecDragDest;
	bool m_bDragging;

	CHandle<CBaseEntity> m_hTargetEnt;
	const char* m_szTargetEntName;
};


BEGIN_DATADESC(CPointViewDrag)
DEFINE_KEYFIELD(m_flMaxTime,FIELD_FLOAT,"maxtime"),
DEFINE_KEYFIELD(m_flDragScale,FIELD_FLOAT,"dragscale"),
DEFINE_KEYFIELD(m_szTargetEntName,FIELD_STRING,"targetent"),

DEFINE_INPUTFUNC(FIELD_VOID,"StartDrag",InputStartDrag),
DEFINE_INPUTFUNC(FIELD_VOID,"StopDrag",InputStopDrag),
DEFINE_INPUTFUNC(FIELD_STRING,"SetTarget",InputSetDragTarget),
DEFINE_THINKFUNC(DragView)
END_DATADESC()

LINK_ENTITY_TO_CLASS(point_viewdrag, CPointViewDrag)

void CPointViewDrag::Spawn()
{
	BaseClass::Spawn();
	if (m_szTargetEntName)
	{
		CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, m_szTargetEntName);
		if (!pEnt)
		{
			Warning("ViewDrag couldn't find target entity\n");
			return;
		}

		m_hTargetEnt.Set(pEnt);
	}
}

void CPointViewDrag::DragView()
{
	CBasePlayer* pPlayer = UTIL_GetLocalPlayer();

	if (!pPlayer)
		return;

	if (m_flMaxTime > 0 && gpGlobals->curtime > m_flMaxTime)
	{
		EndDrag();
		return;
	}

	QAngle angPlayerView = pPlayer->EyeAngles();
	Vector vecEyePos = pPlayer->EyePosition();
	Vector vecDest = GetAbsOrigin();
	if (m_hTargetEnt)
	{
		CBaseEntity* pEnt = m_hTargetEnt.Get();
		vecDest = pEnt->GetAbsOrigin();
		if (pEnt->IsNPC())
			vecDest = pEnt->WorldSpaceCenter();
	}

	Vector vecToTarget = (vecDest - vecEyePos).Normalized();
	QAngle angToTarget;
	VectorAngles(vecToTarget, angToTarget);

	QAngle angDest = Lerp(m_flDragScale * 0.1f, angPlayerView, angToTarget);
	angDest[ROLL] = 0;
	pPlayer->SnapEyeAngles(angDest);
	SetNextThink(gpGlobals->curtime + 0.001f);
}
void CPointViewDrag::StartDrag()
{
	SetThink(&CPointViewDrag::DragView);
	m_bDragging = true;
	SetNextThink(gpGlobals->curtime);
}
void CPointViewDrag::EndDrag()
{
	SetThink(NULL);
	m_bDragging = false;
	SetNextThink(gpGlobals->curtime);
}

void CPointViewDrag::InputSetDragTarget(inputdata_t& data)
{
	if (data.value.String())
	{
		m_szTargetEntName = data.value.String();
		Msg("entity search: %s\n", data.value.String());

		CBaseEntity* pEnt = gEntList.FindEntityByName(NULL, m_szTargetEntName);

		if (!pEnt)
		{
			Warning("ViewDrag couldn't find target entity\n");
			return;
		}

		m_hTargetEnt.Set(pEnt);
	}
}

void CPointViewDrag::InputStartDrag(inputdata_t& data)
{
	StartDrag();
}

void CPointViewDrag::InputStopDrag(inputdata_t& data)
{
	EndDrag();
}
