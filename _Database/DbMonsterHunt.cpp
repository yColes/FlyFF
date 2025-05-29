#include "StdAfx.h"
#include "DbMonsterHunt.h"
extern	APP_INFO	g_appInfo;
#ifdef _MONSTER_HUNTING
void CDbMonsterHunt::SaveMonsterHunts(CQuery* qry, CMover* pMover, char* szQuery)
{
	if (pMover == NULL || qry == NULL)
		return;

	for (auto const& itHunt : pMover->m_PlayerMonsterHunt.GetAllHunts())
	{
		char szMonsterHunt[8000] = { 0, };
		char OneHunt[64] = { 0, };

		auto nHuntIndex = itHunt.first;

		for (auto const& itProgress : itHunt.second.m_mProgress)
		{
			sprintf(OneHunt, "%d,%d,%d/", itProgress.first, itProgress.second.first, itProgress.second.second);
			strncat(szMonsterHunt, OneHunt, sizeof(OneHunt));
		}

		strncat(szMonsterHunt, NullStr, sizeof(NullStr));

		sprintf(szQuery, "{call MONSTER_HUNT_STR('U1', '%02d','%07d', ?, ?)}", g_appInfo.dwSys, pMover->m_idPlayer);

		BOOL bOK[2] = { false, false };
		SQLINTEGER cbLen = SQL_NTS;
		bOK[0] = qry->BindParameter(1, SQL_PARAM_INPUT, SQL_C_SHORT, SQL_SMALLINT, 0, 0, &nHuntIndex, 0, 0);
		bOK[1] = qry->BindParameter(2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, &szMonsterHunt, 0, &cbLen); 
		
		if (!bOK[0] || !bOK[1])
			continue;

		if (FALSE == qry->Exec(szQuery))
		{
			WriteLog("%s, %d\t%s", __FILE__, __LINE__, szQuery);
			return;
		}
	}
}

void CDbMonsterHunt::LoadMonsterHunt(CMover* pMover, CQuery* pQuery)
{
	char szSQL[QUERY_SIZE] = { 0, };

	sprintf(szSQL, "{call MONSTER_HUNT_STR('S1', '%02d','%07d')}", g_appInfo.dwSys, pMover->m_idPlayer);

	if (FALSE == pQuery->Exec(szSQL))
	{
		WriteLog("%s, %d\t%s", __FILE__, __LINE__, szSQL);
		return;
	}

	pMover->m_PlayerMonsterHunt.ClearHunts();

	while (pQuery->Fetch())
	{
		std::uint16_t nHuntIndex = pQuery->GetInt("nHuntIndex");

		char* m_MonsterHunt = pQuery->GetPointerStr("m_MonsterHunt");

		int CountStr = 0;
		while (m_MonsterHunt[CountStr] != '$' && m_MonsterHunt[CountStr] != '\0')
		{
			std::uint8_t nLevel = (std::uint8_t)GetIntPaFromStr(m_MonsterHunt, &CountStr);
			std::uint32_t nProgress = GetIntPaFromStr(m_MonsterHunt, &CountStr);
			bool bClaimed = GetIntPaFromStr(m_MonsterHunt, &CountStr);
			pMover->m_PlayerMonsterHunt.SetMonsterHunt(nHuntIndex, nLevel, nProgress, bClaimed);
			++CountStr;
		}
	}

}
#endif