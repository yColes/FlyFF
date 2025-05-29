#pragma once

#ifdef _MONSTER_HUNTING
class CDbMonsterHunt
{
public:
	void SaveMonsterHunts(CQuery* qry, CMover* pMover, char* szQuery);
	void LoadMonsterHunt(CMover* pMover, CQuery* pQuery);

	static CDbMonsterHunt* GetInstance()
	{
		static CDbMonsterHunt sGetInstance;
		return &sGetInstance;
	}
};
#endif