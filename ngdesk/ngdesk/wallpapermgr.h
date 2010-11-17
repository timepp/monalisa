#pragma once

#include <string>

struct SwitchWallParam
{
	std::wstring imgpath;
	int interval;
	enum {dm_centered, dm_tiled, dm_stretched, dm_proportional} dispmode;
	enum {cm_seq, cm_random, cm_shuffle} cyclemode;
};

class CWallPaperMgr
{
public:
	static CWallPaperMgr* Instance();

	void Run();
	void Reset(const SwitchWallParam& p);

private:
	CWallPaperMgr();
	static CWallPaperMgr s_mgr;

	void Load();
	void Save();
};

