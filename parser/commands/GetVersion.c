#include <stdio.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xpltree.h>
#include "commands/GetVersion.h"

#ifdef __GNUC__
	#include <gnu/libc-version.h>
#else
	#include <iconv.h>
#endif
#include <oniguruma.h>
#ifdef _XEF_HTML_CLEANER_TIDY
# include <tidy/tidy.h>
#endif
#include <zlib.h>
#ifdef _USE_LZMA
# include "lzma.h"
#endif

void xplCmdGetVersionPrologue(xplCommandInfoPtr commandInfo)
{
}

#define NAME_A BAD_CAST("name")
#define COMPILED_A BAD_CAST("compiled")
#define RUNNING_A BAD_CAST("running")
#define NOT_CHECKABLE BAD_CAST ("not checkable")

xmlNodePtr listLibraryVersions(xmlNsPtr ns, xmlChar *tagname, xmlDocPtr doc) // TODO move this out
{
	char buf[16];
	xmlNodePtr ret, tail, cur;
#define APPEND() { tail->next = cur; cur->prev = tail; tail = cur; }
/* libiconv */
	ret = tail = cur = xmlNewDocNode(doc, ns, tagname, NULL);
	xmlNewProp(cur, NAME_A, BAD_CAST "libiconv");
#ifdef __GNUC__
	snprintf(buf, sizeof(buf), "%s", gnu_get_libc_version());
#else
	snprintf(buf, sizeof(buf), "%d.%d", _LIBICONV_VERSION >> 8, _LIBICONV_VERSION & 0xFF);
#endif
	xmlNewProp(cur, COMPILED_A, BAD_CAST buf);
	/* there's no safe & universal way to check */
	xmlNewProp(cur, RUNNING_A, NOT_CHECKABLE);
/* libidn */
	cur = xmlNewDocNode(doc, ns, tagname, NULL);
	xmlNewProp(cur, NAME_A, BAD_CAST "libidn");
	/* no any version info :( */
	xmlNewProp(cur, COMPILED_A, NOT_CHECKABLE);
	xmlNewProp(cur, RUNNING_A, NOT_CHECKABLE);
	APPEND()
/* Oniguruma */
	cur = xmlNewDocNode(doc, ns, tagname, NULL);
	xmlNewProp(cur, NAME_A, BAD_CAST "oniguruma");
	snprintf(buf, sizeof(buf), "%d.%d.%d", ONIGURUMA_VERSION_MAJOR, ONIGURUMA_VERSION_MINOR, ONIGURUMA_VERSION_TEENY);
	xmlNewProp(cur, COMPILED_A, BAD_CAST buf);
	xmlNewProp(cur, RUNNING_A, BAD_CAST onig_version());
	APPEND()
#ifdef _XEF_HAS_HTML_CLEANER
	cur = xmlNewDocNode(doc, ns, tagname, NULL);
#ifdef _XEF_HTML_CLEANER_TIDY
	/* Tidy doesn't export its version */
	xmlNewProp(cur, NAME_A, BAD_CAST "htmltidy");
	xmlNewProp(cur, COMPILED_A, BAD_CAST tidyLibraryVersion());
	xmlNewProp(cur, RUNNING_A, NOT_CHECKABLE);
#else
	xmlNewProp(cur, COMPILED_A, BAD_CAST "not compiled in");
	xmlNewProp(cur, RUNNING_A, BAD_CAST "not compiled in");
#endif
	APPEND()
#endif
/* ToDo: transport */
/* LibXML2 */
	cur = xmlNewDocNode(doc, ns, tagname, NULL);
	xmlNewProp(cur, NAME_A, BAD_CAST "libxml2");
	xmlNewProp(cur, COMPILED_A, BAD_CAST LIBXML_VERSION_STRING""LIBXML_VERSION_EXTRA);
	xmlNewProp(cur, RUNNING_A, BAD_CAST xmlParserVersion);
	APPEND()
/* ZLib */
	cur = xmlNewDocNode(doc, ns, tagname, NULL);
	xmlNewProp(cur, NAME_A, BAD_CAST "zlib");
	xmlNewProp(cur, COMPILED_A, BAD_CAST ZLIB_VERSION);
	xmlNewProp(cur, RUNNING_A, BAD_CAST zlibVersion());
	APPEND()
#ifdef _USE_LZMA
	cur = xmlNewDocNode(doc, ns, tagname, NULL);
	xmlNewProp(cur, NAME_A, BAD_CAST "lzma");
	xmlNewProp(cur, COMPILED_A, BAD_CAST LZMA_VERSION_STRING);
	xmlNewProp(cur, RUNNING_A, BAD_CAST lzma_version_string());
	APPEND()
#endif
#undef APPEND
	return ret;
}
#undef NOT_CHECKABLE

void xplCmdGetVersionEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define PART_ATTR (BAD_CAST "part")
#define TAGNAME_ATTR (BAD_CAST "tagname")
#define REPEAT_ATTR (BAD_CAST "repeat")
	xmlChar *part_attr = NULL;
	xmlChar *tagname_attr = NULL;
	xmlChar *tagname;
	xmlNsPtr tagname_ns = NULL;
	xmlChar *ver;
	xmlNodePtr ret, error;
	bool repeat;

	part_attr = xmlGetNoNsProp(commandInfo->element, PART_ATTR);
	if (!part_attr || !xmlStrcasecmp(part_attr, BAD_CAST "full"))
	{
		ver = BAD_CAST XPL_STRDUP((char*) XPL_VERSION_FULL);
	} else if (!xmlStrcasecmp(part_attr, BAD_CAST "major")) {
		ver = (xmlChar*) XPL_MALLOC(12);
		sprintf((char*) ver, "%d", XPL_VERSION_MAJOR);
	} else if (!xmlStrcasecmp(part_attr, BAD_CAST "minor")) {
		ver = (xmlChar*) XPL_MALLOC(12);
		sprintf((char*) ver, "%d", XPL_VERSION_MINOR);
	} else if (!xmlStrcasecmp(part_attr, BAD_CAST "libs")) {
		tagname_attr = xmlGetNoNsProp(commandInfo->element, TAGNAME_ATTR);
		if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
		{
			ASSIGN_RESULT(error, true, true);
			goto done;
		}
		if (tagname_attr)
		{
			EXTRACT_NS_AND_TAGNAME(tagname_attr, tagname_ns, tagname, commandInfo->element);
		} else
			tagname = BAD_CAST "library";
		ASSIGN_RESULT(listLibraryVersions(tagname_ns, tagname, commandInfo->element->doc), repeat, true);
		goto done;
	} else {
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown part attribute: \"%s\"", part_attr), true, true);
		goto done;
	}
	ret = xmlNewDocText(commandInfo->document->document, NULL);
	ret->content = ver;
	ASSIGN_RESULT(ret, false, true);
done:
	if (part_attr)
		XPL_FREE(part_attr);
	if (tagname_attr)
		XPL_FREE(tagname_attr);
}

xplCommand xplGetVersionCommand = { xplCmdGetVersionPrologue, xplCmdGetVersionEpilogue };
