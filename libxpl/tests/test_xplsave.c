#include "test_xplsave.h"

#include <assert.h>
#include <unistd.h>
#include <libxml/tree.h>
#include <libxml/xmlstring.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <libxpl/xplsave.h>

#define FAIL(x) do {\
	ctxt->error = BAD_CAST x;\
	goto cleanup;\
} while(0);

static xmlDocPtr _createTestDoc()
{
	const char doc_src[] =
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<!DOCTYPE test ["
			"<!ENTITY ent_a \"entity_a\">"
			"<!ENTITY ent_b \"&ent_a;entity_b\">"
		"]>"
		"<Root xmlns:foo=\"http://foo.com/foo\">"
			"<foo:a foo:attr=\"foo-attr\"/>"
			"<b>1&lt;2</b>"
			"<c>&ent_b;</c>"
		"</Root>";
	xmlDocPtr doc;

	doc = xmlReadMemory(doc_src, strlen(doc_src), NULL, "utf-8", 0);
	return(doc);
}

//static void xtsTestSave

#define NODE_A_CODE "<foo:a xmlns:foo=\"http://foo.com/foo\" xmlns:bar=\"http://foo.com/bar\" attr-a=\"value a\" bar:attr-b=\"value&quot; b\"/>"

static xmlNodePtr _buildNodeA()
{
	xmlNodePtr node;
	xmlNsPtr ns_foo, ns_bar;
	xmlAttrPtr prop;

	node = xmlNewNode(NULL, BAD_CAST "a");
	assert(node);
	ns_foo = xmlNewNs(node, BAD_CAST "http://foo.com/foo", BAD_CAST "foo");
	assert(ns_foo);
	xmlSetNs(node, ns_foo);
	ns_bar = xmlNewNs(node, BAD_CAST "http://foo.com/bar", BAD_CAST "bar");
	assert(ns_bar);
	prop = xmlNewProp(node, BAD_CAST "attr-a", BAD_CAST "value a");
	assert(prop);
	prop = xmlNewNsProp(node, ns_bar, BAD_CAST "attr-b", BAD_CAST "value\" b");
	assert(prop);
	return node;
}

#define NODE_B_CODE "<b>test</b>"

static xmlNodePtr _buildNodeB()
{
	xmlNodePtr node, content;

	node = xmlNewNode(NULL, BAD_CAST "b");
	assert(node);
	content = xmlNewText(BAD_CAST "test");
	assert(content);
	xmlAddChild(node, content);
	return node;
}

static bool _testSerializeNodeList(xtsContextPtr ctxt)
{
	xmlNodePtr node_a, node_b;
	xmlChar *result;
	bool ok = false;

	node_a = _buildNodeA();
	node_b = _buildNodeB();
	xmlAddSibling(node_a, node_b);
	result = xplSerializeNodeList(node_a);
	if (!result)
		FAIL("serializeNodeList() returned NULL");
	if (xmlStrcmp(result, BAD_CAST NODE_A_CODE NODE_B_CODE))
		FAIL("incorrect serialization result");
	ok = true;
cleanup:
	if (node_a)
		xmlFreeNodeList(node_a);
	if (result)
		XPL_FREE(result);
	return ok;
}

static bool _testSerializeNodeSet(xtsContextPtr ctxt)
{
	xmlNodePtr node_a, node_b;
	xmlNodeSetPtr set;
	xmlChar *result;
	bool ok = false;

	node_a = _buildNodeA();
	node_b = _buildNodeB();
	set = xmlXPathNodeSetCreate(NULL);
	assert(set);
	xmlXPathNodeSetAdd(set, node_a);
	xmlXPathNodeSetAdd(set, node_b);
	result = xplSerializeNodeSet(set);
	if (!result)
		FAIL("serializeNodeSet() returned NULL");
	if (xmlStrcmp(result, BAD_CAST NODE_A_CODE NODE_B_CODE))
		FAIL("incorrect serialization result");
	ok = true;
cleanup:
	if (node_a)
		xmlFreeNode(node_a);
	if (node_b)
		xmlFreeNode(node_b);
	if (set)
		xmlXPathFreeNodeSet(set);
	if (result)
		XPL_FREE(result);
	return ok;
}

static bool _testSerializeNodeXXWitNullInput(xtsContextPtr ctxt)
{
	bool ok = false;
	xmlChar *result;

	result = xplSerializeNodeList(NULL);
	if (result)
		FAIL("serializeNodeList() must return NULL on NULL input");
	result = xplSerializeNodeSet(NULL);
	if (result)
		FAIL("serializeNodeSet() must return NULL on NULL input");
	ok = true;
cleanup:
	/* if result != NULL there's probably some rubbish and freeing it is dangerous */
	return ok;
}

/*static char temp_tmpl[] = "xts_save_XXXXXX";*/

static bool _saveFixtureSetup(xtsContextPtr ctxt)
{
	xmlDocPtr doc;

/*	if (!mkdtemp(temp_tmpl))
		return false;
	xmlHashAddEntry(ctxt->env, BAD_CAST "dir", temp_tmpl);*/
	if (!(doc = _createTestDoc()))
		return false;
	xmlHashAddEntry(ctxt->env, BAD_CAST "doc", doc);
	return true;
}

static void _saveFixtureTeardown(xtsContextPtr ctxt)
{
	xmlDocPtr doc;
	char *dir;

	if ((doc = (xmlDocPtr) xmlHashLookup(ctxt->env, BAD_CAST "doc")))
		xmlFreeDoc(doc);
	if ((dir = (char*) xmlHashLookup(ctxt->env, BAD_CAST "dir")))
		rmdir(dir);
}

static xtsTest save_tests[] =
{
	{
		.id = BAD_CAST "serialize_node_list",
		.displayName = BAD_CAST "Node list serialization",
		.testFunction = _testSerializeNodeList,
		.flags = XTS_FLAG_CHECK_MEMORY
	}, 	{
		.id = BAD_CAST "serialize_node_set",
		.displayName = BAD_CAST "Node set serialization",
		.testFunction = _testSerializeNodeSet,
		.flags = XTS_FLAG_CHECK_MEMORY
	}, 	{
		.id = BAD_CAST "serialize_null_input",
		.displayName = BAD_CAST "Node list/set serialization with NULL input",
		.testFunction = _testSerializeNodeXXWitNullInput,
		.flags = XTS_FLAG_CHECK_MEMORY
	}
};

xtsFixture xtsTestSaveFixture =
{
	.id = BAD_CAST "save",
	.displayName = BAD_CAST "saving/serialization test group",
	.setup = _saveFixtureSetup,
	.teardown = _saveFixtureTeardown,
	.test_count = sizeof(save_tests) / sizeof(save_tests[0]),
	.tests = save_tests
};
