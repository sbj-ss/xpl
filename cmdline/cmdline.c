#include <getopt.h>
#include <stdio.h>
#include <libxml/xmlsave.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/xplcore.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xplsave.h>

xmlChar* getAppType(void)
{
	return BAD_CAST "console";
}

/*
	-i, --in-file infile.xpl
	-o, --out-file outfile.xml
	-c, --config-file configfile.xml
	-p, --params-file paramsfile.xml
	-s, --session-file sessionfile.xml
	-e, --encoding output encoding
	--save-session
*/

char *in_file = "test.xml";
char *out_file = "test_out.xml";
xmlChar *conf_file = NULL;
xmlChar *params_file = NULL;
xmlChar *session_file = NULL;
char *encoding = NULL;
int save_session = 0;

static struct option option_list[] = 
{
	{
		SFINIT(.name, "in-file"),
		SFINIT(.has_arg, required_argument),
		SFINIT(.flag, NULL),
		SFINIT(.val, 'i')
	},
	{
		SFINIT(.name, "out-file"),
		SFINIT(.has_arg, required_argument),
		SFINIT(.flag, NULL),
		SFINIT(.val, 'o')
	},
	{
		SFINIT(.name, "config-file"),
		SFINIT(.has_arg, required_argument),
		SFINIT(.flag, NULL),
		SFINIT(.val, 'c')
	},
	{
		SFINIT(.name, "params-file"),
		SFINIT(.has_arg, required_argument),
		SFINIT(.flag, NULL),
		SFINIT(.val, 'p')
	},
	{
		SFINIT(.name, "session-file"),
		SFINIT(.has_arg, required_argument),
		SFINIT(.flag, NULL),
		SFINIT(.val, 's')
	},
	{
		SFINIT(.name, "encoding"),
		SFINIT(.has_arg, required_argument),
		SFINIT(.flag, NULL),
		SFINIT(.val, 'e')
	},
	{
		SFINIT(.name, "save-session"),
		SFINIT(.has_arg, no_argument),
		SFINIT(.flag, &save_session),
		SFINIT(.val, 1)
	},
	{
		SFINIT(.name, NULL),
		SFINIT(.has_arg, no_argument),
		SFINIT(.flag, NULL),
		SFINIT(.val, 0)
	}
};

void parseCommandLine(int argc, char **argv)
{
	int option_index;

	while (1)
	{
		int c = getopt_long(argc, argv, "i:o:c:p:s:e:", option_list, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
		case 0:
			break; /* flag */
		case 'i':
			in_file = optarg;
			break;
		case 'o':
			out_file = optarg;
			break;
		case 'c':
			conf_file = optarg;
			break;
		case 'p':
			params_file = optarg;
			break;
		case 's':
			session_file = optarg;
			break;
		case 'e':
			encoding = optarg;
			break;
		default:
			exit(-1);
		}
	}
	if (!in_file)
	{
		fprintf(stderr, "error: no input file specified\n");
		exit(-1);
	}
	if (!out_file)
	{
		fprintf(stderr, "error: no output file specified\n");
		exit(-1);
	}
}

#define XPR_FEATURES_NEEDED (XPR_STARTSTOP_LOW_LEVEL | XPR_STARTSTOP_CONSOLE)
#define HELPER_FILE BAD_CAST "ConsoleHelper.xpl"

int main(int argc, char* argv[])
{
	xmlChar *real_conf_file, *app_path;
	xplError err_code;
	xefStartupParams xef_params;
	xmlChar *xef_error_text;

	xplDocumentPtr doc;
	xplParamsPtr env;
	xplSessionPtr session;
	LEAK_DETECTION_PREPARE

	parseCommandLine(argc, argv);
	/* start low-level layers */
#ifdef _LEAK_DETECTION
	xmlMemSetup(xmlMemFree, xmlMemMalloc, xmlMemRealloc, xmlMemoryStrdup);
#endif
	LEAK_DETECTION_START

	xprParseCommandLine();
	xprStartup(XPR_FEATURES_NEEDED);
	xmlInitParser();
    LIBXML_TEST_VERSION

	if (!xefStartup(&xef_params))
	{
		if (xef_params.error)
		{
			xef_error_text = xefGetErrorText(xef_params.error);
			xplDisplayMessage(xplMsgError, xef_error_text);
			XPL_FREE(xef_error_text);
			xefFreeErrorMessage(xef_params.error);
		} else
			xplDisplayMessage(xplMsgError, BAD_CAST "external libraries startup failed (unknown error)");
		xmlCleanupParser();
		xprShutdown(XPR_FEATURES_NEEDED);
		exit(-2);
	}

	/* figure out parameter values */
	app_path = xprGetProgramPath();
	if (conf_file)
		real_conf_file = XPL_STRDUP(conf_file);
	else {
		real_conf_file = XPL_STRDUP(app_path);
		real_conf_file = XPL_STRDUP(real_conf_file, BAD_CAST XPR_PATH_DELIM_STR"xpl.xml");
	}

	/* start XPL engine */
	err_code = xplInitParser(real_conf_file);
	if (err_code != XPL_ERR_NO_ERROR)
	{
		xplDisplayMessage(xplMsgError, BAD_CAST "error starting interpreter: \"%s\"", xplErrorToString(err_code));
		xefShutdown();
		xmlCleanupParser();
		xprShutdown(XPR_FEATURES_NEEDED);
		exit(-3);
	}
	xplSetGetAppTypeFunc(getAppType);

	/* create aux structures */
	session = xplSessionCreateWithAutoId();
	env = xplParamsCreate();

	/* quick hack: we need to provide working parameters and session somehow.
	 * parsing arbitrary input would mean duplicating the interpreter code.
	 * the quickest way is to run a special xpl file over xml input files and reuse filled environment and session. */
	if (params_file || session_file)
	{
		xplParamAddValue(env, BAD_CAST "ParamsFile", XPL_STRDUP(params_file), XPL_PARAM_TYPE_USERDATA);
		xplParamAddValue(env, BAD_CAST "SessionFile", XPL_STRDUP(session_file), XPL_PARAM_TYPE_USERDATA);
		xplParamAddValue(env, BAD_CAST "HelperFunction", XPL_STRDUP(BAD_CAST "Load"), XPL_PARAM_TYPE_USERDATA);
		err_code = xplProcessFile(app_path, HELPER_FILE, env, session, &doc);
		xplDocumentFree(doc);
		if (err_code != XPL_ERR_NO_ERROR)
		{
			fprintf(stderr, "error: %s returned an error \"%s\" parsing session and/or parameters files\n", HELPER_FILE, xplErrorToString(err_code));
			goto cleanup;
		}
	}

	/* process real document */
	LEAK_DETECTION_START
	err_code = xplProcessFile(app_path, in_file, env, session, &doc);
	if ((err_code >= XPL_ERR_NO_ERROR) && doc)
		saveXmlDocToFile(doc->document, out_file, encoding, XML_SAVE_FORMAT);
	else
		fprintf(stderr, "error processing document: %s\n", xplErrorToString(err_code));
	if (doc)
		xplDocumentFree(doc);
	xmlResetLastError(); 
	LEAK_DETECTION_STOP

	/* save session if requested.
	 * use the helper file again */
	if (save_session)
	{
		xplParamReplaceValue(env, BAD_CAST "SessionFile", XPL_STRDUP(session_file), XPL_PARAM_TYPE_USERDATA);
		xplParamReplaceValue(env, BAD_CAST "HelperFunction", XPL_STRDUP(BAD_CAST "Save"), XPL_PARAM_TYPE_USERDATA);
		err_code = xplProcessFile(app_path, HELPER_FILE, env, session, &doc);
		xplDocumentFree(doc);
		if (err_code != XPL_ERR_NO_ERROR)
			fprintf(stderr, "error: %s returned an error \"%s\" saving session file\n", HELPER_FILE, xplErrorToString(err_code));
	}

cleanup:
	xplDeleteSession(xplSessionGetId(session));
	xplParamsFree(env);
	xplDoneParser();
	xefShutdown();
	xmlCleanupParser();
	xprShutdown(XPR_FEATURES_NEEDED);
	return 0;
}

