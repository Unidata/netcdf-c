/*
 * Copyright 1998-2015 University Corporation for Atmospheric Research/Unidata
 *  See the LICENSE file for more information.
 */

/*
 * NOTE ON ENCODING: every byte above 0x7f in the test data below is written as
 * a three-digit octal escape, and this file is pure ASCII. That is deliberate
 * and must stay that way. Most of the cases here are byte sequences that are
 * *not* valid UTF-8, which is the whole point of the test, and a compiler that
 * transcodes string literals from a source character set cannot carry them
 * through unchanged: MSVC rejects the file outright under /utf-8 and silently
 * rewrites the bytes without it. Escapes are the one spelling every compiler
 * reproduces exactly, and they are why this test can run on Windows at all.
 * Do not "restore" the raw characters.
 */

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "netcdf.h"
#include "ncutf8.h"
/*

This test is taken from the UTF-8 decoder
capability and stress test file created by

Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> - 2015-08-28 - CC BY 4.0

This test file can help you examine, how your UTF-8 decoder handles
various types of correct, malformed, or otherwise interesting UTF-8
sequences. This file is not meant to be a conformance test. It does
not prescribe any particular outcome. Therefore, there is no way to
"pass" or "fail" this test file, even though the text does suggest a
preferable decoder behaviour at some places. Its aim is, instead, to
help you think about, and test, the behaviour of your UTF-8 decoder on a
systematic collection of unusual inputs. Experience so far suggests
that most first-time authors of UTF-8 decoders find at least one
serious problem in their decoder using this file.

The test lines below cover boundary conditions, malformed UTF-8
sequences, as well as correctly encoded UTF-8 sequences of Unicode code
points that should never occur in a correct UTF-8 file.

According to ISO 10646-1:2000, sections D.7 and 2.3c, a device
receiving UTF-8 shall interpret a "malformed sequence in the same way
that it interprets a character that is outside the adopted subset" and
"characters that are not within the adopted subset shall be indicated
to the user" by a receiving device. One commonly used approach in
UTF-8 decoders is to replace any malformed UTF-8 sequence by a
replacement character (U+FFFD), which looks a bit like an inverted
question mark, or a similar symbol. It might be a good idea to
visually distinguish a malformed UTF-8 sequence from a correctly
encoded Unicode character that is just not available in the current
font but otherwise fully legal, even though ISO 10646-1 doesn't
mandate this. In any case, just ignoring malformed sequences or
unavailable characters does not conform to ISO 10646, will make
debugging more difficult, and can lead to user confusion.

Please check, whether a malformed UTF-8 sequence is (1) represented at
all, (2) represented by exactly one single replacement character (or
equivalent signal), and (3) the following quotation mark after an
illegal UTF-8 sequence is correctly displayed, i.e. proper
resynchronization takes place immediately after any malformed
sequence. This file says "THE END" in the last line, so if you don't
see that, your decoder crashed somehow before, which should always be
cause for concern.

All lines in this file are exactly 79 characters long (plus the line
feed). In addition, all lines end with "|", except for the two test
lines 2.1.1 and 2.2.1, which contain non-printable ASCII controls
U+0000 and U+007F. If you display this file with a fixed-width font,
these "|" characters should all line up in column 79 (right margin).
This allows you to test quickly, whether your UTF-8 decoder finds the
correct number of characters in every line, that is whether each
malformed sequences is replaced by a single replacement character.

Note that, as an alternative to the notion of malformed sequence used
here, it is also a perfectly acceptable (and in some situations even
preferable) solution to represent each individual byte of a malformed
sequence with a replacement character. If you follow this strategy in
your decoder, then please ignore the "|" column.
*/


struct Test {
    int xfail;
    const char* id;
    const char* description;
    const char* data;
};
#define NULLTEST {0,NULL,NULL,NULL}

/* The following tests are in envv form */

/*1  Some correct UTF-8 text
     You should see the Greek word 'kosme':
*/
static const struct Test utf8ok[] = {
{0, "1.1.1", "Greek word 'kosme'",
"\316\272\341\275\271\317\203\316\274\316\265"},
NULLTEST
};

static const struct Test utf8boundary[] = {

/*2  Boundary condition test */
/*2.1  First possible sequence of a certain length */
{0,"2.1.1", "1 byte  (U-00000000)",        "\000"},
{0,"2.1.2", "2 bytes (U-00000080)",        "\302\200"},
{0,"2.1.3", "3 bytes (U-00000800)",        "\340\240\200"},
{0,"2.1.4", "4 bytes (U-00010000)",        "\360\220\200\200"},
{1,"2.1.5", "5 bytes (U-00200000)",        "\370\210\200\200\200"},
{1,"2.1.6", "6 bytes (U-04000000)",        "\374\204\200\200\200\200"},

/*2.2  Last possible sequence of a certain length*/
{0,"2.2.1", "1 byte  (U-0000007F)",        ""},
{0,"2.2.2", "2 bytes (U-000007FF)",        "\337\277"},
{0,"2.2.3", "3 bytes (U-0000FFFF)",        "\357\277\277"}, /*See 5.3.2 */
{1,"2.2.4", "4 bytes (U-001FFFFF)",        "\367\277\277\277"},
{1,"2.2.5", "5 bytes (U-03FFFFFF)",        "\373\277\277\277\277"},
{1,"2.2.6", "6 bytes (U-7FFFFFFF)",        "\375\277\277\277\277\277"},

/*2.3  Other boundary conditions*/

{0,"2.3.1", "U-0000D7FF = ed 9f bf", "\355\237\277"},
{0,"2.3.2", "U-0000E000 = ee 80 80", "\356\200\200"},
{0,"2.3.3", "U-0000FFFD = ef bf bd", "\357\277\275"},
{0,"2.3.4", "U-0010FFFF = f4 8f bf bf", "\364\217\277\277"},
{1,"2.3.5", "U-00110000 = f4 90 80 80", "\364\220\200\200"},
NULLTEST
};

static const struct Test utf8bad[] = {

/*3  Malformed sequences*/

/*3.1  Unexpected continuation bytes
       Each unexpected continuation byte should be separately signalled
       as a malformed sequence of its own.
*/
{1,"3.1.1", "First continuation byte 0x80", "\200"},
{1,"3.1.2", "Last  continuation byte 0xbf", "\277"},

{1,"3.1.3", "2 continuation bytes", "\200\277"},
{1,"3.1.4", "3 continuation bytes", "\200\277\200"},
{1,"3.1.5", "4 continuation bytes", "\200\277\200\277"},
{1,"3.1.6", "5 continuation bytes", "\200\277\200\277\200"},
{1,"3.1.7", "6 continuation bytes", "\200\277\200\277\200\277"},
{1,"3.1.8", "7 continuation bytes", "\200\277\200\277\200\277\200"},
{1,"3.1.9", "Sequence of all 64 possible continuation bytes (0x80-0xbf)",
   "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
},

/*3.2  Lonely start characters*/

/*3.2.1  All 32 first bytes of 2-byte sequences (0xc0-0xdf),
       each followed by a space character*/

{1,"3.2.1", "All 32 first bytes of 2-byte sequences",
"\300 \301 \302 \303 \304 \305 \306 \307 \310 \311 \312 \313 \314 \315 \316 \317 \320 \321 \322 \323 \324 \325 \326 \327 \330 \331 \332 \333 \334 \335 \336 \337 "
},

/*3.2.2  All 16 first bytes of 3-byte sequences (0xe0-0xef),
       each followed by a space character:*/
{1,"3.2.2", "All 16 first bytes of 3-byte sequences",
"\340 \341 \342 \343 \344 \345 \346 \347 \350 \351 \352 \353 \354 \355 \356 \357 "
},

/*3.2.3  All 8 first bytes of 4-byte sequences (0xf0-0xf7),
       each followed by a space character:*/
{1,"3.2.3", "All 8 first bytes of 4-byte sequences",
   "\360 \361 \362 \363 \364 \365 \366 \367 "
},

/*3.2.4  All 4 first bytes of 5-byte sequences (0xf8-0xfb),
       each followed by a space character:*/
{1,"3.2.4", "All 4 first bytes of 5-byte sequences",
   "\370 \371 \372 \373 "
},

/*3.2.5  All 2 first bytes of 6-byte sequences (0xfc-0xfd),
       each followed by a space character:*/
{1,"3.2.5", "All 2 first bytes of 6-byte sequences",
   "\374 \375 "
},

/*3.3  Sequences with last continuation byte missing
All bytes of an incomplete sequence should be signalled as a single
malformed sequence, i.e., you should see only a single replacement
character in each of the next 10 tests. (Characters as in section 2)
*/
{1,"3.3.1", "2-byte sequence with last byte missing (U+0000)",     "\300"},
{1,"3.3.2", "3-byte sequence with last byte missing (U+0000)",     "\340\200"},
{1,"3.3.3", "4-byte sequence with last byte missing (U+0000)",     "\360\200\200"},
{1,"3.3.4", "5-byte sequence with last byte missing (U+0000)",     "\370\200\200\200"},
{1,"3.3.5", "6-byte sequence with last byte missing (U+0000)",     "\374\200\200\200\200"},
{1,"3.3.6", "2-byte sequence with last byte missing (U-000007FF)", "\337"},
{1,"3.3.7", "3-byte sequence with last byte missing (U-0000FFFF)", "\357\277"},
{1,"3.3.8", "4-byte sequence with last byte missing (U-001FFFFF)", "\367\277\277"},
{1,"3.3.9", "5-byte sequence with last byte missing (U-03FFFFFF)", "\373\277\277\277"},
{1,"3.3.10", "6-byte sequence with last byte missing (U-7FFFFFFF)", "\375\277\277\277\277"},

/*3.4 Concatenation of incomplete sequences
All the 10 sequences of 3.3 concatenated; you should see 10 malformed
sequences being signalled:
*/
{1, "3.4.1", "All the 10 sequences of 3.3 concatenated",
   "\300\340\200\360\200\200\370\200\200\200\374\200\200\200\200\337\357\277\367\277\277\373\277\277\277\375\277\277\277\277"
},

/*3.5  Impossible bytes
The following two bytes cannot appear in a correct UTF-8 string
*/

{1,"3.5.1", "fe", "\376"},
{1,"3.5.2", "ff", "\377"},
{1,"3.5.3", "fe fe ff ff", "\376\376\377\377"},

/*
4  Overlong sequences

The following sequences are not malformed according to the letter of
the Unicode 2.0 standard. However, they are longer then necessary and
a correct UTF-8 encoder is not allowed to produce them. A "safe UTF-8
decoder" should reject them just like malformed sequences for two
reasons: (1) It helps to debug applications if overlong sequences are
not treated as valid representations of characters, because this helps
to spot problems more quickly. (2) Overlong sequences provide
alternative representations of characters, that could maliciously be
used to bypass filters that check only for ASCII characters. For
instance, a 2-byte encoded line feed (LF) would not be caught by a
line counter that counts only 0x0a bytes, but it would still be
processed as a line feed by an unsafe UTF-8 decoder later in the
pipeline. From a security point of view, ASCII compatibility of UTF-8
sequences means also, that ASCII characters are *only* allowed to be
represented by ASCII bytes in the range 0x00-0x7f. To ensure this
aspect of ASCII compatibility, use only "safe UTF-8 decoders" that
reject overlong UTF-8 sequences for which a shorter encoding exists.
*/

/*4.1  Examples of an overlong ASCII character

With a safe UTF-8 decoder, all of the following five overlong
representations of the ASCII character slash ("/") should be rejected
like a malformed UTF-8 sequence, for instance by substituting it with
a replacement character. If you see a slash below, you do not have a
safe UTF-8 decoder!
*/

{1,"4.1.1", "U+002F = c0 af             ", "\300\257"},
{1,"4.1.2", "U+002F = e0 80 af          ", "\340\200\257"},
{1,"4.1.3", "U+002F = f0 80 80 af       ", "\360\200\200\257"},
{1,"4.1.4", "U+002F = f8 80 80 80 af    ", "\370\200\200\200\257"},
{1,"4.1.5", "U+002F = fc 80 80 80 80 af ", "\374\200\200\200\200\257"},

/*4.2  Maximum overlong sequences

Below you see the highest Unicode value that is still resulting in an
overlong sequence if represented with the given number of bytes. This
is a boundary test for safe UTF-8 decoders. All five characters should
be rejected like malformed UTF-8 sequences.
*/

{1,"4.2.1", "U-0000007F = c1 bf             ", "\301\277"},
{1,"4.2.2", "U-000007FF = e0 9f bf          ", "\340\237\277"},
{1,"4.2.3", "U-0000FFFF = f0 8f bf bf       ", "\360\217\277\277"},
{1,"4.2.4", "U-001FFFFF = f8 87 bf bf bf    ", "\370\207\277\277\277"},
{1,"4.2.5", "U-03FFFFFF = fc 83 bf bf bf bf ", "\374\203\277\277\277\277"},

/*
4.3  Overlong representation of the NUL character

The following five sequences should also be rejected like malformed
UTF-8 sequences and should not be treated like the ASCII NUL
character.
*/

{1,"4.3.1", "U+0000 = c0 80             ", "\300\200"},
{1,"4.3.2", "U+0000 = e0 80 80          ", "\340\200\200"},
{1,"4.3.3", "U+0000 = f0 80 80 80       ", "\360\200\200\200"},
{1,"4.3.4", "U+0000 = f8 80 80 80 80    ", "\370\200\200\200\200"},
{1,"4.3.5", "U+0000 = fc 80 80 80 80 80 ", "\374\200\200\200\200\200"},

/*
5  Illegal code positions

The following UTF-8 sequences should be rejected like malformed
sequences, because they never represent valid ISO 10646 characters and
a UTF-8 decoder that accepts them might introduce security problems
comparable to overlong UTF-8 sequences.
*/
/*5.1 Single UTF-16 surrogates*/

{1,"5.1.1", "U+D800 = ed a0 80 ", "\355\240\200"},
{1,"5.1.2", "U+DB7F = ed ad bf ", "\355\255\277"},
{1,"5.1.3", "U+DB80 = ed ae 80 ", "\355\256\200"},
{1,"5.1.4", "U+DBFF = ed af bf ", "\355\257\277"},
{1,"5.1.5", "U+DC00 = ed b0 80 ", "\355\260\200"},
{1,"5.1.6", "U+DF80 = ed be 80 ", "\355\276\200"},
{1,"5.1.7", "U+DFFF = ed bf bf ", "\355\277\277"},

/*5.2 Paired UTF-16 surrogates */

{1,"5.2.1", "U+D800 U+DC00 = ed a0 80 ed b0 80 ", "\355\240\200\355\260\200"},
{1,"5.2.2", "U+D800 U+DFFF = ed a0 80 ed bf bf ", "\355\240\200\355\277\277"},
{1,"5.2.3", "U+DB7F U+DC00 = ed ad bf ed b0 80 ", "\355\255\277\355\260\200"},
{1,"5.2.4", "U+DB7F U+DFFF = ed ad bf ed bf bf ", "\355\255\277\355\277\277"},
{1,"5.2.5", "U+DB80 U+DC00 = ed ae 80 ed b0 80 ", "\355\256\200\355\260\200"},
{1,"5.2.6", "U+DB80 U+DFFF = ed ae 80 ed bf bf ", "\355\256\200\355\277\277"},
{1,"5.2.7", "U+DBFF U+DC00 = ed af bf ed b0 80 ", "\355\257\277\355\260\200"},
{1,"5.2.8", "U+DBFF U+DFFF = ed af bf ed bf bf ", "\355\257\277\355\277\277"},
NULLTEST
};

/*5.3 Noncharacter code positions

The following "noncharacters" are "reserved for internal use" by
applications, and according to older versions of the Unicode Standard
"should never be interchanged". Unicode Corrigendum #9 dropped the
latter restriction. Nevertheless, their presence in incoming UTF-8 data
can remain a potential security risk, depending on what use is made of
these codes subsequently. Examples of such internal use:

 - Some file APIs with 16-bit characters may use the integer value -1
   = U+FFFF to signal an end-of-file (EOF) or error condition.

 - In some UTF-16 receivers, code point U+FFFE might trigger a
   byte-swap operation (to convert between UTF-16LE and UTF-16BE).

With such internal use of noncharacters, it may be desirable and safer
to block those code points in UTF-8 decoders, as they should never
occur legitimately in incoming UTF-8 data, and could trigger unsafe
behaviour in subsequent processing.

Particularly problematic noncharacters in 16-bit applications:
*/

static const struct Test utf8problematic[] = {
{0,"5.3.1", "U+FFFE = ef bf be ", "\357\277\276"},
{0,"5.3.2", "U+FFFF = ef bf bf ", "\357\277\277"},
NULLTEST
};

/* Other (utf16) noncharacters: */
static const struct Test utf8nonchars[] = {
{0,"5.3.3", "U+FDD0 .. U+FDEF ",
"\357\267\220\357\267\221\357\267\222\357\267\223\357\267\224\357\267\225\357\267\226\357\267\227\357\267\230\357\267\231\357\267\232\357\267\233\357\267\234\357\267\235\357\267\236\357\267\237\357\267\240\357\267\241\357\267\242\357\267\243\357\267\244\357\267\245\357\267\246\357\267\247\357\267\250\357\267\251\357\267\252\357\267\253\357\267\254\357\267\255\357\267\256\357\267\257"
},

/* Do not understand this test; it passes, but should it? */
{0,"5.3.4", "U+nFFFE U+nFFFF (for n = 1..10)",
       "\360\237\277\276\360\237\277\277\360\257\277\276\360\257\277\277\360\277\277\276\360\277\277\277\361\217\277\276\361\217\277\277\361\237\277\276\361\237\277\277\361\257\277\276\361\257\277\277\361\277\277\276\361\277\277\277\362\217\277\276\362\217\277\277\362\237\277\276\362\237\277\277\362\257\277\276\362\257\277\277\362\277\277\276\362\277\277\277\363\217\277\276\363\217\277\277\363\237\277\276\363\237\277\277\363\257\277\276\363\257\277\277\363\277\277\276\363\277\277\277\364\217\277\276\364\217\277\277"
},
NULLTEST
};

static char*
trim(const char* s)
{
    size_t i;
    size_t l = strlen(s);
    char* t = strdup(s);
    for(i=l-1; i-- > 0;) {
        if(t[i] != ' ') break;
    }
    t[i+1] = '\0';
    return t;
}

static int
test(const struct Test* tests, const char* title)
{
    int status = NC_NOERR;
    int failures = 0;
    const struct Test* p;

    fprintf(stderr,"Testing %s...\n",title);
    for(p=tests;p->id;p++) {
	char* id;
        char* description;
        const char* pf;
        id = trim(p->id);
        description = trim(p->description);
        status = nc_utf8_validate((const unsigned char*)p->data);
        if(status == NC_NOERR && p->xfail) {pf = "Fail"; failures++;}
        else if(status != NC_NOERR && p->xfail) pf = "Pass";
        else if(status == NC_NOERR && !p->xfail) pf = "Pass";
        else if(status != NC_NOERR && !p->xfail) {pf = "Fail"; failures++;}
        fprintf(stderr,"%s: %s %s\n",pf,id,description);
        fflush(stderr);
	free(id);
	free(description);
    }
    return failures;
}

int
main(int argc, char** argv)
{
    int failures = 0;

    printf("\n Testing UTF-8 sequences.\n");
    failures += test(utf8ok,"Correct Sequences");
    failures += test(utf8boundary,"Boundary Tests");
    failures += test(utf8bad,"Invalid strings");
    failures += test(utf8problematic,"Problematic strings");
    failures += test(utf8nonchars,"Other non-characters");
    fprintf(stderr,"No. of failures = %d\n",failures);
    exit(failures == 0 ? 0 : 1);
}
