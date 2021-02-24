#include <stdlib.h>
#include <string.h>
#include <libxpl/abstraction/xef.h>
#include <libxpl/abstraction/xpr.h>
#include <libxpl/xplmessages.h>
#include <libxpl/xploptions.h>
#include <libxpl/xplsession.h>
#include <libxpl/xplstring.h>
#include <libxpl/xpltree.h>

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
	bool local;
	XPR_MUTEX locker;
} xplSession;

/* start/stop */
bool xplSessionManagerInit(time_t max_lifetime)
{
	if (!session_mgr)
		session_mgr = xmlHashCreate(16);
	if (!session_mgr)
		return false;
	max_session_lifetime = max_lifetime;
	if (!xprMutexInit(&session_interlock))
	{
		xmlHashFree(session_mgr, NULL);
		return false;
	}
	return true;
}

static void _freeObjectCallback(void *payload, XML_HCBNC xmlChar *name)
{
	xmlNodePtr node = (xmlNodePtr) payload;
	UNUSED_PARAM(name);

	xmlUnlinkNode(node);
	xmlFreeNode(node);
}

static void _sessionFree(xplSessionPtr session)
{
	if (session)
	{
		/* we don't specify a deallocator here because items are doc children */
		xmlHashFree(session->items, NULL);
		xmlFreeDoc(session->doc);
		if (session->id)
			XPL_FREE(session->id);
		if (!session->local)
			if (!xprMutexCleanup(&session->locker))
				DISPLAY_INTERNAL_ERROR_MESSAGE();
		XPL_FREE(session);
	}
}

static void _freeSessionCallback(void *payload, XML_HCBNC xmlChar *name)
{
	xplSessionPtr session = (xplSessionPtr) payload;
	UNUSED_PARAM(name);

	_sessionFree(session);
}

void xplSessionManagerCleanup()
{
	if (!session_mgr)
		return;
	if (!xprMutexAcquire(&session_interlock)) /* TODO what should we do here? */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	xmlHashFree(session_mgr, _freeSessionCallback);
	session_mgr = NULL;
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (!xprMutexCleanup(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

/* manager-level */
static xplSessionPtr _sessionCreateInternal(xmlChar *id, bool local)
{
	xplSessionPtr ret;
	xmlNodePtr root;

	ret = (xplSessionPtr) XPL_MALLOC(sizeof(xplSession));
	if (!ret)
		return NULL;
	memset(ret, 0, sizeof(xplSession));
	if (!local && !xprMutexInit(&ret->locker))
	{
		XPL_FREE(ret);
		return NULL;
	}
	ret->items = xmlHashCreate(16);
	ret->doc = xmlNewDoc(BAD_CAST "1.0");
	root = xmlNewDocNode(ret->doc, NULL, BAD_CAST "Root", NULL);
	if (id)
		(void) xmlNewProp(root, BAD_CAST "id", id);
	xplSetChildren((xmlNodePtr) ret->doc, root);
	time(&ret->init_ts);
	ret->id = id;
	ret->valid = true;
	ret->just_created = true;
	ret->local = local;
	if (!local)
		xmlHashAddEntry(session_mgr, id, (void*) ret);
	return ret;
}

static xplSessionPtr _sessionLookupInternal(const xmlChar *id) // eats id
{
	xplSessionPtr ret;

	ret = (xplSessionPtr) xmlHashLookup(session_mgr, id);
	if (ret)
	{
		/* don't return an expired session */
		if (time(NULL) - ret->init_ts > max_session_lifetime)
		{
			xmlHashRemoveEntry(session_mgr, id, _freeSessionCallback);
			ret = NULL;
		}
	}
	return ret;
}

xplSessionPtr xplSessionCreateOrGetShared(const xmlChar *id)
{
	xplSessionPtr ret;
	xmlChar *id_copy;

	if (!session_mgr)
		return NULL;
	if (!xprMutexAcquire(&session_interlock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	if (!(ret = _sessionLookupInternal(id)))
	{
		id_copy = id? BAD_CAST XPL_STRDUP((char*) id): NULL;
		ret = _sessionCreateInternal(id_copy, false);
	}
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

xplSessionPtr xplSessionCreateWithAutoId()
{
	xmlChar *id;
	unsigned char raw_id[XPL_SESSION_ID_SIZE];
	bool flag = true;
	xplSessionPtr ret;
	xefCryptoRandomParams rp;

	if (!session_mgr)
		return NULL;
	if (!xprMutexAcquire(&session_interlock))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	rp.secure = true;
	rp.alloc_bytes = false;
	rp.size = XPL_SESSION_ID_SIZE;
	rp.bytes = raw_id;
	while (flag)
	{
		/* We don't ask for allocation so the only reason for xefCryptoRandom() to fail
		   is lack of entropy. Keep retrying until the function succeeds. */
		while (!xefCryptoRandom(&rp))
		{
			if (rp.error)
				XPL_FREE(rp.error);
			xprSleep(10);
		}
		id = xstrBufferToHex(rp.bytes, rp.size, false);
		if ((flag = !!_sessionLookupInternal(id)))
			XPL_FREE(id);
	}
	ret = _sessionCreateInternal(id, false);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

xplSessionPtr xplSessionCreateLocal()
{
	return _sessionCreateInternal(NULL, true);
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
	ret = (xplSessionPtr) _sessionLookupInternal(id);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

void xplSessionFreeLocal(xplSessionPtr session)
{
	if (!session->local)
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return;
	}
	_sessionFree(session);
}

void xplSessionDeleteShared(const xmlChar *id)
{
	if (!session_mgr)
		return;
	if (!xprMutexAcquire(&session_interlock)) /* TODO what should we do here? */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	xmlHashRemoveEntry(session_mgr, id, _freeSessionCallback);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

static void _enumStaleSessionsCallback(void *payload, void *data, XML_HCBNC xmlChar *name)
{
	xplSessionPtr s = (xplSessionPtr) payload;

	UNUSED_PARAM(name);
	UNUSED_PARAM(data);
	if (time(NULL) - s->init_ts > max_session_lifetime)
		xmlHashRemoveEntry(session_mgr, name, _freeSessionCallback);
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
	xmlHashScan(session_mgr, _enumStaleSessionsCallback, NULL);
	if (!xprMutexRelease(&session_interlock))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

/* session-level */
bool xplSessionSetObject(xplSessionPtr session, const xmlNodePtr cur, const xmlChar *name)
{
	xmlNodePtr prev;
	int ret;
	xmlNodePtr new_parent;

	if (!session)
		return false;
	if (!xprMutexAcquire(&session->locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	new_parent = xmlNewDocNode(session->doc, NULL, name, NULL);
	xplSetChildren(new_parent, cur->children);
	xplSetChildren(cur, new_parent);
	xplMakeNsSelfContainedTree(new_parent);
	xplSetChildren(cur, NULL);
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
		ret = !xmlHashUpdateEntry(session->items, name, (void*) new_parent, NULL);
	} else
		ret = !xmlHashAddEntry(session->items, name, (void*) new_parent);
	time(&session->init_ts);
	session->valid = true;
	if (!xprMutexRelease(&session->locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	return ret;
}

xmlNodePtr xplSessionGetObject(const xplSessionPtr session, const xmlChar *name)
{
	if (!session || !session->valid)
		return NULL;
	time(&session->init_ts);
	return (xmlNodePtr) xmlHashLookup(session->items, name);
}

xmlNodePtr xplSessionAccessObject(const xplSessionPtr session, const xmlChar *name)
{
	if (!session || !session->valid)
		return NULL;
	if (!xprMutexAcquire(&session->locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	return xplSessionGetObject(session, name);
}

xmlNodePtr xplSessionGetAllObjects(const xplSessionPtr session)
{
	if (!session || !session->valid)
		return NULL;
	time(&session->init_ts);
	return session->doc->children;
}

xmlNodePtr xplSessionAccessAllObjects(const xplSessionPtr session)
{
	if (!session || !session->valid)
		return NULL;
	if (!xprMutexAcquire(&session->locker))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return NULL;
	}
	return xplSessionGetAllObjects(session);
}

void xplSessionUnaccess(const xplSessionPtr session)
{
	if (session && session->valid)
		if (!xprMutexRelease(&session->locker))
			DISPLAY_INTERNAL_ERROR_MESSAGE();
}

void xplSessionRemoveObject(xplSessionPtr session, const xmlChar *name)
{
	if (!session)
		return;
	if (!xprMutexAcquire(&session->locker)) /* TODO what should we do here? */
		DISPLAY_INTERNAL_ERROR_MESSAGE();
	if (xmlHashLookup(session->items, name))
		xmlHashRemoveEntry(session->items, name, _freeObjectCallback);
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
	xmlHashFree(session->items, _freeObjectCallback);
	time(&session->init_ts);
	session->items = xmlHashCreate(16);
	session->valid = false;
	if (!xprMutexRelease(&session->locker))
		DISPLAY_INTERNAL_ERROR_MESSAGE();
}

xmlChar* xplSessionGetId(xplSessionPtr session)
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
	xmlChar* digest_str;
	bool ret;
	xefCryptoDigestParams dp;

	if (!session)
		return false;
	if (!enable)
	{
		session->sa_mode = false;
		return true;
	}
	if (session->sa_mode)
		return true; /* already set */
	if ((!cfgSaPassword || !*cfgSaPassword) && (!password || !*password))
	{
		session->sa_mode = true;
		return true;
	}
	dp.input = password;
	dp.input_size = xmlStrlen(password);
	dp.digest_method = XEF_CRYPTO_DIGEST_METHOD_RIPEMD160;
	if (!xefCryptoDigest(&dp))
	{
		DISPLAY_INTERNAL_ERROR_MESSAGE();
		return false;
	}
	digest_str = xstrBufferToHex(dp.digest, dp.digest_size, false);
	if (!strcmp((const char*) cfgSaPassword, (const char*) digest_str))
	{
		session->sa_mode = true;
		ret = true;
	} else
		ret = false;
	XPL_FREE(dp.digest);
	XPL_FREE(digest_str);
	return ret;
}

bool xplSessionIsJustCreated(xplSessionPtr session)
{
	if (!session)
		return false;
	return session->just_created;
}

void xplSessionMarkAsSeen(xplSessionPtr session)
{
	if (session)
		session->just_created = false;
}
