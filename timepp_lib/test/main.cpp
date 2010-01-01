#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <tplib.h>
#include <cmdlineparser.h>
#include <exception.h>
#include <wincon.h>
#include <opblock.h>



void test_old()
{
	ON_LEAVE(tp::log(tp::a2w("测试自动释放宏，此日志应该在程序退出时输出\n")));
	tp::cmdline_parser parser;
//	parser.parse(argc, argv);
	tp::log(tp::a2w(tp::czA("命令行:%s", tp::w2a(GetCommandLineW()))));
//	for (int i = 0; i < argc; i++)
	{
//		tp::log_indenter li(2);
//		tp::log(tp::cz(L"%02d %s", i, argv[i]));
	}

	tp::throw_winerr_when(true);

	tp::log(L"check main");
	{
		tp::log_indenter li(4);
		unsigned char tmp[] = "ABCDE\xcc\xdd\xee\x00""acc";
		tp::log(2, tp::cz(L"the content of array(%d bytes)\n%s", sizeof(tmp), &tp::hex_dump(tmp, sizeof(tmp))));
	}
}

int wmain(int argc, wchar_t *argv[])
{
	setlocale(LC_ALL, "");
	tp::sc::log_default_console_config();

	OPBLOCK(L"aaa");
	OPBLOCK(L"bbb");
	SETOP(L"ccc");

	tp::log(tp::cz(L"%s", CURRENT_OPLIST().c_str()));

	try
	{
		test_old();
	}
	catch (tp::exception& e)
	{
		tp::log(tp::cz(L"错误：%s  当前操作：(%s)", e.err->desc().c_str(), e.oplist.c_str()));
	}

	return 0;
}
