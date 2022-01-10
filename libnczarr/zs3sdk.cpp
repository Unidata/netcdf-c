/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

#define NOOP

#include "awsincludes.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <streambuf>
#include "netcdf.h"
#include "zincludes.h"
#include "zs3sdk.h"

#ifdef __CYGWIN__
extern char* strdup(const char*);
#endif

#define size64_t unsigned long long

static Aws::SDKOptions zs3options;

/* Forward */
static Aws::S3::Model::BucketLocationConstraint s3findregion(const char* name);
static int s3objectinfo1(const Aws::S3::Model::Object& s3_object, char** fullkeyp, size64_t* lenp);
static int s3objectsinfo(Aws::Vector<Aws::S3::Model::Object> list, size_t*, char*** keysp, size64_t** lenp);
static int s3commonprefixes(Aws::Vector<Aws::S3::Model::CommonPrefix> list, char*** keysp);
static void freestringenvv(char** ss);
static int makes3key(const char* pathkey, const char** keyp);
static int makes3keydir(const char* prefix, char** prefixdirp);
static char** mergekeysets(size_t nkeys1, char** keys1, size_t nkeys2, char** keys2);
    
static const char*
dumps3info(ZS3INFO* info)
{
    static char text[8192];
    snprintf(text,sizeof(text),"host=%s region=%s bucket=%s rootkey=%s profile=%s",
		(info->host?info->host:"null"),
		(info->region?info->region:"null"),
		(info->bucket?info->bucket:"null"),
		(info->rootkey?info->rootkey:"null"),
		(info->profile?info->profile:"null"));
    return text;
}

void
NCZ_s3sdkinitialize(void)
{
    ZTRACE(11,NULL);
    Aws::InitAPI(zs3options);
    ZUNTRACE(NC_NOERR);
}

void
NCZ_s3sdkfinalize(void)
{
    ZTRACE(11,NULL);
    Aws::ShutdownAPI(zs3options);
    ZUNTRACE(NC_NOERR);
}

static char*
makeerrmsg(const Aws::Client::AWSError<Aws::S3::S3Errors> err, const char* key="")
{
    char* errmsg;
    size_t len;
    len = strlen(err.GetExceptionName().c_str()) + strlen(err.GetMessage().c_str()) + strlen(key) + 10;
    if((errmsg = (char*)malloc(len+1))==NULL)
        return NULL;
    snprintf(errmsg,len,"%s %s key=%s",
		err.GetExceptionName().c_str(),
		err.GetMessage().c_str(),
		key);
    return errmsg;
}


static Aws::Client::ClientConfiguration*
s3sdkcreateconfig(ZS3INFO* info)
{
    int stat = NC_NOERR;
    ZTRACE(11,"info=%s", dumps3info(info));

    Aws::Client::ClientConfiguration *config;
    if(info->profile)
        config = new Aws::Client::ClientConfiguration(info->profile);
    else
        config = new Aws::Client::ClientConfiguration();
    config->scheme = Aws::Http::Scheme::HTTPS;
    config->connectTimeoutMs = 300000;
    config->requestTimeoutMs = 600000;
    if(info->region) config->region = info->region;
    if(info->host) config->endpointOverride = info->host;
    config->enableEndpointDiscovery = true;
    config->followRedirects = Aws::Client::FollowRedirectsPolicy::ALWAYS;
    ZUNTRACE(NC_NOERR);
    return config;
}

void*
NCZ_s3sdkcreateclient(ZS3INFO* info)
{
    ZTRACE(11,NULL);

    Aws::Client::ClientConfiguration* config = s3sdkcreateconfig(info);
    Aws::S3::S3Client *s3client;

    if(info->profile == NULL || strcmp(info->profile,"none")==0) {
        Aws::Auth::AWSCredentials creds;
        creds.SetAWSAccessKeyId(Aws::String(""));
        creds.SetAWSSecretKey(Aws::String(""));
        s3client = new Aws::S3::S3Client(creds,*config,
                               Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::RequestDependent,
			       false);
    } else {
        s3client = new Aws::S3::S3Client(*config,
                               Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::RequestDependent,
			       false);
    }
    delete config;
    ZUNTRACE(NC_NOERR);
    return (void*)s3client;
}

int
NCZ_s3sdkbucketexists(void* s3client0, const char* bucket, int* existsp, char** errmsgp)
{
    int stat = NC_NOERR;
    int exists = 0;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;

    ZTRACE(11,"bucket=%s",bucket);
    if(errmsgp) *errmsgp = NULL;
    auto result = s3client->ListBuckets();
    if(!result.IsSuccess()) {
	if(errmsgp) *errmsgp = makeerrmsg(result.GetError());	
	stat = NC_ES3;
    } else {
        Aws::Vector<Aws::S3::Model::Bucket> bucket_list = result.GetResult().GetBuckets();
        for(auto const &awsbucket : bucket_list) {
	   auto name = awsbucket.GetName();
	   if(name == bucket) {exists = 1; break;}
	}
    }
    if(existsp) *existsp = exists;
    return ZUNTRACEX(stat,"exists=%d",*existsp);
}

int
NCZ_s3sdkbucketcreate(void* s3client0, const char* region, const char* bucket, char** errmsgp)
{
    int stat = NC_NOERR;
    
    ZTRACE(11,"region=%s bucket=%s",region,bucket);
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    if(errmsgp) *errmsgp = NULL;
    const Aws::S3::Model::BucketLocationConstraint &awsregion = s3findregion(region);
    if(awsregion == Aws::S3::Model::BucketLocationConstraint::NOT_SET)
        return NC_EURL;
        /* Set up the request */
    Aws::S3::Model::CreateBucketRequest create_request;
    create_request.SetBucket(bucket);
    if(region) {
        /* Specify the region as a location constraint */
        Aws::S3::Model::CreateBucketConfiguration bucket_config;
        bucket_config.SetLocationConstraint(awsregion);
        create_request.SetCreateBucketConfiguration(bucket_config);
    }
#ifdef NOOP
    /* Create the bucket */
    auto create_result = s3client->CreateBucket(create_request);
    if(!create_result.IsSuccess()) {
	if(errmsgp) *errmsgp = makeerrmsg(create_result.GetError());	
	stat = NC_ES3;;
    }
#else
    fprintf(stderr,"create bucket: %s\n",bucket); fflush(stderr);
#endif

    return ZUNTRACE(stat);    
}

int
NCZ_s3sdkbucketdelete(void* s3client0, ZS3INFO* info, char** errmsgp)
{
    int stat = NC_NOERR;

    ZTRACE(11,"info=%s%s",dumps3info(info));

    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    
    if(errmsgp) *errmsgp = NULL;
    const Aws::S3::Model::BucketLocationConstraint &awsregion = s3findregion(info->region);
    if(awsregion == Aws::S3::Model::BucketLocationConstraint::NOT_SET)
        return NC_EURL;
        /* Set up the request */
    Aws::S3::Model::DeleteBucketRequest request;
    request.SetBucket(info->bucket);

#ifdef NOOP
    /* Delete the bucket */
    auto result = s3client->DeleteBucket(request);
    if(!result.IsSuccess()) {
	if(errmsgp) *errmsgp = makeerrmsg(result.GetError());	
	stat = NC_ES3;;
    }
#else
    fprintf(stderr,"delete bucket: %s\n",bucket); fflush(stderr);
#endif

    return ZUNTRACE(stat);    
}

/**************************************************/
/* Object API */

/*
@return NC_NOERR if key points to a content-bearing object.
@return NC_EEMPTY if object at key has no content.
@return NC_EXXX return true error
*/
int
NCZ_s3sdkinfo(void* s3client0, const char* bucket, const char* pathkey, size64_t* lenp, char** errmsgp)
{
    int stat = NC_NOERR;
    const char* key = NULL;

    ZTRACE(11,"bucket=%s pathkey=%s",bucket,pathkey);

    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::HeadObjectRequest head_request;
    if(*pathkey != '/') return NC_EINTERNAL;
    /* extract the true s3 key*/
    if((stat = makes3key(pathkey,&key))) return ZUNTRACE(stat);

    if(errmsgp) *errmsgp = NULL;
    head_request.SetBucket(bucket);
    head_request.SetKey(key);
    auto head_outcome = s3client->HeadObject(head_request);
    if(head_outcome.IsSuccess()) {
	long long l  = head_outcome.GetResult().GetContentLength(); 
	if(lenp) *lenp = (size64_t)l;
    } else {
	if(lenp) *lenp = 0;
	/* Distinquish not-found from other errors */
	switch (head_outcome.GetError().GetErrorType()) {
	case Aws::S3::S3Errors::RESOURCE_NOT_FOUND:
            stat = NC_EEMPTY;
	    break;
	case Aws::S3::S3Errors::ACCESS_DENIED:
            stat = NC_EACCESS;
	    /* fall thru */
	default:
	    if(!stat) stat = NC_ES3;
            if(errmsgp) *errmsgp = makeerrmsg(head_outcome.GetError(),key);
	    break;
	}
    }
    return ZUNTRACEX(stat,"len=%d",(int)(lenp?*lenp:-1));
}

/*
@return NC_NOERR if success
@return NC_EXXX if fail
*/
int
NCZ_s3sdkread(void* s3client0, const char* bucket, const char* pathkey, size64_t start, size64_t count, void* content, char** errmsgp)
{
    int stat = NC_NOERR;
    char range[1024];
    const char* key = NULL;
    size64_t rangeend;

    ZTRACE(11,"bucket=%s pathkey=%s start=%llu count=%llu content=%p",bucket,pathkey,start,count,content);

    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::GetObjectRequest object_request;

    if(count == 0) return ZUNTRACE(stat);

    if(*pathkey != '/') return NC_EINTERNAL;
    if((stat = makes3key(pathkey,&key))) return ZUNTRACE(stat);
    
    object_request.SetBucket(bucket);
    object_request.SetKey(key);
    rangeend = (start+count)-1;
    snprintf(range,sizeof(range),"bytes=%llu-%llu",start,rangeend);
    object_request.SetRange(range);
    auto get_object_result = s3client->GetObject(object_request);
    if(!get_object_result.IsSuccess()) {
	if(errmsgp) *errmsgp = makeerrmsg(get_object_result.GetError(),key);
	stat = NC_ES3;
    } else {
	/* Get the whole result */
	Aws::IOStream &result = get_object_result.GetResultWithOwnership().GetBody();
	std::string str((std::istreambuf_iterator<char>(result)),std::istreambuf_iterator<char>());
	/* Verify actual result size */
	size_t slen = str.size();
	if(slen > count) return NC_ES3;
	const char* s = str.c_str();
	if(content)
	    memcpy(content,s,slen);
    }
    return ZUNTRACE(stat);
}

/*
For S3, I can see no way to do a byterange write;
so we are effectively writing the whole object
*/
int
NCZ_s3sdkwriteobject(void* s3client0, const char* bucket, const char* pathkey,  size64_t count, const void* content, char** errmsgp)
{
    int stat = NC_NOERR;
    const char* key = NULL;

    ZTRACE(11,"bucket=%s pathkey=%s count=%lld content=%p",bucket,pathkey,count,content);
    
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::PutObjectRequest put_request;

    if(*pathkey != '/') return ZUNTRACE(NC_EINTERNAL);
    if((stat = makes3key(pathkey,&key))) return ZUNTRACE(stat);
    
    if(errmsgp) *errmsgp = NULL;
    put_request.SetBucket(bucket);
    put_request.SetKey(key);
    put_request.SetContentLength((long long)count);

    std::shared_ptr<Aws::IOStream> data = std::shared_ptr<Aws::IOStream>(new Aws::StringStream());
    data->rdbuf()->pubsetbuf((char*)content,count);
    put_request.SetBody(data);
    auto put_result = s3client->PutObject(put_request);
    if(!put_result.IsSuccess()) {
        if(errmsgp) *errmsgp = makeerrmsg(put_result.GetError(),key);
        stat = NC_ES3;
    }
    return ZUNTRACE(stat);
}

int
NCZ_s3sdkclose(void* s3client0, ZS3INFO* info, int deleteit, char** errmsgp)
{
    int stat = NC_NOERR;

    ZTRACE(11,"info=%s rootkey=%s deleteit=%d",dumps3info(info),deleteit);
    
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    if(deleteit) {
        /* Delete the root key; ok it if does not exist */
        switch (stat = NCZ_s3sdkdeletekey(s3client0,info->bucket,info->rootkey,errmsgp)) {
        case NC_NOERR: break;
        case NC_EEMPTY: case NC_ENOTFOUND: stat = NC_NOERR; break;
        default: break;
        }
    }
    delete s3client;
    return ZUNTRACE(stat);
}

/*
Return a list of names of legal objects immediately below a specified key.
In theory, the returned list should be sorted in lexical order,
but it possible that it is not.
*/
int
NCZ_s3sdkgetkeys(void* s3client0, const char* bucket, const char* prefixkey0, size_t* nkeysp, char*** keysp, char** errmsgp)
{
    int stat = NC_NOERR;
    size_t nkeys = 0;
    const char* prefix = NULL;
    char* prefixdir = NULL;
    char** realkeys = NULL;
    char** commonkeys = NULL;
    char** allkeys = NULL;

    ZTRACE(11,"bucket=%s prefixkey0=%s",bucket,prefixkey0);
    
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::ListObjectsV2Request objects_request;

    if(*prefixkey0 != '/') return ZUNTRACE(NC_EINTERNAL);
    /* Make sure that the prefix ends with '/' */
    if((stat = makes3keydir(prefixkey0,&prefixdir))) return ZUNTRACE(stat);        
    /* remove leading '/' */
    if((stat = makes3key(prefixdir,&prefix))) return ZUNTRACE(stat);

    if(errmsgp) *errmsgp = NULL;
    objects_request.SetBucket(bucket);
    objects_request.SetPrefix(prefix);
    objects_request.SetDelimiter("/"); /* Force return of common prefixes */
    auto objects_outcome = s3client->ListObjectsV2(objects_request);
    if(objects_outcome.IsSuccess()) {
	size_t nrealkeys = 0;
	size_t ncommonkeys = 0;	
        Aws::Vector<Aws::S3::Model::Object> object_list =
            objects_outcome.GetResult().GetContents();
        nrealkeys = (size_t)object_list.size();
        stat = s3objectsinfo(object_list,NULL,&realkeys,NULL);
	/* Add common prefixes */
        Aws::Vector<Aws::S3::Model::CommonPrefix> common_list =
            objects_outcome.GetResult().GetCommonPrefixes();
        ncommonkeys = (size_t)common_list.size();
        stat = s3commonprefixes(common_list,&commonkeys);
	/* merge the two lists */
	if((allkeys=mergekeysets(nrealkeys, realkeys, ncommonkeys, commonkeys))==NULL)
	    stat = NC_ENOMEM;
	if(stat == NC_NOERR) {
            if(nkeysp) {*nkeysp = nrealkeys + ncommonkeys;}
	    if(keysp) {*keysp = allkeys; allkeys = NULL;}
	}
	freestringenvv(allkeys);
	freestringenvv(realkeys);
	freestringenvv(commonkeys);
    } else {
        if(errmsgp) *errmsgp = makeerrmsg(objects_outcome.GetError());
        stat = NC_ES3;
    }
    if(prefixdir) free(prefixdir);
    return ZUNTRACEX(stat,"nkeys=%u",(unsigned)*nkeysp);
}

/*
Return a list of full keys  of legal objects immediately below a specified key.
Not necessarily sorted.
*/
int
NCZ_s3sdksearch(void* s3client0, const char* bucket, const char* prefixkey0, size_t* nkeysp, char*** keysp, char** errmsgp)
{
    int stat = NC_NOERR;
    size_t nkeys = 0;
    const char* prefix = NULL;
    char* prefixdir = NULL;
    char** keys = NULL;

    ZTRACE(11,"bucket=%s prefixkey0=%s",bucket,prefixkey0);

    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::ListObjectsV2Request objects_request;

    if(*prefixkey0 != '/') return ZUNTRACE(NC_EINTERNAL);
    /* Make sure that the prefix ends with '/' */
    if((stat = makes3keydir(prefixkey0,&prefixdir))) return ZUNTRACE(stat);        
    /* remove leading '/' */
    if((stat = makes3key(prefixdir,&prefix))) return ZUNTRACE(stat);

    if(errmsgp) *errmsgp = NULL;
    objects_request.SetBucket(bucket);
    objects_request.SetPrefix(prefix);
    /* Do not use delimiter so we get all full key paths */
    auto objects_outcome = s3client->ListObjectsV2(objects_request);
    if(objects_outcome.IsSuccess()) {
	size_t nkeys = 0;
        Aws::Vector<Aws::S3::Model::Object> object_list =
            objects_outcome.GetResult().GetContents();
        nkeys = (size_t)object_list.size();
        if((keys=(char**)calloc(sizeof(char*),(nkeys+1)))==NULL) /* NULL terminate list */
            stat = NC_ENOMEM;
        if(!stat)  {
	    int i;
            i = 0;
            for (auto const &s3obj : object_list) {
		const char* s;
	        const Aws::String& name = s3obj.GetKey();
		s = name.c_str();
		if(s != NULL) {
		    char* p;
		    size_t slen = name.length();
		    p = (char*)malloc(slen+1+1); /* nul plus leading '/' */
		    if(s[0] != '/') {p[0] = '/'; memcpy(p+1,s,slen); slen++;} else {memcpy(p,s,slen);}
		    p[slen] = '\0';
		    keys[i++] = p;
		}
	    }
	}
	if(stat == NC_NOERR) {
            if(nkeysp) {*nkeysp = nkeys;}
	    if(keysp) {*keysp = keys; keys = NULL;}
	}
	freestringenvv(keys);
    } else {
        if(errmsgp) *errmsgp = makeerrmsg(objects_outcome.GetError());
        stat = NC_ES3;
    }
    if(prefixdir) free(prefixdir);
    return ZUNTRACEX(stat,"nkeys=%u",(unsigned)*nkeysp);
}

int
NCZ_s3sdkdeletekey(void* s3client0, const char* bucket, const char* pathkey, char** errmsgp)
{
    int stat = NC_NOERR;
    const char* key = NULL;

    ZTRACE(11,"bucket=%s pathkey=%s",bucket,pathkey);

    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::DeleteObjectRequest delete_request;

    if(*pathkey != '/') return NC_EINTERNAL;
    if((stat = makes3key(pathkey,&key))) return ZUNTRACE(stat);
    /* Delete this key object */
    delete_request.SetBucket(bucket);
    delete_request.SetKey(key);
    auto delete_result = s3client->DeleteObject(delete_request);
    if(!delete_result.IsSuccess()) {
        if(errmsgp) *errmsgp = makeerrmsg(delete_result.GetError(),key);
        stat = NC_ES3;
    }
    return ZUNTRACE(stat);
}

/*
Get Info about a single object from a vector
*/
static int
s3objectinfo1(const Aws::S3::Model::Object& s3_object, char** fullkeyp, size64_t* lenp)
{
    int stat = NC_NOERR;
    char* cstr = NULL;

    if(fullkeyp) {
	const char* key = NULL;
	char* cstr = NULL;
        auto s = s3_object.GetKey();
	key = s.c_str();
	if(key[0] == '/') {
	    cstr = strdup(key);
	} else {
	    size_t len = strlen(key);
	    cstr = (char*)malloc(len+1+1);
	    if(cstr != NULL) {
	        cstr[0] = '/';
	        memcpy((void*)(cstr+1),(void*)key,len);	    
		cstr[len+1] = '\0';	    
	    }
	}
        if(cstr == NULL) {
            stat = NC_ENOMEM;
	} else if(fullkeyp) {
	    *fullkeyp = cstr;
	    cstr = NULL;
	}
    }
    if(!stat) {
        if(lenp) *lenp = (size64_t)s3_object.GetSize();
    }
    if(cstr) free(cstr);
    return stat;
}

/*
Get Info about a vector of objects; Keys are fixed up to start with a '/'
*/
static int
s3objectsinfo(Aws::Vector<Aws::S3::Model::Object> list, size_t* nkeysp, char*** keysp, size64_t** lenp)
{
    int stat = NC_NOERR;
    char** keys = NULL;
    size_t nkeys;
    size64_t *lengths = NULL;
    int i;

    nkeys = list.size();
    if(nkeysp) *nkeysp = nkeys;
    if((keys=(char**)calloc(sizeof(char*),(nkeys+1)))==NULL)
        stat = NC_ENOMEM;
    if(!stat) {
        if((lengths=(size64_t*)calloc(sizeof(size64_t),(nkeys)))==NULL)
            stat = NC_ENOMEM;
    }
    if(!stat)  {
        i = 0;
        for (auto const &s3_object : list) {
            stat = s3objectinfo1(s3_object,&keys[i],&lengths[i]);
            i++;
	    if(stat) break;
	}
    }
    if(!stat) {
        if(keysp) {keys[nkeys] = NULL; *keysp = keys; keys = NULL;}
        if(lenp) {*lenp = lengths; lengths = NULL;}
    }
    if(keys != NULL) freestringenvv(keys);
    if(lengths != NULL) free(lengths);
    return stat;
}

static int
s3commonprefixes(Aws::Vector<Aws::S3::Model::CommonPrefix> list, char*** keysp)
{
    int stat = NC_NOERR;
    char** keys = NULL;
    size_t nkeys;
    int i;

    nkeys = list.size();
    if((keys=(char**)calloc(sizeof(char*),(nkeys+1)))==NULL)
        stat = NC_ENOMEM;
    if(!stat)  {
        i = 0;
        for (auto const &s3prefix : list) {
	    char* p; char* p1;
	    size_t len;
	    const Aws::String& prefix = s3prefix.GetPrefix();
	    len = prefix.length();
	    if((p = (char*) malloc(len+1+1))==NULL) /* for nul + leading '/' */
		stat = NC_ENOMEM;
	    if(stat == NC_NOERR) {
		if(*p == '/') {p1 = p;} else {*p = '/'; p1 = p+1;}
		memcpy(p1,prefix.c_str(),len);
		p1[len] = '\0';
	        keys[i++] = p;
	    }
	}
    }
    if(keysp) {keys[nkeys] = NULL; *keysp = keys; keys = NULL;}
    if(keys != NULL) freestringenvv(keys);
    return stat;
}

static Aws::S3::Model::BucketLocationConstraint
s3findregion(const char* name)
{
    return Aws::S3::Model::BucketLocationConstraintMapper::GetBucketLocationConstraintForName(name);
}

struct MemBuf: std::streambuf {
    MemBuf(char* base, size_t size) {
        char* p(base);
        this->setg(p, p, p + size);
    }
};

static void
freestringenvv(char** ss)
{
    char** p;
    if(ss != NULL) {
        for(p=ss;*p;p++)
            if(*p) free(*p);
        free(ss);
    }
}

static int
makes3key(const char* pathkey, const char** keyp)
{
    assert(pathkey != NULL && pathkey[0] == '/');
    if(keyp) *keyp = pathkey+1;
    return NC_NOERR;    
}

static int
makes3keydir(const char* prefix, char** prefixdirp)
{
    size_t plen = strlen(prefix);
    char* prefixdir = (char*)malloc(plen+1+1); /* '/' + nul */
    if(prefixdir == NULL) return NC_ENOMEM;
    memcpy(prefixdir,prefix,plen);
    if(prefixdir[plen-1] != '/') {
	prefixdir[plen++] = '/';
    }
    prefixdir[plen] = '\0';
    *prefixdirp = prefixdir; prefixdir = NULL;
    return NC_NOERR;
}

static char**
mergekeysets(size_t nkeys1, char** keys1, size_t nkeys2, char** keys2)
{
    char** merge = NULL;
    char** p1;
    char** p2;

    merge = (char**)malloc(sizeof(char*)*(nkeys1+nkeys2+1));
    if(merge == NULL) return NULL;
    if(nkeys1 > 0) {
        memcpy(merge,keys1,sizeof(char*)*nkeys1);
        /* avoid double free */
        memset(keys1,0,sizeof(char*)*nkeys1);
    }
    if(nkeys2 > 0) {
        memcpy(merge+nkeys1,keys2,sizeof(char*)*nkeys2);
        /* avoid double free */
        memset(keys2,0,sizeof(char*)*nkeys2);
    }
    merge[nkeys1+nkeys2] = NULL;   
    return merge;
}
