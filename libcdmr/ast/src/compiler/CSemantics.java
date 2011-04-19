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

import java.util.List;
import java.util.ArrayList;
import java.io.*;

import static unidata.protobuf.compiler.AST.*;
import static unidata.protobuf.compiler.Debug.*;

/**
 * Implement any needed semantic tests
 *for generating C code.
 */

public class CSemantics extends Semantics
{

//////////////////////////////////////////////////

AST.Root root = null;
ASTFactory factory = null;
String[] argv = null;

//////////////////////////////////////////////////
// Constructor
public CSemantics() {}

//////////////////////////////////////////////////

public boolean
initialize(AST.Root root, String[] argv, ASTFactory factory)
{
    this.factory = factory;
    this.root = root;
    this.factory = factory;
    this.argv = argv;

    // Define the predefined options
    List<OptionDef> odefs = root.getOptionDefs();

    // Add the predefined optiondefs; user defined options
    // will have already been added by parser
    odefs.add(new OptionDef("c_file", "string"));
    odefs.add(new OptionDef("include", "string"));
    odefs.add(new OptionDef("config_h", "bool"));

    return true;
}

//////////////////////////////////////////////////

public boolean
process(AST.Root root)
{
    if(!checkdupctypes(root)) return false;
    if(!fixstringoptions(root)) return false;
    return true;
}

boolean
checkdupctypes(AST.Root root)
{
    // Since the C code generates all message types as top-level
    // structs, we must check to see that we do not have any
    // duplicates; similarly for enums.
    List<AST> allenums = new ArrayList<AST>();
    List<AST> allmsgs = new ArrayList<AST>();
    for(AST node: root.getNodeSet()) {
        switch (node.getSort()) {
        case ENUM:
	    for(AST e: allenums) {
		if(e.getName().equals(node.getName()))
		    return duperror(node,e,"Duplicate Enum Names; will prevent proper C code generation");
            }
            break;
        case MESSAGE:
	    for(AST msg: allmsgs) {
		if(msg.getName().equals(node.getName()))
		    return duperror(node,msg,"Duplicate Message Names; will prevent proper C code generation");
            }
            break;
        default:
	    break;
        }
    }
    return true;
}

boolean
fixstringoptions(AST.Root root)
{
    for(AST node: root.getNodeSet()) {
	if(node.getSort() != AST.Sort.OPTION) continue;
	AST.Option option = (AST.Option)node;
	if(!option.isStringValued()) continue;
        String value = option.getValue();
	/* Add quotes and escapes */
	value = '"'
	        + AuxFcns.escapify(value,'"',AuxFcns.EscapeMode.EMODE_C)
		+ '"';
        option.setValue(value);
    }
    return true;
}

} // class Semantics
