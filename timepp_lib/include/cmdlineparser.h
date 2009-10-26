#pragma once

#include <string>
#include <list>
#include <map>

namespace tp
{

class cmdline_parser
{
public:
	typedef bool cvt_func(const wchar_t * str, void* val);

private:
	struct parse_option
	{
		wchar_t short_name;
		std::wstring long_name;
		void * addr;
		bool has_param;
		cvt_func * cf;
		std::wstring value;
	};
	typedef std::list<parse_option> polist_t;
	typedef std::map<std::wstring, std::wstring> ssmap_t;

	polist_t m_po;
	ssmap_t m_om;
	std::list<std::wstring> m_targets;

	parse_option* lookup(const wchar_t* opt)
	{
		if (!opt) return NULL;

		for (polist_t::iterator it = m_po.begin(); it != m_po.end(); ++it)
		{
			if (it->short_name == *opt && opt[1] == 0 || it->long_name == opt)
			{
				return &(*it);
			}
		}

		return NULL;
	}

	void save_option(parse_option* po, const wchar_t *value)
	{
		po->value = value;
		if (po->cf && po->addr)
		{
			(*po->cf)(value, po->addr);
		}
	}
	bool save_option(const wchar_t* opt, const wchar_t* value)
	{
		parse_option* po = lookup(opt);
		if (po)
		{
			save_option(po, value);
			return true;
		}
		return false;
	}


public:
	
	static bool cf_bool(const wchar_t *, void* val)
	{
		*reinterpret_cast<bool*>(val) = true;
		return true;
	}
	static bool cf_string(const wchar_t * str, void * val)
	{
		*reinterpret_cast<std::wstring*>(val) = str;
		return true;
	}
	static bool cf_int(const wchar_t * str, void * val)
	{
		*reinterpret_cast<int*>(val) = _wtoi(str);
		return true;
	}

	bool add_option(const wchar_t short_name, const wchar_t * long_name, void * addr, bool has_param = false, cvt_func * cf = 0)
	{
		parse_option po;
		po.short_name = short_name;
		po.long_name  = long_name?  long_name:  L"";
		po.addr = addr;
		po.cf = cf;
		po.has_param = has_param;
		if (!has_param && !cf)
		{
			po.cf = &cmdline_parser::cf_bool;
		}
		m_po.push_back(po);

		return true;
	}

	bool parse(const wchar_t* cmd_line)
	{
		/** windows下对命令行的处理：参数中间有空格时，可以用引号引起来。这样就有一个如何表示引号的问题。
		 *  使用\"来表示"。\如果在引号之前，表示那是一个普通引号。\在其它位置保持其本意
		 */
		std::list<std::wstring> param_list;
		const wchar_t * p = cmd_line;
		while(*p)
		{
			bool in_quote = false;
			std::wstring param;
			const wchar_t *q = p;
			while (true)
			{
				if (q[0]== L'\\' && q[1] == L'\"')
				{
					param += L'\"';
					q += 2;
					continue;
				}

				if (q[0] == L'\"')
				{
					in_quote = !in_quote;
					q++;
					continue;
				}

				if (q[0] == L' ' && !in_quote)
				{
					p = q+1;
					break;
				}

				if (q[0] == L'\0')
				{
					p = q;
					break;
				}

				param += *q;
			}

			param_list.push_back(param);
		}

		int argc = static_cast<int>(param_list.size());
		const wchar_t** argv = new const wchar_t* [argc];
		int i = 0;
		for (std::list<std::wstring>::const_iterator it = param_list.begin(); it != param_list.end(); ++it)
		{
			argv[i++] = it->c_str();
		}

		bool ret = parse(argc, argv);

		delete [] argv;

		return ret;
	}

	bool parse(int argc, const wchar_t* const * argv)
	{
		if (!argv) return false;

		for (int i = 1; i < argc; i++)
		{
			if (!argv[i]) return false;

			if (argv[i][0] != '-')
			{
				m_targets.push_back(argv[i]);
				continue;
			}

			if (argv[i][1] == '-')
			{
				// 两个减号
				const wchar_t *p = wcschr(argv[i], L'=');
				if (p != NULL)
				{
					std::wstring opt(argv[i] + 2, p - argv[i] - 2);
					if (!save_option(opt.c_str(), p + 1)) return false;
				}
				else
				{
					parse_option* po = lookup(argv[i] + 2);
					if (!po) return false;
					if (po->has_param)
					{
						if (i >= argc - 1) return false;
						save_option(po, argv[++i]);
					}
					else
					{
						save_option(po, L"");
					}
				}
			}
			else
			{
				// 一个减号
				for (const wchar_t * p = argv[i] + 1; *p; p++)
				{
					wchar_t key[] = {*p, 0};
					parse_option* po = lookup(key);
					if (!po) return false;
					if (po->has_param)
					{
						if (p[1]) return false;
						if (i >= argc - 1) return false;
						save_option(po, argv[++i]);
					}
					else
					{
						save_option(po, L"");
					}
				}
			}
		}

		return true;
	}

	std::wstring get_option(const wchar_t * opt)
	{
		if (!opt) return L"";

		ssmap_t::const_iterator it = m_om.find(opt);
		if (it != m_om.end())
		{
			return it->second;
		}

		parse_option * p = lookup(opt);
		if (p != NULL)
		{
			return p->value;
		}

		return L"";
	}

	std::wstring get_target(size_t index) const
	{
		std::list<std::wstring>::const_iterator it = m_targets.begin();
		while (it != m_targets.end())
		{
			if (index == 0) return *it;
			--index;
			++it;
		}
		return L"";
	}

	size_t get_target_connt() const
	{
		return m_targets.size();
	}

	std::list<std::wstring> get_targets() const
	{
		return m_targets;
	}
};

}