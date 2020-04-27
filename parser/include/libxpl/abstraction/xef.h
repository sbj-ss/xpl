/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xef_H
#define __xef_H

#include "Configuration.h"
#include "Common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Все сторонние химеры и слонопотамы в том случае, когда можно использовать несколько
 * вариантов (для лавирования между багами, учёта зависимости от платформы и т.д.),
 * собираются здесь в единый зоопарк. Что именно используется, задаётся в Configuration.h.
 * Специфические для внешних фич заголовочники сюда попадать не должны. */


/* == Общая часть == */

/* Сообщения об ошибках. Структура непрозрачная, т.к. у каждой сторонней 
 * библиотеки свой серпентарий в головах и форматы этих сообщений. Из полученного сообщения
 * программисту интерпретатора надлежит извлечь текст вызовом xefGetErrorText, а само сообщение
 * прихлопнуть вызовом xefFreeErrorMessage(), не делая никаких предположений о его происхождении и начинке. */

/* Сама структура и указатель на неё */
typedef struct _xefErrorMessage xefErrorMessage, *xefErrorMessagePtr;
/* Возвращает текст с желательной контекстной детализацией. Результат необходимо освободить. */
XPLPUBFUN xmlChar* XPLCALL 
	xefGetErrorText(xefErrorMessagePtr msg);
/* Освобождает связанные с сообщением об ошибке ресурса. */
XPLPUBFUN void XPLCALL
	xefFreeErrorMessage(xefErrorMessagePtr msg);

/* Запуск и завершение внешней сволочи. Интерпретатор автоматически это не вызывает. Функции НЕ потокобезопасны. */
typedef struct _xefStartupParams
{
	/* Выход */
	xefErrorMessagePtr error;		/* сообщение об ошибке. если не NULL - нужно освободить. */
} xefStartupParams, *xefStartupParamsPtr;

XPLPUBFUN bool XPLCALL
	xefStartup(xefStartupParamsPtr params);
XPLPUBFUN bool XPLCALL
	xefIsStarted(void);
XPLPUBFUN void XPLCALL
	xefShutdown(void);


/* == Регулярные выражения == */
/* до лучших времён */

/* == xTP == */
typedef struct _xefFetchDocumentParams
{
	/* Входные параметры - заполняются перед вызовом */
	xmlChar *uri;				/* URI документа */
	xmlChar *extra_query;		/* в частности, POST в HTTP */
	/* Результаты */
	xmlChar *encoding;			/* кодировка. освобождать надо */
	xmlChar *document;			/* тело. освобождать надо. нули внутри допустимы */
	xmlChar *real_uri;			/* может быть пустым. непустой нужно освободить */
	int status_code;			/* может быть не определён */
	size_t document_size;		/* длина в байтах. для строки учитывается терминирующий нуль. */
	xefErrorMessagePtr error;	/* сообщение об ошибке. если не NULL - нужно освободить. */
} xefFetchDocumentParams, *xefFetchDocumentParamsPtr;

XPLPUBFUN bool XPLCALL
	xefFetchDocument(xefFetchDocumentParamsPtr params);
/* Очищает только выходные параметры */
XPLPUBFUN void XPLCALL
	xefFetchParamsClear(xefFetchDocumentParamsPtr params);


/* == Базы данных == */
/* Очевидно, пора - старый код ужасен. 
   Здесь имеет смысл частично вернуться к "модели" - абстракции, предложенной Ф. 
 */

/* сначала низкоуровневый код - сам движок использует базы данных.
   рано или поздно это уйдёт в командно-специфическую секцию, но не сейчас. */
XPLPUBFUN void XPLCALL /* нужно ли это снаружи? */
	xefDbDeallocateDb(void *db_handle);
XPLPUBFUN bool XPLCALL
	xefDbCheckAvail(const xmlChar* connString, const xmlChar *name, xmlChar **msg);

typedef enum _xefDbStreamType 
{
	XEF_DB_STREAM_TDS,
	XEF_DB_STREAM_XML
} xefDbStreamType;

typedef struct _xefDbRowDesc 
{
	size_t count;
	xmlChar **names;
	void **db_objects;
} xefDbRowDesc, *xefDbRowDescPtr;

typedef struct _xefDbField 
{
	bool is_null;
	xmlChar *value;
	size_t value_size; /* в пересчёте на байты, включая нуль-терминатор */
	bool needs_copy;
} xefDbField, *xefDbFieldPtr;

typedef struct _xefDbRow 
{
	xefDbFieldPtr fields;
} xefDbRow, *xefDbRowPtr;

/* скроем реализацию */
typedef struct xefDbContext *xefDbContextPtr;

typedef bool (*xefDbGetRowCallback)(xefDbRowDescPtr desc, xefDbRowPtr row, void *userData);

typedef struct _xplDBList xplDBList, *xplDBListPtr;

typedef struct _xefDbQueryParams 
{
	/* input */
	xefDbStreamType stream_type;
	bool cleanup_nonprintable;
	xmlChar *query;
	xplDBListPtr db_list;
	void *user_data;
	/* output */
	xefErrorMessagePtr error;
} xefDbQueryParams, *xefDbQueryParamsPtr;

XPLPUBFUN xefDbRowDescPtr XPLCALL
	xefDbGetRowDesc(xefDbContextPtr ctxt);
XPLPUBFUN xefDbRowPtr XPLCALL
	xefDbGetRow(xefDbContextPtr ctxt);
XPLPUBFUN xefErrorMessagePtr XPLCALL
	xefDbGetError(xefDbContextPtr ctxt);
XPLPUBFUN void* XPLCALL
	xefDbGetUserData(xefDbContextPtr ctxt);
XPLPUBFUN bool XPLCALL
	xefDbHasRecordset(xefDbContextPtr ctxt);

XPLPUBFUN xefDbContextPtr XPLCALL
	xefDbQuery(xefDbQueryParamsPtr params);
XPLPUBFUN bool XPLCALL
	xefDbNextRowset(xefDbContextPtr ctxt);
XPLPUBFUN void XPLCALL 
	xefDbEnumRows(xefDbContextPtr ctxt, xefDbGetRowCallback cb);
XPLPUBFUN xmlChar* XPLCALL
	xefDbAccessStreamData(xefDbContextPtr ctxt, size_t *size);
XPLPUBFUN void XPLCALL
	xefDbUnaccessStreamData(xefDbContextPtr ctxt, xmlChar *data);
XPLPUBFUN void XPLCALL
	xefDbFreeContext(xefDbContextPtr ctxt);
XPLPUBFUN void XPLCALL
	xefDbFreeParams(xefDbQueryParamsPtr params, bool freeCarrier);

/* == Вычистка HTML == */
typedef struct _xefCleanHtmlParams
{
	/* Вход */
	xmlChar *document;			/* должен быть в utf-8 */
	/* Выход */
	xmlChar *clean_document;	/* результат. необходимо освободить */
	size_t clean_document_size;	/* размер результата */
	xefErrorMessagePtr error;	/* сообщение об ошибке. если не NULL - нужно освободить. */
} xefCleanHtmlParams, *xefCleanHtmlParamsPtr;

XPLPUBFUN bool XPLCALL
	xefCleanHtml(xefCleanHtmlParamsPtr params);

/* Немного проверок и ругани */
#ifdef _XEF_HTML_CLEANER_TIDY
	#ifdef _XEF_HAS_HTML_CLEANER
		#error XEF: another html cleaner is used already
	#else
		#define _XEF_HAS_HTML_CLEANER
	#endif
#endif

#ifndef _XEF_HAS_HTML_CLEANER
	#pragma message("WARNING: no HTML cleaner is used, some xpl:include features will be unavailable")
#endif

#ifdef _XEF_TRANSPORT_WINHTTP
	#ifdef _XEF_HAS_TRANSPORT
		#error XEF: another document transport is used already
	#else
		#define _XEF_HAS_TRANSPORT
	#endif
#endif

#ifndef _XEF_HAS_TRANSPORT
	#pragma message("WARNING: no external document transport is used, some xpl:include features will be unavailable")
#endif

#ifdef _XEF_DB_ADO
	#ifdef _XEF_HAS_DB
		#error XEF: another database connector is used already
	#else
		#define _XEF_HAS_DB
	#endif
#endif

#ifdef _XEF_DB_ODBC
	#ifdef _XEF_HAS_DB
		#error XEF: another database connector is used already
	#else
		#define _XEF_HAS_DB
	#endif
#endif

#ifndef _XEF_HAS_DB
	#pragma message("WARNING: no database module, xpl:sql is unusable")
#endif


#ifdef __cplusplus
}
#endif

#endif
