/******************************************/
/* Polaris project: the pure C XPL engine */
/* (c) НИЛ ИТС, Подковырин, 2006-2020     */
/******************************************/
#ifndef __xplsave_H
#define __xplsave_H

#include "Configuration.h"
#include <stdbool.h>
#include <stdio.h>
#include <libxml/xmlstring.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
#endif
