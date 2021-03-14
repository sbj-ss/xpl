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
#include <libxpl/xplversion.h>

static xmlChar* _getAppType(void)
{
	return BAD_CAST "console";
}

#define XPR_FEATURES_NEEDED (XPR_STARTSTOP_LOW_LEVEL | XPR_STARTSTOP_CONSOLE)

xmlChar *in_file = BAD_CAST "test.xml";
xmlChar *out_file = BAD_CAST "test_out.xml";
xmlChar *start_file = NULL;
char *out_encoding = NULL;
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
		.name = "start-file",
		.has_arg = required_argument,
		.flag = NULL,
		.val = 's'
	}, {
		.name = "out-encoding",
		.has_arg = required_argument,
		.flag = NULL,
		.val = 'e'
	}, {
		.name = "verbose",
		.has_arg = no_argument,
		.flag = &verbose,
		.val = 1
	}, {
		.name = "help",
		.has_arg = no_argument,
		.flag = NULL,
		.val = 'h'
	}, {
		.name = "version",
		.has_arg = no_argument,
		.flag = NULL,
		.val = 'V'
	}, {
		.name = NULL,
		.has_arg = no_argument,
		.flag = NULL,
		.val = 0
	}
};

static void _printUsage(void)
{
	fprintf(stderr, "XPL batch interpreter.\n");
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "xpl -h/--help\n");
	fprintf(stderr, "xpl -V/--version\n");
	fprintf(stderr, "xpl [options]\n");
	fprintf(stderr, "\t-i/--in-file INFILE (default: test.xpl)\n");
	fprintf(stderr, "\t-o/--out-file OUTFILE (default: test_out.xml)\n");
	fprintf(stderr, "\t-e/--out-encoding ENCODING\n");
	fprintf(stderr, "\t-v/--verbose\n\n");
}

static void _printVersion(void)
{
	xmlChar *libs, *xef;

	fprintf(stderr, "%s\n", XPL_VERSION_FULL);
	xplInitLibraryVersions();
	libs = xplLibraryVersionsToString(xplGetCompiledLibraryVersions(), xplGetRunningLibraryVersions());
	xef = xplXefImplementationsToString(xplGetXefImplementations());
	fprintf(stderr, "Libraries:\n%sXEF:\n%s", libs, xef);
	XPL_FREE(libs);
	XPL_FREE(xef);
	xplCleanupLibraryVersions();
}

typedef enum _ExitCode
{
	EC_OK = EXIT_SUCCESS,
	EC_BAD_OPTION = 1,
	EC_CANNOT_START_ENGINE = 2,
	EC_BAD_START_FILE = 3,
	EC_CANNOT_SAVE_OUTPUT = 4,
	EC_PROCESSING_FAILED = 5
} ExitCode;

// TODO windows and file names (wchar_t command line is needed)
static void _parseCommandLine(int argc, char **argv)
{
	int option_index;

	while (1)
	{
		int c = getopt_long(argc, argv, "i:o:s:e:vhV", option_list, &option_index);
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
		case 's':
			start_file = BAD_CAST optarg;
			break;
		case 'e':
			out_encoding = optarg;
			break;
		case 'v':
			verbose = true;
			break;
		case 'h':
			_printUsage();
			exit(EXIT_SUCCESS);
		case 'V':
			_printVersion();
			exit(EXIT_SUCCESS);
		default:
			_printUsage();
			exit(EC_BAD_OPTION);
		}
	}
}

int main(int argc, char* argv[])
{
	xmlChar *app_path = NULL;
	xplError doc_result;
	xmlChar *error_text = NULL;
	xplDocumentPtr doc;
	xplParamsPtr params = NULL;
	xplSessionPtr session = NULL;
	xplStartParams start_params;
	int exit_code = EC_OK;
	LEAK_DETECTION_PREPARE

	xplInitMemory(xplDefaultDebugAllocation, xplDefaultUseTcmalloc);
	_parseCommandLine(argc, argv);

	app_path = BAD_CAST getcwd(NULL, 0);
	memcpy(&start_params, &xplDefaultStartParams, sizeof(xplStartParams));
	start_params.verbose = (bool) verbose;

	/* start XPL engine */
	if (!xplStartEngine((const xplStartParamsPtr) &start_params, argc, (const char**) argv, &error_text))
	{
		printf("Error starting XPL engine: %s\n", error_text);
		if (error_text)
			XPL_FREE(error_text);
		exit_code = EC_CANNOT_START_ENGINE;
		goto cleanup;
	}

	if (!cfgDocRoot)
		cfgDocRoot = BAD_CAST XPL_STRDUP((char*) app_path);
	xplSetGetAppTypeFunc(_getAppType);

	/* process startup file if specified */
	if (start_file)
	{
		doc_result = xplProcessStartupFile(app_path, start_file);
		if (doc_result != XPL_ERR_NO_ERROR)
		{
			fprintf(stderr, "Failed to process start file %s%c%s: %s\n", app_path, XPR_PATH_DELIM, in_file, xplErrorToString(doc_result));
			exit_code = EC_BAD_START_FILE;
			goto cleanup;
		}
		xmlResetLastError();
	}

	LEAK_DETECTION_START();
	session = xplSessionCreateLocal();
	params = xplParamsCreate();
	doc_result = xplProcessFileEx(app_path, in_file, params, session, &doc);
	/* save document even if processing failed */
	if (doc->document && !xplSaveXmlDocToFile(doc->document, out_file, out_encoding, XML_SAVE_FORMAT))
	{
		fprintf(stderr, "can't save output file '%s': %s\n", out_file, strerror(errno));
		exit_code = EC_CANNOT_SAVE_OUTPUT;
	}
	if (doc_result < XPL_ERR_NO_ERROR)
	{
		fprintf(stderr, "error processing document: %s (%s)\n", xplErrorToString(doc_result), doc? doc->error: NULL);
		exit_code = EC_PROCESSING_FAILED;
	}
	if (doc)
		xplDocumentFree(doc);
cleanup:
	if (session)
		xplSessionFreeLocal(session);
	if (params)
		xplParamsFree(params);
	xmlResetLastError();
	if (exit_code == EC_OK)
		LEAK_DETECTION_STOP_AND_REPORT();
	xplShutdownEngine();
	if (app_path)
		free(app_path);
	xplCleanupMemory();
	return LEAK_DETECTION_RET_CODE(exit_code);
}
