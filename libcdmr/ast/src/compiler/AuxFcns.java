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

import java.util.*;
import java.io.*;

public class AuxFcns
{

enum EscapeMode {EMODE_C};

static int uid = 0; // for generating unique ids.

static int nextuid() {return ++uid;}

// Define which nodes are scope units
static boolean
isscopeunit(AST node)
{
    switch (node.getSort()) {
    case FILE: case FIELD: case ENUM: case MESSAGE: return true;
    default: break;
    }
    return false;
}

/* Compute the scope name for any kind of node */
static String
computescopename(AST node)
{
    String scopename = node.getName(); // default
    // For some nodes, we may need to modify the qualified name
    switch (node.getSort()) {
    case EXTEND:
	scopename = String.format("%s_%d","$extend",nextuid());
	break;
    case FILE:    
	// If the file has a package, then use that name.
	AST.File file = (AST.File)node;
	if(file.getFilePackage() != null) {
	    scopename = file.getFilePackage().getName();
	} else {
	    scopename = "";
	    /*
	    // remove leading path
	    int index = scopename.lastIndexOf("/");
	    if(index >= 0)
		scopename = scopename.substring(index+1,scopename.length());
	    // remove trailing .proto (if any) unless remaining name is ""
	    if(scopename.endsWith(".proto")) {
	        index = scopename.lastIndexOf(".");
	        if(index > 0) // not >= to avoid ""
		    scopename = scopename.substring(0,index);
	    }
            */
	} break;
    case ROOT:
        scopename = "";
        break;
    default: break; // use default
    }
    return scopename;
}

// Compute path from root to this node.
// Root is 0 entry, file is 1 entry  and the node is last entry
static List<AST>
computepath(AST node)
{
    List<AST> path = new ArrayList<AST>();
    while(node != null) {
        path.add(0,node);
        node = node.getParent();
    }
    return path;
}

/* Compute the qualified name for any kind of node */
static String
computequalifiedname(AST node)
{
    List<AST> path = computepath(node);
    String qualname = "";
    for(int i=1;i<path.size()-1;i++) {
	AST p = path.get(i);
	if(AuxFcns.isscopeunit(p))
            qualname = qualname + "." + p.getScopeName();
    }
    // Unconditionally add final node
    AST p = path.get(path.size()-1);
    qualname = qualname + "." + p.getScopeName();

    return qualname;
}

static public String
locatefile(String suffix, List<String> includepaths)
{
    suffix = suffix.trim();
    if(suffix.charAt(0) == '/') return suffix;
    if(includepaths != null)
        for(int i=0;i<includepaths.size();i++) {
            String path = includepaths.get(i)+"/"+suffix;
            File f = new File(path);
            if(f.canRead()) return path;
        }
    // Try raw suffix as last resort
    File f = new File(suffix);
    if(f.canRead()) return suffix;
    return null;
}

// Replace '.' characters with '_'
static public String
escapedname(String name)
{
    return name.replace('.','_');
}

/**
Given a reference to a type name in the context of a specified node,
Try to locate all nodes that might match using the following rules:
1. Check for primitive type names or absolute path typename
2. Walk up the parent chain until the root file is reached.
3. At each parent, try to match the typename against all possible
   paths starting at that parent and ending at a type node
4. return the complete set of matching types to allow caller to
   decide which is the correct one.

 * @param typename to search for
 * @param node defining the innermost context for the name
 & @return List<AST.Type> of matching nodes
 */

static List<AST.Type>
findtypebyname(String typename, AST node)
{
    AST.Root root = node.getRoot();
    List<AST.Type> typematches = new ArrayList<AST.Type>();
    // First, see is this a primitive type name
    List<AST.PrimitiveType> primitives = root.getPrimitiveTypes();
    for(AST.PrimitiveType pt: primitives) 
        if(typename.equals(pt.getName())) {typematches.add(pt); return typematches;}

    // If the typename is absolute, then find it
    if(typename.charAt(0) == '.') {
	// find exact match(es)
	for(AST.File file : root.getFileSet()) {
	    for(AST ast: file.getNodeSet()) {
	        if(ast instanceof AST.Type
		   && ast.getQualifiedName().equals(typename))
		    typematches.add((AST.Type)ast);
	    }
	}
        return typematches;
    }

    // Finally, collect all the type nodes whose qualified name
    // suffix matches the typename
    // Note that since multiple files may be equivalent in scope
    // because they have no associated package, but we can skip
    // all files that have a different package name
    AST.File file1 = node.getSrcFile();
    AST.Package p1 = file1.getFilePackage();
    for(AST.File file2: root.getFileSet()) {
        AST.Package p2 = file2.getFilePackage();
        if(file2 != file1 && p1 != p2) {
            // See if the package names match
            if(p1 == null || p2 == null || !p1.getName().equals(p2.getName()))
                continue; // not a match
        }
        for(AST subnode: file2.getNodeSet()) {
            if(subnode instanceof AST.Type
               && subnode.getQualifiedName().endsWith("."+typename)) {
               typematches.add((AST.Type)subnode);
            }
        }
    }
    return typematches;
}

static List<String>
parsepath(String name)
{
    String[] segments = name.split("[.]");
    List<String> slist = new ArrayList<String>();
    for(String s: segments)  slist.add(s);
    return slist;
}

/*
Given a node, see if the given path can match starting at that node
*/
static boolean
matchpath(List<String> path, AST node, List<AST> matches)
{
    assert(path.size() > 0);
    // match the first element against this node
    if(!node.getName().equals(path.get(0))) return false; // cannot possibly match
    // First path element matches; are we done?
    if(path.size() == 1) {
	matches.add(node);
	return true;
    }
    // Try the rest recursively
    if(node.getChildSet() != null) {
       for(AST ast : node.getChildSet()) { 
	    matchpath(path.subList(1,path.size()),ast,matches); // recurse
        }
    }
    return (matches.size() > 0);
}

// Collect path of parent nodes upto
// and (optionally) including the package
static void
collectpath(AST ast, List<AST> path, boolean thrupackage)
{
    path.clear();
    AST parent = ast;
    while(parent.getSort() != AST.Sort.ROOT) {
        if(parent.getSort() == AST.Sort.PACKAGE && !thrupackage) break;
        path.add(0,parent);
        parent = parent.getParent();
    }
}


// Printable chars that must be escaped

// Add in escapes to a string
static String
escapify(String s, char quotemark, EscapeMode emode)
{
    StringBuilder es = new StringBuilder();
    for(char c: s.toCharArray()) {
	switch (emode) {
	case EMODE_C:
	    if(c == '\n') es.append("\\n");
	    else if(c == '\r') es.append("\\r");
	    else if(c == '\t') es.append("\\t");
	    else if(c < ' ' || c == '\177') {
		// octal encoding
		String octal = Integer.toOctalString((int)c);
		while(octal.length() < 3) octal = '0' + octal;
		es.append("\\"+octal);
	    } else if(c == quotemark) {
		es.append("\\\"");
	    } else
		es.append(c);
	    break;
	}
    }    
    return es.toString();
}


static String
getBaseName(String base)
{
    // strip off the path part
    int index = base.lastIndexOf("/");
    if(index >= 0) base = base.substring(index+1,base.length());
    // strip off any extension
    index = base.lastIndexOf(".");
    if(index > 0) base = base.substring(0,index);
    return base;
}

static String
getFilePrefix(String path)
{
    // strip off the path part and return it
    int index = path.lastIndexOf("/");
    if(index < 0)
	path = "";
    else
	path = path.substring(0,index);
    return path;
}

static public List<AST.Field>
sortFieldIds(List<AST.Field> fields)
{
    if(fields == null) return null;
    // Assume # fields is small enough that an insertion sort is ok
    List<AST.Field> local = new ArrayList<AST.Field>(fields);
    List<AST.Field> sorted = new ArrayList<AST.Field>();
    AST.Field min = null;
    while(local.size() > 0) {
	min = null;
	for(AST.Field field: local) {
	    if(min == null || (field.getId() < min.getId()))
		min = field;
	}
	sorted.add(min);  local.remove(min);
    }
    return sorted;
}

static boolean
getbooleanvalue(String optionvalue)
{
    boolean boolvalue = false;
    if(optionvalue == null)
	boolvalue = false;
    else if(optionvalue.equalsIgnoreCase("true"))
	boolvalue = true;
    else if(optionvalue.equalsIgnoreCase("false"))
	boolvalue = false;
    else try {
	int num = Integer.parseInt(optionvalue);
	boolvalue = (num != 0);
	} catch (NumberFormatException nfe) {} // ignore
    return boolvalue;
}


static AST.OptionDef
getOptionDef(AST node, String key)
{
    AST.Root root = node.getRoot();
    for(AST.OptionDef od: root.getOptionDefs()) {
	if(od.name.equals(key)) return od;
    }
    return null;
}


} // class AuxFcns

