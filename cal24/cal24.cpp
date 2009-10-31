#include <algorithm>
#include <string>
#include <cmath>
#include <sstream>
#include <iostream>
#include <numeric>
#include <list>
#include "../timepp_lib/include/cmdlineparser.h"

static bool g_debug = false;

bool double_equ(double a, double b)
{
	return std::fabs(a - b) < 1E-10;
}

double calc_frac(double n)
{
	int ival = (int)floor(n + 0.5);
	if (ival < 3 || ival > 20 || !double_equ(n, ival))
	{
		return std::numeric_limits<double>::quiet_NaN();
	}

	double ret = 1;
	for (double i = 2; i <= ival; i++) ret *= i;
	return ret;
}

bool next_n_hex_number(int* b, int* e, int n)
{
	int *p;
	for (p = e-1; p >= b; p--) 
	{
		if (*p < n-1) break;
	}
	for (int* q = p+1; q < e; q++) *q = 0;
	if (p >= b)
	{
		(*p)++;
		return true;
	}
	return false;
}

int get_pred(wchar_t op)
{
	switch (op)
	{
	case '+':
	case '-': return 1;
	case '*':
	case '/': return 2;
	case '^': return 3;
	case '_': return 4;
	}
	return -1;
}

bool exchangable(wchar_t op)
{
	return op == '+' || op == '*';
}
bool associative(wchar_t op)
{
	return op == '+' || op == '-' || op == '*' || op == '/';
}




enum nodetype {nt_op, nt_num};
struct node
{
	node* l_child;
	node* r_child;
	nodetype t;
	wchar_t op;
	std::wstring translist;
	double num;

	explicit node(nodetype type, wchar_t op = '+', int num = 0)
	{
		t = type;
		l_child = r_child = 0;
		this->op = op;
		this->num = num;
		translist.reserve(10);
	}
	~node()
	{
		delete l_child;
		delete r_child;
	}

	double val() const
	{
		double ret = 0;
		if (t == nt_num)
		{
			ret = num;
		}
		else
		{
			switch (op)
			{
			case '+': ret = l_child->val() + r_child->val(); break;
			case '-': ret = l_child->val() - r_child->val(); break;
			case '*': ret = l_child->val() * r_child->val(); break;
			case '/': ret = l_child->val() / r_child->val(); break;
			case '^': ret = pow(l_child->val(), r_child->val()); break;
			case '_': ret = l_child->val() * 10 + r_child->val(); break;
			}
		}
		
		for (size_t i = 0; i < translist.size(); i++)
		{
			switch (translist[i])
			{
			case '!': ret = calc_frac(ret); break;
			}
		}
		return ret;
	}
};

struct uti // unary translation info
{
	wchar_t op; // 一元操作符
	int at_ub;  // 最多的应用次数
};
typedef std::list<uti> uti_list_t;

// 包括自身
int get_op_count(const node* p)
{
	if (!p || p->t == nt_num) return 0;
	return get_op_count(p->l_child) + get_op_count(p->r_child) + 1;
}

// 构造初始运算树，n为操作符的个数
node* initial_state(int n)
{
	if (n == 0) return new node(nt_num);
	node* p = new node(nt_op);
	p->r_child = new node(nt_num);
	p->l_child = initial_state(n-1);
	return p;
}

// 骨架树变到下一骨架。所有骨架树的总数应该是catalan数C(2n,n)/(n+1)
// 对传统24点，n=3，骨架树共有C(6,3)/4 = 5种
bool go_next_state(node* p)
{
	if (!p) return false;
	// r_child^
	if (go_next_state(p->r_child)) return true;
	// l_child^, r_child=0
	if (go_next_state(p->l_child))
	{
		node* r_child_new = initial_state(get_op_count(p->r_child));
		delete (p->r_child);
		p->r_child = r_child_new;
		return true;
	}
	// l_child--=0 r_child++=0
	int lcount = get_op_count(p->l_child);
	int rcount = get_op_count(p->r_child);
	if (lcount > 0)
	{
		node* l_child_new = initial_state(lcount-1);
		node* r_child_new = initial_state(rcount+1);
		delete (p->l_child);
		delete (p->r_child);
		p->l_child = l_child_new;
		p->r_child = r_child_new;
		return true;
	}

	return false;
}

std::wstring get_trans(const node* p, const std::wstring& exp)
{
	std::wstring ret = exp;
	for (size_t i = 0; i < p->translist.size(); i++)
	{
		switch (p->translist[i])
		{
		case '!':
			if (p->t == nt_num && i == 0)
			{
				ret += p->translist[i];
			}
			else
			{
				ret = std::wstring(L"(") + ret + std::wstring(L")") + p->translist[i];
			}
			break;
		}
	}
	return ret;
}

std::wstring get_exp(const node* p)
{
	if (!p) return L"";
	if (p->t == nt_num)
	{
		std::wostringstream oss;
		oss << p->num;
		return get_trans(p, oss.str());
	}
	std::wstring lexp = get_exp(p->l_child);
	std::wstring rexp = get_exp(p->r_child);
	
	if (p->l_child->t == nt_op && get_pred(p->l_child->op) < get_pred(p->op))
	{
		lexp = std::wstring(L"(") + lexp + L")";
	}
	if (p->r_child->t == nt_op && !(get_pred(p->op) < get_pred(p->r_child->op)))
	{
		rexp = std::wstring(L"(") + rexp + L")";
	}
	
	std::wstring e;
	if (p->op == L'_') 
		e = lexp + rexp;
	else 
		e = lexp + L" " + p->op + L" " + rexp;
	return get_trans(p, e);
}

// 在线性空间对表达式树生成所有node的引用，方便为node们赋值
void get_node_ref(node* p, node*** op_nodes, node*** num_nodes)
{
	if (!p) return;
	if (p->t == nt_op) *(*op_nodes)++ = p;
	if (p->t == nt_num) *(*num_nodes)++ = p;
	get_node_ref(p->l_child, op_nodes, num_nodes);
	get_node_ref(p->r_child, op_nodes, num_nodes);
}

// 形状比较
bool shape_less(const node* n1, const node* n2, bool check_num_val = true)
{
	if (!n2) return false;
	if (!n1) return true;
	if (n1->t == nt_num && n2->t == nt_op)  return true;
	if (n1->t == nt_op  && n2->t == nt_num) return false;
	if (n1->t == nt_num && n2->t == nt_num) return check_num_val && n1->num < n2->num;
	if (n1->t == nt_op  && n2->t == nt_op)
	{
		if (n1->op < n2->op) return true;
		if (n1->op > n2->op) return false;
		if (n1->op == n2->op)
		{
			if (shape_less(n1->l_child, n2->l_child)) return true;
			if (shape_less(n2->l_child, n1->l_child)) return false;
			if (shape_less(n1->r_child, n2->r_child)) return true;
			if (shape_less(n2->r_child, n1->r_child)) return false;
		}
	}
	return false;
}

bool check_bone(const node* p, int rr_level)
{
	if (!p || p->t == nt_num) return true;

	// '_'是连字符，要求右孩子是数，左孩子是数或者也是个'_'
	if (p->op == L'_')
	{
		if (p->r_child->t == nt_op) return false;
		if (p->l_child->t == nt_op && p->l_child->op != L'_') return false;
	}

	if (rr_level >= 1)
	{
		// 交换率：规定两个分支的顺序
		if (exchangable(p->op))
		{
			// 不可以在这里判断，在check_exp时对满足交换率的左右子树采用了求值的方法：大值在左
			// if (shape_less(p->l_child, p->r_child, false)) return false;
		}

		// 不能出现 A-B+C A/B*C，应该是A+C-B, A*C/B
		if (p->l_child->t == nt_op && get_pred(p->op) == get_pred(p->l_child->op))
		{
			if (p->op < p->l_child->op) return false;
		}

		// 结合率
		// 不能出现 A+(B+C), A+(B-C), A-(B+C), A-(B-C)这样的情况，乘除类似
		if (p->r_child->t == nt_op && associative(p->op) && get_pred(p->op) == get_pred(p->r_child->op)) return false;
	}

	if (!check_bone(p->l_child, rr_level)) return false;
	if (!check_bone(p->r_child, rr_level)) return false;
	return true;
}

// 这个是在分配好一元操作符之后调用
bool check_bone2(const node* p, int rr_level)
{
	if (!p || p->t == nt_num) return true;
	
	if (p->op == L'_')
	{
		if (p->l_child->t == nt_num)
		{
			if (p->l_child->translist.size() > 0) return false;
			int ival = (int)floor(p->l_child->num + 0.5);
			if (ival < 0 || ival > 9 || !double_equ(p->l_child->num, ival)) return false;
		}
		if (p->r_child->t == nt_num)
		{
			if (p->r_child->translist.size() > 0) return false;
			int ival = (int)floor(p->r_child->num + 0.5);
			if (ival < 0 || ival > 9 || !double_equ(p->r_child->num, ival)) return false;
		}
	}

	if (!check_bone2(p->l_child, rr_level)) return false;
	if (!check_bone2(p->r_child, rr_level)) return false;
	return true;
}

bool check_exp(const node* p, int rr_level)
{
	if (!p || p->t == nt_num) return true;
	// rrlvl1: 交换率，以下两个判断保证了A+B+C+...+Z这种形式下，只选择A>B>C>...>Z这一种组合
	if (rr_level >= 1)
	{
		if (exchangable(p->op))
		{
			double v1 = p->l_child->val();
			double v2 = p->r_child->val();
			if (v1 < v2) return false;
			if (double_equ(v1, v2) && shape_less(p->l_child, p->r_child)) return false;
		}
		if (p->l_child->t == nt_op && p->l_child->op == p->op && associative(p->op))
		{
			double v1 = p->l_child->r_child->val();
			double v2 = p->r_child->val();
			if (v1 < v2) return false;
			if (double_equ(v1, v2) && shape_less(p->l_child->r_child, p->r_child)) return false;
		}
	}

	// rrlvl2: 去除/1和-0。/1可以用*1替换，-0可以用+0替换。
	if (rr_level >= 2)
	{
		if (p->op == L'/' && double_equ(p->r_child->val(), 1)) return false;
		if (p->op == L'-' && double_equ(p->r_child->val(), 0)) return false;
	}

	if (!check_exp(p->l_child, rr_level)) return false;
	if (!check_exp(p->r_child, rr_level)) return false;

	return true;
}

void iterate_num_permutation(node* exp, node** num_nodes, double* nums, int num_count, double result, int rr_level)
{
	do
	{
		for (int i = 0; i < num_count; i++) num_nodes[i]->num = nums[i];
		if (check_exp(exp, rr_level))
		{
			if (g_debug)
			{
		    	std::wcout << get_exp(exp) << " = " << exp->val() << std::endl;
			}
			if (double_equ(exp->val(), result))
			{
				std::wcout << get_exp(exp) << " = " << result << std::endl;
			}
		}
	}while (std::next_permutation(nums, nums + num_count));
}

// 在一个固定形状表达式树上进行搜索
void calc_on_exptree(node* exp, const wchar_t* bop, const uti_list_t& uop, double* nums, int num_count, double result, int rr_level)
{
	// 首先得到所有结点的线性引用
	int n = get_op_count(exp);
	node** op_nodes = new node*[n];
	node** num_nodes = new node*[n+1];

	node** tmp_opnodes = op_nodes;
	node** tmp_numnodes = num_nodes;
	get_node_ref(exp, &tmp_opnodes, &tmp_numnodes);

	std::sort(nums, nums + num_count);

	size_t oup_count = 1;
	for (uti_list_t::const_iterator it = uop.begin(); it != uop.end(); ++it) oup_count *= (it->at_ub+1);

	// 遍历所有操作符组合和操作数组合
	int* op_w = new int[n]();
	int* uop_w = new int[2*n+1]();
	size_t boplen = wcslen(bop);
	int i;
	do
	{
		for (i = 0; i < n; i++) op_nodes[i]->op = bop[op_w[i]];
		if (check_bone(exp, rr_level))
		{
			do
			{
				// 为每个结点分uop
				for (i = 0; i < n*2 + 1; i++)
				{
					node* p = (i < n)? op_nodes[i] : num_nodes[i - n];
					int pattern = uop_w[i];
					p->translist.clear();
					for (uti_list_t::const_iterator it = uop.begin(); it != uop.end(); ++it)
					{
						int c = pattern % (it->at_ub+1);
						pattern /= (it->at_ub+1);
						p->translist.append(c, it->op);
					}
				}
				if (check_bone2(exp, rr_level))
				{
					iterate_num_permutation(exp, num_nodes, nums, num_count, result, rr_level);
				}
			} while (next_n_hex_number(uop_w, uop_w + 2*n + 1, oup_count));
		}
	} while (next_n_hex_number(op_w, op_w + n, boplen));

	delete [] op_w;
	delete [] op_nodes;
	delete [] num_nodes;
}

void usage()
{
	std::wcout << 
		"usage: cal24 [options] num num [...] result\n"
		"options: \n"
		"  -p,--opset\n"
		"      operator set to use. default: +-*/\n"
		"      '-p all' to use the full set: +-*/^_!.\n"
		"      '_' means concat 2 oprands as one decimal number.  e.g. 3_5 <=> 35.\n"
		"      a number can follow '!' to specify max apply times when it's used.\n"
		"  -r,--rrlvl\n"
		"      redundant removal level. default: 1\n"
		"      0: no redundant removal \n"
		"      1: remove equivalents under the commulative and associative property\n"
		"         for example, '1+2' would be removed, yet '2+1' be selected\n"
		"      2: also remove '/1' and '-0'. \n"
		"         for example, '4-0', '6*7/1', '(4+5)/(9-8)' would be removed\n"
		"  -h,--help    show help\n"
		"\n"
		"examples: \n"
		"  cal24 5 5 5 1 24\n"
		"  cal24 1 2 3 4 5 6 100\n"
		"  cal24 4 1 25 -p \"+!\"\n"
		"  cal24 --opset=\"+-*/^!2\" 1 2 3 4 89\n"
		"  cal24 1 2 3 4 24 -r 2 -p \"+-*/_\"\n"
		"\n"
		"any bugs, please report to timepp@126.com\n"
		;
}

int wmain(int argc, wchar_t* argv[])
{
	std::wstring op_set = L"+-*/";
	std::wstring all_bop = L"+-*/^_";
	std::wstring all_uop = L"!";
	bool show_help = false;
	int rr_level = 1;

	tp::cmdline_parser parser;
	parser.add_option(L'p', L"opset", &op_set, true, &tp::cmdline_parser::cf_string);
	parser.add_option(L'h', L"help", &show_help, false, &tp::cmdline_parser::cf_bool);
	parser.add_option(L'r', L"rrlvl", &rr_level, true, &tp::cmdline_parser::cf_int);
	parser.add_option(0, L"debug", &g_debug, false, &tp::cmdline_parser::cf_bool);
	if (!parser.parse(argc, argv))
	{
		usage();
		return 1;
	}
	if (show_help)
	{
		usage();
		return 0;
	}
	if (op_set == L"all") op_set = all_bop + all_uop;

	size_t c = parser.get_target_connt();
	if (c < 3)
	{
		usage();
		return 1;
	}

	// op_set -> bop+uop
	std::wstring bop;
	uti_list_t uop;
	for (size_t i = 0; i < op_set.size(); i++)
	{
		if (all_bop.find(op_set[i]) != std::wstring::npos )
		{
			bop += op_set[i];
		}
		else if (all_uop.find(op_set[i]) != std::wstring::npos)
		{
			uti u = {op_set[i], 0};
			for (i++; op_set[i] >= '0' && op_set[i] <= '9'; i++)
			{
				u.at_ub = u.at_ub * 10 + op_set[i] - '0';
			}
			if (u.at_ub == 0) u.at_ub = 1;
			uop.push_back(u);
		}
		else
		{
			std::wcout << op_set[i] << L" is not a valid operator, run 'cal24 --help' to see valid operators" << std::endl;
			return -2;
		}
	}

	double d = _wtof(parser.get_target(c-1).c_str());
	size_t num_count = c-1;
	double* nums = new double[num_count];
	for (size_t i = 0; i < num_count; i++)
	{
		nums[i] = _wtof(parser.get_target(i).c_str());
	}

	node* p = initial_state(num_count-1);
	do
	{
		calc_on_exptree(p, bop.c_str(), uop, nums, num_count, d, rr_level);
	}while (go_next_state(p));

	delete p;
	delete [] nums;
	return 0;
}

