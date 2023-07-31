/* Copyright 2018-2018 University Corporation for Atmospheric  Research/Unidata. */

#include <stdlib.h>
#include <string.h>
#include <libxml2/libxml/parser.h>
#include <libxml/tree.h>
#include "ncxml.h"

#ifndef nulldup
#define nulldup(s) ((s)?strdup(s):NULL)
#endif

static int ncxml_initialized = 0;

void
ncxml_initialize(void)
{
    ncxml_initialized = 1;
}

void
ncxml_finalize(void)
{
    ncxml_initialized = 0;
    xmlCleanupParser();
}

ncxml_doc_t
ncxml_parse(char* contents, size_t len)
{
    xmlDocPtr doc; /* the resulting document tree */
    doc = xmlReadMemory(contents, (int)len, "dap4.xml", NULL, 0);
    return (ncxml_doc_t)doc;
}

void
ncxml_free(ncxml_doc_t doc0)
{
    xmlDoc *doc = (xmlDoc*)doc0;
    xmlFreeDoc(doc);
 }

ncxml_t
ncxml_root(ncxml_doc_t doc0)
{
    xmlDoc *doc = (xmlDoc*)doc0;
    return (ncxml_t)xmlDocGetRootElement(doc);
}

const char*
ncxml_name(ncxml_t xml0)
{
    xmlNode* xml = (xmlNode*)xml0;    
    return (const char*)(xml?xml->name:NULL);
}

char*
ncxml_attr(ncxml_t xml0, const char* key)
{
    xmlNode* xml = (xmlNode*)xml0;
    xmlChar* value = NULL;
    char* s = NULL;

    value = xmlGetProp(xml,(const xmlChar*)key);
    s = nulldup((char*)value);
    xmlFree(value);
    return s;
}

/* First child by name */
ncxml_t
ncxml_child(ncxml_t xml0, const char* name)
{
    xmlNode* xml = (xmlNode*)xml0;
    xmlNode* child = NULL;

    for(child=xml->children;child; child = child->next) {
        if(child->type == XML_ELEMENT_NODE && strcmp((const char*)child->name,name)==0) 
	    return (ncxml_t)child;
    }
    return NULL;
}

ncxml_t
ncxml_next(ncxml_t xml0, const char* name)
{
    xmlNode* xml = (xmlNode*)xml0;
    xmlNode* next = NULL;

    for(next=xml->next;next; next = next->next) {
        if(next->type == XML_ELEMENT_NODE && strcmp((const char*)next->name,name)==0) 
	    return (ncxml_t)next;
    }
    return NULL;
}

char*
ncxml_text(ncxml_t xml0)
{
    xmlNode* xml = (xmlNode*)xml0;
    xmlChar* txt = NULL;
    char* s = NULL;
    if(xml == NULL) return NULL;
    txt = xmlNodeGetContent(xml);
    s = nulldup((char*)txt);
    xmlFree(txt);
    return s;
}

/* Nameless versions of child and next */
ncxml_t
ncxml_child_first(ncxml_t xml0)
{
    xmlNode* xml = (xmlNode*)xml0;
    xmlNode* child = NULL;

    if(xml == NULL) return NULL;
    for(child=xml->children;child; child = child->next) {
        if(child->type == XML_ELEMENT_NODE) return child;
    }
    return NULL;
}

ncxml_t
ncxml_child_next(ncxml_t xml0)
{
    xmlNode* xml = (xmlNode*)xml0;

    if(xml == NULL) return NULL;
    for(xml=xml->next;xml; xml = xml->next) {
        if(xml->type == XML_ELEMENT_NODE) return xml;
    }
    return NULL;
}

int
ncxml_attr_pairs(ncxml_t xml0, char*** pairsp)
{
    char** pairs = NULL;
    xmlNode* xml = (xmlNode*)xml0;
    xmlAttr* attr = NULL;
    int i,count = 0;

    if(xml == NULL) return 0;
    /* First count */
    for(attr=xml->properties;attr;attr=attr->next) count++;
    /* Allocate */
    pairs = (char**)malloc(sizeof(char*)*((2*count)+1));
    if(pairs == NULL) return 0;
    /* Collect */
    for(i=0,attr=xml->properties;attr;i+=2,attr=attr->next) {
	xmlChar* value;
        pairs[i] = nulldup((char*)attr->name);
        value = xmlNodeListGetString(xml->doc, attr->children, 1);
	pairs[i+1] = nulldup((char*)value);
	xmlFree(value);
    }
    pairs[2*count] = NULL;
    if(pairsp) *pairsp = pairs;
    return 1;
}
