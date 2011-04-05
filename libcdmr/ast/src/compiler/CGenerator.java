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

// For code taken from google protobuf src:
//
// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/*
This generator generates two files per proto file.
1. <protofile>.h and
2. <protofile>.c
Thus, if a user has imports, then those import files
must be separately compiled.
Exception: if the option "compile=..." references a file,
then its declarations are generated also.
*/

package unidata.protobuf.compiler;

import gnu.getopt.Getopt;

import java.util.*;
import java.io.*;

public class CGenerator extends Generator
{
//////////////////////////////////////////////////

static final String LANGUAGE = "C";

static final String FCNPREFIX = "ast_";

//////////////////////////////////////////////////

static final String DFALTDIR = ".";

static final String DIGITCHARS = "0123456789";
static final String IDCHARS =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    + "abcdefghijklmnopqrstuvwxyz"
    + DIGITCHARS
    + "_$";

/* Protobuf identifiers are same as C Identifiers */
static final String CCHARS = IDCHARS;

static final short IDMAX = 0x7f;
static final short IDMIN = ' ';

static final String LBRACE = "{";
static final String RBRACE = "}";

static final String[] CKeywords = new String[] {
"auto", "break", "case", "char", "const",
"continue", "default", "do", "double", "else",
"enum", "extern", "float", "for", "goto",
"if", "int", "long", "register", "return",
"short", "signed", "sizeof", "static", "struct",
"switch", "typedef", "union", "unsigned", "void",
"volatile", "while"
};



//////////////////////////////////////////////////
// Define the per-node extra info; grouped here into a single class.

static public class Annotation
{
    String fileprefix = null;
    String filebase = null;
    List<AST.File> includes = null;
}

//////////////////////////////////////////////////
// Misc. static functions

static boolean isPrimitive(AST.Field field)
{
    return (field.getType().getSort() == AST.Sort.PRIMITIVETYPE);
}

static AST.PrimitiveSort getPrimitiveSort(AST.Field field)
{
    if(!isPrimitive(field)) return null;
    return ((AST.PrimitiveType)(field.getType())).getPrimitiveSort();
}

static boolean isEnum(AST.Field field)
{
    return (field.getType().getSort() == AST.Sort.ENUM);
}

static boolean isMessage(AST.Field field)
{
    return (field.getType().getSort() == AST.Sort.MESSAGE);
}

static boolean isRequired(AST.Field field)
{
    return (field.getCardinality() == AST.Cardinality.REQUIRED);
}

static boolean isOptional(AST.Field field)
{
    return (field.getCardinality() == AST.Cardinality.OPTIONAL);
}

static boolean isRepeated(AST.Field field)
{
    return (field.getCardinality() == AST.Cardinality.REPEATED);
}


//////////////////////////////////////////////////
// Instance variables

String outputdir = null;

String filebase = null; // used also for the output file name

//////////////////////////////////////////////////
// Constructor

public
CGenerator()
{
}

//////////////////////////////////////////////////
// Command line processing

List<String>
processcommandline(String[] argv)
{
    int c;
    List<String> arglist = new ArrayList<String>();
    Getopt g = new Getopt(LANGUAGE+"Generator",argv,"-:o",null);
    while ((c = g.getopt()) != -1) {
	switch (c) {
	case 1: // intermixed non-option
	    arglist.add(g.getOptarg());
	    break;
	case ':':
	    System.err.println("Command line option requires argument "+g.getOptopt());
	    System.exit(1);
	case '?':
	    System.err.println("Illegal cmd line option: "+g.getOptopt());
	    System.exit(1);
	// True options start here
	case 'o':
	    String dir = g.getOptarg();
	    if(dir != null && dir.length() > 0) outputdir = dir;
	    break;
	default:
	    System.err.println("Unexpected getopt tag: "+c);
	    System.exit(1);
	}
    }
    return arglist;
}
//////////////////////////////////////////////////

/*
- compute which files will generate code
- compute the filename for the top package
- compute the reference path for each message and enum
- for each enum in code generating files
   - generate the top level enum definitions
   - generate the top level message classes
 - for each generated message, generate the (de)serialize functions
   and the free function.
*/

public boolean
generate(AST.Root root, String[] argv) throws Exception
{
    List<String> arglist = processcommandline(argv);
    List<AST.File> codefiles = new ArrayList<AST.File>();
    AST.File topfile = root.getTopFile();

    // Assign annotation objects
    for(AST ast: root.getNodeSet()) {
	switch (ast.getSort()) {
	case PACKAGE: case FILE: case MESSAGE: case ENUM:
	    Annotation a = new Annotation();
	    ast.setAnnotation(a);
	    break;
	default: break;
	}
    }

    // Find files that will contribute code
    List<String> includes = new ArrayList<String>();
    // Topfile is always treated as compiled
    // prime the search
    codefiles.add(topfile);
    String tmp = (String)topfile.optionLookup("compile");
    if(tmp != null && tmp.length() > 0) {
        String[] compilefiles = tmp.split(",");
        for(String cfile: compilefiles) {
	    // Locate the file
            for(AST.File file: root.getFileSet()) {
	        if(cfile.equals(file.getName())) {
		    if(!codefiles.contains(file))
		        codefiles.add(file);
		}
	    }
	}
    }

    // Compute the C output file name
    String prefix = AuxFcns.getFilePrefix(topfile.getName());
    String basename = AuxFcns.getBaseName(topfile.getName());
    String cfilename = (String)topfile.optionLookup("c_file");
    if(cfilename != null) {
	if(!AuxFcns.getFilePrefix(cfilename).equals("")) {
	    prefix = AuxFcns.getFilePrefix(cfilename);
	}
        basename = AuxFcns.getBaseName(cfilename);
    } else {
	basename = AuxFcns.getBaseName(topfile.getName());
    }

    // outputdir overrides any prefix
    if(outputdir != null) prefix = outputdir;
    if(prefix.length() == 0) prefix = ".";
    Annotation a = (Annotation)topfile.getAnnotation();
    a.filebase = basename;
    a.fileprefix = prefix;

    // Truncate both .h and the .c files
    FileWriter filewriterH = null;
    FileWriter filewriterC = null;
    String filename = a.fileprefix + "/" + a.filebase;
    try {
	filewriterH = new FileWriter(filename+".h");
	filewriterC = new FileWriter(filename+".c");
    } catch (Exception e) {
	System.err.println("Cannot access file: "+filename+".[hc]");
	return false;
    }
    // close the files to truncate
    try {
	filewriterH.close();
	filewriterC.close();
    } catch (Exception e) {};

    // Generate the top files <filebase>.[hc] content
    Printer printer = null;
    FileWriter wfile = null;
    File file = null;

    try {
	// Open the output .h file
	file = new File(filename+".h");
	if(!file.canWrite()) {
	    System.err.println("Cannot access: "+file);
	    return false;
	}
	wfile = new FileWriter(file);
	printer = new Printer(wfile);
	generate_h(topfile,codefiles,printer);
	printer.close(); wfile.close();
    } catch (Exception e) {
	System.err.println("Generation Failure: "+file+":"+e);
	e.printStackTrace();
	return false;
    }
    try {
	// Open the .c output file
	file = new File(filename+".c");
	if(!file.canWrite()) {
	    System.err.println("Cannot access: "+file);
	    return false;
	}
	wfile = new FileWriter(file);
	printer = new Printer(wfile);
	generate_c(topfile,codefiles,printer);
	printer.close(); wfile.close();
    } catch (Exception e) {
	System.err.println("Generation Failure: "+file+":"+e);
	e.printStackTrace();
	return false;
    }
    return true;
} // generate()


static AST.File
matchfile(String fname, List<AST.File> files)
{
    for(AST.File f: files) {
	if(f.getName().equals(fname)) return f;
    }
    return null;
}

void
generate_h(AST.File topfile, List<AST.File> files, Printer printer)
	throws Exception
{
    Annotation a = (Annotation)topfile.getAnnotation();
    printer.printf("#ifndef %s_H\n",a.filebase.toUpperCase());
    printer.printf("#define %s_H\n",a.filebase.toUpperCase());
    printer.blankline();

    for(AST.File f: files) {
        // Generate the enum definitions
        for(AST.Enum ast: f.getEnums()) {
	    generate_enum(ast,printer);
	}
    }

    // Generate the message structure forwards
    printer.blankline();
    printer.printf("/* Forward definitions */\n");
    for(AST.File f: files) {
        for(AST.Message ast: f.getMessages()) {
            printer.printf("typedef struct %s %s;\n",
		converttocname(ast.getName()),
		converttocname(ast.getName()));
	}
    }

    // Generate the message structures
    for(AST.File f: files) {
        for(AST.Message ast: f.getMessages()) {
	    generate_messagestruct(ast,printer);
	}
    }

    printer.blankline();
    printer.printf("#endif /*%s_H*/\n",a.filebase.toUpperCase());
}

void
generate_enum(AST.Enum e, Printer printer) throws Exception
{
    printer.blankline();
    Annotation a = (Annotation)e.getAnnotation();
    printer.blankline();
    printer.printf("typedef enum %s {\n",e.getName());
    printer.indent();
    List<AST.EnumValue> values = e.getEnumValues();
    int nvalues = values.size();
    for(int i=0;i<nvalues;i++) {
	AST.EnumValue eval = values.get(i);
	printer.printf("%s=%d%s\n",
	    eval.getName(),
	    eval.getValue(),
	    (i == (nvalues - 1)?"":","));
    }
    printer.outdent();
    printer.printf("} %s;\n",e.getName());
}

void
generate_messagestruct(AST.Message msg, Printer printer) throws Exception
{
    // If the "declare" option is set, then do nothing
    if(AuxFcns.getbooleanvalue((String)msg.optionLookup("declare")))
	return;

    Annotation a = (Annotation)msg.getAnnotation();
    printer.blankline();
    printer.printf("struct %s {\n",converttocname(msg.getName()));
    printer.indent();
    // Generate the fields
    for(AST.Field field: msg.getFields()) {
	String star = (isPrimitive(field) || isEnum(field) ? "" : "*");
	if(isRequired(field)) {
	    printer.printf("%s%s %s;\n",
			    ctypefor(field.getType()),star,
			    cfieldvar(field));
	} else if(isOptional(field)) {
	    printer.printf("struct {int defined; %s%s value;} %s;\n",
		    ctypefor(field.getType()),star,
		    cfieldvar(field));
	} else { // isRepeated(field)
	    printer.printf("struct {size_t count; %s*%s values;} %s;\n",
		    ctypefor(field.getType()),star,
		    cfieldvar(field));
	}
    }
    printer.outdent();
    printer.println("};\n");

    // Generate the per-message-type function prototypes
    printer.blankline();
    printer.printf("extern ast_err %s_write(ast_runtime*,%s*);\n",
		    cfcnname(msg),
		    msg.getName());
    printer.printf("extern ast_err %s_read(ast_runtime*,%s**);\n",
		    cfcnname(msg),
		    msg.getName());
    printer.printf("extern ast_err %s_reclaim(ast_runtime*,%s*);\n",
		    cfcnname(msg),
		    msg.getName());
    printer.printf("extern size_t %s_get_size(ast_runtime*,%s*);\n",
		    cfcnname(msg),
		    msg.getName());
}


void
generate_c(AST.File topfile, List<AST.File> files, Printer printer)
	throws Exception
{
    // Add includes

    // Special handling for config.h
    String configh = (String)topfile.optionLookup("config_h");
    if(configh.equals("true")) {
        printer.println("#ifdef HAVE_CONFIG_H");
        printer.println("#include \"config.h\"");
        printer.println("#endif");
        printer.blankline();
    }

    List<String> includes = new ArrayList<String>();
    for(AST.File f: files) {
        String optinclude = (String)f.optionLookup("include");
	if(optinclude == null || optinclude.length() == 0) continue;
	String[] includelist = optinclude.split(",");
	for(String s: includelist) {
	    String ss = s.trim();
	    if(!includes.contains(ss)) includes.add(ss);
	}
    }

    printer.printf("#include <stdlib.h>\n");
    printer.printf("#include <stdio.h>\n");
    printer.blankline();
    printer.printf("#include <ast_runtime.h>\n");
    printer.blankline();

    if(includes.size() > 0) {
        for(String s: includes)
	    if(s.charAt(0) == '<')
                printer.printf("#include %s",s);
	    else
                printer.printf("#include \"%s\"",s);
        printer.blankline();
    }

    // dump in reverse order to match dependencies
    List<AST.File> subfiles = topfile.getRoot().getFileSet();
    for(int i=subfiles.size()-1;i>=0;i--) {
	AST.File f1 = subfiles.get(i);
	if(files.contains(f1)) continue; // only include non-generated files
	Annotation a = (Annotation)f1.getAnnotation();
	printer.printf("#include \"%s.h\"\n",a.filebase);
    }
    Annotation a = (Annotation)topfile.getAnnotation();
    printer.printf("#include \"%s.h\"\n",a.filebase);
    printer.blankline();

    // Generate the per-message functions
    for(AST.File f: files) {
        for(AST ast: f.getNodeSet()) {
	    if(ast.getSort() != AST.Sort.MESSAGE) continue;
	    generate_messagefunctions((AST.Message)ast,printer);
	}
    }
}

void
generate_messagefunctions(AST.Message msg, Printer printer)
    throws Exception
{
    // If the "declare" option is set, then do nothing
    if(AuxFcns.getbooleanvalue((String)msg.optionLookup("declare")))
	return;

    generate_writefunction(msg,printer);
    printer.blankline();
    generate_readfunction(msg,printer);
    printer.blankline();
    generate_reclaimfunction(msg,printer);
    printer.blankline();
    generate_sizefunction(msg,printer);
}

void
generate_writefunction(AST.Message msg, Printer printer)
    throws Exception
{
    printer.printf("ast_err\n%s_write(ast_runtime* rt, %s* %s)\n",
		    cfcnname(msg),
		    ctypefor(msg), cmsgvar(msg));
    printer.println(LBRACE);
    printer.indent();
    printer.println("ast_err status = AST_NOERR;");

    printer.blankline();
    // Generate the field serializations
    for(AST.Field field: msg.getFields()) {
	// add braces to allow local variables 
	printer.println(LBRACE);
	printer.indent();
	if(!isPrimitive(field) && !isEnum(field))
		printer.println("size_t size;");
	if(isRequired(field)) {
	    if(isPrimitive(field) || isEnum(field)) {
		printer.printf("status = ast_write_primitive(rt,%s,%d,&%s->%s);\n",
			       ctypesort(field.getType()),field.getId(),
			       cmsgvar(msg),cfieldvar(field));
	    } else if(isMessage(field)) {
		// Write the tag + count
		printer.printf("status = ast_write_tag(rt,ast_counted,%d);\n",
				field.getId());
	        printer.println("if(status != AST_NOERR) {goto done;}");
	        /* prefix msg serialization with encoded message size */
		printer.printf("size = %s_get_size(rt,%s->%s);\n",
			   cfcnname(field.getType()),cmsgvar(msg),cfieldvar(field));
		printer.println("status = ast_write_count(rt,size);");
		printer.println("if(status != AST_NOERR) {goto done;}");
		printer.printf("status = %s_write(rt,%s->%s);\n",
			       cfcnname(field.getType()),
			       cmsgvar(msg),cfieldvar(field));
	    } else throw new Exception("unknown field type");
	    printer.println("if(status != AST_NOERR) {goto done;}");
	} else if(isOptional(field)) {
	    printer.printf("if(%s->%s.defined) "+LBRACE+"\n",
			    cmsgvar(msg),cfieldvar(field));
	    printer.indent();
	    if(isPrimitive(field) || isEnum(field)) {
		printer.printf("status = ast_write_primitive(rt,%s,%d,&%s->%s.value);\n",
			       ctypesort(field.getType()), field.getId(),
			       cmsgvar(msg),cfieldvar(field));
	    } else if(isMessage(field)) {
		/* precede msg serialization with the tag */
		printer.printf("status = ast_write_tag(rt,ast_counted,%d);\n",
				field.getId());
	        printer.println("if(status != AST_NOERR) {goto done;}");
	        /* prefix msg serialization with encoded message size */
		printer.printf("size = %s_get_size(rt,%s->%s.value);\n",
			   cfcnname(field.getType()),cmsgvar(msg),cfieldvar(field));
		printer.println("status = ast_write_count(rt,size);");
		printer.println("if(status != AST_NOERR) {goto done;}");
		printer.printf("status = %s_write(rt,%s->%s.value);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
	    } else throw new Exception("unknown field type");
	    printer.println("if(status != AST_NOERR) {goto done;}");
	    printer.outdent();
	    printer.printf(RBRACE+"\n");
	} else { // field.getCardinality() == AST.Cardinality.REPEATED
	    if(isPrimitive(field) || isEnum(field)) {
                /* Write the data */
		if(field.isPacked()) {
                    printer.printf("status = ast_write_primitive_packed(rt,%s,%d,&%s->%s);\n",
                                   ctypesort(field.getType()),
                                   field.getId(),
                                   cmsgvar(msg),cfieldvar(field));
		} else {
		    printer.println("int i = 0;");
	            printer.printf("for(i=0;i<%s->%s.count;i++) "+LBRACE+"\n",
		    	           cmsgvar(msg),cfieldvar(field));
	            printer.indent();
		    printer.printf("status = ast_write_primitive(rt,%s,%d,&%s->%s.values[i]);\n",
			       ctypesort(field.getType()), field.getId(),
			       cmsgvar(msg),cfieldvar(field));
	            printer.println("if(status != AST_NOERR) {goto done;}");
	            printer.outdent();
		    printer.println(RBRACE);
		}
	    } else if(isMessage(field)) {
		printer.println("int i;");
                printer.printf("for(i=0;i<%s->%s.count;i++) "+LBRACE+"\n",
                               cmsgvar(msg),cfieldvar(field));
                printer.indent();
                /* precede msg serialization with the tag */
                printer.printf("status = ast_write_tag(rt,ast_counted,%d);\n",
                                field.getId());
                printer.println("if(status != AST_NOERR) {goto done;}");
	        /* prefix msg serialization with encoded message size */
		printer.printf("size = %s_get_size(rt,%s->%s.values[i]);\n",
			   cfcnname(field.getType()),cmsgvar(msg),cfieldvar(field));
		printer.println("status = ast_write_count(rt,size);");
		printer.println("if(status != AST_NOERR) {goto done;}");
                printer.printf("status = %s_write(rt,%s->%s.values[i]);\n",
                            cfcnname(field.getType()),
                            cmsgvar(msg),cfieldvar(field));
                printer.println("if(status != AST_NOERR) {goto done;}");
                printer.outdent();
                printer.println(RBRACE);
	    } else throw new Exception("unknown field type");
	}
	printer.outdent();
	printer.println(RBRACE);
    }
    printer.outdent();
    printer.blankline();
    if(msg.getFields().size() > 0)
        printer.println("done:");
    printer.indent();
    printer.println("return status;");
    printer.outdent();
    printer.blankline();
    printer.printf(RBRACE+" /*%s_write*/\n",msg.getName());
}

void
generate_readfunction(AST.Message msg, Printer printer)
    throws Exception
{
    printer.printf("ast_err\n%s_read(ast_runtime* rt, %s** %sp)\n",
		    cfcnname(msg),
		    ctypefor(msg), cmsgvar(msg));
    printer.println("{");
    printer.indent();
    printer.println("ast_err status = AST_NOERR;");
    printer.println("uint32_t wiretype, fieldno;");
    printer.printf("%s* %s;\n",ctypefor(msg),cmsgvar(msg));

    // Create the target instance
    printer.blankline();
    printer.printf("%s = (%s*)ast_alloc(rt,sizeof(%s));\n",
		    cmsgvar(msg),ctypefor(msg),ctypefor(msg));
    printer.printf("if(%s == NULL) return AST_ENOMEM;\n",cmsgvar(msg));

    printer.blankline();
    printer.println("while(status == AST_NOERR) {");
    printer.indent();
    printer.println("status = ast_read_tag(rt,&wiretype,&fieldno);");
    printer.println("if(status == AST_EOF) {status = AST_NOERR; break;}");
    printer.println("if(status != AST_NOERR) break;");
    // Generate the field de-serializations
    printer.println("switch (fieldno) {");
    for(AST.Field field: msg.getFields()) {
	printer.printf("case %d: {\n",field.getId());
	printer.indent();
	if(isPrimitive(field)) {
	    generate_read_primitive(msg,field,field.isPacked(),printer);
	} else if(isEnum(field)) {
	    generate_read_enum(msg,field,field.isPacked(),printer);
	} else {
	    // Generate needed local variables
	    if(!isPrimitive(field))
	        printer.println("size_t count;");
	    if(isRepeated(field))
	        printer.printf("%s* tmp;\n",ctypefor(field.getType()));
	    // Verify that the wiretype == ast_counted
	    printer.println("if(wiretype != ast_counted) {status=AST_EFAIL; goto done;}");
	    //  get the encoded message size and mark input
  	    generate_mark(printer);
	    generate_read_message(msg,field,printer);
	    generate_unmark(printer);
	}
	printer.println("} break;");
	printer.outdent();
    }
    // add default
    printer.println("default:");
    printer.indent();
    printer.println("status = ast_skip_field(rt,wiretype,fieldno);");
    printer.println("if(status != AST_NOERR) {goto done;}");
    printer.outdent();
    printer.println("}; /*switch*/"); // switch
    printer.outdent();
    printer.println("};/*while*/"); // while
    // Generate defaults for primitive typed optionals
    for(AST.Field field: msg.getFields()) {
	if(isOptional(field)
	   && field.getType().getSort() == AST.Sort.PRIMITIVETYPE) {
	    generate_read_default_primitive(msg,field,printer);
	}
    }
    printer.println("if(status != AST_NOERR) {goto done;}");
    // return result
    printer.printf("if(%sp) *%sp = %s;\n",cmsgvar(msg),cmsgvar(msg),cmsgvar(msg));
    printer.outdent();
    printer.println("done:");
    printer.indent();
    printer.println("return status;");
    printer.outdent();
    printer.printf("} /*%s_read*/\n",msg.getName());
}

void
generate_mark(Printer printer) throws IOException
{
    // Get the count and mark the input
    printer.println("status = ast_read_count(rt,&count);");
    printer.println("if(status != AST_NOERR) {goto done;}");
    printer.println("status = ast_mark(rt,count);");
    printer.println("if(status != AST_NOERR) {goto done;}");
}

void
generate_unmark(Printer printer) throws IOException
{
    printer.println("status = ast_unmark(rt);");
    printer.println("if(status != AST_NOERR) {goto done;}");
}

void
generate_read_primitive(AST.Message msg, AST.Field field, boolean ispacked, Printer printer)
    throws Exception
{
    AST.PrimitiveSort psort = getPrimitiveSort(field);

    switch (field.getCardinality()) {

    case REQUIRED:
        switch (psort) {
        case STRING: case BYTES:
	default: break;
	}
	printer.printf("status = ast_read_primitive(rt,%s,%d,&%s->%s);\n",
			       ctypesort(field.getType()),
			       field.getId(),
			       cmsgvar(msg),cfieldvar(field));
	break;

    case OPTIONAL:
        printer.printf("%s->%s.defined = 1;\n",
                        cmsgvar(msg),cfieldvar(field));
        switch (psort) {
        case STRING:
            printer.printf("%s->%s.value = NULL;\n", cmsgvar(msg),cfieldvar(field));
            break;
        case BYTES:
            printer.printf("%s->%s.value.nbytes = 0;\n", cmsgvar(msg),cfieldvar(field));
            printer.printf("%s->%s.value.bytes = NULL;\n", cmsgvar(msg),cfieldvar(field));
            break;
        default:
            printer.printf("%s->%s.value = 0;\n", cmsgvar(msg),cfieldvar(field));
            break;
        }
        printer.printf("status = ast_read_primitive(rt,%s,%d,&%s->%s.value);\n",
                           ctypesort(field.getType()),
			   field.getId(),
                           cmsgvar(msg),cfieldvar(field));
        break;

    case REPEATED:
	if(ispacked) {
            printer.printf("status = ast_read_primitive_packed(rt,%s,%d,&%s->%s);\n",
                                ctypesort(field.getType()),
				field.getId(),
                                cmsgvar(msg),cfieldvar(field));;
	} else {
            printer.printf("%s tmp;\n",ctypefor(field.getType()));
            printer.printf("status = ast_read_primitive(rt,%s,%d,&tmp);\n",
                                ctypesort(field.getType()),field.getId());
            printer.println("if(status != AST_NOERR) {goto done;}");
	    printer.printf("status = ast_repeat_append(rt,%s,&%s->%s,&tmp);\n",
                            ctypesort(field.getType()),
                            cmsgvar(msg),cfieldvar(field));
	}
        printer.println("if(status != AST_NOERR) {goto done;}");
        break;
    }
}

void
generate_read_enum(AST.Message msg, AST.Field field, boolean ispacked, Printer printer)
    throws Exception
{
    switch (field.getCardinality()) {

    case REQUIRED:
	printer.printf("status = ast_read_primitive(rt,%s,%d,&%s->%s);\n",
			       ctypesort(field.getType()),
			       field.getId(),
			       cmsgvar(msg),cfieldvar(field));
	break;

    case OPTIONAL:
        printer.printf("%s->%s.defined = 1;\n",
                        cmsgvar(msg),cfieldvar(field));
        printer.printf("%s->%s.value = 0;\n", cmsgvar(msg),cfieldvar(field));
        printer.printf("status = ast_read_primitive(rt,%s,%d,&%s->%s.value);\n",
                           ctypesort(field.getType()),
			   field.getId(),
                           cmsgvar(msg),cfieldvar(field));
        break;

    case REPEATED:
	if(ispacked) {
            printer.printf("status = ast_read_primitive_packed(rt,%s,%d,&%s->%s);\n",
                                ctypesort(field.getType()),
				field.getId(),
                                cmsgvar(msg),cfieldvar(field));;
	} else {
            printer.printf("%s tmp;\n",ctypefor(field.getType()));
            printer.printf("status = ast_read_primitive(rt,%s,%d,&tmp);\n",
                                ctypesort(field.getType()),field.getId());
            printer.println("if(status != AST_NOERR) {goto done;}");
	    printer.printf("status = ast_repeat_append(rt,%s,&%s->%s,&tmp);\n",
                            ctypesort(field.getType()),
                            cmsgvar(msg),cfieldvar(field));
	}
        printer.println("if(status != AST_NOERR) {goto done;}");
        break;
    }
}

void
generate_read_message(AST.Message msg, AST.Field field, Printer printer)
    throws Exception
{
    switch (field.getCardinality()) {
    case REQUIRED:
        printer.printf("status = %s_read(rt,&%s->%s);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
	printer.println("if(status != AST_NOERR) {goto done;}");
	break;

    case OPTIONAL:
	printer.printf("%s->%s.defined = 1;\n",
			    cmsgvar(msg),cfieldvar(field));
  	printer.printf("%s->%s.value = NULL;\n",
			    cmsgvar(msg),cfieldvar(field));
	printer.printf("status = %s_read(rt,&%s->%s.value);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
        printer.println("if(status != AST_NOERR) {goto done;}");
        break;

    case REPEATED:
	printer.printf("status = %s_read(rt,&tmp);\n",ctypefor(field.getType()));
        printer.printf("status = ast_repeat_append(rt,%s,&%s->%s,&tmp);\n",
				ctypesort(field.getType()),
				cmsgvar(msg),cfieldvar(field));
	printer.println("if(status != AST_NOERR) {goto done;}");
    }
}

void
generate_read_default_primitive(AST.Message msg, AST.Field field, Printer printer)
    throws Exception
{
    AST.PrimitiveSort psort = ((AST.PrimitiveType)(field.getType())).getPrimitiveSort();
    String field_default = defaultfor(field);

    if(field_default == null) {
        switch (psort) {
	case STRING: field_default = null; break;
	case BYTES: field_default = null; break;
	case BOOL: field_default = "true"; break;
        default: field_default = "0"; break;
        }
    } 

    // Do some conversions for some types
    switch (psort) {
    case BOOL: {
        if("true".equalsIgnoreCase(field_default.toString()))
	    field_default = "1";
        else if("false".equalsIgnoreCase(field_default.toString()))
	    field_default = "0";
    } break;
    case STRING:
        if(field_default != null)
	    field_default = '"' + AuxFcns.escapify(field_default,'"',
				      AuxFcns.EscapeMode.EMODE_C) + '"';
     break;
    default: break;
    }

    printer.printf("if(!%s->%s.defined) {\n",
                        cmsgvar(msg),cfieldvar(field));
    printer.indent();
    switch (psort) {
    case STRING:
        printer.printf("%s->%s.value = %s;\n",
                        cmsgvar(msg),cfieldvar(field),
			(field_default==null?"NULL":field_default)
			);
	break;
    case BYTES:
        printer.printf("%s->%s.value.nbytes = %d;\n",
                        cmsgvar(msg),cfieldvar(field),
			(field_default == null ? 0:field_default.length()/2)
			);
        printer.printf("%s->%s.value.bytes = %s;\n",
                        cmsgvar(msg),cfieldvar(field),
			(field_default==null?"NULL":field_default)
			);
	break;
    default:
        printer.printf("%s->%s.value = %s;\n",
                        cmsgvar(msg),cfieldvar(field),field_default);
	break;
    }
    printer.outdent();
    printer.println("}");
}

void generate_reclaimfunction(AST.Message msg, Printer printer)
    throws Exception
{
    printer.printf("ast_err\n%s_reclaim(ast_runtime* rt, %s* %s)\n",
		    cfcnname(msg),
		    ctypefor(msg), cmsgvar(msg));
    printer.println("{");
    printer.indent();
    printer.println("ast_err status = AST_NOERR;");
    printer.blankline();

    // Generate the field reclaims
    for(AST.Field field: msg.getFields()) {
        AST.Sort sort = field.getType().getSort();
        AST.PrimitiveSort psort = getPrimitiveSort(field);
	// Only need to reclaim fields whose type is a message pointer
	// or a string or a byte string.
	switch (sort) {
	case MESSAGE: break;
	case PRIMITIVETYPE:
	    switch (getPrimitiveSort(field)) {
	    case STRING: break;
	    case BYTES: break;
	    default: continue;
	    }
	    break;
	default: continue;
	}
	printer.println(LBRACE);
	printer.indent();
	if(isRequired(field)) {
	    if(isPrimitive(field)) {
		if(psort == AST.PrimitiveSort.STRING)
		    printer.printf("status = ast_reclaim_string(rt,%s->%s);\n",
			       cmsgvar(msg),cfieldvar(field));
		else if(psort == AST.PrimitiveSort.BYTES) 
	   	    printer.printf("status = ast_reclaim_bytes(rt,&%s->%s);\n",
			       cmsgvar(msg),cfieldvar(field));
	    } else if(isEnum(field)) { // do nothing
	    } else if(isMessage(field)) {
		printer.printf("status = %s_reclaim(rt,%s->%s);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
	    } else throw new Exception("unknown field type");
	    printer.println("if(status != AST_NOERR) {goto done;}");
	} else if(isOptional(field)) {
	    printer.printf("if(%s->%s.defined) {\n",
			    cmsgvar(msg),cfieldvar(field));
	    printer.indent();
	    if(isPrimitive(field)) {
		if(psort == AST.PrimitiveSort.STRING)
		    printer.printf("status = ast_reclaim_string(rt,%s->%s.value);\n",
			       cmsgvar(msg),cfieldvar(field));
		else if(psort == AST.PrimitiveSort.BYTES) 
	   	    printer.printf("status = ast_reclaim_bytes(rt,&%s->%s.value);\n",
			       cmsgvar(msg),cfieldvar(field));
	    } else if(isEnum(field)) { // do nothing
	    } else if(isMessage(field)) {
		printer.printf("status = %s_reclaim(rt,%s->%s.value);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
	    } else throw new Exception("unknown field type");
	    printer.println("if(status != AST_NOERR) {goto done;}");
	    printer.outdent();
	    printer.printf("}\n");
	} else { // field.getCardinality() == AST.Cardinality.REPEATED
	    printer.println("int i;");
	    printer.printf("for(i=0;i<%s->%s.count;i++) {\n",
			    cmsgvar(msg),cfieldvar(field));
	    printer.indent();
	    if(isPrimitive(field)) {
		if(psort == AST.PrimitiveSort.STRING)
		    printer.printf("status = ast_reclaim_string(rt,%s->%s.values[i]);\n",
			       cmsgvar(msg),cfieldvar(field));
		else if(psort == AST.PrimitiveSort.BYTES) 
	   	    printer.printf("status = ast_reclaim_bytes(rt,&%s->%s.values[i]);\n",
			       cmsgvar(msg),cfieldvar(field));
	    } else if(isEnum(field)) { //do nothing
	    } else if(isMessage(field)) {
		printer.printf("status = %s_reclaim(rt,%s->%s.values[i]);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
	    } else throw new Exception("unknown field type");
	    printer.println("if(status != AST_NOERR) {goto done;}");
	    printer.outdent();
	    printer.printf("}\n");
	    printer.printf("ast_free(rt,%s->%s.values);\n",
			    cmsgvar(msg),field.getName());
	}
	printer.outdent();
	printer.println(RBRACE);
    }
    // Finally reclaim the whole message
    printer.printf("ast_free(rt,(void*)%s);\n",cmsgvar(msg));
    printer.println("goto done;");
    printer.outdent();
    printer.blankline();
    printer.println("done:");
    printer.indent();
    printer.println("return status;");
    printer.outdent();
    printer.blankline();
    printer.printf("} /*%s_reclaim*/\n",msg.getName());
}


void generate_sizefunction(AST.Message msg, Printer printer)
    throws Exception
{
    printer.printf("size_t\n%s_get_size(ast_runtime* rt, %s* %s)\n",
		    cfcnname(msg),
		    ctypefor(msg), cmsgvar(msg));
    printer.println("{");
    printer.indent();
    printer.println("size_t totalsize = 0;");
    if(msg.getFields().size() > 0)
        printer.println("size_t fieldsize = 0;");
    printer.blankline();

    // sum the field sizes; make sure to include the tag if not packed
    for(AST.Field field: msg.getFields()) {
	switch (field.getType().getSort()) {
	case MESSAGE: break;
	case ENUM: break;
	case PRIMITIVETYPE: break;
	default: continue;
	}

	printer.println(LBRACE);
	printer.indent();

	if(isRequired(field)) {
	    if(isPrimitive(field) || isEnum(field)) {
		// Add in the prefix tag size
		printer.printf("fieldsize += ast_get_tagsize(rt,%s,%d);\n",
			(field.isPacked()?ctypesort(field.getType()):"ast_counted"),
			field.getId());
		printer.printf("fieldsize += ast_get_size(rt,%s,&%s->%s);\n",
			       ctypesort(field.getType()),
			       cmsgvar(msg),cfieldvar(field));
	    } else if(isMessage(field)) {
		printer.printf("fieldsize += %s_get_size(rt,%s->%s);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
		// Add in the prefix tag size
		printer.printf("fieldsize += ast_get_tagsize(rt,%s,%d);\n",
			(field.isPacked()?ctypesort(field.getType()):"ast_counted"),
			field.getId());
		// Add in the prefix count
		printer.println("fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);");
	    } else throw new Exception("unknown field type");
	} else if(isOptional(field)) {
	    printer.printf("if(%s->%s.defined) {\n",
			    cmsgvar(msg),cfieldvar(field));
	    printer.indent();
	    if(isPrimitive(field) || isEnum(field)) {
		// Add in the prefix tag size
		printer.printf("fieldsize += ast_get_tagsize(rt,%s,%d);\n",
			(field.isPacked()?ctypesort(field.getType()):"ast_counted"),
			field.getId());
		printer.printf("fieldsize += ast_get_size(rt,%s,&%s->%s.value);\n",
			       ctypesort(field.getType()),
			       cmsgvar(msg),cfieldvar(field));
	    } else if(isMessage(field)) {
		printer.printf("fieldsize += %s_get_size(rt,%s->%s.value);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
		// Add in the prefix count
		printer.println("fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);");
		// Add in the prefix tag size
		printer.printf("fieldsize += ast_get_tagsize(rt,%s,%d);\n",
			(field.isPacked()?ctypesort(field.getType()):"ast_counted"),
			field.getId());
	    } else throw new Exception("unknown field type");
	    printer.outdent();
	    printer.printf("}\n");
	} else { // field.getCardinality() == AST.Cardinality.REPEATED
	    printer.println("int i;");
	    printer.printf("for(i=0;i<%s->%s.count;i++) {\n",
			    cmsgvar(msg),cfieldvar(field));
	    printer.indent();
	    if(isPrimitive(field) || isEnum(field)) {
		// Add in the prefix tag size
		printer.printf("fieldsize += ast_get_tagsize(rt,%s,%d);\n",
			(field.isPacked()?ctypesort(field.getType()):"ast_counted"),
			field.getId());
		printer.printf("fieldsize += ast_get_size(rt,%s,&%s->%s.values[i]);\n",
			       ctypesort(field.getType()),
			       cmsgvar(msg),cfieldvar(field));
	    } else if(isMessage(field)) {
		printer.printf("fieldsize += %s_get_size(rt,%s->%s.values[i]);\n",
			    cfcnname(field.getType()),
			    cmsgvar(msg),cfieldvar(field));
		// Add in the prefix count
		printer.println("fieldsize += ast_get_size(rt,ast_uint32,&fieldsize);");
		// Add in the prefix tag size
		printer.printf("fieldsize += ast_get_tagsize(rt,%s,%d);\n",
			(field.isPacked()?ctypesort(field.getType()):"ast_counted"),
			field.getId());
	    } else throw new Exception("unknown field type");
	    printer.outdent();
	    printer.println(RBRACE);
	}
        printer.println("totalsize += fieldsize;");
	printer.outdent();
	printer.println(RBRACE);
    }

    printer.println("return totalsize;");
    printer.outdent();
    printer.blankline();
    printer.printf("} /*%s_get_size*/\n",msg.getName());
    printer.blankline();
}


//////////////////////////////////////////////////

// Convert a msg name to an acceptable C variable name
String
cmsgvar(AST.Message msg)
{
    String cname = msg.getName().toLowerCase() + "_v";
    return cname;
}

// Convert a field name to an acceptable C variable name
String
cfieldvar(AST.Field field)
{
    String cname = converttocname(field.getName());
    return cname;
}

String
converttocname(String name)
{
    /* C and protobuf identifiers are same,
       except we ned to rename C keywords
    */
    if(Arrays.binarySearch((Object[])CKeywords,(Object)name) >= 0)
	name = name + "_";
    return name;
}

String
cfcnname(AST.Type asttype)
{
    String typename = null;
    if(asttype.getSort() == AST.Sort.PRIMITIVETYPE) {
	typename = ((AST.PrimitiveType)asttype).getPrimitiveSort().getName();
	typename = FCNPREFIX + typename;
    } else if(asttype.getSort() == AST.Sort.ENUM
	      || asttype.getSort() == AST.Sort.MESSAGE) {
	typename = asttype.getName();
    } else { // Illegal
	System.err.println("Illegal type: "+asttype.getName());

    }
    return converttocname(typename);
}

String
ctypefor(AST.Type asttype)
{
    String typ = null;

    if(asttype.getSort() == AST.Sort.PRIMITIVETYPE) {
	switch (((AST.PrimitiveType)asttype).getPrimitiveSort()) {
	case SINT32:
	case SFIXED32:
	case INT32:   typ = "int32_t"; break;

	case FIXED32:
	case UINT32:   typ = "uint32_t"; break;

	case SINT64:
	case SFIXED64:
	case INT64:   typ = "int64_t"; break;

	case FIXED64:
	case UINT64:   typ = "uint64_t"; break;

	case FLOAT:   typ = "float"; break;
	case DOUBLE:  typ = "double"; break;

	case BOOL:    typ = "bool_t"; break;
	case STRING:  typ = "char*"; break;

	case BYTES:   typ = "bytes_t"; break;
	// No default because we want the compiler to complain if any new
	// types are added.
	}
    } else if(asttype.getSort() == AST.Sort.ENUM
	      || asttype.getSort() == AST.Sort.MESSAGE) {
	typ = asttype.getName();
    } else { // Illegal
	System.err.println("Cannot translate type to C Type: "+asttype.getName());

    }
    return typ;
}

String
defaultfor(AST.Field field)
{
    AST.Type fieldtype = field.getType();
    if(isRepeated(field)) {
	// repeated default is always a list of the type of the field
	return String.format("new ArrayList<%s>()",ctypefor(fieldtype));
    } else {
	// See if the field has a defined default
	Object value = field.optionLookup("DEFAULT");
	if(value == null) return null;
	if(fieldtype.getSort() == AST.Sort.PRIMITIVETYPE) {
	    return (String)value;
	} else if(fieldtype.getSort() == AST.Sort.ENUM) {
	    return (String)value;
	} else if(fieldtype.getSort() == AST.Sort.MESSAGE) {
	    return String.format("%s.getDefaultInstance()",
				    fieldtype.getName());
	}
    }
    return null;
}


String
ctypesort(AST.Type asttype)
{
    String sort = null;
    if(asttype.getSort() == AST.Sort.PRIMITIVETYPE) {
	switch (((AST.PrimitiveType)asttype).getPrimitiveSort()) {
	case SINT32: return "ast_sint32";
	case SFIXED32: return "ast_sfixed32";
	case UINT32: return "ast_uint32";
	case FIXED32: return "ast_fixed32";
	case INT32: return "ast_int32";
	case SINT64: return "ast_sint64";
	case SFIXED64: return "ast_sfixed64";
	case UINT64: return "ast_uint64";
	case FIXED64: return "ast_fixed64";
	case INT64: return "ast_int64";
	case FLOAT: return "ast_float";
	case DOUBLE: return "ast_double";
	case BOOL: return "ast_bool";
	case STRING: return "ast_string";
	case BYTES: return "ast_bytes";
	// No default because we want the compiler to complain if any new
	// types are added.
	}
    } else if(asttype.getSort() == AST.Sort.ENUM) {
        return "ast_enum";
    } else if(asttype.getSort() == AST.Sort.MESSAGE) {
        return "ast_message";
    } else {
	System.err.println("Cannot translate type to C sort:" + asttype.getSort().toString());
    }
    return null;
}

} // CGenerator
