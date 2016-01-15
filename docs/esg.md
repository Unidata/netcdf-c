 Accessing ESG Data Through netCDF {#esg}
=================================

[TOC]

Introduction {#esg_intro}
==================

It is possible to access Earth Systems Grid (ESG) datasets from ESG
servers through the netCDF API. This requires building the netCDF
library with DAP2 protocol support using the "--enable-dap" flag to the
configure program.

In order to access ESG datasets, however, it is necessary to register as
a user with ESG and to setup your environment so that proper
authentication is established between a netCDF client program and the
ESG data server. Specifically, it is necessary to use what is called
"client-side keys" to enable this authentication. Normally, when a
client accesses a server in a secure fashion (using "https"), the server
provides an authentication certificate to the client. With client-side
keys, the client must also provide a certificate to the server so that
the server can know with whom it is communicating.

It is possible to set up the netCDF library to use client-side keys,
although the process is somewhat complicated. The DAP2 support in netCDF
uses the *curl* library and it is that underlying library that must be
properly configured.

Terminology {#esg_term}
==================

The key elements for client-side keys requires the constructions of two
"stores" on the client side.

-   Keystore - a repository to hold the client side key.
-   Truststore - a repository to hold a chain of certificates that can
    be used to validate the certificate sent by the server to
    the client.

The server actually has a similar set of stores, but the client need not
be concerned with those.

Initial Steps {#esg_initial_steps}
==================

The first step is to obtain authorization from ESG. Note that this
information may evolve over time, and may be out of date. This
discussion is in terms of BADC ESG. You will need to substitute the ESG
site for BADC in the following.

1.  Register at http://badc.nerc.ac.uk/register to obtain access to badc
    and to obtain an openid, which will looks something like:

        https://ceda.ac.uk/openid/Firstname.Lastname

2.  Ask BADC for access to whatever datasets are of interest.
3.  Obtain short term credentials at
    http://grid.ncsa.illinois.edu/myproxy/MyProxyLogon/ You will need to
    download and run the MyProxyLogon program. This will create a
    keyfile in, typically, the directory globus. The keyfile will have a
    name similar to this: x509up\_u13615 The other elements in
    \~/.globus are certificates to use in validating the certificate
    your client gets from the server.
4.  Obtain the program source ImportKey.java from this location:
    http://www.agentbob.info/agentbob/79-AB.html (read the whole page,
    it will help you understand the remaining steps).

Building the KeyStore {#esg_keystore}
==================

You will have to modify the keyfile in the previous step and then create
a keystore and install the key and a certificate. The commands are
these:

        openssl pkcs8 -topk8 -nocrypt -in x509up_u13615 -inform PEM -out key.der -outform DER

        openssl x509 -in x509up_u13615 -inform PEM -out cert.der -outform DER

        java -classpath  -Dkeypassword="" -Dkeystore=./ key.der cert.der

Note, the file names "key.der" and "cert.der" can be whatever you
choose. It is probably best to leave the .der extension, though.

Building the TrustStore {#esg_truststore}
==================

Building the truststore is a bit tricky because as provided, the
certificates in "globus" need some massaging. See the script below for
the details. The primary command is this, which is executed for every
certificate, c, in globus. It sticks the certificate into the file named
"truststore"

      keytool -trustcacerts -storepass "password" -v -keystore "truststore"  -importcert -file "${c}"

Running the C Client {#esg_c_client}
==================

The file ".dodsrc" is used to configure curl. This file must reside
either in the current directory or in your home directory. It has lines
of the form

-   KEY=VALUE, or
-   \[http//x.y/\]KEY=VALUE

The first form provides a configuration parameter that applies to all
DAP2 accesses. The second form only applies to accesses to the server at
"x.y".

The following keys must be set in ".dodsrc" to support ESG access.

-   HTTP.SSL.VALIDATE=1
-   HTTP.COOKIEJAR=.dods\_cookies
-   HTTP.SSL.CERTIFICATE=esgkeystore
-   HTTP.SSL.KEY=esgkeystore
-   HTTP.SSL.CAPATH=.globus

For ESG, the HTTP.SSL.CERTIFICATE and HTTP.SSL.KEY entries should have
same value, which is the file path for the certificate produced by
MyProxyLogon. The HTTP.SSL.CAPATH entry should be the path to the
"certificates" directory produced by MyProxyLogon.

Running the Java Client {#esg_java_client}
==================

If you are using the Java netCDF client, then you need to add some
parameters to the "java" command. Specifically, add the following flags.

       -Dkeystore="path to keystore file" -Dkeystorepassword="keystore password"

Script for creating Stores {#esg_script}
==================

The following script shows in detail how to actually construct the key
and trust stores. It is specific to the format of the globus file as it
was when ESG support was first added. It may have changed since then, in
which case, you will need to seek some help in fixing this script. It
would help if you communicated what you changed to the author so this
document can be updated.


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



Change Log {#esg_change_log}
==================

**Version 1.0:**

-   10/17/2013 – Initial Release

Document Information {#esg_doc_info}
==================

  ------------------------------------------------------------------------
  Created:
  10/17/2013

  Last Revised:
  10/17/2013\

  Version:
  1.0

  Author:
  Dennis Heimbigner

  Affiliation:
  Unidata/UCAR

  email:
  dmh@unida.ucar.edu
  ------------------------------------------------------------------------
