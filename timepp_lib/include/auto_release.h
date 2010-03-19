#pragma once
#include "defs.h"

// ----------------------------------------------------------------------------
// 资源自动释放宏AUTO_RELEASE
// 在当前块结束时自动调用注册的f(r)
// 例: AUTO_RELEASE(fp, fclose)


#define ON_LEAVE(statement) \
	struct LINE_NAME(ols_) { \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_);

#define ON_LEAVE_1(statement, type, var) \
	struct LINE_NAME(ols_) { \
		type var; \
		LINE_NAME(ols_)(type v): var(v) {} \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_)(var);

#define ON_LEAVE_2(statement, type1, var1, type2, var2) \
	struct LINE_NAME(ols_) { \
		type1 var1; type2 var2; \
		LINE_NAME(ols_)(type1 v1, type2 v2): var1(v1), var2(v2) {} \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_)(var1, var2);

#define ON_LEAVE_3(statement, type1, var1, type2, var2, type3, var3) \
	struct LINE_NAME(ols_) { \
		type1 var1; type2 var2; type3 var3; \
		LINE_NAME(ols_)(type1 v1, type2 v2, type3 v3): var1(v1), var2(v2), var3(v3) {} \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_)(var1, var2, var3);

#define ON_LEAVE_4(statement, type1, var1, type2, var2, type3, var3, type4, var4) \
	struct LINE_NAME(ols_) { \
		type1 var1; type2 var2; type3 var3; type4 var4; \
		LINE_NAME(ols_)(type1 v1, type2 v2, type3 v3, type4 v4): var1(v1), var2(v2), var3(v3), var4(v4) {} \
		~LINE_NAME(ols_)() { statement; } \
	} LINE_NAME(olv_)(var1, var2, var3, var4);


namespace tp
{

namespace _inner
{

	template <template <typename> class V, typename T>
	V<T> creator_helper(T t)
	{
		return V<T>(t);
	}

	template <typename T1>
	struct type_trait
	{
		typedef T1 arg_type;
	};

	// call_on_ret_base
	struct call_on_ret_base
	{
		mutable bool is_runner;
		call_on_ret_base() : is_runner(true) {}
		call_on_ret_base(const call_on_ret_base& cor) : is_runner(true)
		{
			cor.is_runner = false;
		}
	};

	template <typename T>
	struct cor : public call_on_ret_base
	{
		T _1;
		cor(T v) : _1(v) {}
		cor(const cor& c) : call_on_ret_base(c) {}
		~cor()
		{
			fclose(_1);
		}
	};

// call_on_ret
template <typename F, typename P>
struct call_on_ret : call_on_ret_base
{
        mutable F m_fun_ptr;               // 资源释放函数
        P m_param;                         // 资源
        call_on_ret(F f, P p) : m_fun_ptr(f), m_param(p) {}
        call_on_ret(const call_on_ret& c)
                : m_fun_ptr(c.m_fun_ptr), m_param(c.m_param)
        {
                c.m_fun_ptr = 0;
        }
        ~call_on_ret()
        {
                if (m_fun_ptr) m_fun_ptr(m_param);
        }
};
template <typename F, typename P>
call_on_ret<F, P> f_coe(F f, P p)
{
        return call_on_ret<F, P>(f, p);
}

} // namespace _inner

} // namespace tp
