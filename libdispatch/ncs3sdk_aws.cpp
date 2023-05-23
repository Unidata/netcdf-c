/*
 *	Copyright 2018, University Corporation for Atmospheric Research
 *      See netcdf/COPYRIGHT file for copying and redistribution conditions.
 */

/* WARNING: changes to this file may need to be propagated to libsrc/s3sdk.cpp */

#define NOOP
#define DEBUG

#include "awsincludes.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <streambuf>
#include "netcdf.h"
#include "ncrc.h"

#include "ncs3sdk.h"

#undef NCTRACING
#ifdef NCTRACING
#define NCTRACE(level,fmt,...) nctrace((level),__func__,fmt,##__VA_ARGS__)
#define NCTRACEMORE(level,fmt,...) nctracemore((level),fmt,##__VA_ARGS__)
#define NCUNTRACE(e) ncuntrace(__func__,THROW(e),NULL)
#define NCUNTRACEX(e,fmt,...) ncuntrace(__func__,THROW(e),fmt,##__VA_ARGS__)
#else
#define NCTRACE(level,fmt,...)
#define NCTRACEMORE(level,fmt,...)
#define NCUNTRACE(e) (e)
#define NCUNTRACEX(e,fmt,...) (e)
#endif

#ifdef __CYGWIN__
extern char* strdup(const char*);
#endif

#define size64_t unsigned long long

struct KeySet {
    size_t nkeys;
    size_t alloc;
    char** keys;
    KeySet() {nkeys = 0; alloc = 0; keys = NULL;}
    ~KeySet() {clear();}
    void push(char* key) {
       if(alloc == 0) {
           keys = (char**)calloc(10,sizeof(char*));
	   alloc = 10;
       }
       if(nkeys >= alloc) {
           char** newkeys = (char**)calloc(alloc*2,sizeof(char*));
	   memcpy(newkeys,keys,sizeof(char*)*nkeys);
	   alloc *= 2;
	   free(keys);
	   keys = newkeys;
	}     
	keys[nkeys++] = key;
    }
    char** extractkeys() {char** outkeys = keys; keys = NULL; clear(); return outkeys;}
    char* extractithkey(size_t i) {
        if(i >= nkeys) return NULL;
	char* k = keys[i];
	keys[i] = NULL;
	return k;
    }
    size_t getnkeys() {return nkeys;}
    void clear() {
        if(keys) {
	    for(size_t i=0;i<nkeys;i++)
	        {if(keys[i] != NULL) free(keys[i]);}
            free(keys);
        }
	nkeys = 0;
	alloc = 0;
	keys = NULL;
    }
};

static int ncs3_initialized = 0;
static int ncs3_finalized = 0;

static Aws::SDKOptions ncs3options;

/* Forward */
static Aws::S3::Model::BucketLocationConstraint s3findregion(const char* name);
static int s3objectinfo1(const Aws::S3::Model::Object& s3_object, char** fullkeyp);
static int s3objectsinfo(Aws::Vector<Aws::S3::Model::Object> list, KeySet* keys);
static int s3commonprefixes(Aws::Vector<Aws::S3::Model::CommonPrefix> list, KeySet* keys);
static int makes3key(const char* pathkey, const char** keyp);
static int makes3keydir(const char* prefix, char** prefixdirp);
static int mergekeysets(KeySet* keys1, KeySet* keys2, KeySet* merge);
    
const char*
NCS3_dumps3info(NCS3INFO* info)
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

EXTERNL int
NC_s3sdkinitialize(void)
{
    if(!ncs3_initialized) {
	ncs3_initialized = 1;
	ncs3_finalized = 0;
        NCTRACE(11,NULL);
	Aws::InitAPI(ncs3options);
#ifdef DEBUG
	ncs3options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;
#endif
    }
    return NCUNTRACE(NC_NOERR);
}

EXTERNL int
NC_s3sdkfinalize(void)
{
    if(!ncs3_finalized) {
	ncs3_initialized = 0;
	ncs3_finalized = 1;
        NCTRACE(11,NULL);
        Aws::ShutdownAPI(ncs3options);
    }
    return NCUNTRACE(NC_NOERR);
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


static Aws::Client::ClientConfiguration
s3sdkcreateconfig(NCS3INFO* info)
{
    int stat = NC_NOERR;
    NCTRACE(11,"info=%s", dumps3info(info));

    Aws::Client::ClientConfiguration config;

    if(info->profile)
        config.profileName = info->profile;
    config.scheme = Aws::Http::Scheme::HTTPS;
    config.connectTimeoutMs = 300000;
    config.requestTimeoutMs = 600000;
    if(info->region) config.region = info->region;
    if(info->host) config.endpointOverride = info->host;
    config.enableEndpointDiscovery = true;
    config.followRedirects = Aws::Client::FollowRedirectsPolicy::ALWAYS;
    stat = NCUNTRACE(stat);
    return config;
}

EXTERNL void*
NC_s3sdkcreateclient(NCS3INFO* info)
{
    int stat = NC_NOERR;
    NCTRACE(11,NULL);

    Aws::Client::ClientConfiguration config = s3sdkcreateconfig(info);
    Aws::S3::S3Client *s3client;

    if(info->profile == NULL || strcmp(info->profile,"none")==0) {
        Aws::Auth::AWSCredentials creds;
        creds.SetAWSAccessKeyId(Aws::String(""));
        creds.SetAWSSecretKey(Aws::String(""));
        s3client = new Aws::S3::S3Client(creds,config,
                               Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::RequestDependent,
			       false);
    } else {
        s3client = new Aws::S3::S3Client(config,
                               Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::RequestDependent,
			       false);
    }
//    delete config;
    stat = NCUNTRACE(stat);
    return (void*)s3client;
}

EXTERNL int
NC_s3sdkbucketexists(void* s3client0, const char* bucket, int* existsp, char** errmsgp)
{
    int stat = NC_NOERR;
    int exists = 0;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;

    NCTRACE(11,"bucket=%s",bucket);
    if(errmsgp) *errmsgp = NULL;
#if 0
    char* errmsg = NULL;
    auto result = s3client->ListBuckets();
    if(!result.IsSuccess()) {
	auto re = result.GetError();
        errmsg = makeerrmsg(re);
	fprintf(stderr,">>> errmsg=%s\n",errmsg);
	if(errmsgp) {*errmsgp = errmsg; errmsg = NULL;}
	stat = NC_ES3;
    } else {
        Aws::Vector<Aws::S3::Model::Bucket> bucket_list = result.GetResult().GetBuckets();
        for(auto const &awsbucket : bucket_list) {
	   auto name = awsbucket.GetName();
	   if(name == bucket) {exists = 1; break;}
	}
    }
#else
    Aws::S3::Model::HeadBucketRequest request;
    request.SetBucket(bucket);
    auto result = s3client->HeadBucket(request);
    exists = result.IsSuccess() ? 1 : 0;
#endif
    if(existsp) *existsp = exists;
    return NCUNTRACEX(stat,"exists=%d",*existsp);
}

EXTERNL int
NC_s3sdkbucketcreate(void* s3client0, const char* region, const char* bucket, char** errmsgp)
{
    int stat = NC_NOERR;
    
    NCTRACE(11,"region=%s bucket=%s",region,bucket);
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

    return NCUNTRACE(stat);    
}

EXTERNL int
NC_s3sdkbucketdelete(void* s3client0, NCS3INFO* info, char** errmsgp)
{
    int stat = NC_NOERR;

    NCTRACE(11,"info=%s%s",dumps3info(info));

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

    return NCUNTRACE(stat);    
}

/**************************************************/
/* Object API */

/*
@return NC_NOERR if key points to a content-bearing object.
@return NC_EEMPTY if object at key has no content.
@return NC_EXXX return true error
*/
EXTERNL int
NC_s3sdkinfo(void* s3client0, const char* bucket, const char* pathkey, size64_t* lenp, char** errmsgp)
{
    int stat = NC_NOERR;
    const char* key = NULL;

    NCTRACE(11,"bucket=%s pathkey=%s",bucket,pathkey);

    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::HeadObjectRequest head_request;
    if(*pathkey != '/') return NC_EINTERNAL;
    /* extract the true s3 key*/
    if((stat = makes3key(pathkey,&key))) return NCUNTRACE(stat);

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
    return NCUNTRACEX(stat,"len=%d",(int)(lenp?*lenp:-1));
}

/*
@return NC_NOERR if success
@return NC_EXXX if fail
*/
EXTERNL int
NC_s3sdkread(void* s3client0, const char* bucket, const char* pathkey, size64_t start, size64_t count, void* content, char** errmsgp)
{
    int stat = NC_NOERR;
    char range[1024];
    const char* key = NULL;
    size64_t rangeend;

    NCTRACE(11,"bucket=%s pathkey=%s start=%llu count=%llu content=%p",bucket,pathkey,start,count,content);

    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::GetObjectRequest object_request;

    if(count == 0) return NCUNTRACE(stat);

    if(*pathkey != '/') return NC_EINTERNAL;
    if((stat = makes3key(pathkey,&key))) return NCUNTRACE(stat);
    
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
    return NCUNTRACE(stat);
}

/*
For S3, I can see no way to do a byterange write;
so we are effectively writing the whole object
*/
EXTERNL int
NC_s3sdkwriteobject(void* s3client0, const char* bucket, const char* pathkey,  size64_t count, const void* content, char** errmsgp)
{
    int stat = NC_NOERR;
    const char* key = NULL;

    NCTRACE(11,"bucket=%s pathkey=%s count=%lld content=%p",bucket,pathkey,count,content);
    
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::PutObjectRequest put_request;

    if(*pathkey != '/') return NCUNTRACE(NC_EINTERNAL);
    if((stat = makes3key(pathkey,&key))) return NCUNTRACE(stat);
    
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
    return NCUNTRACE(stat);
}

EXTERNL int
NC_s3sdkclose(void* s3client0, NCS3INFO* info, int deleteit, char** errmsgp)
{
    int stat = NC_NOERR;

    NCTRACE(11,"info=%s rootkey=%s deleteit=%d",dumps3info(info),deleteit);
    
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    if(deleteit) {
        /* Delete the root key; ok it if does not exist */
        switch (stat = NC_s3sdkdeletekey(s3client0,info->bucket,info->rootkey,errmsgp)) {
        case NC_NOERR: break;
        case NC_EEMPTY: case NC_ENOTFOUND: stat = NC_NOERR; break;
        default: break;
        }
    }
    delete s3client;
    return NCUNTRACE(stat);
}

/*
Return a list of names of legal objects immediately below a specified key.
In theory, the returned list should be sorted in lexical order,
but it possible that it is not.
*/
static int
getkeys(void* s3client0, const char* bucket, const char* prefixkey0, const char* delim, size_t* nkeysp, char*** keysp, char** errmsgp)
{
    int stat = NC_NOERR;
    const char* prefix = NULL;
    char* prefixdir = NULL;
    bool istruncated = false;
    char* continuetoken = NULL;
    Aws::S3::S3Client* s3client = NULL;
    KeySet commonkeys;
    KeySet realkeys;
    KeySet allkeys;
    
    NCTRACE(11,"bucket=%s prefixkey0=%s",bucket,prefixkey0);
    
    if(*prefixkey0 != '/') {stat = NC_EINTERNAL; goto done;}
    if(errmsgp) *errmsgp = NULL;

    s3client = (Aws::S3::S3Client*)s3client0;

    do {
        Aws::S3::Model::ListObjectsV2Request objects_request;
        if(prefixdir != NULL) free(prefixdir);
	realkeys.clear();
	commonkeys.clear();
        /* Make sure that the prefix ends with '/' */
        if((stat = makes3keydir(prefixkey0,&prefixdir))) goto done;
        /* remove leading '/' */
        if((stat = makes3key(prefixdir,&prefix))) goto done;
        objects_request.SetBucket(bucket);
        objects_request.SetPrefix(prefix);
        if(delim != NULL) {
            objects_request.SetDelimiter(delim); /* Force return of common prefixes */
        }
	if(istruncated && continuetoken != NULL) {
            objects_request.SetContinuationToken(continuetoken);
            free(continuetoken); continuetoken = NULL;
        }
        auto objects_outcome = s3client->ListObjectsV2(objects_request);
        if(objects_outcome.IsSuccess()) {
	    const Aws::S3::Model::ListObjectsV2Result& result = objects_outcome.GetResult();
	    istruncated = result.GetIsTruncated();
	    if(istruncated)
		continuetoken = strdup(result.GetNextContinuationToken().c_str());
            Aws::Vector<Aws::S3::Model::Object> object_list =
                objects_outcome.GetResult().GetContents();
            if((stat = s3objectsinfo(object_list,&realkeys))) goto done;
	    /* Add common prefixes */
            Aws::Vector<Aws::S3::Model::CommonPrefix> common_list =
                objects_outcome.GetResult().GetCommonPrefixes();
            if((stat = s3commonprefixes(common_list,&commonkeys))) goto done;
	    /* merge the two lists; target list might not be empty */
	    if((stat=mergekeysets(&realkeys, &commonkeys, &allkeys))) goto done;
	} else {
            if(errmsgp) *errmsgp = makeerrmsg(objects_outcome.GetError());
            stat = NC_ES3;
            goto done;
        }
    } while(istruncated);
    if(nkeysp) {*nkeysp = allkeys.getnkeys();}
    if(keysp) {*keysp = allkeys.extractkeys();}
done:
    realkeys.clear();
    commonkeys.clear();
    allkeys.clear();
    if(prefixdir != NULL) free(prefixdir);
    if(continuetoken != NULL) free(continuetoken);
    if(stat)
        return NCUNTRACE(stat);        
    else
	return NCUNTRACEX(stat,"nkeys=%u",(unsigned)*nkeysp);
}

/*
Return a list of full keys  of legal objects immediately below a specified key.
Not necessarily sorted.
*/
EXTERNL int
NC_s3sdkgetkeys(void* s3client0, const char* bucket, const char* prefixkey0, size_t* nkeysp, char*** keysp, char** errmsgp)
{
    return getkeys(s3client0, bucket, prefixkey0, "/", nkeysp, keysp, errmsgp);
}

/*
Return a list of full keys  of legal objects immediately below a specified key.
Not necessarily sorted.
*/
EXTERNL int
NC_s3sdksearch(void* s3client0, const char* bucket, const char* prefixkey0, size_t* nkeysp, char*** keysp, char** errmsgp)
{
    return getkeys(s3client0, bucket, prefixkey0, NULL, nkeysp, keysp, errmsgp);
}

EXTERNL int
NC_s3sdkdeletekey(void* s3client0, const char* bucket, const char* pathkey, char** errmsgp)
{
    int stat = NC_NOERR;
    const char* key = NULL;

    NCTRACE(11,"bucket=%s pathkey=%s",bucket,pathkey);

    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::DeleteObjectRequest delete_request;

    assert(pathkey != NULL && *pathkey == '/');
    if((stat = makes3key(pathkey,&key))) return NCUNTRACE(stat);
    /* Delete this key object */
    delete_request.SetBucket(bucket);
    delete_request.SetKey(key);
    auto delete_result = s3client->DeleteObject(delete_request);
    if(!delete_result.IsSuccess()) {
        if(errmsgp) *errmsgp = makeerrmsg(delete_result.GetError(),key);
        stat = NC_ES3;
    }
    return NCUNTRACE(stat);
}

/*
Get Info about a single object from a vector
*/
static int
s3objectinfo1(const Aws::S3::Model::Object& s3_object, char** fullkeyp)
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
    if(cstr) free(cstr);
    return stat;
}

/*
Get Info about a vector of objects; Keys are fixed up to start with a '/'
*/
static int
s3objectsinfo(Aws::Vector<Aws::S3::Model::Object> list, KeySet* keys)
{
    int stat = NC_NOERR;
    int i;

    i = 0;
    for (auto const &s3_object : list) {
	char* akey = NULL;
        stat = s3objectinfo1(s3_object,&akey);
        i++;
        if(stat) break;
	keys->push(akey);
    }
    return stat;
}

static int
s3commonprefixes(Aws::Vector<Aws::S3::Model::CommonPrefix> list, KeySet* keys)
{
    int stat = NC_NOERR;

    for (auto const &s3prefix : list) {
        char* p; const char* px; char* p1;
	size_t len, alloc;
	const Aws::String& prefix = s3prefix.GetPrefix();
	len = prefix.length();
	alloc = len + 1 + 1; /* for nul + leading '/' */
	if((p = (char*) malloc(alloc))==NULL)
	    stat = NC_ENOMEM;
	if(stat == NC_NOERR) {
	    px = prefix.c_str();
	    p1 = p;
	    if(*px != '/') *p1++ = '/';
	    memcpy(p1,px,len);
	    p1 += len;
	    *p1 = '\0';
	    keys->push(p);
	}
    }
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

#if 0
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
#endif

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

static int
mergekeysets(KeySet* keys1, KeySet* keys2, KeySet* merge)
{
    size_t i;
    for(i=0;i<keys1->getnkeys();i++)
	merge->push(keys1->extractithkey(i));
    for(i=0;i<keys2->getnkeys();i++)
	merge->push(keys2->extractithkey(i));
    return NC_NOERR;
}
