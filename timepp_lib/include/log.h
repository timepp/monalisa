#pragma once

#include <set>
#include <list>
#include <map>
#include <string>
#include <stdlib.h>

#include "api_wrapper.h"
#include "tstring.h"
#include "lock.h"

#define TP_LOG_MULTI_THREAD

namespace tp
{
	// receives formatted log string and write them to file, console, etc...
	class log_device
	{
	public:
		virtual bool open() = 0;
		virtual bool close() = 0;
		virtual size_t write(const wchar_t * buf, size_t len, int context_id) = 0;
		virtual bool flush() = 0;
		virtual ~log_device(){}
	};

	// log context is one kind of info which is at the beginning of each log line
	class log_context
	{
	public:
		log_context(int id) : m_id(id) {}
		virtual ~log_context(){}

		int id() const { return m_id; }
		virtual std::wstring value(unsigned int type) const = 0;
	protected:
		log_context& operator=(const log_context&);
	private:
		const int m_id;
	};

	namespace _inner
	{
		// singleton
		class logger
		{
		private:
			logger()
			{
#ifdef TP_LOG_MULTI_THREAD
				::InitializeCriticalSection(&m_cs);
#endif
			}
			~logger()
			{
				while (m_lds.size() > 0)
				{
					remove_device(m_lds.begin()->first);
				}

#ifdef TP_LOG_MULTI_THREAD
				::DeleteCriticalSection(&m_cs);
#endif
			}
			logger(const logger&);
			logger& operator = (const logger&);

			typedef std::list<log_context*> lcs_t;
			struct device_info
			{
				unsigned int mask;
				lcs_t lcs;
				bool auto_delete;
			};

			typedef std::map<log_device*, device_info> lds_t;
			lds_t m_lds;
			CRITICAL_SECTION m_cs;

		public:
			static logger * instance()
			{
				static logger s_logger;
				return &s_logger;
			}

			void add_device(log_device * ld, unsigned int mask, bool auto_delete)
			{
				device_info di;
				di.auto_delete = auto_delete;
				di.mask = mask;

				lds_t::iterator it = m_lds.find(ld);
				if (it == m_lds.end())
				{
					ld->open();
					m_lds.insert(lds_t::value_type(ld, di));
				}
			}

			void remove_device(log_device * ld)
			{
				lds_t::iterator it = m_lds.find(ld);
				if (it != m_lds.end())
				{
					const device_info& di = it->second;

					ld->close();
					if (di.auto_delete) delete ld;
					for (lcs_t::const_iterator it2 = di.lcs.begin(); it2 != di.lcs.end(); ++it2)
					{
						delete *it2;
					}
				}
				m_lds.erase(it);
			}

			bool add_context(log_device * ld, log_context * lc)
			{
				lds_t::iterator it = m_lds.find(ld);
				if (it != m_lds.end())
				{
					it->second.lcs.push_back(lc);
				}

				return false;
			}

			void log(unsigned int log_type, const wchar_t * text, bool flush = false)
			{
#ifdef TP_LOG_MULTI_THREAD
				critical_lock lock(m_cs);
#endif
				for (lds_t::const_iterator it = m_lds.begin(); it != m_lds.end(); ++it)
				{
					log_device * ld = it->first;
					const device_info& di = it->second;

					if (di.mask & (1 << log_type))
					{
						const wchar_t * p = text;
						const wchar_t * q = text;
						do
						{
							if (*q == '\n' || (*q == 0 && q > p))
							{
								for (lcs_t::const_iterator it2 = di.lcs.begin(); it2 != di.lcs.end(); ++it2)
								{
									std::wstring v = (*it2)->value(log_type);
									ld->write(v.c_str(), v.length(), (*it2)->id());
								}
								ld->write(p, static_cast<size_t>(q - p), 0);
								ld->write(L"\n", 1, 0);
								p = q + 1;
							}
						} while (*q++);
						
						if (flush) 
						{
							ld->flush();
						}
					}
				}
			}

		}; // class logger


	} // namespace _inner

	inline void log_add_device(log_device * ld, unsigned int mask, bool auto_delete = true)
	{
		_inner::logger::instance()->add_device(ld, mask, auto_delete);
	}

	inline void log_remove_device(log_device * ld)
	{
		_inner::logger::instance()->remove_device(ld);
	}

	inline void log_add_context(log_device * ld, log_context * lc)
	{
		_inner::logger::instance()->add_context(ld, lc);
	}

	inline void log(unsigned int log_type, const wchar_t * text, bool flush = true)
	{
		_inner::logger::instance()->log(log_type, text, flush);
	}
	inline void log(const wchar_t * text, bool flush = true)
	{
		_inner::logger::instance()->log(0, text, flush);
	}
};
