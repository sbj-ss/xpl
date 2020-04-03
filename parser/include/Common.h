/* stdafx.h : include file for standard system include files, */
/* or project specific include files that are used frequently, but */
/* are changed infrequently */

#pragma once
#ifndef _common_H
#define _common_H

#ifdef _WIN32
# define _CRT_SECURE_NO_WARNINGS
# define _CRT_RAND_S
# define _CRT_NON_CONFORMING_SWPRINTFS
# define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
# include <windows.h>
#endif

#include <string.h>
#include <ctype.h>

#include <libxml/HTMLparser.h>
#include <libxml/hash.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlIO.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlstring.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#endif
