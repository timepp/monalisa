#include "wallpapermgr.h"
#include "util_win.h"

CWallPaperMgr CWallPaperMgr::s_mgr;

CWallPaperMgr::CWallPaperMgr()
{
	Load();
}

CWallPaperMgr* CWallPaperMgr::Instance()
{
	return &s_mgr;
}

void CWallPaperMgr::Run()
{

}

void CWallPaperMgr::Load()
{
	std::wstring exepath = tp::oswin::get_module_path(NULL);
	std::wstring inipath = exepath.substr(0, exepath.length() - 3) + L"ini";

}
