#pragma once

#define MAX_MONSTERHUNT_BONUS 2

struct MonsterHuntItems
{
	std::uint32_t	m_nItemId = 0;
	std::uint16_t	m_nQuantity = 0;

	bool			m_bBind = true;
};

struct MonsterHuntObjectives
{
	std::uint32_t	m_nKillNeeded = 0;
	std::uint32_t	m_nGoldReward = 0;

	std::map<std::uint16_t, std::uint16_t>	m_mBonuses;

	std::vector<MonsterHuntItems>	m_vecItemRewards;
};

struct MonsterHuntProp
{
	std::vector<std::uint32_t>	m_vecMonsters;

	std::string m_szTitle;

	std::map<std::uint8_t, MonsterHuntObjectives>	m_mObjectives;
public:
	const bool IsValidMonster(std::uint32_t nMonsterId) const;
};

class CMonsterHunt
{
	std::map<std::uint16_t, MonsterHuntProp> m_mPropMonsterHunt;
public:
	MonsterHuntProp* GetHuntProperty(std::uint16_t nHuntIndex);

	auto GetAllHuntProperty() { return m_mPropMonsterHunt; }

	void ReadFile();

	static CMonsterHunt* GetInstance()
	{
		static CMonsterHunt sGetInstance;
		return &sGetInstance;
	}
};

struct playerHunts
{
	std::map<std::uint8_t, std::pair<std::uint32_t, bool>> m_mProgress;

public:
	std::uint32_t GetProgressForLevel(std::uint8_t nLevel) const;

	void Serialize(CAr& ar);
};

class CPlayerMonsterHunt
{
	std::map<std::uint16_t, playerHunts> m_mCurrentHunts;
public:
	playerHunts* GetMonsterHunt(std::uint16_t nHuntIndex);

	bool IsHuntLevelComplete(std::uint32_t nPlayerProgress, std::uint32_t nNeeded);
	bool IsClaimedReward(const std::uint16_t nHuntIndex, const std::uint8_t nLevel);

#ifdef __WORLDSERVER
	void ClaimRewardsForHunt(CUser* pUser, std::uint16_t nHuntIndex);
	void SendUserRewards(CUser* pAttacker, MonsterHuntObjectives const objectivesRewards);
	void AddMonsterHuntToPlayer(CUser* pAttacker, std::uint32_t nMonsterId);
	void SetBonuses(CUser* pUser);
	void SetBonusesForHunt(CUser* pUser, const std::uint16_t nHuntIndex);
	void ResetLastBonuses(CUser* pUser, std::uint16_t nHuntIndex);
#endif
#ifndef __DBSERVER
	void AddMonsterHuntToPlayer(CMover* pAttacker, std::uint32_t nMonsterId);
#endif

	std::uint8_t GetLastLevel(std::uint16_t nHuntIndex);

	void Serialize(CAr& ar);

	auto GetAllHunts() { return m_mCurrentHunts; }

	void SetMonsterHunt(std::uint16_t nHuntIndex, std::uint8_t nLevel, std::uint32_t nProgress, bool bClaimed);
	void ClearHunts() { m_mCurrentHunts.clear(); }
};

#ifdef __CLIENT

#endif