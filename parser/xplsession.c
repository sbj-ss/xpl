#include <stdlib.h>
#include <openssl/ripemd.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplutils.h>

/* inner singletons */
static xmlHashTablePtr session_mgr = NULL;
static time_t max_session_lifetime;
XPR_MUTEX session_interlock;

/* xplSession internals */
typedef struct _xplSession 
{
	xmlHashTablePtr items;
	xmlDocPtr doc;
	time_t init_ts;
	xmlChar* id;
	bool valid;
	bool sa_mode;
	bool just_created;
	XPR_MUTEX locker;
} xplSession;

/* start/stop */ /* TODO bool ret */
int xplSessionManagerInit(time_t max_lifetime)
{
	if (!session_mgr)
		session_mgr = xmlHashCreate(16);
	if (!session_mgr)
		return -1;
	max_session_lifetime = max_lifetime;
	if (!xprMutexInit(&session_interlock))
	{
		xmlHashFree(session_mgr, NULL);
		return -1;
	}
	return 0;
}

static void freeObjectCallback(void *payload, xmlChar *name)
{
	xmlUnlinkNode((xmlNodePtr) payload);
	xmlFreeNode((xmlNodePtr) payload);
}

static void xplSessionFree(xplSessionPtr session)
{
	if (session)
	{
		/* we don't specify a deallocator here because items are doc children */
		xmlHashFree(session->items, NULL);
		xmlFreeDoc(session->doc);
		xmlFree(session->id);
		if (!xprMutexCleanup(&session->locker))
			DISPLAY_INTERNAL_ERROR_MESSAGE();
		xmlFree(session);
	}
}

static void freeSessionCallback(void *payload, xmlChar *name)
{
	xplSessionFree((xplSessionPtr) payload);
}

void xplSessionManagerCleanup()
{
	if (!session_mgr)
		return;
	if (!xprMutexAcquire(&session_interlock)) /* TODO what should we do here? */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	xmlHashFree(session_mgr, freeSessionCallback);
	session_mgr = NULL;
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (!xprMutexCleanup(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

/* manager-level */
static xplSessionPtr xplSessionCreateInner(const xmlChar *id)
{
	xplSessionPtr ret;
	xmlNodePtr root;

	ret = (xplSessionPtr) xmlMalloc(sizeof(xplSession));
	if (!ret)
		return NULL;
	memset(ret, 0, sizeof(xplSession));
	if (!xprMutexInit(&ret->locker))
	{
		xmlFree(ret);
		return NULL;
	}
	ret->items = xmlHashCreate(16);
	ret->doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewDocNode(ret->doc, NULL, BAD_CAST "Root", NULL);
	(void) xmlNewProp(root, BAD_CAST "id", id);
	ret->doc->children = root;
	time(&ret->init_ts);
	ret->id = xmlStrdup(id);
	ret->valid = true;
	ret->just_created = true;
	xmlHashAddEntry(session_mgr, id, (void*) ret);
	return ret;
}

static xplSessionPtr xplSessionLookupInternal(const xmlChar *id)
{
	xplSessionPtr ret;

	ret = (xplSessionPtr) xmlHashLookup(session_mgr, id);
	if (ret)
	{
		/* don't return an expired session */
		if (time(NULL) - ret->init_ts > max_session_lifetime)
		{
			xmlHashRemoveEntry(session_mgr, id, freeSessionCallback);
			ret = NULL;
		}
	}
	return ret;
}

xplSessionPtr xplSessionCreate(const xmlChar *id)
{
	xplSessionPtr s;

	if (!session_mgr)
		return NULL;
	if (!xprMutexAcquire(&session_interlock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	s = xplSessionLookupInternal(id);
	if (!s)
		s = xplSessionCreateInner(id);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return s;
}

xplSessionPtr xplSessionCreateWithAutoId()
{
	xmlChar id[XPL_SESSION_ID_SIZE + 1];
	bool flag = true;
	xplSessionPtr ret;

	if (!session_mgr)
		return NULL;
	if (!xprMutexAcquire(&session_interlock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	while (flag)
	{
		snprintf((char*) id, sizeof(id), "%08X", (unsigned int) lrand48());
		flag = (xplSessionLookupInternal(id) != NULL);
	}
	ret = xplSessionCreateInner(id);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

xplSessionPtr xplSessionLookup(const xmlChar *id)
{
	xplSessionPtr ret;

	if (!session_mgr)
		return NULL;
	if (!xprMutexAcquire(&session_interlock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	ret = (xplSessionPtr) xplSessionLookupInternal(id);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

void xplDeleteSession(const xmlChar *id)
{
	if (!session_mgr)
		return;
	if (!xprMutexAcquire(&session_interlock)) /* TODO what should we do here? */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	xmlHashRemoveEntry(session_mgr, id, freeSessionCallback);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

void enumStaleSessionsCallback(void *payload, void *data, xmlChar *name)
{
	xplSessionPtr s = (xplSessionPtr) payload;
	if (time(NULL) - s->init_ts > max_session_lifetime)
		xmlHashRemoveEntry(session_mgr, name, freeSessionCallback);
}

void xplCleanupStaleSessions()
{
	if (!session_mgr)
		return;
	if (!xprMutexAcquire(&session_interlock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return;
	}
	xmlHashScan(session_mgr, enumStaleSessionsCallback, NULL);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

/* session-level */
int xplSessionSetObject(xplSessionPtr session, const xmlNodePtr cur, const xmlChar *name)
{
	xmlNodePtr prev;
	int ret;
	xmlNodePtr new_parent;

	if (!session)
		return -1;
	if (!xprMutexAcquire(&session->locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return -1;
	}
	new_parent = xmlNewDocNode(session->doc, NULL, name, NULL);
	setChildren(new_parent, cur->children);
	setChildren(cur, new_parent);
	makeNsIndepTree(new_parent);
	setChildren(cur, NULL);
	prev = new_parent->children;
	while (prev)
	{
		xmlSetTreeDoc(prev, session->doc);
		prev = prev->next;
	}
	xmlAddChild(session->doc->children, new_parent);
	prev = (xmlNodePtr) xmlHashLookup(session->items, name);
	if (prev)
	{
		xmlUnlinkNode(prev);
		xmlFreeNode(prev);
		ret = xmlHashUpdateEntry(session->items, name, (void*) new_parent, NULL);
	} else
		ret = xmlHashAddEntry(session->items, name, (void*) new_parent);
	time(&session->init_ts);
	session->valid = 1;
	if (!xprMutexRelease(&session->locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

xmlNodePtr xplSessionGetObject(const xplSessionPtr session, const xmlChar *name)
{
	xmlNodePtr carrier = NULL;

	if (!session)
		return NULL;
	if (!xprMutexAcquire(&session->locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	if (session->valid)
	{
		carrier = (xmlNodePtr) xmlHashLookup(session->items, name);
		time(&session->init_ts);
	}
	if (!xprMutexRelease(&session->locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return carrier;
}

xmlNodePtr xplSessionGetAllObjects(const xplSessionPtr session)
{
	xmlNodePtr ret;

	if (!session)
		return NULL;
	if (!xprMutexAcquire(&session->locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	time(&session->init_ts);
	ret = session->doc->children->children;
	if (!xprMutexRelease(&session->locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

void xplSessionRemoveObject(xplSessionPtr session, const xmlChar *name)
{
	if (!session)
		return;
	if (!xprMutexAcquire(&session->locker)) /* TODO what should we do here? */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (xmlHashLookup(session->items, name))
		xmlHashRemoveEntry(session->items, name, freeObjectCallback);
	time(&session->init_ts);
	if (!xprMutexRelease(&session->locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

void xplSessionClear(xplSessionPtr session)
{
	if (!session)
		return;
	if (!xprMutexAcquire(&session->locker)) /* TODO what should we do here? */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	xmlHashFree(session->items, freeObjectCallback);
	time(&session->init_ts);
	session->items = xmlHashCreate(16);
	session->valid = 0;
	if (!xprMutexRelease(&session->locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

xmlChar *xplSessionGetId(xplSessionPtr session)
{
	if (!session)
		return NULL;
	return session->id;
}

bool xplSessionIsValid(xplSessionPtr session)
{
	if (!session)
		return false;
	return session->valid;
}

bool xplSessionGetSaMode(xplSessionPtr session)
{
	if (!cfgCheckSAMode)
		return true;
	if (!session)
		return false;
	return session->sa_mode;
}

bool xplSessionSetSaMode(xplSessionPtr session, bool enable, xmlChar *password)
{
	unsigned char* digest;
	xmlChar* digest_str;
	bool ret;

	if (!session)
		return false;
	if (!enable)
	{
		session->sa_mode = false;
		return true;
	}
	if (session->sa_mode)
		return true; /* already set */
	digest = RIPEMD160((const unsigned char*) password, xmlStrlen(password), NULL);
	digest_str = bufferToHex(digest, RIPEMD160_DIGEST_LENGTH, false);
	if (!strcmp((const char*) cfgSaPassword, (const char*) digest_str))
	{
		session->sa_mode = true;
		ret = true;
	} else
		ret = false;
	xmlFree(digest_str);
	return ret;
}

bool xplSessionIsJustCreated(xplSessionPtr session)
{
	if (!session)
		return false;
	return session->just_created;
}

void xplMarkSessionAsSeen(xplSessionPtr session)
{
	if (session)
		session->just_created = false;
}
