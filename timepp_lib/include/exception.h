#ifndef TP_EXCEPTION_H_INCLUDED
#define TP_EXCEPTION_H_INCLUDED

#include "format_shim.h"
#include "opblock.h"

namespace tp
{
	struct error
	{
		virtual std::wstring desc() const = 0;
		virtual ~error() {}
	};
	struct error_win : public error
	{
		error_win(DWORD e) : errcode(e) {}
		DWORD errcode;
		virtual std::wstring desc() const { return std::wstring(tp::edwin(errcode)); }
	};
	struct error_std : public error
	{
		error_std(int e) : errcode(e) {}
		int errcode;
		virtual std::wstring desc() const { return std::wstring(tp::edstd(errcode)); }
	};
	struct error_com : public error
	{
		error_com(HRESULT e) : errcode(e) {}
		HRESULT errcode;
		virtual std::wstring desc() const { return std::wstring(tp::edcom(errcode)); }
	};


	struct exception
	{
		std::wstring oplist;
		std::wstring message;
		std::auto_ptr<tp::error> err;

		explicit exception(error* e, const wchar_t* msg = 0) : err(e), oplist(CURRENT_OPLIST())
		{
			if (msg) message = msg;
		}
	};

	void throw_winerr_when(bool cond)
	{
		if (cond) 
		{
			throw tp::exception(new tp::error_win(GetLastError()));
		}
	}
	void throw_stderr_when(bool cond)
	{
		if (cond)
		{
			throw tp::exception(new tp::error_std(errno));
		}
	}
	
}

#endif