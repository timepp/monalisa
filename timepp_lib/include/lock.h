#ifndef TP_LOG_H_INCLUDED
#define TP_LOG_H_INCLUDED

// ÁÙ½çÇøËø
class critical_lock
{
public:

	critical_lock() : m_cs(0)
	{
	}

	explicit critical_lock(CRITICAL_SECTION& cs)
	{
		m_cs = &cs;
		::EnterCriticalSection(m_cs);
	}

	~critical_lock()
	{
		::LeaveCriticalSection(m_cs);
	}

private:
	CRITICAL_SECTION * m_cs;
};


#endif