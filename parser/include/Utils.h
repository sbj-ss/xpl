/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __utils_H
#define __utils_H

#include "Configuration.h"
#include "Common.h"
#include "abstraction/xpr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EXTRACT_NS_AND_TAGNAME(src, ns, tagName, parent) do { \
	tagName = BAD_CAST xmlStrchr((src), ':'); \
	if (tagName) \
	{ \
		*tagName = 0; \
		ns = xmlSearchNs((parent)->doc, (parent), (src)); \
		*tagName++ = ':'; \
	} else { \
		tagName = BAD_CAST (src); \
		ns = NULL; \
	} \
} while(0);

#define XML_NODE_DELETION_REQUEST_FLAG ((xmlElementType) 0x0080UL)
#define XML_NODE_DELETION_DEFERRED_FLAG ((xmlElementType) 0x0100UL)
#define XML_NODE_DELETION_MASK (XML_NODE_DELETION_DEFERRED_FLAG | XML_NODE_DELETION_REQUEST_FLAG)

/* Отцепляет содержимое от узла */
XPLPUBFUN xmlNodePtr XPLCALL 
	detachContent(xmlNodePtr el);
/* Устанавливает список дочерними объектами указанного элемента, возвращает последний элемент списка */
XPLPUBFUN xmlNodePtr XPLCALL
	setChildren(xmlNodePtr el, xmlNodePtr list);
/* Дописывает список в хвост дочерних объектов указанного элемента, возвращает последний элемент списка */
XPLPUBFUN xmlNodePtr XPLCALL
	appendChildren(xmlNodePtr el, xmlNodePtr list);

/* Добавляет список после указанного элемента, возвращает последний элемент добавляемого списка */
XPLPUBFUN xmlNodePtr XPLCALL
	appendList(xmlNodePtr el, xmlNodePtr list);
/* Добавляет список до указанного элемента, возвращает последний элемент добавляемого списка */
XPLPUBFUN xmlNodePtr XPLCALL
	prependList(xmlNodePtr el, xmlNodePtr list);
/* Заменяет узел на список, возвращает старый узел */
XPLPUBFUN xmlNodePtr XPLCALL
	replaceWithList(xmlNodePtr el, xmlNodePtr list);

/* Обрезать с боков пробелы, табуляцию и переводы строки
   Возвращает свежесозданную строку (нужен вызов xmlFree), не трогает существующую
 */
XPLPUBFUN xmlChar* XPLCALL
	strTrim(xmlChar* str);

/* Есть ли что-то, кроме форматирования */
XPLPUBFUN bool XPLCALL
	strNonblank(xmlChar *str);
/* ^[0-9]+(\.[0-9]+)$ */
XPLPUBFUN bool XPLCALL
	isNumber(xmlChar *str);

/* Получение последней ошибки libxml2 в удобочитаемом виде.
   Результат необходимо удалить. 
 */
XPLPUBFUN xmlChar* XPLCALL
	getLastLibxmlError(void);

/* Добавить следующий элемент с трансформацией атрибута в текст (после XPath-выборки) */
XPLPUBFUN xmlNodePtr XPLCALL
	cloneAttrAsText(xmlNodePtr cur, xmlNodePtr parent);

/* Проверка, что в списке содержатся только текстовые узлы, CDATA и ссылки на сущности */
XPLPUBFUN bool XPLCALL
	checkNodeListForText(xmlNodePtr start);
XPLPUBFUN bool XPLCALL
	checkNodeSetForText(xmlNodeSetPtr s);

/* Пометить фрагмент родительской оси к удалению
   bottom - стартовая точка, top - конечная (обе - включительно)
 */
XPLPUBFUN void XPLCALL 
	markAncestorAxisForDeletion(xmlNodePtr bottom, xmlNodePtr top);
XPLPUBFUN void XPLCALL
	markDOSAxisForDeletion(xmlNodePtr cur, int bitwiseAttribute, bool doMark);
/* Расчистить соседей на всех уровнях до указанной точки */
XPLPUBFUN void XPLCALL
	deleteNeighbours(xmlNodePtr cur, xmlNodePtr boundary, bool markAncestorAxis);

/* Является ли test родителем cur */
XPLPUBFUN int XPLCALL
	isAncestor(xmlNodePtr cur, xmlNodePtr test);

/* Найти хвост цепочки */
XPLPUBFUN xmlNodePtr XPLCALL
	findTail(xmlNodePtr cur);

/* Копирование узла и списка узлов с учётом иерархии
   (см. комментарии в реализации)
   Родитель используется как подсказка при поиске пространств имён
   и НЕ приписывается элементу!
 */
XPLPUBFUN void XPLCALL
	initNamePointers(void);
XPLPUBFUN xmlNodePtr XPLCALL
	cloneNode(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc);
XPLPUBFUN xmlNodePtr XPLCALL
	cloneNodeList(xmlNodePtr node, xmlNodePtr parent, xmlDocPtr doc);

/* Трансляция NsDef вниз перед удалением узла (списка узлов)
   Считаем, что они уже отцеплены. ns_list - список nsDef бывшего родителя.
 */
XPLPUBFUN void XPLCALL
	downshiftNodeNsDef(xmlNodePtr cur, xmlNsPtr ns_list);
XPLPUBFUN void XPLCALL
	downshiftNodeListNsDef(xmlNodePtr cur, xmlNsPtr ns_list);

/* Скопировать все определения ns на элемент верхнего уровня */
XPLPUBFUN void XPLCALL
	makeNsIndepTree(xmlNodePtr top);
  
/* Расширения XPath */
XPLPUBFUN void XPLCALL
	xplRegisterXPathExtensions(xmlXPathContextPtr ctxt);

/* Возвращает содержимое атрибута. Результат необходимо освободить. */
XPLPUBFUN xmlChar* XPLCALL 
	getPropValue(xmlAttrPtr prop);
XPLPUBFUN void XPLCALL
	unlinkProp(xmlAttrPtr cur);


/* Кодировки */
/* Проверка на корректную utf-8 запись */
XPLPUBFUN bool XPLCALL
	isValidUtf8Sample(xmlChar *s, size_t len, bool isCompleteString);
/* Получение смещения до следующего UTF-8 символа. При неверной записи возвращает 0! */
XPLPUBFUN size_t XPLCALL
	getOffsetToNextUTF8Char(xmlChar *cur);
/* Поддержка кириллических (и не только) URI */
XPLPUBFUN xmlChar* XPLCALL
	encodeUriIdn(xmlChar *uri);

#define DETECTED_ENC_UNKNOWN (-1)
#define DETECTED_ENC_866 1
#define DETECTED_ENC_1251 2
#define DETECTED_ENC_KOI8 3
#define DETECTED_ENC_UTF8 4
#define DETECTED_ENC_UTF16LE 5
#define DETECTED_ENC_UTF16BE 6

#define DEFAULT_ENC_DET_SAMPLE_LEN 256

/* Автоматическое определение кодировки */
XPLPUBFUN int XPLCALL 
	detectEncoding(char* str, size_t sampleLen);

/* Перекодировка строки */
XPLPUBFUN int XPLCALL
	iconv_string (const char* tocode, const char* fromcode,
				  const char* start, const char* end,
				  char** resultp, size_t* lengthp);
/* Шестнадцатеричный дамп буфера. Результат необходимо освободить. */
XPLPUBFUN xmlChar* XPLCALL
	bufferToHex(void* buf, size_t len, bool prefix);
/* base-64 запись буфера. Память под результат должна быть выделена до вызова функции, result и resultSize заполнены! */
XPLPUBFUN int XPLCALL
	base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize);
XPLPUBFUN size_t XPLCALL
	base64decode(const char* data_buf, size_t dataLength, char* result, size_t resultSize);



/* "Грязное" сохранение - пишется только заголовок, узлы и атрибуты
   Не использует менеджер памяти */
XPLPUBFUN void XPLCALL
	safeSerializeContent(FILE *fp, xmlChar* content);
XPLPUBFUN void XPLCALL
	safeSerializeNode(FILE *fp, xmlNodePtr node, int indent);
XPLPUBFUN void XPLCALL
	safeSerializeNodeList(FILE *fp, xmlNodePtr list, int indent);
XPLPUBFUN void XPLCALL
	safeSerializeDocument(char *filename, xmlDocPtr doc);

/* Сохранение документа в файл с учётом символов нац. алфавитов в путях */
XPLPUBFUN bool XPLCALL
	saveXmlDocToFile(xmlDocPtr doc, xmlChar *filename, char *encoding, int options);

/* Для сериализаторов */
XPLPUBFUN xmlChar* XPLCALL 
	serializeNodeList(xmlNodePtr cur);
XPLPUBFUN xmlChar* XPLCALL
	serializeNodeSet(xmlNodeSetPtr set);

XPLPUBFUN void XPLCALL
	composeAndSplitPath(xmlChar *basePath, xmlChar *relativePath, xmlChar **normalizedPath, xmlChar **normalizedFilename);

/* Примитивы для команд */
/* Найти результирующее ns. Если оно объявлено на invoker, но не на parent - создаётся дубликат. */
XPLPUBFUN xmlNsPtr XPLCALL
	getResultingNs(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name);

/* Прицепить к элементу атрибут */
XPLPUBFUN void XPLCALL
	assignAttribute(xmlNodePtr src, xmlNodePtr dst, xmlChar *name, xmlChar *value, bool allowReplace);

/* Создать узел-элемент с заданным именем */
XPLPUBFUN xmlNodePtr XPLCALL
	createElement(xmlNodePtr parent, xmlNodePtr invoker, xmlChar *name);

XPLPUBFUN bool XPLCALL
	checkNodeEquality(xmlNodePtr a, xmlNodePtr b);
XPLPUBFUN bool XPLCALL
	checkNodeListEquality(xmlNodePtr a, xmlNodePtr b);
XPLPUBFUN bool XPLCALL
	checkPropListEquality(xmlAttrPtr a, xmlAttrPtr b);
/* Сравнить два набора узлов по декартову произведению */
XPLPUBFUN bool XPLCALL
	checkNodeSetEquality(xmlNodeSetPtr a, xmlNodeSetPtr b);
/* Сравнить два набора узлов по декартову произведению */
XPLPUBFUN bool XPLCALL
	checkNodeSetIdentity(xmlNodeSetPtr a, xmlNodeSetPtr b);

/* Сравнить два результата XPath-выборок. В случае набора узлов сравнение идёт по декартову произведению.
   При типах, отличных от string/boolean/number/nodeset, всегда возвращает false. */
XPLPUBFUN bool XPLCALL
	compareXPathSelections(xmlXPathObjectPtr a, xmlXPathObjectPtr b, bool checkEquality);

/* Дописать в конец строки шестнадцатиричную запись id. Возвращает указатель на новую строку (xmlRealloc). */
XPLPUBFUN xmlChar* XPLCALL
	appendThreadIdToString(xmlChar *str, XPR_THREAD_ID id);

#ifdef __cplusplus
}
#endif
#endif
