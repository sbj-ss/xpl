/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __reszbuf_H
#define __reszbuf_H

/* =======================================================================
   Буфер динамического размера.
   Написан собственный, потому что аналог из libxml2 сделан, как обычно,
   на статических функциях и макросах. 
   =======================================================================
*/

#include "Configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Стратегия роста буфера при заполнении */
typedef enum _ReszBufGrowStrategy 
{
	RESZ_BUF_GROW_UNKNOWN = -1, /* Только возврат getReszBufGrowStrategy */
	RESZ_BUF_GROW_EXACT = 0,    /* Ровно столько места, сколько нужно */
	RESZ_BUF_GROW_DOUBLE,		/* Удвоение размера */
	RESZ_BUF_GROW_INCREMENT,	/* Увеличение на линейный инкремент */
	RESZ_BUF_GROW_FIXED,		/* Не увеличивать, отбить ошибку */
	RESZ_BUF_GROW_FLUSH			/* Вызвать пользовательскую функцию сброса, записать новые данные в начало */
} ReszBufGrowStrategy;

typedef enum _ReszBufOpResult
{
	RESZ_BUF_RESULT_OK = 0,
	RESZ_BUF_RESULT_NO_MEMORY = -1,
	RESZ_BUF_RESULT_FLUSH_FAILED = -2,
	RESZ_BUF_RESULT_INVALID = -3
} ReszBufOpResult;

typedef struct _ReszBuf *ReszBufPtr;

/* Пользовательская функция сброса буфера в IO, когда в него "перестаёт лезть".
   0 = ОК
   <0 = сбой, не сбрасывать
   Функция должна быть способна принять блок данных размером до размера
   буфера включительно.
 */
typedef int (*ReszBufFlushCallback)(void* data, size_t size);

/* Создать буфер с параметрами по умолчанию */
XPLPUBFUN ReszBufPtr XPLCALL
	createReszBuf(void);
/* Создать буфер указанного начального размера */
XPLPUBFUN ReszBufPtr XPLCALL
	createReszBufSize(size_t initialSize);
/* Создать буфер с указанными начальным размером, стратегией роста и линейным инкрементом */
XPLPUBFUN ReszBufPtr XPLCALL
	createReszBufParams(size_t initialSize, ReszBufGrowStrategy strategy, size_t increment);
/* Получить стратегию роста буфера */
XPLPUBFUN ReszBufGrowStrategy XPLCALL
	getReszBufGrowStrategy(ReszBufPtr buf);
/* Установить стратегию роста буфера */
XPLPUBFUN void XPLCALL
	setReszBufGrowStrategy(ReszBufPtr buf, ReszBufGrowStrategy strategy);
/* Получить инкремент линейного роста буфера */
XPLPUBFUN size_t XPLCALL
	getReszBufGrowIncrement(ReszBufPtr buf);
/* Установить инкремент линейного роста буфера */
XPLPUBFUN void XPLCALL
	setReszBufGrowIncrement(ReszBufPtr buf, size_t increment);
/* Получить размер содержимого буфера в байтах */
XPLPUBFUN size_t XPLCALL
	getReszBufContentSize(ReszBufPtr buf);
/* Получить функцию сброса буфера при переполнении */
XPLPUBFUN ReszBufFlushCallback XPLCALL
	getReszBufFlushCallback(ReszBufPtr buf);
/* Установить функцию сброса буфера при переполнении */
XPLPUBFUN ReszBufFlushCallback XPLCALL
	setReszBufFlushCallback(ReszBufPtr buf, ReszBufFlushCallback cb);
/* Получить указатель на содержимое буфера */
XPLPUBFUN void* XPLCALL
	getReszBufContent(ReszBufPtr buf);
/* Получить указатель на содержимое буфера, отцепить содержимое от управляющей структуры */
XPLPUBFUN void* XPLCALL
	detachReszBufContent(ReszBufPtr buf);
/* Записать в буфер size байт */
XPLPUBFUN ReszBufOpResult XPLCALL
	addDataToReszBuf(ReszBufPtr buf, void* content, size_t size);
/* Отмотать внутренний указатель буфера на начало (память не освобождается) */
XPLPUBFUN void XPLCALL
	rewindReszBuf(ReszBufPtr buf);
/* Получить прямой указатель на текущую позицию */
XPLPUBFUN void* XPLCALL
	getReszBufPosition(ReszBufPtr buf);
/* Продвинуть текущее положение вперед */
XPLPUBFUN ReszBufOpResult XPLCALL
	advanceReszBufferPosition(ReszBufPtr buf, size_t delta);
/* Убедиться, что в буфере достаточно места для прямой записи */
XPLPUBFUN ReszBufOpResult XPLCALL
	ensureReszBufFreeSize(ReszBufPtr buf, size_t minfree);
/* Принудительно сбросить буфер*/
XPLPUBFUN ReszBufOpResult XPLCALL
	flushReszBuf(ReszBufPtr buf);
/* Удалить буфер (память, занятая содержимым, освобождается */
XPLPUBFUN void XPLCALL
	freeReszBuf(ReszBufPtr buf);

#ifdef __cplusplus
}
#endif
#endif
