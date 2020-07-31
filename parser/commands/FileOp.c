#include <time.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplutils.h>
#include "commands/FileOp.h"

/* TODO review and probably rework getting rid of XPR_FS_ stuff */

void xplCmdFileOpPrologue(xplCommandInfoPtr commandInfo)
{
}

typedef enum 
{
	FILE_OP_UNKNOWN,
	FILE_OP_LIST,
	FILE_OP_DELETE,
	FILE_OP_COPY,
	FILE_OP_MOVE,
	FILE_OP_MKDIR
} FileOpType;

static FileOpType fileOpTypeFromString(xmlChar *s)
{
	if (!xmlStrcasecmp(s, BAD_CAST "list"))
		return FILE_OP_LIST;
	if (!xmlStrcasecmp(s, BAD_CAST "delete") || !xmlStrcasecmp(s, BAD_CAST "erase"))
		return FILE_OP_DELETE;
	if (!xmlStrcasecmp(s, BAD_CAST "copy"))
		return FILE_OP_COPY;
	if (!xmlStrcasecmp(s, BAD_CAST "move"))
		return FILE_OP_MOVE;
	if (!xmlStrcasecmp(s, BAD_CAST "mkdir"))
		return FILE_OP_MKDIR;
	return FILE_OP_UNKNOWN;
}

typedef struct _FileScanContext
{
	XPR_FILE_FIND_DATA data;
	XPR_FS_CHAR *full_name;
	XPR_FS_CHAR *destination;
	xplCommandInfoPtr command_info;
	xmlChar *tag_name;
	xmlNsPtr ns;
	xmlDocPtr doc;
	bool fail_if_exists;
	bool create_destination;
	/* scanner must not change fields above this comment! */
	xmlNodePtr head;
	xmlNodePtr tail;
} FileScanContext, *FileScanContextPtr;

typedef void (*fileScanner)(FileScanContextPtr ctxt);

static XPR_FS_CHAR* makeFSPath(const xmlChar *source, const xmlChar *appPath, bool absPath)
{
	xmlChar *full_path;
	XPR_FS_CHAR *fs_path = NULL;

	if (absPath)
	{
		if (!strrchr((char*) source, XPR_PATH_DELIM))
			return NULL; /* this is not absolute path */
		full_path = BAD_CAST source;
	} else
		full_path = xplFullFilename(source, appPath);
	xprConvertSlashes(full_path);
	if (xstrIconvString(XPR_FS_ENCODING, "utf-8", (char*) full_path, (char*) full_path + xmlStrlen(full_path), (char**) &fs_path, NULL) == -1)
	{
		XPL_FREE(full_path);
		return NULL;
	}
	if (full_path != source)
		XPL_FREE(full_path);
	return fs_path;
}

/* forward declarations */
static void scanThisLevel(
	XPR_FS_CHAR *source, 
	XPR_FS_CHAR *filemask,
	XPR_FS_CHAR *destination, 
	fileScanner scanner, 
	FileScanContextPtr ctxt
);
static void scanSublevels(
	XPR_FS_CHAR *source, 
	XPR_FS_CHAR *filemask,
	XPR_FS_CHAR *destination, 
	bool recursive, 
	fileScanner scanner, 
	FileScanContextPtr ctxt, 
	bool subfoldersFirst
);

static void scanFolder(
	 XPR_FS_CHAR *source,
	 XPR_FS_CHAR *filemask,
	 XPR_FS_CHAR *destination,
	 bool recursive,
	 fileScanner scanner, 
	 FileScanContextPtr ctxt, /* only for scanner */
	 bool subfoldersFirst
)
{
	if (subfoldersFirst)
	{
		scanSublevels(source, filemask, destination, recursive, scanner, ctxt, true);
		scanThisLevel(source, filemask, destination, scanner, ctxt);
	} else {
		scanThisLevel(source, filemask, destination, scanner, ctxt);
		scanSublevels(source, filemask, destination, recursive, scanner, ctxt, false);
	}
}

static void scanThisLevel(
	XPR_FS_CHAR *source,
	XPR_FS_CHAR *filemask,
	XPR_FS_CHAR *destination,
	fileScanner scanner, 
	FileScanContextPtr ctxt /* only for scanner */
)
{
	XPR_FS_CHAR *source_path_end;		/* last delimiter position */
	size_t source_path_len, destination_len;
	XPR_FILE_FIND_HANDLE search_handle;

	if (!source || !scanner || !ctxt)
		return;
	if (XPR_FILE_FIND_DONE(search_handle = XPR_FILE_FIND_FIRST(source, &ctxt->data)))
		return;

	source_path_end = XPR_FS_STRRCHR(source, XPR_FS_PATH_DELIM);
	source_path_len = source_path_end - source + 1; /* in characters! */
	if (destination)
		destination_len = XPR_FS_STRLEN(destination);

	do 
	{
		if (XPR_FS_STRCMP(XPR_FILE_NAME_FROM_FDP(&ctxt->data), XPR_MK_FS_STRING(".")) && 
			XPR_FS_STRCMP(XPR_FILE_NAME_FROM_FDP(&ctxt->data), XPR_MK_FS_STRING("..")))
			/* skip . and .. */
		{
			ctxt->full_name = (XPR_FS_CHAR*) XPL_MALLOC((source_path_len + XPR_FS_STRLEN(XPR_FILE_NAME_FROM_FDP(&ctxt->data)) + 1)*sizeof(XPR_FS_CHAR));
			/* d:\somedir\ */
			XPR_FS_STRNCPY(ctxt->full_name, source, source_path_len);
			/* d:\somedir\file.ext */
			XPR_FS_STRCPY(ctxt->full_name + source_path_len, XPR_FILE_NAME_FROM_FDP(&ctxt->data));
			if (destination)
			{
				ctxt->destination = (XPR_FS_CHAR*) XPL_MALLOC((destination_len
					+ XPR_FS_STRLEN(XPR_FILE_NAME_FROM_FDP(&ctxt->data))
					+ XPR_FS_STRLEN(XPR_FS_PATH_DELIM_STR) 
					+ 1
					)*sizeof(XPR_FS_CHAR));
				/* d:\otherdir */
				XPR_FS_STRCPY(ctxt->destination, destination);
				/* d:\otherdir\ */
				if (ctxt->destination[destination_len - 1] != XPR_PATH_DELIM)
					XPR_FS_STRCAT(ctxt->destination, XPR_FS_PATH_DELIM_STR);
				/* d:\otherdir\file.ext */
				XPR_FS_STRCAT(ctxt->destination, XPR_FILE_NAME_FROM_FDP(&ctxt->data));
			} else
				ctxt->destination = NULL;
			scanner(ctxt);
			XPL_FREE(ctxt->full_name);
			if (ctxt->destination)
				XPL_FREE(ctxt->destination);
		}
	} while (!XPR_FILE_FIND_DONE(XPR_FILE_FIND_NEXT(search_handle, &ctxt->data)));
	XPR_FILE_FIND_CLOSE(search_handle);
}

static void scanSublevels(
	XPR_FS_CHAR *source,
	XPR_FS_CHAR *filemask,
	XPR_FS_CHAR *destination,
	bool recursive,
	fileScanner scanner, 
	FileScanContextPtr ctxt, /* only for scanner */
	bool subfoldersFirst
)
{
	XPR_FS_CHAR *fs_subdir;
	XPR_FS_CHAR *source_path_end;		/* last delimiter position */
	XPR_FS_CHAR *filename;				/* just filename */
	XPR_FS_CHAR *rec_source, *rec_destination; /* for recursion */
	size_t source_path_len, filename_len, destination_len;
	XPR_FILE_FIND_HANDLE search_handle;

	if (!source || !scanner || !ctxt)
		return;

	source_path_end = XPR_FS_STRRCHR(source, XPR_FS_PATH_DELIM);
	source_path_len = source_path_end - source + 1; /* in characters! */
	if (destination)
		destination_len = XPR_FS_STRLEN(destination);

	/* file.ext */
	filename = XPR_FS_STRDUP(source_path_end);
	filename_len = XPR_FS_STRLEN(filename);
	/* d:\somedir\file.ext -> d:\somedir\* */
	fs_subdir = (XPR_FS_CHAR*) XPL_MALLOC((source_path_len + 2)*sizeof(XPR_FS_CHAR)); /* +2: '*', \0 */
	XPR_FS_STRNCPY(fs_subdir, source, source_path_len);
	fs_subdir[source_path_len] = XPR_MK_FS_CHAR('*');
	fs_subdir[source_path_len + 1] = 0;

	if (!XPR_FILE_FIND_DONE(search_handle = XPR_FILE_FIND_FIRST(fs_subdir, &ctxt->data)))
	{
		do {
			/* skip non-directories */
			if (!XPR_FILE_IS_DIRECTORY(&ctxt->data))
				continue;
			/* skip . and .. */
			if (!XPR_FS_STRCMP(XPR_FILE_NAME_FROM_FDP(&ctxt->data), XPR_MK_FS_STRING(".")) || 
				!XPR_FS_STRCMP(XPR_FILE_NAME_FROM_FDP(&ctxt->data), XPR_MK_FS_STRING("..")))
				continue;
			rec_source = (XPR_FS_CHAR*) XPL_MALLOC((source_path_len
				+ XPR_FS_STRLEN(XPR_FILE_NAME_FROM_FDP(&ctxt->data)) 
				+ filename_len 
				+ 1
			)*sizeof(XPR_FS_CHAR));
			/* d:\somedir\ */
			XPR_FS_STRNCPY(rec_source, source, source_path_len);
			/* d:\somedir\subdir\ */
			XPR_FS_STRCPY(rec_source + source_path_len, XPR_FILE_NAME_FROM_FDP(&ctxt->data));
			/* d:\somedir\subdir\file.ext */
			XPR_FS_STRCAT(rec_source, filename);
			if (destination)
			{
				rec_destination = (XPR_FS_CHAR*) XPL_MALLOC((destination_len
					+ XPR_FS_STRLEN(XPR_FILE_NAME_FROM_FDP(&ctxt->data)) 
					+ 2
				)*sizeof(XPR_FS_CHAR));
				/* d:\otherdir */
				XPR_FS_STRCPY(rec_destination, destination);
				/* d:\otherdir\ */
				if (destination_len && (destination[destination_len - 1] != XPR_FS_PATH_DELIM))
					XPR_FS_STRCAT(rec_destination, XPR_FS_PATH_DELIM_STR);
				/* d:\otherdir\subdir */
				XPR_FS_STRCAT(rec_destination, XPR_FILE_NAME_FROM_FDP(&ctxt->data));
			} else
				rec_destination = NULL;
			scanFolder(rec_source, filemask, rec_destination, true, scanner, ctxt, subfoldersFirst);
			XPL_FREE(rec_source);
			if (rec_destination)
				XPL_FREE(rec_destination);
		} while (!XPR_FILE_FIND_DONE(XPR_FILE_FIND_NEXT(search_handle, &ctxt->data)));
		XPR_FILE_FIND_CLOSE(search_handle);
	}
	XPL_FREE(fs_subdir);
	free(filename); /* NOT xmlFree! */
}

static void deleteFilesScanner(FileScanContextPtr ctxt)
{
	xmlNodePtr error;
	xmlChar *utf8name = NULL;

	if XPR_FILE_UNLINK_FAILED(XPR_FILE_UNLINK(ctxt->full_name))
	{
		xstrIconvString("utf-8", XPR_FS_ENCODING, (const char*) ctxt->full_name, 
		(const char*) ctxt->full_name + XPR_FS_STRLEN(ctxt->full_name)*sizeof(XPR_FS_CHAR), (char**) &utf8name, NULL);
		error = xplCreateErrorNode(ctxt->command_info->element, BAD_CAST "cannot erase file \"%s\"", utf8name);
		XPL_FREE(utf8name);

		if (!ctxt->head)
			ctxt->head = ctxt->tail = error;
		else
			ctxt->tail = xmlAddNextSibling(ctxt->tail, error);
	}
}

static void deleteFiles(
	XPR_FS_CHAR* source,
	XPR_FS_CHAR *filemask,
	bool recursive, 
	FileScanContextPtr ctxt
)
{
	/* subfolders first */
	scanFolder(source, filemask, NULL, recursive, deleteFilesScanner, ctxt, true);
}

__inline void markAttribute(xmlNodePtr cur, xmlChar *attrName, bool value)
{
	if (value)
		xmlNewProp(cur, attrName, BAD_CAST "true");
	else
		xmlNewProp(cur, attrName, BAD_CAST "false");
}

__inline void markTime(xmlNodePtr cur, const xmlChar* name, const __time64_t *t)
{
	xmlChar buf[32];
	struct tm _tm;

	if (!_localtime64_s(&_tm, t))
	{
		strftime((char*) buf, 32, "%d.%m.%Y %H:%M:%S", &_tm);
		xmlNewProp(cur, name, buf);
	} else
		xmlNewProp(cur, name, BAD_CAST "unknown");
}

static void listFilesScanner(FileScanContextPtr ctxt)
{
	xmlNodePtr ret;
	xmlChar buf[32], *utf8name = NULL;

	if (!ctxt->tag_name)
		return;
	xstrIconvString("utf-8", XPR_FS_ENCODING, (const char*) ctxt->full_name, (const char*) ctxt->full_name + XPR_FS_STRLEN(ctxt->full_name)*2, (char**) &utf8name, NULL);
	ret = xmlNewDocNode(ctxt->doc, ctxt->ns, ctxt->tag_name, NULL);
	xmlNewProp(ret, BAD_CAST "name", utf8name);
	XPL_FREE(utf8name);
	markAttribute(ret, BAD_CAST "archive",   XPR_FILE_IS_ARCHIVE(&ctxt->data));
	markAttribute(ret, BAD_CAST "directory", XPR_FILE_IS_DIRECTORY(&ctxt->data));
	markAttribute(ret, BAD_CAST "hidden",    XPR_FILE_IS_HIDDEN(&ctxt->data));
	markAttribute(ret, BAD_CAST "readonly",  XPR_FILE_IS_READONLY(&ctxt->data));
	markAttribute(ret, BAD_CAST "system",    XPR_FILE_IS_SYSTEM(&ctxt->data));
	_ui64toa(XPR_FILE_SIZE_FROM_FDP(&ctxt->data), (char*) buf, 10);
	xmlNewProp(ret, BAD_CAST "size", buf);
	markTime(ret, BAD_CAST "accesstime", XPR_FILE_ATIME_FROM_FDP(&ctxt->data));
	markTime(ret, BAD_CAST "createtime", XPR_FILE_CTIME_FROM_FDP(&ctxt->data));
	markTime(ret, BAD_CAST "writetime",  XPR_FILE_WTIME_FROM_FDP(&ctxt->data));

	if (!ctxt->head)
		ctxt->head = ctxt->tail = ret;
	else
		ctxt->tail = xmlAddNextSibling(ctxt->tail, ret);		
}

static void listFiles(
	XPR_FS_CHAR* source, 
	XPR_FS_CHAR *filemask,
	bool recursive, 
	FileScanContextPtr ctxt
)
{
	/* higher levels first */
	scanFolder(source, filemask, NULL, recursive, listFilesScanner, ctxt, false);
}

static void copyFilesScanner(FileScanContextPtr ctxt)
{
	xmlNodePtr error;
	xmlChar *utf8name = NULL, *utf8dst = NULL, *utf8error_msg;

	if (XPR_FILE_IS_DIRECTORY(&ctxt->data)) /* directories aren't copied by functions */
	{
		xprEnsurePathExistence(ctxt->destination);
		return;
	}
	if XPR_FILE_COPY_FAILED(XPR_FILE_COPY(ctxt->full_name, ctxt->destination, ctxt->fail_if_exists))
	{
		if (ctxt->create_destination)
		{
			/* maybe it failed because destination doesn't exist? */
			if (xprEnsurePathExistence(ctxt->destination))
				if (!XPR_FILE_COPY_FAILED(XPR_FILE_COPY(ctxt->full_name, ctxt->destination, ctxt->fail_if_exists)))
					return; /* succeeded finally */
		}
		utf8error_msg = xprFormatSysError(XPR_GET_OS_ERROR());
		xstrIconvString("utf-8", XPR_FS_ENCODING, (const char*) ctxt->full_name, 
			(const char*) ctxt->full_name + XPR_FS_STRLEN(ctxt->full_name)*sizeof(XPR_FS_CHAR), (char**) &utf8name, NULL);
		xstrIconvString("utf-8", XPR_FS_ENCODING, (const char*) ctxt->destination, 
				(const char*) ctxt->destination + XPR_FS_STRLEN(ctxt->destination)*sizeof(XPR_FS_CHAR), (char**) &utf8dst, NULL);
		error = xplCreateErrorNode(ctxt->command_info->element, BAD_CAST "cannot copy file \"%s\" to \"%s\", OS error \"%s\"", utf8name, utf8dst, utf8error_msg);
		if (utf8dst) XPL_FREE(utf8dst);
		if (utf8name) XPL_FREE(utf8name);
		if (utf8error_msg) XPL_FREE(utf8error_msg);
		if (!ctxt->head)
			ctxt->head = ctxt->tail = error;
		else
			ctxt->tail = xmlAddNextSibling(ctxt->tail, error);
	}
}

static void copyFiles(
	XPR_FS_CHAR *source, 
	XPR_FS_CHAR *filemask,	
	XPR_FS_CHAR *destination, 
	bool recursive, 
	FileScanContextPtr ctxt
)
{
	/* higher levels first */
	scanFolder(source, filemask, destination, recursive, copyFilesScanner, ctxt, false);
}

static void moveFilesScanner(FileScanContextPtr ctxt)
{
	xmlNodePtr error;
	xmlChar *utf8name = NULL, *utf8dst = NULL, *utf8error_msg;

	if XPR_FILE_MOVE_FAILED(XPR_FILE_MOVE(ctxt->full_name, ctxt->destination, ctxt->fail_if_exists))
	{
		if (ctxt->create_destination)
		{
			/* maybe it failed because destination doesn't exist? */
			if (xprEnsurePathExistence(ctxt->destination))
				if (!XPR_FILE_MOVE_FAILED(XPR_FILE_MOVE(ctxt->full_name, ctxt->destination, ctxt->fail_if_exists)))
					return; /* succeeded finally */
		}
		utf8error_msg = xprFormatSysError(XPR_GET_OS_ERROR());
		xstrIconvString("utf-8", XPR_FS_ENCODING, (const char*) ctxt->full_name, 
			(const char*) ctxt->full_name + XPR_FS_STRLEN(ctxt->full_name)*sizeof(XPR_FS_CHAR), (char**) &utf8name, NULL);
		xstrIconvString("utf-8", XPR_FS_ENCODING, (const char*) ctxt->destination, 
			(const char*) ctxt->destination + XPR_FS_STRLEN(ctxt->destination)*sizeof(XPR_FS_CHAR), (char**) &utf8dst, NULL);
		error = xplCreateErrorNode(ctxt->command_info->element, BAD_CAST "cannot move file \"%s\" to \"%s\", OS error \"%s\"", utf8name, utf8dst, utf8error_msg);
		if (utf8dst) XPL_FREE(utf8dst);
		if (utf8name) XPL_FREE(utf8name);
		if (utf8error_msg) XPL_FREE(utf8error_msg);
		if (!ctxt->head)
			ctxt->head = ctxt->tail = error;
		else
			ctxt->tail = xmlAddNextSibling(ctxt->tail, error);
	}
}

static void moveFiles(
	XPR_FS_CHAR *source, 
	XPR_FS_CHAR *filemask,	
	XPR_FS_CHAR *destination, 
	bool recursive, 
	FileScanContextPtr ctxt
)
{
	/* higher levels first (balance folder creation) */
	scanFolder(source, filemask, destination, recursive, moveFilesScanner, ctxt, false);
}


void xplCmdFileOpEpilogue(xplCommandInfoPtr commandInfo, xplResultPtr result)
{
#define SOURCE_ATTR (BAD_CAST "source")
#define SELECT_ATTR (BAD_CAST "select")
#define FILEMASK_ATTR (BAD_CAST "filemask")
#define DESTINATION_ATTR (BAD_CAST "destination")
#define OPERATION_ATTR (BAD_CAST "operation")
#define RECURSIVE_ATTR (BAD_CAST "recursive")
#define ABSPATH_ATTR (BAD_CAST "abspath")
#define SRCABSPATH_ATTR (BAD_CAST "srcabspath")
#define DSTABSPATH_ATTR (BAD_CAST "dstabspath")
#define TAGNAME_ATTR (BAD_CAST "tagname")
#define FAILIFEXISTS_ATTR (BAD_CAST "failifexists")
#define CREATE_DESTINATION_ATTR (BAD_CAST "createdestination")
#define REPEAT_ATTR (BAD_CAST "repeat")

	xmlChar *source_attr = NULL;
	xmlChar *filemask_attr = NULL;
	xmlChar *destination_attr = NULL;
	xmlChar *operation_attr = NULL;
	xmlChar *tagname_attr = NULL;
	bool recursive;
	bool srcabspath = false, dstabspath = false;
	bool repeat;
	FileOpType op_type;
	xmlNodePtr error;
	XPR_FS_CHAR *fs_source = NULL, *fs_destination = NULL, *fs_filemask = NULL;
	FileScanContext ctxt;
	size_t src_len;

	operation_attr = xmlGetNoNsProp(commandInfo->element, OPERATION_ATTR);
	if (!operation_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "operation not specified"), true, true);
		return;
	}
	op_type = fileOpTypeFromString(operation_attr);
	if (op_type == FILE_OP_UNKNOWN)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "unknown operation: %s", operation_attr), true, true);
		goto done;
	}
	source_attr = xmlGetNoNsProp(commandInfo->element, SOURCE_ATTR);
	if (!source_attr) 
		source_attr = xmlGetNoNsProp(commandInfo->element, SELECT_ATTR);
	filemask_attr = xmlGetNoNsProp(commandInfo->element, FILEMASK_ATTR);
	if (!source_attr && !filemask_attr)
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "neither source nor filemask attribute specified"), true, true);
		goto done;
	}
	destination_attr = xmlGetNoNsProp(commandInfo->element, DESTINATION_ATTR);
	if (!destination_attr && ((op_type == FILE_OP_COPY) || (op_type == FILE_OP_MOVE)))
	{
		ASSIGN_RESULT(xplCreateErrorNode(commandInfo->element, BAD_CAST "no destination attribute specified"), true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, RECURSIVE_ATTR, &recursive, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, ABSPATH_ATTR, &srcabspath, false))) /* not a typo */
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if (srcabspath)
		dstabspath = true;
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, SRCABSPATH_ATTR, &srcabspath, srcabspath)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, DSTABSPATH_ATTR, &dstabspath, dstabspath)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, FAILIFEXISTS_ATTR, &ctxt.fail_if_exists, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, REPEAT_ATTR, &repeat, true)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	if ((error = xplDecodeCmdBoolParam(commandInfo->element, CREATE_DESTINATION_ATTR, &ctxt.create_destination, false)))
	{
		ASSIGN_RESULT(error, true, true);
		goto done;
	}
	tagname_attr = xmlGetNoNsProp(commandInfo->element, TAGNAME_ATTR);

	ctxt.command_info = commandInfo;
	ctxt.head = ctxt.tail = NULL;
	ctxt.doc = commandInfo->document->document;
	ctxt.full_name = NULL;
	ctxt.destination = NULL;
	if (tagname_attr)
	{
		EXTRACT_NS_AND_TAGNAME(tagname_attr, ctxt.ns, ctxt.tag_name, commandInfo->element)
	} else {
		ctxt.tag_name = NULL;
		ctxt.ns = NULL;
	}

	if (source_attr)
		fs_source = makeFSPath(source_attr, commandInfo->document->app_path, srcabspath);
	if (filemask_attr)
		xstrIconvString(XPR_FS_ENCODING, "utf-8", filemask_attr, filemask_attr + xmlStrlen(filemask_attr), (char**) &fs_filemask, NULL);
	if (destination_attr)
		fs_destination = makeFSPath(destination_attr, commandInfo->document->app_path, dstabspath);

	switch (op_type)
	{
	case FILE_OP_DELETE:
		deleteFiles(fs_source, fs_filemask, recursive, &ctxt);
		break;
	case FILE_OP_LIST:
		if (!ctxt.tag_name)
			ctxt.tag_name = BAD_CAST "file";
		listFiles(fs_source, fs_filemask, recursive, &ctxt);
		break;
	case FILE_OP_COPY:
		copyFiles(fs_source, fs_filemask, fs_destination, recursive, &ctxt);
		break;
	case FILE_OP_MOVE:
		moveFiles(fs_source, fs_filemask, fs_destination, recursive, &ctxt);
		break;
	case FILE_OP_MKDIR:
		/* trailing delimiter needed */
		src_len = XPR_FS_STRLEN(fs_source);
		if (fs_source[src_len - 1] != XPR_FS_PATH_DELIM)
		{
			fs_source = (XPR_FS_CHAR*) XPL_REALLOC(fs_source, (src_len + 2)*sizeof(XPR_FS_CHAR));
			fs_source[src_len] = XPR_FS_PATH_DELIM;
			fs_source[src_len+1] = 0;
		}
		xprEnsurePathExistence(fs_source);
		break;
	default:
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	}
	ASSIGN_RESULT(ctxt.head, repeat, true);
done:
	if (source_attr) XPL_FREE(source_attr);
	if (filemask_attr) XPL_FREE(filemask_attr);
	if (destination_attr) XPL_FREE(destination_attr);
	if (operation_attr) XPL_FREE(operation_attr);
	if (tagname_attr) XPL_FREE(tagname_attr);
	if (fs_source) XPL_FREE(fs_source);
	if (fs_destination) XPL_FREE(fs_destination);
	if (fs_filemask) XPL_FREE(fs_filemask);
}

xplCommand xplFileOpCommand = { xplCmdFileOpPrologue, xplCmdFileOpEpilogue };

