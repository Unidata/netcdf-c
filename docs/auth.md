Authorization Support in the netDF-C Libraries {#auth}
=============================================

\tableofcontents

It is possible to support a number of authorization schemes in the netCDF-C library.

With one exception, authorization in the netCDF-C library is delegated to the oc2 code, which in turn delegates it to the libcurl library. The exception is that the location of the rc file can be specified by setting the environment variable *NCRCFILE*.

> Note that the value of this environment variable should be the absolute path of the rc file, not the path to its containing directory.

Following is the authorization documentation as taken from the oc library.


    Author: Dennis Heimbigner
    Email: dmh at ucar dot edu
    Draft: 11/21/2014
    Last Revised: 12/23/2014
    OC Version 2.1


# Introduction {#auth-introduction}

OC can support user authorization using those provided by the curl library. This includes basic password authentication as well as certificate-based authorization.

With some exceptions (e.g. see the section on [redirection](#auth_REDIR)) The libcurl authorization mechanisms can be accessed in two ways

1.  Inserting the username and password into the url, or
2.  Accessing information from a so-called *rc* file named either *.daprc* or *.dodsrc*

# URL-Based Authentication {#auth_URL_AUTH}


For simple password based authentication, it is possible to directly insert the username and the password into a url in this form.

        http://username:password@host/...

This username and password will be used if the server asks for authentication. Note that only simple password authentication is supported in this format. Specifically note that [redirection](#auth_REDIR) based authorization will not work with this.

# RC File Authentication {#auth_DODSRC}


The oc library supports an *rc* file mechanism to allow the passing of a number of parameters to liboc and libcurl.

The file must be called one of the following names: ".daprc" or ".dodsrc" If both .daprc and .dodsrc exist, then the .daprc file will take precedence.

Searching for the rc file first looks in the current directory and then in the home directory (as defined by the HOME environment variable). It is also possible to specify a direct path using the *-R* option to ocprint or using the *oc\_set\_rcfile* procedure (see oc.h). Note that for these latter cases, the path must be to the file itself, not to the containing directory.

The rc file format is a series of lines of the general form:

    [<host:port>]<key>=<value>

where the bracket-enclosed host:port is optional and will be discussed subsequently.

The currently defined set of authorization-related keys are as follows.
The second column is the affected curl\_easy\_setopt option(s).

Key |  curl\_easy\_setopt Option
---- | ----
HTTP.COOKIEJAR | CURLOPT\_COOKIEJAR, CURLOPT\_COOKIEFILE
HTTP.PROXY\_SERVER | CURLOPT\_PROXY, CURLOPT\_PROXYPORT, CURLOPT\_PROXYUSERPWD
HTTP.SSL.CERTIFICATE | CURLOPT\_SSLCERT
HTTP.SSL.KEY |CURLOPT\_SSLKEY
HTTP.SSL.KEYPASSWORD | CURLOPT\_KEYPASSWORD
HTTP.SSL.CAINFO |  CURLOPT\_SSLCAINFO
HTTP.SSL.CAPATH | CURLOPT\_SSLCAPATH
HTTP.SSL.VERIFYPEER | CURLOPT\_SSL\_VERIFYPEER
HTTP.CREDENTIALS.USERPASSWORD | CURLOPT\_USERPASSWORD

## Password Authentication {#auth_password_authentication}

The key HTTP.CREDENTIALS.USERPASSWORD can be used to set the simple password authentication. This is an alternative to setting it in the url. The value must be of the form "username:password". 

## Cookie Jar {#auth_cookie_jar}

The `HTTP.COOKIEJAR` key specifies the name of file from which to read cookies (`CURLOPT_COOKIEJAR`) and also the file into which to store cookies (`CURLOPT_COOKIEFILE`). The same value is used for both CURLOPT values. It defaults to in-memory storage.

## Certificate Authentication {#auth_certificate_authentication}

`HTTP.SSL.CERTIFICATE` specifies a file path for a file containing a PEM cerficate. This is typically used for client-side authentication.

`HTTP.SSL.KEY` is essentially the same as HTTP.SSL.CERTIFICATE and should usually have the same value.

`HTTP.SSL.KEYPASSWORD` specifies the password for accessing the HTTP.SSL.KEY HTTP.SSL.CERTIFICATE file.

`HTTP.SSL.CAPATH` specifies the path to a directory containing trusted certificates for validating server sertificates.

`HTTP.SSL.VALIDATE` is a boolean (1/0) value that if true (1) specifies that the client should verify the server's presented certificate.

`HTTP.PROXY_SERVER` specified the url for accessing the proxy: (e.g.http://[username:password@]host[:port])

## Redirection-Based Authentication {#auth_REDIR}


Some sites provide authentication by using a third party site to to the authentication. One example is [URS](https://uat.urs.earthdata.nasa.gov), the EOSDIS User Registration System.

The process is usually as follows.

1.  The client contacts the server of interest (SOI), the actual data provider.
2.  The SOI sends a redirect to the client to connect to the URS system.
3.  The client authenticates with URS.
4.  URS sends a redirect (with authorization information) to send the client back to the SOI to actually obtain the data.

In order for this to work with libcurl, the client will usually need to provide a .netrc` file so that the redirection will work correctly. The format of this `.netrc` file will contain content that typically look like this.

    machine uat.urs.earthdata.nasa.gov login xxxxxx password yyyyyy

where the machine is the one to which the client is redirected for authorization, and the login and password are those needed to authenticate.

The .netrc file can be specified in two ways.

1.  Specify the netrc file to liboc using the procedure in oc.h:

        oc_set_netrc(OClink* link, const char* file)

    (This is equivalent to the -N flag to ocprint).

2.  Put the following line in your .daprc/.dodsrc file.

        HTTP.NETRC=<path to netrc file>

One final note. In using this, it is probable that you will need to specify a cookie jar (HTTP.COOKIEJAR) so that the redirect site can pass back authorization information.

## URL Constrained RC File Entries {#auth_URLCONS}


Each line of the rc file can begin with a host+port enclosed in square brackets. The form is "host:port". If the port is not specified then the form is just "host". The reason that more of the url is not used is that libcurl's authorization grain is not any finer than host level.

Examples.

    [remotetest.unidata.ucar.edu]HTTP.VERBOSE=1
    or
    [fake.ucar.edu:9090]HTTP.VERBOSE=0

If the url request from, say, the *oc\_open* method has a host+port matchine one of the prefixes in the rc file, then the corresponding entry will be used, otherwise ignored.

For example, the URL

    http://remotetest.unidata.ucar.edu/thredds/dodsC/testdata/testData.nc

will have HTTP.VERBOSE set to 1.

Similarly,

    http://fake.ucar.edu:9090/dts/test.01

will have HTTP.VERBOSE set to 0.

# Client-Side Certificates {#auth_CLIENTCERTS}

Some systems, notably ESG (Earth System Grid), requires the use of client-side certificates, as well as being [re-direction based](#auth_REDIR). This requires setting the following entries:

-   `HTTP.COOKIEJAR` — a file path for storing cookies across re-direction.
-   `HTTP.NETRC` — the path to the netrc file.
-   `HTTP.SSL.CERTIFICATE` — the file path for the client side certificate file.
-   `HTTP.SSL.KEY` — this should have the same value as `HTTP.SSL.CERTIFICATE`.
-   `HTTP.SSL.CAPATH` — the path to a "certificates" directory.
-   `HTTP.SSL.VALIDATE` — force validation of the server certificate.

Note that the first two are to support re-direction based authentication.

# Appendix A. All RC-File Keys {#auth_allkeys}

For completeness, this is the list of all rc-file keys.

Key | curl\_easy\_setopt Option
---- | ----
HTTP.DEFLATE | CUROPT\_DEFLATE <br>with value "deflate,gzip"
HTTP.VERBOSE | CUROPT\_VERBOSE
HTTP.TIMEOUT |CUROPT\_TIMEOUT
HTTP.USERAGENT | CUROPT\_USERAGENT
HTTP.COOKIEJAR | CUROPT\_COOKIEJAR
HTTP.COOKIE\_JAR | CUROPT\_COOKIEJAR
HTTP.PROXY\_SERVER | CURLOPT\_PROXY, <br>CURLOPT\_PROXYPORT, <br>CURLOPT\_PROXYUSERPWD
HTTP.SSL.CERTIFICATE |  CUROPT\_SSLCERT
HTTP.SSL.KEY | CUROPT\_SSLKEY
HTTP.SSL.KEYPASSWORD | CUROPT\_KEYPASSWORD
HTTP.SSL.CAINFO | CUROPT\_SSLCAINFO
HTTP.SSL.CAPATH | CUROPT\_SSLCAPATH
HTTP.SSL.VERIFYPEER |  CUROPT\_SSL\_VERIFYPEER
HTTP.CREDENTIALS.USERPASSWORD | CUROPT\_USERPASSWORD
HTTP.NETRC | CURLOPT\_NETRC,<br>CURLOPT\_NETRC\_FILE

# Appendix B. ESG Access in Detail {#auth_ESGDETAIL}

It is possible to access Earth Systems Grid (ESG) datasets from ESG servers through the C API using the techniques described in the section on [Client-Side Certificates](#auth_CLIENTCERTS).

In order to access ESG datasets, however, it is necessary to register as a user with ESG and to setup your environment so that proper authentication is established between an oc client program and the ESG data server. Specifically, it is necessary to use what is called "client-side keys" to enable this authentication. Normally, when a client accesses a server in a secure fashion (using "https"), the server provides an authentication certificate to the client. With client-side keys, the client must also provide a certificate to the server so that the server can know with whom it is communicating.

The oc library uses the *curl* library and it is that underlying library that must be properly configured.

## Terminology {#auth_terminology}

The key elements for client-side keys requires the constructions of two "stores" on the client side.

-   `Keystore` - a repository to hold the client side key.
-   `Truststore` - a repository to hold a chain of certificates that can
    be used to validate the certificate sent by the server to the
    client.

The server actually has a similar set of stores, but the client need not be concerned with those.

## Initial Steps {#auth_initial_steps}

The first step is to obtain authorization from ESG. Note that this information may evolve over time, and may be out of date. This discussion is in terms of BADC and NCSA. You will need to substitute as necessary.

1.  Register at http://badc.nerc.ac.uk/register to obtain access to badc and to obtain an openid, which will looks something like:

        https://ceda.ac.uk/openid/Firstname.Lastname

2.  Ask BADC for access to whatever datasets are of interest.
3.  Obtain short term credentials at http://grid.ncsa.illinois.edu/myproxy/MyProxyLogon/ You will need to download and run the MyProxyLogon program. This will create a keyfile in, typically, the directory ".globus". The keyfile will have a name similar to this: "x509up\_u13615" The other elements in ".globus" are certificates to use in validating the certificate your client gets from the server.
4.  Obtain the program source ImportKey.java from this location: http://www.agentbob.info/agentbob/79-AB.html (read the whole page, it will help you understand the remaining steps).

## Building the KeyStore {#auth_build_keystore}

You will have to modify the keyfile in the previous step and then create
a keystore and install the key and a certificate. The commands are
these:

~~~.bash
        openssl pkcs8 -topk8 -nocrypt -in x509up_u13615 -inform PEM -out key.der -outform DER

        openssl x509 -in x509up_u13615 -inform PEM -out cert.der -outform DER

        java -classpath  -Dkeypassword="" -Dkeystore=./ key.der cert.der
~~~

Note, the file names "key.der" and "cert.der" can be whatever you choose. It is probably best to leave the .der extension, though.

### Building the TrustStore {#auth_build_truststore}

Building the truststore is a bit tricky because as provided, the certificates in ".globus" need some massaging. See the script below for the details. The primary command is this, which is executed for every certificate, c, in globus. It sticks the certificate into the file named "truststore"

~~~.bash
      keytool -trustcacerts -storepass "password" -v -keystore "truststore"  -importcert -file "${c}"
~~~

## Running the C Client {#auth_run_c_client}

Refer to the section on [Client-Side Certificates](#auth_CLIENTCERTS). The keys specified there must be set in the rc file to support ESG access.

-   `HTTP.COOKIEJAR`=\~/.dods\_cookies
-   `HTTP.NETRC`=\~/.netrc
-   `HTTP.SSL.CERTIFICATE`=\~/esgkeystore
-   `HTTP.SSL.KEY`=\~/esgkeystore
-   `HTTP.SSL.CAPATH`=\~/.globus
-   `HTTP.SSL.VALIDATE`=1

Of course, the file paths above are suggestions only; you can modify as needed. The `HTTP.SSL.CERTIFICATE` and `HTTP.SSL.KEY` entries should have same value, which is the file path for the certificate produced by MyProxyLogon. The `HTTP.SSL.CAPATH` entry should be the path to the "certificates" directory produced by MyProxyLogon.

As noted, also uses re-direction based authentication. So, when it receives an initial connection from a client, it redirects to a separate authentication server. When that server has authenticated the client, it redirects back to the original url to complete the request.

### Script for creating Stores {#auth_script_for_creating_stores}

The following script shows in detail how to actually construct the key
and trust stores. It is specific to the format of the globus file as it
was when ESG support was first added. It may have changed since then, in
which case, you will need to seek some help in fixing this script. It
would help if you communicated what you changed to the author so this
document can be updated.

~~~.bash
    #!/bin/sh -x
    KEYSTORE="esgkeystore"
    TRUSTSTORE="esgtruststore"
    GLOBUS="globus"
    TRUSTROOT="certificates"
    CERT="x509up_u13615"
    TRUSTROOTPATH="$GLOBUS/$TRUSTROOT"
    CERTFILE="$GLOBUS/$CERT"
    PWD="password"

    D="-Dglobus=$GLOBUS"
    CCP="bcprov-jdk16-145.jar"
    CP="./build:${CCP}"
    JAR="myproxy.jar"

    # Initialize needed directories
    rm -fr build
    mkdir build
    rm -fr $GLOBUS
    mkdir $GLOBUS
    rm -f $KEYSTORE
    rm -f $TRUSTSTORE

    # Compile MyProxyCmd and ImportKey
    javac -d ./build -classpath "$CCP" *.java
    javac -d ./build ImportKey.java

    # Execute MyProxyCmd
    java -cp "$CP myproxy.MyProxyCmd

    # Build the keystore
    openssl pkcs8 -topk8 -nocrypt -in $CERTFILE -inform PEM -out key.der -outform DER
    openssl x509 -in $CERTFILE -inform PEM -out cert.der -outform DER
    java -Dkeypassword=$PWD -Dkeystore=./${KEYSTORE} -cp ./build ImportKey key.der cert.der

    # Clean up the certificates in the globus directory
    for c in ${TRUSTROOTPATH}/*.0 ; do
        alias=`basename $c .0`
        sed -e '0,/---/d' <$c >/tmp/${alias}
        echo "-----BEGIN CERTIFICATE-----" >$c
        cat /tmp/${alias} >>$c
    done

    # Build the truststore
    for c in ${TRUSTROOTPATH}/*.0 ; do
        alias=`basename $c .0`
        echo "adding: $TRUSTROOTPATH/${c}"
        echo "alias: $alias"
        yes | keytool -trustcacerts -storepass "$PWD" -v -keystore ./$TRUSTSTORE -alias $alias -importcert -file "${c}"
    done
    exit
~~~
