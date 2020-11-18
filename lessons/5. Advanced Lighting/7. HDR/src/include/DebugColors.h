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