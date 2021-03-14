#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libxml/xmlsave.h>
#include <libxpl/xplcore.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsave.h>
#include <libxpl/xplstart.h>

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

xmlChar *in_file = BAD_CAST "test.xml";
xmlChar *out_file = BAD_CAST "test_out.xml";
xmlChar *conf_file = NULL;
xmlChar *params_file = NULL;
xmlChar *session_file = NULL;
char *encoding = NULL;
int save_session = 0;
int verbose = 0;

static struct option option_list[] = 
{
	{
		.name = "in-file",
		.has_arg = required_argument,
		.flag = NULL,
		.val ='i'
	}, {
		.name = "out-file",
		.has_arg = required_argument,
		.flag = NULL,
		.val = 'o'
	}, {
		.name = "config-file",
		.has_arg = required_argument,
		.flag = NULL,
		.val = 'c'
	}, {
		.name = "params-file",
		.has_arg = required_argument,
		.flag = NULL,
		.val = 'p'
	}, {
		.name = "session-file",
		.has_arg = required_argument,
		.flag = NULL,
		.val = 's'
	}, {
		.name = "encoding",
		.has_arg = required_argument,
		.flag = NULL,
		.val = 'e'
	}, {
		.name = "save-session",
		.has_arg = no_argument,
		.flag = &save_session,
		.val = 1
	}, {
		.name = "verbose",
		.has_arg = no_argument,
		.flag = &verbose,
		.val = 1
	}, {
		.name = NULL,
		.has_arg = no_argument,
		.flag = NULL,
		.val = 0
	}
};

// TODO windows and file names (wchar_t command line is needed)
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
			in_file = BAD_CAST optarg;
			break;
		case 'o':
			out_file = BAD_CAST optarg;
			break;
		case 'c':
			conf_file = BAD_CAST optarg;
			break;
		case 'p':
			params_file = BAD_CAST optarg;
			break;
		case 's':
			session_file = BAD_CAST optarg;
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
	xmlChar *app_path = NULL;
	xplError err_code;
	xmlChar *error_text = NULL;
	xplDocumentPtr doc;
	xplParamsPtr env = NULL;
	xplSessionPtr session = NULL;
	xplStartParams start_params;
	int ret_code = 0;
	LEAK_DETECTION_PREPARE

	xplInitMemory(xplDefaultDebugAllocation, xplDefaultUseTcmalloc);
	parseCommandLine(argc, argv);

	app_path = BAD_CAST getcwd(NULL, 0);
	memcpy(&start_params, &xplDefaultStartParams, sizeof(xplStartParams));
	start_params.verbose = (bool) verbose;

	/* start XPL engine */
	if (!xplStartEngine((const xplStartParamsPtr) &start_params, argc, (const char**) argv, &error_text))
	{
		printf("Error starting XPL engine: %s\n", error_text);
		if (error_text)
			XPL_FREE(error_text);
		ret_code = 1;
		goto cleanup;
	}

	if (!cfgDocRoot)
		cfgDocRoot = BAD_CAST XPL_STRDUP((char*) app_path);
	xplSetGetAppTypeFunc(getAppType);

	/* create auxiliary structures */
	session = xplSessionCreateWithAutoId();
	env = xplParamsCreate();

	/* quick hack: we need to provide working parameters and session somehow.
	 * parsing arbitrary input would mean duplicating the interpreter code.
	 * the quickest way is to run a special XPL file over XML input files and reuse filled environment and session. */
	if (params_file || session_file)
	{
		xplParamAddValue(env, BAD_CAST "ParamsFile", BAD_CAST XPL_STRDUP(params_file? (char*) params_file: ""), XPL_PARAM_TYPE_USERDATA);
		xplParamAddValue(env, BAD_CAST "SessionFile", BAD_CAST XPL_STRDUP(session_file? (char*) session_file: ""), XPL_PARAM_TYPE_USERDATA);
		xplParamAddValue(env, BAD_CAST "HelperFunction", BAD_CAST XPL_STRDUP("Load"), XPL_PARAM_TYPE_USERDATA);
		err_code = xplProcessFile(app_path, HELPER_FILE, env, session, &doc);
		xplDocumentFree(doc);
		xplParamsFree(env);
		env = xplParamsCreate();
		if (err_code != XPL_ERR_NO_ERROR)
		{
			fprintf(stderr, "error: %s returned an error \"%s\" parsing session and/or parameters files\n", HELPER_FILE, xplErrorToString(err_code));
			ret_code = 2;
			goto cleanup;
		}
	}

	/* process real document */
	LEAK_DETECTION_START();
	err_code = xplProcessFileEx(app_path, in_file, env, session, &doc);
	if (doc->document && !xplSaveXmlDocToFile(doc->document, out_file, encoding, XML_SAVE_FORMAT))
	{
		fprintf(stderr, "can't save output file '%s': %s\n", out_file, strerror(errno));
		ret_code = 3;
	}
	if (err_code < XPL_ERR_NO_ERROR)
	{
		fprintf(stderr, "error processing document: %s (%s)\n", xplErrorToString(err_code), doc? doc->error: NULL);
		ret_code = 4;
	}
	if (doc)
		xplDocumentFree(doc);
	xmlResetLastError(); 
	LEAK_DETECTION_STOP_AND_REPORT();

	/* save session if requested.
	 * use the helper file again */
	if (save_session)
	{
		if (!session_file)
			fprintf(stderr, "can't save session: no session file name specified!");
		else {
			xplParamReplaceValue(env, BAD_CAST "SessionFile", BAD_CAST XPL_STRDUP((char*) session_file), XPL_PARAM_TYPE_USERDATA);
			xplParamReplaceValue(env, BAD_CAST "HelperFunction", BAD_CAST XPL_STRDUP("Save"), XPL_PARAM_TYPE_USERDATA);
			err_code = xplProcessFile(app_path, HELPER_FILE, env, session, &doc);
			xplDocumentFree(doc);
			if (err_code != XPL_ERR_NO_ERROR)
				fprintf(stderr, "error: %s returned an error \"%s\" saving session file\n", HELPER_FILE, xplErrorToString(err_code));
		}
	}

cleanup:
	if (session)
		xplSessionDeleteShared(xplSessionGetId(session));
	if (env)
		xplParamsFree(env);
	xplShutdownEngine();
	if (app_path)
		free(app_path);
	xplCleanupMemory();
	return LEAK_DETECTION_RET_CODE(ret_code);
}
