#include "StdAfx.h"
#include "MonsterHunt.h"
#include "defineObj.h"
#ifdef __WORLDSERVER
#include "DPDatabaseClient.h"
extern CDPDatabaseClient g_dpDBClient;
#include "DPCoreClient.h"
extern CDPCoreClient g_DPCoreClient;
#include "User.h"
extern CUserMng g_UserMng;
#include "worldmng.h"
extern CWorldMng g_WorldMng;
#endif
#include <sstream>
#include <iomanip>

#ifdef _MONSTER_HUNTING
void CMonsterHunt::ReadFile() {
	CScript s;
	if (!s.Load("MonsterHunt.inc"))
	{
		Error(__FUNCTION__ " Cannot load MonsterHunt.inc");
		return;
	}
	s.GetToken(); // AddMonsterHunt
	std::uint32_t nHuntIndex = 0;
	while (s.tok != FINISHED)
	{
		if (s.Token == _T("AddMonsterHunt"))
		{
			MonsterHuntProp monsterHuntingProperty;
			s.GetToken(); // {

			s.GetToken();// szTitle
			s.GetToken();
			monsterHuntingProperty.m_szTitle = s.Token;
							

			/// Add Monsters		
			s.GetToken(); // AddMonsters
			s.GetToken(); // { 
			std::uint32_t nMonsterId = s.GetNumber(); // 
			while (*s.token != '}')
			{
				monsterHuntingProperty.m_vecMonsters.push_back(nMonsterId);
				nMonsterId = s.GetNumber(); // AddMonsters
			}


			s.GetToken(); // Rewards
			s.GetToken(); // { 
			s.GetToken(); // Level
			while (*s.token != '}')
			{
				MonsterHuntObjectives objectives;

				s.GetToken(); //(
				std::uint8_t nLevel = s.GetNumber(); //
				s.GetToken(); //)
				s.GetToken(); // { 
				s.GetToken(); // KillQuantity
				while (*s.token != '}')
				{
					if (s.Token == "KillQuantity")
					{
						s.GetToken(); // (
						objectives.m_nKillNeeded = s.GetNumber(); // 
						s.GetToken(); // )
					}
					else if (s.Token == "AddReward")
					{
						s.GetToken(); // (

						s.GetToken(); // rewardtype
						std::string szTypeReward = s.Token;

						if (szTypeReward == "ITEM")
						{
							MonsterHuntItems itemRewards;

							s.GetToken(); // ,
							itemRewards.m_nItemId = s.GetNumber();
							s.GetToken(); // ,
							itemRewards.m_nQuantity = s.GetNumber();
							s.GetToken(); // ,
							s.GetToken();
							if (s.Token == "false")
								itemRewards.m_bBind = false;

							objectives.m_vecItemRewards.push_back(itemRewards);

							s.GetToken(); // )
						}
						else if (szTypeReward == "GOLD")
						{
							s.GetToken(); // ,
							objectives.m_nGoldReward = s.GetNumber();
							s.GetToken(); // )
						}
						else if (szTypeReward == "BONUS")
						{
							s.GetToken(); // ,
							std::uint32_t nDST = s.GetNumber();
							s.GetToken(); // ,
							std::uint32_t nAmountBonus = s.GetNumber();

							objectives.m_mBonuses.insert_or_assign(nDST, nAmountBonus);

							s.GetToken(); // )
						}
					}

					s.GetToken(); // KillQuantity
				}

				monsterHuntingProperty.m_mObjectives.insert_or_assign(nLevel, objectives);

				s.GetToken(); //Level
			}
			m_mPropMonsterHunt.insert_or_assign(nHuntIndex++, monsterHuntingProperty);
		}
		s.GetToken();
	}
}

const bool MonsterHuntProp::IsValidMonster(std::uint32_t nMonsterId) const {
	auto& itMonster = std::find(m_vecMonsters.begin(), m_vecMonsters.end(), nMonsterId);
	return itMonster != m_vecMonsters.end();
}


MonsterHuntProp* CMonsterHunt::GetHuntProperty(std::uint16_t nHuntIndex) {
	auto& it = m_mPropMonsterHunt.find(nHuntIndex);
	return it != m_mPropMonsterHunt.end() ? &it->second : nullptr;
}

bool CPlayerMonsterHunt::IsHuntLevelComplete(std::uint32_t nPlayerProgress, std::uint32_t nNeeded) {
	return nPlayerProgress >= nNeeded;
}

#ifndef __DBSERVER
#ifdef __WORLDSERVER
void CPlayerMonsterHunt::AddMonsterHuntToPlayer(CUser * pAttacker, std::uint32_t nMonsterId) {
#else
void CPlayerMonsterHunt::AddMonsterHuntToPlayer(CMover * pAttacker, std::uint32_t nMonsterId) {
#endif

#ifdef __CLIENT
	if (g_pPlayer != pAttacker) return;
#endif

	for (auto const& itHunts : CMonsterHunt::GetInstance()->GetAllHuntProperty())
	{
		const MonsterHuntProp hunt = itHunts.second;

		if (hunt.IsValidMonster(nMonsterId) == false) continue;

		MoverProp* pMoverProp = prj.GetMoverProp(nMonsterId);
		if (pMoverProp == nullptr)
			return;

		if (IsValidObj(pAttacker) == false)
			return;

		for (auto const& itObjective : hunt.m_mObjectives)
		{
#ifdef __WORLDSERVER
			playerHunts* pPlayerHunt = pAttacker->m_PlayerMonsterHunt.GetMonsterHunt(itHunts.first);
#else
			playerHunts* pPlayerHunt = g_WndMng.m_PlayerMonsterHunt.GetMonsterHunt(itHunts.first);
#endif
			const MonsterHuntObjectives objective = itObjective.second;

			if (IsHuntLevelComplete(pPlayerHunt->m_mProgress[itObjective.first].first, objective.m_nKillNeeded))
				continue;

			pPlayerHunt->m_mProgress[itObjective.first].first++;

			break;
		}
	}
}
#endif

#ifdef __WORLDSERVER
void CPlayerMonsterHunt::SendUserRewards(CUser* pAttacker, MonsterHuntObjectives const objectivesRewards) {
	std::string szText = "Your reward for finishing a level of monster hunt !";

	CItemElem itemElem;

	if( pAttacker->AddGold(objectivesRewards.m_nGoldReward) == false )
		g_dpDBClient.SendQueryPostMail(pAttacker->m_idPlayer, 0, itemElem, objectivesRewards.m_nGoldReward, "Monster hunting !", const_cast<char*>(szText.c_str()));

	for (auto const& itReward : objectivesRewards.m_vecItemRewards)
	{
		itemElem.m_dwItemId = itReward.m_nItemId;
		itemElem.m_nItemNum = itReward.m_nQuantity;

		if (itReward.m_bBind)
			itemElem.SetFlag(CItemElem::binds);

		if (!pAttacker->CreateItem(&itemElem))
			g_dpDBClient.SendQueryPostMail(pAttacker->m_idPlayer, 0, itemElem, 0, "Monster hunting !", const_cast<char*>(szText.c_str()));
	}

	pAttacker->AddText("You successfuly claimed your monster hunt reward !");
}

void CPlayerMonsterHunt::SetBonuses(CUser* pUser) {
	if (IsValidObj(pUser) == false)
		return;

	for (auto const& itUserHunt : pUser->m_PlayerMonsterHunt.GetAllHunts())
	{
		auto pHuntProp = CMonsterHunt::GetInstance()->GetHuntProperty(itUserHunt.first);
		if (pHuntProp == nullptr) continue;

		for (auto itObjectives = pHuntProp->m_mObjectives.rbegin(); itObjectives != pHuntProp->m_mObjectives.rend(); itObjectives++)
		{
			auto nProgress = itUserHunt.second.GetProgressForLevel(itObjectives->first);

			playerHunts* pPlayerHunt = pUser->m_PlayerMonsterHunt.GetMonsterHunt(itUserHunt.first);

			if (pPlayerHunt->m_mProgress[itObjectives->first].second == false )
				continue;

			if (IsHuntLevelComplete(nProgress, itObjectives->second.m_nKillNeeded) == false) continue;

			for (auto const& itBonuses : itObjectives->second.m_mBonuses)
			{
				pUser->SetDestParam(itBonuses.first, itBonuses.second, NULL_CHGPARAM);
			}

			break;
		}
	}
}

void CPlayerMonsterHunt::SetBonusesForHunt(CUser* pUser, const std::uint16_t nHuntIndex ) {
	auto pPlayerHunt = pUser->m_PlayerMonsterHunt.GetMonsterHunt(nHuntIndex);

	auto pHuntProp = CMonsterHunt::GetInstance()->GetHuntProperty(nHuntIndex);
	if (pHuntProp == nullptr) return;

	for (auto itObjectives = pHuntProp->m_mObjectives.rbegin(); itObjectives != pHuntProp->m_mObjectives.rend(); itObjectives++)
	{
		if (pPlayerHunt->m_mProgress[itObjectives->first].second == false)
			continue;

		if (IsHuntLevelComplete(pPlayerHunt->GetProgressForLevel(itObjectives->first), itObjectives->second.m_nKillNeeded) == false) continue;

		for (auto const& itBonuses : itObjectives->second.m_mBonuses)
		{
			pUser->SetDestParam(itBonuses.first, itBonuses.second, NULL_CHGPARAM);
		}

		break;
	}
}

void CPlayerMonsterHunt::ResetLastBonuses(CUser* pUser, std::uint16_t nHuntIndex) {
	auto pPlayerHunt = pUser->m_PlayerMonsterHunt.GetMonsterHunt(nHuntIndex);

	auto pHuntProp = CMonsterHunt::GetInstance()->GetHuntProperty(nHuntIndex);
	if (pHuntProp == nullptr) return;

	for (auto itObjectives = pHuntProp->m_mObjectives.rbegin(); itObjectives != pHuntProp->m_mObjectives.rend(); itObjectives++)
	{
		if (pPlayerHunt->m_mProgress[itObjectives->first].second == false)
			continue;

		if (IsHuntLevelComplete(pPlayerHunt->GetProgressForLevel(itObjectives->first), itObjectives->second.m_nKillNeeded) == false) continue;

		for (auto const& itBonuses : itObjectives->second.m_mBonuses)
		{
			pUser->ResetDestParam(itBonuses.first, itBonuses.second);
		}

		break;
	}
}

void CPlayerMonsterHunt::ClaimRewardsForHunt(CUser* pUser, std::uint16_t nHuntIndex) {
	auto pHunt = CMonsterHunt::GetInstance()->GetHuntProperty(nHuntIndex);

	if (pHunt == nullptr)
	{
		pUser->AddText("This hunt doesn't exist !", 0xFFFF0000);
		return;
	}

	ResetLastBonuses(pUser, nHuntIndex);


	for (auto const& itObjective : pHunt->m_mObjectives)
	{
		playerHunts* pPlayerHunt = pUser->m_PlayerMonsterHunt.GetMonsterHunt(nHuntIndex);
		const MonsterHuntObjectives objective = itObjective.second;

		if (pPlayerHunt->m_mProgress[itObjective.first].second)
			continue;

		if (IsHuntLevelComplete(pPlayerHunt->m_mProgress[itObjective.first].first, objective.m_nKillNeeded))
		{
			SendUserRewards(pUser, objective); 
			pPlayerHunt->m_mProgress[itObjective.first].second = true;
		}
	}

	SetBonusesForHunt(pUser, nHuntIndex);

	pUser->AddMonsterHunt();
}
#endif

std::uint32_t playerHunts::GetProgressForLevel(std::uint8_t nLevel) const {
	auto it = m_mProgress.find(nLevel);
	return it != m_mProgress.end() ? it->second.first : 0;
}

playerHunts* CPlayerMonsterHunt::GetMonsterHunt(std::uint16_t nHuntIndex) {
	return &m_mCurrentHunts[nHuntIndex];
}

void playerHunts::Serialize(CAr& ar) {
	if (ar.IsStoring())
	{
		ar << (std::uint8_t)m_mProgress.size();
		for (auto const& it : m_mProgress)
			ar << it.first << it.second.first << it.second.second;
	}
	else
	{
		m_mProgress.clear();

		std::uint8_t nSize = 0;
		ar >> nSize;
		for (std::uint8_t i = 0; i < nSize; i++)
		{
			std::uint8_t nLevel = 0;
			ar >> nLevel;
			std::uint32_t nProgress = 0;
			ar >> nProgress;
			bool bClaimed = false;
			ar >> bClaimed;

			m_mProgress[nLevel] = { nProgress,bClaimed };
		}
	}
}

void CPlayerMonsterHunt::Serialize(CAr& ar)
{
	if (ar.IsStoring())
	{
		ar << (std::uint8_t)m_mCurrentHunts.size();
		for (auto & it : m_mCurrentHunts)
		{
			ar << it.first;
			it.second.Serialize(ar);
		}
	}
	else
	{
		m_mCurrentHunts.clear();

		std::uint8_t nSize = 0;
		ar >> nSize;
		for (std::uint8_t i = 0; i < nSize; i++)
		{
			std::uint16_t nHunt = 0;
			ar >> nHunt;
			playerHunts hunt;
			hunt.Serialize(ar);

			m_mCurrentHunts.insert_or_assign(nHunt, hunt);
		}
	}
}

void CPlayerMonsterHunt::SetMonsterHunt(std::uint16_t nHuntIndex, std::uint8_t nLevel, std::uint32_t nProgress, bool bClaimed)
{
	m_mCurrentHunts[nHuntIndex].m_mProgress[nLevel].first = nProgress;
	m_mCurrentHunts[nHuntIndex].m_mProgress[nLevel].second = bClaimed;
}

std::uint8_t CPlayerMonsterHunt::GetLastLevel(std::uint16_t nHuntIndex)
{
	auto& itHunt = m_mCurrentHunts.find(nHuntIndex);
	if (itHunt == m_mCurrentHunts.end())
		return 1;

	auto pHunt = CMonsterHunt::GetInstance()->GetHuntProperty(nHuntIndex);

	if (pHunt == nullptr)
		return 1;

	for (auto const& itProgress : itHunt->second.m_mProgress)
	{
		if (IsHuntLevelComplete(itProgress.second.first, pHunt->m_mObjectives[itProgress.first].m_nKillNeeded))
			continue;

		return itProgress.first;
	}

	return 1;
}

bool CPlayerMonsterHunt::IsClaimedReward(const std::uint16_t nHuntIndex, const std::uint8_t nLevel) {
	return m_mCurrentHunts[nHuntIndex].m_mProgress[nLevel].second;
}
#endif
