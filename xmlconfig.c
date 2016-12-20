/*
** section: xmlReader
** synopsis: Parse an XML file with an xmlReader
** purpose: Demonstrate the use of xmlReaderForFile() to parse an XML file
**          and dump the informations about the nodes found in the process.
**          (Note that the XMLReader functions require libxml2 version later
**          than 2.6.)
** usage: reader1 <filename>
** test: reader1 test2.xml > reader1.tmp ; diff reader1.tmp reader1.res ; rm reader1.tmp
** author: Daniel Veillard
** copy: see Copyright for the status of this software.
*/

#include <stdio.h>
#include <libxml/xmlreader.h>
#include <memory.h>

#include "caliper.h"

struct caliper *caliper_1st = NULL;
struct caliper **caliper_cur = &caliper_1st;

#ifdef LIBXML_READER_ENABLED

/*
** processNode:
** @reader: the xmlReader
**
** Dump information about the current node
*/
static void
processNode(xmlTextReaderPtr reader)
{
#if 1
  const xmlChar *name, *value;
#else
  char *name, *value;
#endif
  int i, attributes = 0;
  struct caliper *cali;
  
  name = xmlTextReaderConstName(reader);
  if (name == NULL)
    name = BAD_CAST "--";
  value = xmlTextReaderConstValue(reader);
  attributes = xmlTextReaderAttributeCount(reader);

#if 0
  /* reading this will modify the pointer, should be
  ** commented out for normal fuction
  */
  printf("%d %d %s %d %d attrs=%d", 
         xmlTextReaderDepth(reader),
         xmlTextReaderNodeType(reader),
         name,
         xmlTextReaderIsEmptyElement(reader),
         xmlTextReaderHasValue(reader),
         attributes
    );
  if (value == NULL)
    printf("");
  else {
    if (xmlStrlen(value) > 40)
      printf(" %.40s...", value);
    else
      printf(" %s", value);
  }
  for(i = 0; i < attributes; i++)
  {
    xmlTextReaderMoveToNextAttribute(reader);
    printf(" %s=\"%s\"",
           xmlTextReaderConstName(reader),
           xmlTextReaderConstValue(reader)
      );
  }
  printf("\n");
#endif

  if(strcmp((char *) name, "connect") == 0)
  {
    const xmlChar *aname, *avalue;
    printf("CONNECT");
    /* allocate caliper struct, attributes will follow */
    cali = (struct caliper *) malloc(sizeof(struct caliper));
    if(cali)
    {
      memset(cali, 0, sizeof(struct caliper));
      *caliper_cur = cali;
      cali->next = NULL;
      for(i = 0; i < attributes; i++)
      {
        xmlTextReaderMoveToNextAttribute(reader);
        aname = xmlTextReaderConstName(reader);
        avalue = xmlTextReaderConstValue(reader);
        printf(" %s=\"%s\"",
               xmlTextReaderConstName(reader),
               xmlTextReaderConstValue(reader)
          );
        if(strcmp((char *)aname, "device") == 0)
          asprintf(&(cali->name), "%s", (char *)avalue);
        if(strcmp((char *)aname, "persist") == 0)
          cali->persist = atoi((char *)avalue);
        if(strcmp((char *)aname, "plus") == 0)
          cali->suppress_plus = atoi((char *)avalue) > 0 ? 0 : 1;
        if(strcmp((char *)aname, "zero") == 0)
          cali->suppress_zero = atoi((char *)avalue) > 0 ? 0 : 1;
      }
      printf("\n");
      caliper_cur = &(cali->next);
    }
  }

}

/*
** streamFile:
** @filename: the file name to parse
**
** Parse and print information about an XML file.
*/
static void
streamFile(const char *filename)
{
  xmlTextReaderPtr reader;
  int ret;

  reader = xmlReaderForFile(filename, NULL, 0);
  if(reader != NULL) 
  {
    ret = xmlTextReaderRead(reader);
    while(ret == 1)
    {
      processNode(reader);
      ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);
    if (ret != 0)
    {
      fprintf(stderr, "%s : failed to parse\n", filename);
    }
  }
  else 
  {
    fprintf(stderr, "Unable to open %s\n", filename);
  }
}

void printstruct(void)
{
  struct caliper *calip = caliper_1st;

  for(; calip != NULL; calip = calip->next)
  {
    printf("device=%s, suppress_plus=%d, suppress_zero=%d\n",
           calip->name, calip->suppress_plus, calip->suppress_zero);
  }
  return;
}

int main(int argc, char **argv)
{
  if (argc != 2)
    return(1);

  /*
  ** this initialize the library and check potential ABI mismatches
  ** between the version it was compiled for and the actual shared
  ** library used.
  */
  LIBXML_TEST_VERSION;

  streamFile(argv[1]);

  printstruct();
  /*
  ** Cleanup function for the XML library.
  **/
  xmlCleanupParser();
  /*
  ** this is to debug memory for regression tests
  */
  xmlMemoryDump();
  return(0);
}
#else
int main(void)
{
  fprintf(stderr, "XInclude support not compiled in\n");
  exit(1);
}
#endif
