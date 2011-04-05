/*
 * Copyright (c) 1998 - 2010. University Corporation for Atmospheric Research/Unidata
 * Portions of this software were developed by the Unidata Program at the
 * University Corporation for Atmospheric Research.
 *
 * Access and use of this software shall impose the following obligations
 * and understandings on the user. The user is granted the right, without
 * any fee or cost, to use, copy, modify, alter, enhance and distribute
 * this software, and any derivative works thereof, and its supporting
 * documentation for any purpose whatsoever, provided that this entire
 * notice appears in all copies of the software, derivative works and
 * supporting documentation.  Further, UCAR requests that the user credit
 * UCAR/Unidata in any publications that result from the use of this
 * software or in any product that includes this software. The names UCAR
 * and/or Unidata, however, may not be used in any advertising or publicity
 * to endorse or promote any products or commercial entity unless specific
 * written permission is obtained from UCAR/Unidata. The user also
 * understands that UCAR/Unidata is not obligated to provide the user with
 * any support, consulting, training or assistance of any kind with regard
 * to the use, operation and performance of this software nor to provide
 * the user with any updates, revisions, new versions or "bug fixes."
 *
 * THIS SOFTWARE IS PROVIDED BY UCAR/UNIDATA "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL UCAR/UNIDATA BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE ACCESS, USE OR PERFORMANCE OF THIS SOFTWARE.
 */

package unidata.protobuf.compiler;

import java.io.*;
import java.util.Stack;
import java.util.List;

import unidata.protobuf.compiler.AST.Position;

import static unidata.protobuf.compiler.ProtobufParser.*;

class ProtobufLexer implements Lexer {

    /* Define 1 and > 1st legal characters */
    /* Note: '.' is included but legality will be checked by semantic checker */
    static final String wordchars1 =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    static final String wordcharsn =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";

    /* Number characters, but see below for handling leading '.' */
    static final String numchars1 = ".0123456789";
    static final String numcharsn = ".+-0123456789Ee";
    static final char floatchars[] = new char[] {'.','E','e'};
    // Following are used for hex and octal integers only
    static final String hexcharsn = "0123456789abcdefABCDEF";
    static final String octcharsn = "01234567";

    static String FLOATREGEXP = 
	"([0-9]+([.][0-9]*)?|[.][0-9]+)([Ee][+-]?[0-9]+)?";

    static String INTREGEXP = "[0-9]+";

    static String[] keywords = new String[]{
            "import",
            "package",
            "option",
            "message",
            "extend",
            "extensions",
            "enum",
            "service",
            "rpc",
            "returns",
            "to",
            "max",
            "required",
            "optional",
            "repeated",
            "double",
            "float",
            "int32",
            "int64",
            "uint32",
            "uint64",
            "sint32",
            "sint64",
            "fixed32",
            "fixed64",
            "sfixed32",
            "sfixed64",
            "bool",
            "string",
            "bytes",
            "true",
            "false",
            "inf",
            "nan"
    };

    static int[] keytokens = new int[]{
            IMPORT,
            PACKAGE,
            OPTION,
            MESSAGE,
            EXTEND,
            EXTENSIONS,
            ENUM,
            SERVICE,
            RPC,
            RETURNS,
            TO,
            MAX,
            REQUIRED,
            OPTIONAL,
            REPEATED,
            DOUBLE,
            FLOAT,
            INT32,
            INT64,
            UINT32,
            UINT64,
            SINT32,
            SINT64,
            FIXED32,
            FIXED64,
            SFIXED32,
            SFIXED64,
            BOOL,
            STRING,
            BYTES,
            TRUE,
            FALSE,
            POSINF,
            POSNAN
    };

    // structure for file stack

    static class FileEntry {
        String filename;
        Reader stream;
        AST.Position pos;
    }

    /**
     * **********************************************
     */
    /* Per-lexer state */

    ProtobufActions parsestate = null;
    Reader stream = null;
    StringBuilder input = null;
    StringBuilder yytext = null;
    Position startpos = new Position();
    Position endpos = new Position();
    AST.Position pos = new AST.Position();
    Object lval = null;
    StringBuilder lookahead = null;
    Stack<FileEntry> filestack = null;
    boolean eof2 = false;
    List<String> includepaths = null;
    boolean namestate = false;


    /**
     * *********************************************
     */

    /* Constructor(s) */
    public ProtobufLexer(ProtobufActions state) {
        reset(state);
    }

    public void reset(ProtobufActions state) {
        this.parsestate = state;
        input = new StringBuilder(); /* InputStream so far */
        yytext = new StringBuilder();
        lookahead = new StringBuilder();
        lval = null;
        pos.setLine(1);
        pos.setChar(1);
        pos.setFile(state.filename);
        this.stream = null;
        filestack = new Stack<FileEntry>();
    }


    /* Get/Set */

    public void setStream(String filename, Reader stream) {
        this.pos.setFile(filename);
        this.stream = stream;
    }

    int
    peek() throws IOException {
        int c = read();
        pushback(c);
        return c;
    }

    void
    pushback(int c) {
        lookahead.insert(0, (char) c);
        pos.moveChar(-1);
        if (pos.getChar() == 0 || c == '\n') {
            pos.moveLine(-1);
            pos.setChar(1);
        }
    }

    int
    read() throws IOException {
        int c;
        if (lookahead.length() == 0) {
            c = stream.read();
            if (c < 0) c = 0;
        } else {
            c = lookahead.charAt(0);
            lookahead.deleteCharAt(0);
        }
        pos.moveChar(1);
        if (c == '\n') {
            pos.moveLine(1);
            pos.setChar(1);
        }
        return c;
    }

    /* This is part of the Lexer interface */


    public int
    yylex()
            throws IOException {
        int token;
        int c;
        token = 0;
        yytext.setLength(0);

        // Capture start pos
        startpos = pos.clone();

        token = -1;

        while (token < 0) {
            c = read();
            if (c == 0) {
                // If stack is not empty, or this is the first eof
                // then return ENDFILE otherwise return true eof
                if (filestack.empty()) {
                    if (eof2) {
                        yytext.append("EOF");
                        token = EOF;
                    } else { // !eof2
                        yytext.append("ENDFILE");
                        token = ENDFILE;
                        eof2 = true;
                    }
                } else { // !filestack.empty()
                    if (!popFileStack())
                        lexerror("Could not pop file stack");
                    yytext.append("ENDFILE");
                    token = ENDFILE;
                }
            } else if (c <= ' ' || c == '\177') {
                /* whitespace: ignore */
            } else if (c == '/' && peek() == '/') { // Comment
                do {
                    c = read();
                } while (c != '\0' && c != '\n');
                continue; // start over
            } else if (c == '"' || c == '\'') {
		token = parsestring(c);
	    } else if(c == '.') { // May be either a number
			          // or a single char token
	        token = parsenumber(c);
		if(token == 0) {// assume a single char token
		    yytext.append((char)c);
		    token = c;
		}
            } else if (numchars1.indexOf(c) >= 0) {
		token = parsenumber(c);
            } else if (wordchars1.indexOf(c) >= 0) {
		token = parsename(c);
            } else {
                /* we have a single char token */
                token = c;
                yytext.append((char) c);
            }
        }
        if (token < 0) {
            token = 0;
            lval = null;
        } else {
            lval = (yytext.length() == 0 ? (String) null : yytext.toString());
        }
        if (parsestate.getDebugLevel() > 0) dumptoken(token, (String) lval);
        // Capture end pos
        endpos = pos.clone();
	namestate = false;
        return token;       /* Return the type of the token.  */
    }

    int
    parsename(int c) throws IOException
    {
	int token = 0;
	yytext.append((char) c);
        while ((c = read()) > 0) {
            if (wordcharsn.indexOf(c) < 0) {
		pushback(c);
		break;
            }
            yytext.append((char) c);
        }
	token = NAME; // Default
        String tokentext = yytext.toString();
        if(!namestate) {
            // check for keyword: treat as case sensitive
            for (int i = 0; i < keywords.length; i++) {
                if (keywords[i].equals(tokentext)) {
                    token = keytokens[i];
                    break;
                }
	    }
        }
	return token;
    }

    int
    parsestring(int quotemark) throws IOException
    {
	int c;
        boolean more = true;
        while (more && (c = read()) > 0) {
            if (c == quotemark)
                more = false;
            else if (c == '\\') {
                c = read();
                if (c < 0) more = false;
                    /* Handle the typical \r \n etc */
                else switch (c) {
                    case 'n':
                        c = '\n';
                        break;
                    case 'r':
                        c = '\r';
                        break;
                    case 't':
                        c = '\t';
                        break;
                    case 'x': {
                        c = hexescape();
                        if (c < 0) {
                            lexerror("Illegal hex escape character");
                            more = false;
                        }
                    }
                    break;
                    case '0': { // warning, might be less than four digits
                        c = octalescape();
                        if (c < 0) {
                            lexerror("Illegal octal escape character");
                            more = false;
                        }
                    }
                    break;
                    default:
                        break;
                }
            }
            if (more) yytext.append((char) c);
        }
        return STRINGCONST;
    }

    int
    parsenumber(int c) throws IOException
    {
	// Figure out the radix
        int radix = 10;
        if (c == '0') {// Hex or octal integer
            yytext.append((char) c);
            // read enough characters to determine what we have
            int c1 = peek();
            if (c1 == 'x' || c1 == 'X') {
                yytext.append('x');
                radix = 16;
		read(); // skip 'x'
            } else {
                radix = 8;
            }
        } else
            yytext.append((char) c);
        boolean more = true;
        while (more) {
            c = read();
            if (radix == 16) more = (hexcharsn.indexOf(c) >= 0);
            else if (radix == 8) more = (octcharsn.indexOf(c) >= 0);
            else if (radix == 10) more = (numcharsn.indexOf(c) >= 0);
            else more = false;
            if (more) yytext.append((char) c);
        }
        pushback(c);

        int token = INTCONST;
        if (radix == 10) { // check for float constant
            // Should be either a decimal integer or decimal float or nothing
	    String s = yytext.toString();
	    // Do a regexp match to see if this is a float
	    if(s.matches(INTREGEXP)) token = INTCONST;
	    else if(s.matches(FLOATREGEXP)) token = FLOATCONST;
	    else { // Look for leading '.'
	        if(s.charAt(0) == '.') {
		    token = '.';
		    // pushback the remainder
		    for(int i=1;i<s.length();i++)
			pushback(s.charAt(i));
		    yytext.setLength(1); // keep only the '.'
		}
	    }
        }
        return token;
    }

    void
    dumptoken(int token, String lval) {
        switch (token) {
            case STRING:
                System.err.printf("TOKEN = |\"%s\"|\n", lval);
                break;
            case NAME:
	        if(namestate)
                System.err.printf("TOKEN = #|%s|\n", lval);
		else
                System.err.printf("TOKEN = |%s|\n", lval);
                break;
            case INTCONST:
            case FLOATCONST:
                System.err.printf("TOKEN = |%s|\n", lval);
                break;
            default:
                // See if this is a keyword
                for (int i = 0; i < keytokens.length; i++) {
                    if (keytokens[i] == token) {
                        System.err.printf("TOKEN = |%s|\n", keywords[i]);
                        return;
                    }
                }
                System.err.printf("TOKEN = |%c|\n", (char) token);
                break;
        }
    }


    int hexescape() throws IOException {
        int[] digits = new int[]{-1, -1};
        int c = read();
        int d;
        for (int i = 0; i < 2; i++) {
            d = tohex(c);
            if (d < 0) {
                pushback(c);
                break;
            }
            digits[i] = d;
        }
        // shift digits right
        while (digits[1] < 0) {
            digits[1] = digits[0];
            digits[0] = 0;
        }
        // Compute constant
        c = (digits[0] << 4) | digits[1];
        return c;
    }

    int octalescape() throws IOException {
        int[] digits = new int[]{-1, -1, -1};
        int c = read();
        int d;
        for (int i = 0; i < 3; i++) {
            d = tooct(c);
            if (d < 0) {
                pushback(c);
                break;
            }
            digits[i] = d;
        }
        // shift digits right
        while (digits[2] < 0) {
            digits[2] = digits[1];
            digits[1] = digits[0];
            digits[0] = 0;
        }
        // Compute octal constant
        c = (digits[0] << 6) | (digits[1] << 3) | digits[2];
        return c;
    }

    static int
    tohex(int c) {
        if (c >= 'a' && c <= 'f') return (c - 'a') + 0xa;
        if (c >= 'A' && c <= 'F') return (c - 'A') + 0xa;
        if (c >= '0' && c <= '9') return (c - '0');
        return -1;
    }

    static int
    tooct(int c) {
        if (c >= '0' && c <= '7') return (c - '0');
        return -1;
    }

    /**
     * **********************************************
     */
    /* Capture and restore to/from file stack */
    public boolean pushFileStack(String importfile)
            throws IOException {
	// Check for cyclic imports
	for(FileEntry e: filestack) {
            if(importfile.equals(e.filename))
	        throw new IOException("Cyclic import sequence: "+importfile);
        }
        importfile = AuxFcns.locatefile(importfile, includepaths); // use include paths
        File f = new File(importfile);
        if (!f.canRead()) return false;
        FileReader fr = new FileReader(f);
        FileEntry entry = new FileEntry();
        entry.filename = importfile;
        entry.stream = stream;
        entry.pos = this.pos.clone();
        filestack.push(entry);
        pos = new AST.Position(1, 1, importfile);
        startpos = pos.clone();
        endpos = pos.clone();
        stream = fr;
        if (Debug.enabled("trace.imports"))
            System.err.printf("[%d] enter: %s ",
                    filestack.size() - 1, pos.toString());
        return true;
    }


    public boolean popFileStack() {
        if (filestack.empty()) return false;
        try {
            stream.close();
        } catch (IOException ioe) {
        }
        ;
        FileEntry entry = filestack.pop();
        stream = entry.stream;
        pos = entry.pos;
        startpos = pos.clone();
        endpos = pos.clone();
        if (Debug.enabled("trace.imports"))
            System.err.printf("[%d] re-enter: %s ",
                    filestack.size(), pos.toString());
        return true;
    }

    /**************************************************/
    /* Lexer Interface */

    /**
     * Method to retrieve the semantic value of the last scanned token.
     *
     * @return the semantic value of the last scanned token.
     */
    public Object getLVal() {
        return this.lval;
    }

    /**
     * Entry point for the scanner.	 Returns the token identifier corresponding
     * to the next token and prepares to return the semantic value
     * of the token.
     * @return the token identifier corresponding to the next token. */
    // int yylex() throws ParseException
    // Defined above

    /**
     * Entry point for error and warning reporting.  Emits an report
     * in a user-defined way.
     *
     * @param s         The string for the error/warning message.
     */
    public void yyerror(String s) {
        yyreport(s, false);
    }

    public void yywarning(String s) {
        yyreport(s, true);
    }

    public void yyreport(String s, boolean iswarning) {
        System.err.println(String.format("%s @ %s; %s",
                (iswarning ? "warning" : "error"),
                pos.toString(), s));
        if (yytext.length() > 0)
            System.err.print("; near |" + yytext + "|");
        System.err.println();
    }

    public void lexerror(String msg) {
        StringBuilder nextline = new StringBuilder();
        int c;
        try {
            while ((c = read()) != -1) {
                if (c == '\n') break;
                nextline.append((char) c);
            }
        } catch (IOException ioe) {
        }
        System.out.printf("Lex error: %s; at %s; %s |%s|\n",
                msg, pos.toString(), yytext, nextline);
    }

}
