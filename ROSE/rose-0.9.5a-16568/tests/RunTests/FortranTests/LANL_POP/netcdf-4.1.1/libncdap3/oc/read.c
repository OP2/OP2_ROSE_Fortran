/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#include "ocinternal.h"
#include "ocdebug.h"
#include "http.h"
#include "read.h"

extern int oc_curl_file_supported;

/*Forward*/
static int readpacket(CURL*, DAPURL*, OCbytes*, OCdxd, long*);
static int readfile(char* path, char* suffix, OCbytes* packet);
#ifdef OC_DISK_STORAGE
static int readfiletofile(char* path, char* suffix, FILE* stream, unsigned long*);
#endif

int
readDDS(OCstate* state, OCtree* tree)
{
   int status;
   long lastmodified = -1;
   dapurlsetconstraints(&state->url,tree->constraint);
   status = readpacket(state->curl,&state->url,state->packet,OCDDS,
			&lastmodified);
   if(status == OC_NOERR) state->ddslastmodified = lastmodified;
   return status;
}

int
readDAS(OCstate* state, OCtree* tree)
{
   dapurlsetconstraints(&state->url,tree->constraint);
   return readpacket(state->curl,&state->url,state->packet,OCDAS,NULL);
}

int
readversion(CURL* curl, DAPURL* url, OCbytes* packet)
{
   return readpacket(curl,url,packet,OCVER,NULL);
}

static
char* ocdxdextension[] ={
".dds",  /*OCDDS*/
".das",  /*OCDAS*/
".dods", /*OCDATADDS*/
".vers", /*OCVERS*/
};

static int
readpacket(CURL* curl,DAPURL* url,OCbytes* packet,OCdxd dxd,long* lastmodified)
{
   int stat;
   int fileprotocol = 0;
   char* suffix = ocdxdextension[dxd];

   fileprotocol = (strncmp(url->base,"file:",5)==0);

   if(fileprotocol && !oc_curl_file_supported) {
        /* Short circuit file://... urls*/
	/* We do this because the test code always needs to read files*/
	stat = readfile(url->base,suffix,packet);
    } else {
        char* fetchurl = dapurlgeturl(url,NULL,suffix,!fileprotocol);
	MEMCHECK(fetchurl,OC_ENOMEM);
	if(ocdebug > 0)
            {fprintf(stderr,"fetch url=%s\n",fetchurl); fflush(stderr);}
        stat = ocfetchurl(curl,fetchurl,packet,lastmodified);
	if(ocdebug > 0)
            {fprintf(stderr,"fetch complete\n"); fflush(stderr);}
        free(fetchurl);
    }
    return THROW(stat);
}

int
readDATADDS(OCstate* state, OCtree* tree)
{
   int stat;
   long lastmod = -1;
#ifndef OC_DISK_STORAGE
   dapurlsetconstraints(&state->url,tree->constraint);
   stat = readpacket(state->curl,&state->url,state->packet,OCDATADDS,&lastmod);
   if(stat == OC_NOERR)
        state->datalastmodified = lastmod;
   tree->data.datasize = ocbyteslength(state->packet);
#else /*OC_DISK_STORAGE*/
   DAPURL* url = &state->url;
   int fileprotocol = 0;

   fileprotocol = (strncmp(url->base,"file:",5)==0);

    if(fileprotocol && !oc_curl_file_supported) {
	stat = readfiletofile(url->base, ".dods", tree->data.file, &tree->data.datasize);
    } else {
        char* fetchurl;
        dapurlsetconstraints(url,tree->constraint);
        fetchurl = dapurlgeturl(url,NULL,".dods",!fileprotocol);
        MEMCHECK(fetchurl,OC_ENOMEM);
	if (ocdebug > 0) 
	    {fprintf(stderr, "fetch url=%s\n", fetchurl);fflush(stderr);}
	stat = ocfetchurl_file(state->curl, fetchurl, tree->data.file,
                               &tree->data.datasize, &lastmod);
        if(stat == OC_NOERR)
	    state->datalastmodified = lastmod;
	if (ocdebug > 0) 
            {fprintf(stderr,"fetch complete\n"); fflush(stderr);}
	free(fetchurl);
    }
#endif /*OC_DISK_STORAGE*/
    return THROW(stat);
}

#ifdef OC_DISK_STORAGE
static int
readfiletofile(char* path, char* suffix, FILE* stream, unsigned long* sizep)
{
    int stat;
    OCbytes* packet = ocbytesnew();
    size_t len;
    /* check for leading file:/// */
    if(strncmp(path,"file:///",8)==0) path += 7; /* assume absolute path*/
    stat = readfile(path,suffix,packet);
    if(stat != OC_NOERR) goto unwind;
    len = oclistlength(packet);
    if(stat == OC_NOERR) {
	size_t written;
        fseek(stream,0,SEEK_SET);
	written = fwrite(ocbytescontents(packet),1,len,stream);
	if(written != len) stat = OC_EIO;
    }
    if(sizep != NULL) *sizep = len;
unwind:
    ocbytesfree(packet);
    return THROW(stat);
}
#endif

static int
readfile(char* path, char* suffix, OCbytes* packet)
{
    int stat;
    char buf[1024];
    char filename[1024];
    int count,size,fd;
    /* check for leading file:/// */
    if(strncmp(path,"file://",7)==0) path += 7; /* assume absolute path*/
    strcpy(filename,path);
    if(suffix != NULL) strcat(filename,suffix);
    fd = open(filename,O_RDONLY);
    if(fd < 0) {
	oc_log(LOGERR,"open failed:%s",filename);
	return THROW(OC_EOPEN);
    }
    size=0;
    stat = OC_NOERR;
    for(;;) {
	count = read(fd,buf,sizeof(buf));
	if(count <= 0)
	    break;
	else if(count <  0) {
	    stat = OC_EIO;
	    oc_log(LOGERR,"read failed: %s",filename);
	    break;
	}
	ocbytesappendn(packet,buf,count);
	size += count;
    }
    close(fd);
    return THROW(stat);
}
