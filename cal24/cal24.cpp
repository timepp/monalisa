﻿#include <algorithm>
#include <string>
#include <cmath>
#include <sstream>
#include <iostream>
#include "../timepp_lib/include/cmdlineparser.h"

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

bool double_equ(double a, double b)
{
	return std::fabs(a - b) < 1E-6;
}


enum nodetype {nt_op, nt_num};
struct node
{
	node* l_child;
	node* r_child;
	nodetype t;
	wchar_t op;
	double num;

	explicit node(nodetype type, wchar_t op = '+', int num = 0)
	{
		t = type;
		l_child = r_child = 0;
		this->op = op;
		this->num = num;
	}
	~node()
	{
		delete l_child;
		delete r_child;
	}

	double val() const
	{
		if (t == nt_num)
		{
			return num;
		}
		else
		{
			switch (op)
			{
			case '+': return l_child->val() + r_child->val();
			case '-': return l_child->val() - r_child->val();
			case '*': return l_child->val() * r_child->val();
			case '/': return l_child->val() / r_child->val();
			case '^': return pow(l_child->val(), r_child->val());
			case '_': return l_child->val() * 10 + r_child->val();
			}
		}
		return 0;
	}
};

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

std::wstring get_exp(const node* p)
{
	if (!p) return L"";
	if (p->t == nt_num)
	{
		std::wostringstream oss;
		oss << p->num;
		return oss.str();
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
	
	if (p->op == L'_') 
		return lexp + rexp;
	else 
		return lexp + L" " + p->op + L" " + rexp;
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

// 在一个固定形状表达式树上进行搜索
void calc_on_exptree(node* exp, const wchar_t* ops, int* nums, int num_count, int result, int rr_level)
{
	// 首先得到所有结点的线性引用
	int n = get_op_count(exp);
	node** op_nodes = new node*[n];
	node** num_nodes = new node*[n+1];

	node** tmp_opnodes = op_nodes;
	node** tmp_numnodes = num_nodes;
	get_node_ref(exp, &tmp_opnodes, &tmp_numnodes);

	// 遍历所有操作符组合和操作数组合
	int* op_w = new int[n]();
	int i;
	for(;;)
	{
		for (i = 0; i < n; i++) op_nodes[i]->op = ops[op_w[i]];
		if (check_bone(exp, rr_level))
		{
			std::sort(nums, nums + num_count);
			do
			{
				for (i = 0; i < num_count; i++) num_nodes[i]->num = nums[i];
				if (check_exp(exp, rr_level))
				{
					if (double_equ(exp->val(), result))
					{
						std::wcout << get_exp(exp) << " = " << result << std::endl;
					}
				}
			}while (std::next_permutation(nums, nums + num_count));
		}
		// 递增op_w
		for (i = n-1; i >=0; i--) if (op_w[i] < wcslen(ops)-1) break;
		if (i < 0) break;
		op_w[i]++;
		for (i++; i < n; i++) op_w[i] = 0;
	}

	delete [] op_w;
	delete [] op_nodes;
	delete [] num_nodes;
}

void usage()
{
	std::wcout << 
		"usage: cal24 [options] num num [...] result\n"
		"options: \n"
		"  -p,--opset   set operator set to use. the full set is \"+-*/^_\"\n"
		"               the default set is \"+-*/\"\n"
		"               use '-p all' to use all op set\n"
		"  -r,--rrlvl   redundant removal level, larger value = cleaner output\n"
		"               level 0: no redundant removal \n"
		"               level 1: the default level, remove equivalents under\n"
		"                        the 'exchang' and 'assciation' translations\n"
		"               level 2: remove '/1' and '-0'. for example,\n"
		"                        '4-0', '6*7/1', '(4+5)/(9-8)' would be removed\n"
		"  -h,--help    show help\n"
		"\n"
		"examples: \n"
		"  cal24 5 5 5 1 24\n"
		"  cal24 1 2 3 4 5 6 100\n"
		"  cal24 --opset=\"+-*/^\" 1 2 3 4 82\n"
		"  cal24 1 2 3 4 24 -r 2 -p \"+-*/_\""
		"\n"
		"any bugs, please report to timepp@126.com\n"
		;
}

int wmain(int argc, wchar_t* argv[])
{
	std::wstring op_set = L"+-*/";
	std::wstring op_set_all = L"+-*/^_";
	bool show_help = false;
	int rr_level = 1;
	tp::cmdline_parser parser;
	parser.add_option(L'p', L"opset", &op_set, true, &tp::cmdline_parser::cf_string);
	parser.add_option(L'h', L"help", &show_help, false, &tp::cmdline_parser::cf_bool);
	parser.add_option(L'r', L"rrlvl", &rr_level, true, &tp::cmdline_parser::cf_int);
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
	if (op_set == L"all") op_set = op_set_all;

	size_t c = parser.get_target_connt();
	if (c < 3)
	{
		usage();
		return 1;
	}

	int d = _wtoi(parser.get_target(c-1).c_str());
	size_t num_count = c-1;
	int* nums = new int[num_count];
	for (size_t i = 0; i < num_count; i++)
	{
		nums[i] = _wtoi(parser.get_target(i).c_str());
	}

	node* p = initial_state(num_count-1);
	do
	{
		calc_on_exptree(p, op_set.c_str(), nums, num_count, d, rr_level);
	}while (go_next_state(p));

	delete p;
	delete [] nums;
	return 0;
}
