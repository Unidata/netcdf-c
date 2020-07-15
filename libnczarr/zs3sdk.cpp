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
static void freestringenvv(char** ss);
static int makes3key(const char* pathkey, const char** keyp);
    
#ifndef nullfree
#define nullfree(x) {if(x) {free(x);}}
#endif

void
NCZ_s3sdkinitialize(void)
{
//    zs3options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Error;
    Aws::InitAPI(zs3options);
}

void
NCZ_s3sdkfinalize(void)
{
    Aws::ShutdownAPI(zs3options);
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


int
NCZ_s3sdkcreateconfig(const char* host, const char* region, void** configp)
{
    int stat = NC_NOERR;
    Aws::Client::ClientConfiguration *config = new Aws::Client::ClientConfiguration();
    config->scheme = Aws::Http::Scheme::HTTPS;
    config->connectTimeoutMs = 30000;
    config->requestTimeoutMs = 600000;
    if(region) config->region = region;
    if(host) config->endpointOverride = host;
    config->enableEndpointDiscovery = true;
    config->followRedirects = true;
    if(configp) * configp = config;
    return stat;
}

int
NCZ_s3sdkcreateclient(void* config0, void** clientp)
{
    Aws::Client::ClientConfiguration* config = (Aws::Client::ClientConfiguration*) config0;
    Aws::S3::S3Client *s3client
        = new Aws::S3::S3Client(*config,
                               Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::RequestDependent,
			       false,
			       Aws::S3::US_EAST_1_REGIONAL_ENDPOINT_OPTION::NOT_SET
			       );
    if(clientp) *clientp = (void*)s3client;
    return NC_NOERR;
}

int
NCZ_s3sdkbucketexists(void* s3client0, const char* bucket, int* existsp, char** errmsgp)
{
    int stat = NC_NOERR;
    int exists = 0;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;

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
    return stat;    
}

int
NCZ_s3sdkbucketcreate(void* s3client0, const char* region, const char* bucket, char** errmsgp)
{
    int stat = NC_NOERR;
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

    return stat;    
}

int
NCZ_s3sdkbucketdelete(void* s3client0, void* config0, const char* region, const char* bucket, char** errmsgp)
{
    int stat = NC_NOERR;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::Client::ClientConfiguration *config = (Aws::Client::ClientConfiguration*)config0;

    if(errmsgp) *errmsgp = NULL;
    const Aws::S3::Model::BucketLocationConstraint &awsregion = s3findregion(region);
    if(awsregion == Aws::S3::Model::BucketLocationConstraint::NOT_SET)
        return NC_EURL;
        /* Set up the request */
    Aws::S3::Model::DeleteBucketRequest request;
    request.SetBucket(bucket);
    if(region) {
	config->region = region; // Will this work?
    }
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

    return stat;    
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
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::HeadObjectRequest head_request;
    const char* key = NULL;

    if(*pathkey != '/') return NC_EINTERNAL;
    /* extract the true s3 key*/
    if((stat = makes3key(pathkey,&key))) return stat;

    if(errmsgp) *errmsgp = NULL;
    head_request.SetBucket(bucket);
    head_request.SetKey(key);
    auto head_outcome = s3client->HeadObject(head_request);
    if(head_outcome.IsSuccess()) {
	long long l  = head_outcome.GetResult().GetContentLength(); 
	if(lenp) *lenp = (size64_t)l;
    } else {
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
    return (stat);
}

/* Define object key 
@return NC_NOERR if success
@return NC_EXXX if fail
*/
int
NCZ_s3sdkcreatekey(void* s3client0, const char* bucket, const char* pathkey, char** errmsgp)
{
    int stat = NC_NOERR;
    const char* key = NULL;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::PutObjectRequest put_request;

    if(*pathkey != '/') return NC_EINTERNAL;
    if((stat = makes3key(pathkey,&key))) return stat;
    
    if(errmsgp) *errmsgp = NULL;
    put_request.SetBucket(bucket);
    put_request.SetKey(key);
    auto put_result = s3client->PutObject(put_request);
    if(!put_result.IsSuccess()) {
        if(errmsgp) *errmsgp = makeerrmsg(put_result.GetError(),key);
        stat = NC_ES3;
    }
    return (stat);
}

/*
@return NC_NOERR if success
@return NC_EXXX if fail
*/
int
NCZ_s3sdkread(void* s3client0, const char* bucket, const char* pathkey, size64_t start, size64_t count, void* content, char** errmsgp)
{
    int stat = NC_NOERR;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::GetObjectRequest object_request;
    char range[1024];
    const char* key = NULL;

    if(*pathkey != '/') return NC_EINTERNAL;
    if((stat = makes3key(pathkey,&key))) return stat;
    
    object_request.SetBucket(bucket);
    object_request.SetKey(key);
    snprintf(range,sizeof(range),"bytes=%llu-%llu",start,(start+count)-1);
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
    return (stat);
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
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::PutObjectRequest put_request;

    if(*pathkey != '/') return NC_EINTERNAL;
    if((stat = makes3key(pathkey,&key))) return stat;
    
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
    return (stat);
}

#if 0
/*
@return NC_NOERR if success
@return NC_EXXX if fail
*/
int
NCZ_s3sdkreadobject(void* s3client0, const char* bucket, const char* pathkey, size64_t* sizep, void** contentp, char** errmsgp)
{
    int stat = NC_NOERR;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::GetObjectRequest object_request;
    size64_t size, red;
    void* content = NULL;
    const char* key = NULL;

    if(*pathkey != '/') return NC_EINTERNAL;
    if((stat = makes3key(pathkey,&key))) return stat;

    if(errmsgp) *errmsgp = NULL;
    object_request.SetBucket(bucket);
    object_request.SetKey(key);
    auto get_result = s3client->GetObject(object_request);
    if(!get_result.IsSuccess()) {
	if(errmsgp) *errmsgp = makeerrmsg(get_result.GetError(),key);
        stat = NC_ES3;
    } else {
	/* Get the size */
	size = (size64_t)get_result.GetResult().GetContentLength();	
	/* Get the whole result */
        Aws::IOStream &result = get_result.GetResultWithOwnership().GetBody();
#if 0
        std::string str((std::istreambuf_iterator<char>(result)),std::istreambuf_iterator<char>());
	red = str.length();
	if((content = malloc(red))==NULL)
	    {stat = NC_ENOMEM; goto done;}
	memcpy(content,str.c_str(),red);
#else
        red = result.rdbuf()->pubseekoff(0,std::ios::ios_base::end);
        result.rdbuf()->pubseekoff(0,std::ios::ios_base::beg); /* reset for reading */
	if((content = malloc(red))==NULL)
	    stat = NC_ENOMEM;
	else
   	    result.rdbuf()->sgetn((char*)content,red);
#endif
	if(!stat) {
  	    /* Verify actual result size */
	    if(red != size) {stat = NC_ES3; goto done;}
	    if(sizep) *sizep = red;
	    if(contentp) {*contentp = content; content = NULL;}
	}
    }
done:
    nullfree(content);
    return (stat);
}
#endif /*0*/

int
NCZ_s3sdkclose(void* s3client0, void* config0, const char* bucket, const char* rootkey, int deleteit, char** errmsgp)
{
    int stat = NC_NOERR;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::Client::ClientConfiguration *config = (Aws::Client::ClientConfiguration*)config0;
    if(deleteit) {
        /* Delete the root key; ok it if does not exist */
        switch (stat = NCZ_s3sdkdeletekey(s3client0,bucket,rootkey,errmsgp)) {
        case NC_NOERR: break;
        case NC_EEMPTY: case NC_ENOTFOUND: stat = NC_NOERR; break;
        default: break;
        }
    }
    delete s3client;
    delete config;
    return (stat);
}

/*
Return a list of keys for objects "below" a specified rootkey.
In theory, the returned list should be sorted in lexical order,
but it possible that it is not.
*/
int
NCZ_s3sdkgetkeys(void* s3client0, const char* bucket, const char* rootkey0, size_t* nkeysp, char*** keysp, char** errmsgp)
{
    int stat = NC_NOERR;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    size_t nkeys = 0;
    const char* rootkey = NULL;
    Aws::S3::Model::ListObjectsV2Request objects_request;

    if(*rootkey0 != '/') return NC_EINTERNAL;
    if((stat = makes3key(rootkey0,&rootkey))) return stat;

    if(errmsgp) *errmsgp = NULL;
    objects_request.SetBucket(bucket);
    objects_request.SetPrefix(rootkey);
    auto objects_outcome = s3client->ListObjectsV2(objects_request);
    if(objects_outcome.IsSuccess()) {
        Aws::Vector<Aws::S3::Model::Object> object_list =
            objects_outcome.GetResult().GetContents();
        nkeys = (size_t)object_list.size();
        if(nkeysp) *nkeysp = nkeys;
        stat = s3objectsinfo(object_list,NULL,keysp,NULL);
    } else {
        if(errmsgp) *errmsgp = makeerrmsg(objects_outcome.GetError());
        stat = NC_ES3;
    }
    return stat;
}

int
NCZ_s3sdkdeletekey(void* s3client0, const char* bucket, const char* pathkey, char** errmsgp)
{
    int stat = NC_NOERR;
    Aws::S3::S3Client* s3client = (Aws::S3::S3Client*)s3client0;
    Aws::S3::Model::DeleteObjectRequest delete_request;
    const char* key = NULL;

    if(*pathkey != '/') return NC_EINTERNAL;
    if((stat = makes3key(pathkey,&key))) return stat;
    /* Delete this key object */
    delete_request.SetBucket(bucket);
    delete_request.SetKey(key);
    auto delete_result = s3client->DeleteObject(delete_request);
    if(!delete_result.IsSuccess()) {
        if(errmsgp) *errmsgp = makeerrmsg(delete_result.GetError(),key);
        stat = NC_ES3;
    }
    return stat;
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
    nullfree(cstr);
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
            nullfree(*p);
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
