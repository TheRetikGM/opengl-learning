#pragma once
#define DC_RED		    "\x1b[31m"
#define DC_GREEN	    "\x1b[32m"
#define DC_YELLOW		"\x1b[33m"
#define DC_CYAN		    "\x1b[36m"
#define DC_MAGNETA	    "\x1b[35m"
#define DC_DEFAULT	 	"\x1b[0m"
#define DC_ERROR	 	DC_RED    "[ERROR]"		DC_DEFAULT
#define DC_SUCCESS		DC_GREEN  "[SUCCESS]"	DC_DEFAULT
#define DC_INFO			DC_CYAN	  "[INFO]"		DC_DEFAULT
#define DC_WARNING		DC_YELLOW "[WARNING]"	DC_DEFAULT

#define DC_CURSOR_MOVE(y, x)   std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H"
#define DC_CURSOR_SAVE()       std::cout << "\033[s"
#define DC_CURSOR_RESTORE()    std::cout << "\033[u"
#define DC_CLRTOEOL()          std::cout << "\033[K"
#define DC_CLRSCR()            std::cout << "\033[2J"