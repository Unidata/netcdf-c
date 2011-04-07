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

public class ProtobufSemantics extends Semantics
{

//////////////////////////////////////////////////
// Constructor
public ProtobufSemantics()
{
}

static boolean ctrl = true;

//////////////////////////////////////////////////

AST.Root root = null;
ASTFactory factory = null;
String[] argv = null;

//////////////////////////////////////////////////

public boolean
initialize(AST.Root root, String[] argv, ASTFactory factory)
{
    this.root = root;
    this.factory = factory;
    this.argv = argv;

    // Define the predefined options
    List<OptionDef> odefs = root.getOptionDefs();

    // Add the predefined optiondefs; user defined options
    // will have already been added by parser
    odefs.add(new OptionDef("java_package", "string"));
    odefs.add(new OptionDef("java_outer_classname", "string"));
    odefs.add(new OptionDef("optimize_for", "string"));
    odefs.add(new OptionDef("packed", "bool"));
    odefs.add(new OptionDef("cc_generic_services", "bool"));
    odefs.add(new OptionDef("java_generic_services", "bool"));
    odefs.add(new OptionDef("py_generic_services", "bool"));
    odefs.add(new OptionDef("deprecated", "bool"));
    odefs.add(new OptionDef("compile", "string"));
    odefs.add(new OptionDef("declare", "bool"));
    odefs.add(new OptionDef("DEFAULT", "string")); // but see maptions() below

    return true;    
}

//////////////////////////////////////////////////

public boolean
process(AST.Root root)
{
    PrintWriter w = new PrintWriter(System.err);
    Debug.printprops.qualified = true;
    //    Debug.printprops.useuid = true;

    if (!rootsetup(root)) return false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\nrootsetup:");
        Debug.printTreeNodes(root, w);
    }

    if (!collectglobalnodesets(root, root)) return false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\ncollectglobalnodesets:");
        Debug.printTreeNodes(root, w);
    }

    if (!collectsubtrees(root)) return false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\ncollectsubtrees:");
        Debug.printTreeNodes(root, w);
    }

    if (!setfilelink(root)) return false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\nsetfilelink:");
        Debug.printTreeNodes(root, w);
    }

    if (!setpackagelink(root)) return false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\nsetpackagelink:");
        Debug.printTreeNodes(root, w);
    }

    if (!collectnodesets(root)) return false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\ncollectnodesets:");
        Debug.printTreeNodes(root, w);
    }

    if (!setscopenames(root)) return false;
    Debug.printprops.useuid = false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\nsetscopenames:");
        Debug.printTreeNodes(root, w);
    }

    if (!qualifynames(root)) return false;
    Debug.printprops.useuid = false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\nqualifynames:");
        Debug.printTreeNodes(root, w);
    }

    if (!checkduplicatenames(root)) return false;
    Debug.printprops.useuid = false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\ncheckduplicatenames:");
        Debug.printTreeNodes(root, w);
    }

    if (!dereference(root)) return false;
    if (ctrl && Debug.enabled("trace.semantics.steps")) {
        w.println("\ndereference:");
        Debug.printTreeNodes(root, w);
    }

    if (!checkmisc1(root)) return false;

    if (!applyExtensions(root)) return false;

    if (!defineoptions(root)) return false;

    if (!mapoptions(root)) return false;

    Debug.resetprintprops();
    // Print two ways
    if (ctrl && Debug.enabled("trace.semantics")) {
        System.err.println("-------------------------");
        System.err.println("Tree Format:");
        Debug.printTree(root, w);
        w.flush();
    }
    if (ctrl && Debug.enabled("trace.semantics")) {
        System.err.println("-------------------------");
        System.err.println("Proto Format:");
        Debug.print(root, w);
        w.flush();
    }

    return true;
}

/*IGNORE
void verify(AST.Root root)
{
    // verify that everynode is in allnodes
    for(AST ast : root.getNodeSet()) ast.visited = true;
    System.err.println("begin missing");
    for(AST ast : root.getNodeSet()) {
	if(ast.visited) continue;
        if(ast instanceof AST.PrimitiveType) continue;
	// We have an uncaptured node
	System.err.printf("%s: %s\n",ast.getName(),ast.getSort().getName());
    }
    System.err.println("end missing");
}
*/

/**
 * Pass does the following:
 * - Place the set of primitive type nodes in the root
 * - Place the set of predefined options in the option definitions
 * - Cross Link package to file
 * - Check for multiple package decls in a file
 * - check for multiple instances of same file
 *
 * @param root The AST root
 * @param root The AST tree root
 * @return true if the processing succeeded.
 */

boolean
rootsetup(AST.Root root)
{
    // Place the set of primitive type nodes in the root
    // Construct the primitive type nodes
    List<AST.PrimitiveType> primitives = new ArrayList<AST.PrimitiveType>();
    for (AST.PrimitiveSort prim : AST.PrimitiveSort.values()) {
        AST.PrimitiveType pt = factory.newPrimitiveType(prim);
        primitives.add(pt);
    }
    root.setPrimitiveTypes(primitives);

    // Remove duplicate imported files
    List<AST.File> dups = new ArrayList<AST.File>();
    for (AST ast : root.getChildSet()) {
        if (ast.getSort() != AST.Sort.FILE) continue;
        AST.File file = (AST.File) ast;
        for (AST ast1 : root.getChildSet()) {
            if (ast1.getSort() != AST.Sort.FILE) continue;
            if (ast1 == file) continue;
            AST.File file1 = (AST.File) ast1;
            if (file1.getName().equals(file.getName())) {
                dups.add(file);
            }
        }
    }
    for (AST.File file : dups) {
        root.getChildSet().remove(file);
    }

    // Cross link files and packages
    for (AST ast : root.getChildSet()) {
        if (ast.getSort() != AST.Sort.FILE) continue;
        AST.File file = (AST.File) ast;
        AST.Package p = null;
        for (AST decl : file.getChildSet()) {
            if (decl.getSort() == AST.Sort.PACKAGE) {
                if (p != null) {
                    // complain and ignore
                    semerror(file, "File: " + file.getName() + "; multiple package declarations");
                } else {
                    p = (AST.Package) decl;
                }
            }
        }
        file.setFilePackage(p);
        if (p != null) { // move to front of decllist
            file.getChildSet().remove(p);
            file.getChildSet().add(0, p);
            p.setSrcFile(file);
        }
    }
    return true;
}

/**
 * Pass does the following:
 * - Collect all nodes
 * - Collect all package nodes and all file nodes
 * - link all nodes to the root
 * - link children to parent
 *
 * @param node The current node being walked
 * @param root The AST tree root
 * @return true if the processing succeeded.
 */

boolean
collectglobalnodesets(AST node, AST.Root root)
{
    if (node == null) return true;
    root.getNodeSet().add(node);
    node.setRoot(root);
    // Collect selected sets of nodes
    switch (node.getSort()) {
    case FILE:
        if (!root.getFileSet().contains((AST.File) node)) {
            root.getFileSet().add((AST.File) node);
        }
        // Make sure the package is included
        if (!collectglobalnodesets(((AST.File) node).getFilePackage(), root)) return false;
        break;
    case PACKAGE:
        AST.Package p = (AST.Package) node;
        boolean match = false;
        // See if this is a duplicate package?
        for (AST.Package p2 : root.getPackageSet()) {
            if (p2.getName().equals(p.getName())) {
                if (Debug.enabled("trace.duplicate.packages")) {
                    duperror(p, p2, "Duplicate Packages: " + p.getName());
                }
                match = true;
                break;
            }
        }
        if (!match) {
            root.getPackageSet().add(p);
            // Make sure package node set is defined
            p.setNodeSet(new ArrayList<AST>());
        }
        break;
    default:
        break;
    }
    // recurse on child set
    if (node.getChildSet() != null) {
        for (AST subnode : node.getChildSet()) {
            subnode.setParent(node);
            if (!collectglobalnodesets(subnode, root)) return false;
        }
    }
    return true;
}

/**
 * Pass does the following:
 * - for each node collect the set of nodes in the subtree below it
 * [this is, roughly, the transitive closure of the child set]
 * Special cases:
 * 1. Each file node excludes any contained file subtrees
 *
 * @param root AST.Root
 * @return true if the processing succeeded.
 */
boolean
collectsubtrees(AST.Root root)
{
    for (AST.File file : root.getFileSet()) {
        if (!collectsubtreesr(file)) return false;
    }
    return true;
}

/**
 * Recursive part of collectsubtrees
 *
 * @param thisnode An AST node whose subtree will be computed
 * @return true if no error, false otherwise
 */
boolean
collectsubtreesr(AST thisnode)
{
    List<AST> thisset = new ArrayList<AST>();
    for (AST node : thisnode.getChildSet()) {
        if (node.getSort() != AST.Sort.FILE) {
            if (!collectsubtreesr(node)) return false;
        }
        thisset.add(node);
        thisset.addAll(node.getNodeSet());
    }
    thisnode.setNodeSet(thisset);
    return true;
}

/**
 * Pass does the following:
 * - Link subnodes to src file
 * - Verify that each file has at most one package
 *
 * @param root        The current root
 * @return true if the processing succeeded.
 */

boolean
setfilelink(AST.Root root)
{
    for (AST.File f : root.getFileSet()) {
        for (AST node : f.getNodeSet()) {
            node.setSrcFile(f);
        }
    }
    return true;
}

/**
 * Pass does the following:
 * - Link subnodes to closest containing package
 * [note: implements #include semantics]
 *
 * @param root AST.root
 * @return true if the processing succeeded.
 */

boolean
setpackagelink(AST.Root root)
{
    for (AST.File f : root.getFileSet()) {
        AST.Package p = f.getFilePackage();
        if (p == null) continue;
        for (AST ast : f.getNodeSet()) {
            ast.setPackage(p);
        }
    }
    return true;
}

/**
 * Pass does the following:
 * - Fill in various fields for each node type
 *
 * @param root The ast tree root
 * @return true if the processing succeeded.
 */

boolean
collectnodesets(AST.Root root)
{
    List<AST> allnodes = root.getNodeSet();
    // Divide children lists
    for (AST node : allnodes) {
        switch (node.getSort()) {
        case FILE:
            AST.File astfile = (AST.File) node;
            astfile.setMessages(new ArrayList<AST.Message>());
            astfile.setExtends(new ArrayList<AST.Extend>());
            astfile.setEnums(new ArrayList<AST.Enum>());
            astfile.setServices(new ArrayList<AST.Service>());
            for (AST ast : astfile.getNodeSet()) {
                switch (ast.getSort()) {
                case ENUM:
                    astfile.getEnums().add((AST.Enum) ast);
                    break;
                case EXTEND:
                    astfile.getExtends().add((AST.Extend) ast);
                    break;
                case MESSAGE:
                    astfile.getMessages().add((AST.Message) ast);
                    break;
                case SERVICE:
                    astfile.getServices().add((AST.Service) ast);
                    break;
                default: /*ignore*/
                    break;
                }
            }
	    // For files, collect only the top-level OPTIONS
            astfile.setOptions(new ArrayList<AST.Option>());
            for (AST ast : astfile.getChildSet()) {
                if(ast.getSort() == AST.Sort.OPTION) {
                    astfile.getOptions().add((AST.Option) ast);
	        }
	    }
            break;
        case MESSAGE:
            // Group the message elements
            AST.Message msg = (AST.Message) node;
            msg.setFields(new ArrayList<AST.Field>());
            msg.setEnums(new ArrayList<AST.Enum>());
            msg.setMessages(new ArrayList<AST.Message>());
            msg.setExtenders(new ArrayList<AST.Extend>());
            msg.setExtensions(new ArrayList<AST.Extensions>());
            msg.setOptions(new ArrayList<AST.Option>());
            msg.setGroups(new ArrayList<AST.Group>());
            for (AST ast : msg.getChildSet()) {
                switch (ast.getSort()) {
                case FIELD:
                    msg.getFields().add((AST.Field) ast);
                    break;
                case GROUP:
                    msg.getGroups().add((AST.Group) ast);
                    break;
                case ENUM:
                    msg.getEnums().add((AST.Enum) ast);
                    break;
                case MESSAGE:
                    msg.getMessages().add((AST.Message) ast);
                    break;
                case EXTEND:
                    msg.getExtenders().add((AST.Extend) ast);
                    break;
                case EXTENSIONS:
                    msg.getExtensions().add((AST.Extensions) ast);
                    break;
                case OPTION:
                    msg.getOptions().add((AST.Option) ast);
                    break;
                default:
                    assert (false) : "Illegal ast case";
                    break;
                }
            }
            break;
        case SERVICE:
            AST.Service svc = (AST.Service) node;
            svc.setOptions(new ArrayList<AST.Option>());
            svc.setRPCs(new ArrayList<AST.RPC>());
            for (AST ast : svc.getChildSet()) {
                switch (ast.getSort()) {
                case OPTION:
                    svc.getOptions().add((AST.Option) ast);
                    break;
                case RPC:
                    svc.getRPCs().add((AST.RPC) ast);
                    break;
                default:
                    assert (false) : "Illegal ast case";
                    break;
                }
            }
            break;
        case ENUM:
            AST.Enum astenum = (AST.Enum) node;
            astenum.setEnumValues(new ArrayList<AST.EnumValue>());
            for (AST ast : astenum.getChildSet()) {
                switch (ast.getSort()) {
                case ENUMVALUE:
                    astenum.getEnumValues().add((AST.EnumValue) ast);
                    break;
                case OPTION:
                    astenum.getOptions().add((AST.Option) ast);
                    break;
                default:
                    assert (false) : "Illegal ast case";
                    break;
                }

            }
            break;
        case EXTEND:
            AST.Extend astextend = (AST.Extend) node;
            astextend.setFields(new ArrayList<AST.Field>());
            astextend.setGroups(new ArrayList<AST.Group>());
            for (AST ast : astextend.getChildSet()) {
                switch (ast.getSort()) {
                case FIELD:
                    astextend.getFields().add((AST.Field) ast);
                    break;
                case GROUP:
                    astextend.getGroups().add((AST.Group) ast);
                    break;
                default:
                    assert (false) : "Illegal ast case";
                    break;
                }
            }
            break;
        case FIELD:
            AST.Field astfield = (AST.Field) node;
            astfield.setOptions(new ArrayList<AST.Option>());
            for (AST ast : astfield.getChildSet()) {
                switch (ast.getSort()) {
                case OPTION:
                    astfield.getOptions().add((AST.Option) ast);
                    break;
                case PACKAGE: /*astfield.setPackage((AST.Package)ast);*/
                    assert (false);
                    break;
                default:
                    assert (false) : "Illegal ast case";
                    break;
                }
            }
            break;
        case ENUMVALUE:
            AST.EnumValue astenumvalue = (AST.EnumValue) node;
            astenumvalue.setOptions(new ArrayList<AST.Option>());
            for (AST ast : astenumvalue.getChildSet()) {
                switch (ast.getSort()) {
                case OPTION:
                    astenumvalue.getOptions().add((AST.Option) ast);
                    break;
                default:
                    assert (false) : "Illegal ast case";
                    break;
                }
            }
            break;
        // Cases where no extra action is required in pass
        case GROUP:
        case ROOT:
        case PACKAGE:
        case EXTENSIONS:
        case OPTION:
        case RPC:
        case PRIMITIVETYPE:
            break;
        default: {// should not happen
            assert (false) : "Illegal astcase";
        }
        break;
        }
    }
    return true;
}

/**
 * Pass does the following:
 * - compute scope names
 *
 * @param root of the tree
 * @return true if the processing succeeded.
 */

boolean
setscopenames(AST.Root root)
{
    // Assign the scope names
    for (AST ast : root.getNodeSet()) {
        ast.setScopeName(AuxFcns.computescopename(ast));
    }
    return true;
}

/**
 * Pass does the following:
 * - assign qualified names
 * - check for duplicate qualified names
 *
 * @param root of the tree
 * @return true if the processing succeeded.
 */

boolean
qualifynames(AST.Root root)
{
    String qname = "";
    for (AST ast : root.getChildSet()) {
        if (!qualifynamesr(ast, qname)) return false;
    }
    return true;
}

/**
 * Recursive companion to qualifynames
 *
 * @param ast the current node under consideration
 * @return true if the processing succeeded.
 */

boolean
qualifynamesr(AST ast, String qualprefix)
{
    AST.Root root = ast.getRoot();
    String qname = null;
    switch (ast.getSort()) {

    // Following cases all do simple suffixing
    case ENUM:
    case ENUMVALUE:
    case MESSAGE:
    case RPC:
    case SERVICE:
        if (qualprefix == null) qualprefix = "";
        qname = qualprefix + "." + ast.getScopeName();
        break;

    case FIELD:
        // fields that are part of an extend have no qualified name
        if (ast.getParent().getSort() == AST.Sort.EXTEND) {
            qname = null;
        } else {
            if (qualprefix == null) qualprefix = "";
            qname = qualprefix + "." + ast.getScopeName();
        }
        break;

    case FILE:
        AST.File file = (AST.File) ast;
        if (file.getFilePackage() != null) {
            qname = file.getFilePackage().getScopeName();
        } else {
            qname = "";
        }
        break;

    // Ignore the prefix
    case PACKAGE:
    case PRIMITIVETYPE:
        qname = root.getScopeName() + "." + ast.getScopeName();
        break;

    // These do not have qualifiednames
    case EXTEND:
    case OPTION:
    case GROUP:
    case EXTENSIONS:
    case ROOT:
    case PAIR:
    case COMPOUNDCONSTANT:
    default:
        break;
    }
    ast.setQualifiedName(qname);
    for (AST subnode : ast.getChildSet()) {
        if (!qualifynamesr(subnode, qname)) return false;
    }
    return true;
}

/**
 * Pass does the following:
 * - check for duplicate qualified names
 *
 * @param root of the tree
 * @return true if the processing succeeded.
 */

boolean
checkduplicatenames(AST.Root root)
{
    List<AST> allnodes = root.getNodeSet();
    for (AST ast1 : allnodes) {
        if (ast1.getSort() == AST.Sort.FILE || ast1.getSort() == AST.Sort.PACKAGE) continue;
        for (AST ast2 : allnodes) {
            if (ast2 == ast1 || ast2.getQualifiedName() == null) continue;
            if (ast2.getSort() == AST.Sort.FILE || ast2.getSort() == AST.Sort.PACKAGE) continue;
            // special case testing
            if (ast2.getQualifiedName().equals(ast1.getQualifiedName())) {
                // report and keep going
                duperror(ast1, ast2, "Duplicate qualified name: "
                        + ast1.getQualifiedName());
            }
        }
    }
    return true;
}

/**
 * Pass does the following:
 * - Deref all references to other objects
 *
 * @param root The AST tree root
 * @return true if the processing succeeded.
 */

boolean
dereference(AST.Root root)
{
    boolean found;
    String qualname;
    List<AST.Type> matches;
    List<AST> allnodes = root.getNodeSet();
    for (AST node : allnodes) {
        switch (node.getSort()) {
        case EXTEND:
            // deref the msg name
            AST.Extend extender = (AST.Extend) node;
            String msgname = extender.getName();
            matches = AuxFcns.findtypebyname(msgname, node);
            if (matches.size() > 1) {
                return semerror(node, "Extend msg name is ambiguous: "
                        + msgname);
            }
            found = false;
            AST match = null;
            for (AST ast : matches) {
                if (ast instanceof AST.Message) {
                    extender.setMessage((AST.Message) ast);
                    found = true;
                    break;
                }
            }
            if (!found) {
                return semerror(node, "Extend refers to undefined message: "
                        + msgname);
            }
            break;

        case FIELD:
            // deref the field type name
            AST.Field field = (AST.Field) node;
            String typename = (String) field.getAnnotation();
            field.setAnnotation(null);
            // Compute absolute name relative to the parent message
            List<AST.Type> typematches = AuxFcns.findtypebyname(typename, node);
            if (typematches.size() == 0) {
                return semerror(node, "Field refers to undefined type: " + typename);
            } else if (typematches.size() > 1) {
                return duperror(typematches.get(0), typematches.get(1),
                                "Duplicate qualified type names: "
                                        + typematches.get(0).getName());
            } else { // typematches.size() == 1
                field.setType(typematches.get(0));
            }
            break;

        case RPC:
            // deref the argtype name and the returntype name
            AST.RPC rpc = (AST.RPC) node;
            String[] names = (String[]) rpc.getAnnotation();
            rpc.setAnnotation(null);
            typematches = AuxFcns.findtypebyname(names[0], node);
            if (typematches.size() == 0) {
                return semerror(node, "RPC returntype refers to undefined type: " + names[0]);
            } else if (typematches.size() > 1) {
                return duperror(typematches.get(0), typematches.get(1),
                                "Duplicate qualified type names: "
                                        + typematches.get(0).getName());
            } else {// typematches.size() == 1
                rpc.setArgType(typematches.get(0));
            }
            typematches = AuxFcns.findtypebyname(names[1], node);
            if (typematches.size() == 0) {
                return semerror(node, "RPC returntype refers to undefined type: " + names[1]);
            } else if (typematches.size() > 1) {
                return duperror(typematches.get(0), typematches.get(1),
                                "Duplicate qualified type names: "
                                        + typematches.get(0).getName());
            } else {// typematches.size() == 1
                rpc.setReturnType(typematches.get(0));
            }
            break;

        default:
            break; // ignore
        }
    }
    return true;
}

static public boolean
applyExtensions(AST.Root root)
{
    // Locate extension nodes and insert into the corresponding
    // base message; also mark fields as extensions
    for (AST.File f : root.getFileSet()) {
        for (AST.Extend extend : f.getExtends()) {
            AST.Message base = extend.getMessage();
            for (AST.Field efield : extend.getFields()) {
                efield.setExtend(extend);
                // fix links : package and parent
                efield.setSrcFile(base.getSrcFile());
                efield.setParent(base.getParent());
                base.getFields().add(efield);
                // Recompute qualified name
                efield.setQualifiedName(AuxFcns.computequalifiedname(efield));
            }
            //base.setFields(AuxFcns.sortFieldIds(base.getFields()));
        }
    }
    return true;
}

/**
 * Pass does the following:
 * - Check that all msg ids appear legal and are not duplicates
 * - Check for duplicate enum field values
 * - Check that extend fields do not conflict with the base message
 *
 * @param root AST.root
 * @return true if the processing succeeded.
 */

boolean
checkmisc1(AST.Root root)
{
    for (AST node : root.getNodeSet()) {
        switch (node.getSort()) {
        case ENUM:
            // check for duplicates
            for (AST.EnumValue field1 : ((AST.Enum) node).getEnumValues()) {
                for (AST.EnumValue field2 : ((AST.Enum) node).getEnumValues()) {
                    if (field1 == field2) continue;
                    if (field1.getValue() == field2.getValue()) {
                        duperror(field1, field2,
                                 String.format("Duplicate enum field numbers: %s=%s and %s=%s",
                                               field1.getName(), field1.getValue(),
                                               field2.getName(), field2.getValue()));
                        break;
                    }
                }
            }
            break;
        case MESSAGE:
            // check for duplicates
            for (AST.Field field1 : ((AST.Message) node).getFields()) {
                for (AST.Field field2 : ((AST.Message) node).getFields()) {
                    if (field1 == field2) continue;
                    if (field1.getId() == field2.getId()) {
                        duperror(field1, field2,
                                 "Duplicate message field numbers: " + field1.getId());
                        break;
                    }
                }
            }
            break;

        case FIELD:
            // Check legality of field number
            AST.Field field = (AST.Field) node;
            if (field.getId() < 0 || field.getId() >= AST.MAXFIELDID) {
                semerror(node, "Illegal message field id" + field.getId());
            }
            break;

        case EXTEND:
            AST.Extend extend = (AST.Extend) node;
            AST.Message base = extend.getMessage();
            for (AST.Field efield : extend.getFields()) {
                // Check to see if a field of the same name or id
                // already exists
                for (AST.Field mfield : base.getFields()) {
                    if (mfield.getName().equals(efield.getName())
                            || mfield.getId() == efield.getId()) {
                        semerror(efield, String.format("Extension field %s.%s duplicates Message field %s.%s\n",
                                                       extend.getName(), efield.getName(),
                                                       base.getName(), mfield.getName()));

                    }
                }
            }
            break;

        default:
            break;
        }
    }

    return true;
}


/**
 * Pass does the following:
 * - Define predefined options
 * - Deref the optiondef type
 *
 * @param root The AST tree root
 * @return true if the processing succeeded.
 */

boolean
defineoptions(AST.Root root)
{
    for (AST.OptionDef od : root.getOptionDefs()) {
        // Deref the type of the optiondef
        String typename = od.typeref;
        List<AST.Type> typematches = AuxFcns.findtypebyname(typename, root);
        if (typematches.size() == 0) {
            return semerror(root, "Option has undefined type: " + od.name + ":" + typename);
        }
        AST.Type t = typematches.get(0);
        od.typedef = t;
    }
    return true;
}

/**
 * Pass does the following:
 * - Copy the raw options for each node into the optionmap for that node
 * - Capture value of selected true/false options (e.g. packed)
 * - Pull in any definable options from the System.getProperties() list
 *
 * @param root The AST tree root
 * @return true if the processing succeeded.
 */

boolean
mapoptions(AST.Root root)
{
    List<AST> allnodes = root.getNodeSet();
    for (AST node : allnodes) {
        // See if this node as any attached options
	for(AST.Option option: node.getOptions()) {
	    if(!validateoption(root,option)) return false;
	    if(node.getOptionMap().get(option.getOptionDef()) != null) {
		return semerror(node,String.format("Duplicate options: %s [%s]",
						    node.getName(),option.getName()));
	    } else
	        node.getOptionMap().put(option.getOptionDef(),option.getValue());
        }
    }

    // now look for defined options in System.getProperties()
    for(String key: System.getProperties().stringPropertyNames()) {
        for(OptionDef od : root.getOptionDefs()) {
            if(od.name.equalsIgnoreCase(key)) {
		// create and add an option
		String value = System.getProperty(key);
		if(value == null || value.length() == 0)
		    value = "true"; // assume this boolean default
		AST.Option option = factory.newOption(od.name,value);
                option.setOptionDef(od);
                root.getTopFile().getOptions().add(option);
                root.getTopFile().getChildSet().add(option);
                option.setParent(root.getTopFile());
	        if(!validateoption(root,option)) return false;
	        root.getTopFile().getOptionMap().put(option.getOptionDef(),option.getValue());
		break;
	    }
        }
    }	
    return true;
}

boolean
validateoption(AST.Root root, AST.Option option)
{
    option.setOptionDef(null);
    for (OptionDef od : root.getOptionDefs()) {
        if (od.name.equalsIgnoreCase(option.getName())) {
            option.setOptionDef(od);
            break;
        }
    }
    if (option.getOptionDef() == null) {
        return semerror(option, "Unknown option: " + option.getName());
    }
    // Canonicalize the name
    option.setName(option.getOptionDef().name);
    // Validate the value
    AST.Type t = option.getOptionDef().typedef;

    // Exception: when the option is DEFAULT, use the fields' type
    if (option.getName().equalsIgnoreCase("DEFAULT")) {
        AST.Field field = (AST.Field) option.getParent();
        t = field.getType();
    }

    boolean typeok = true;
    String value = option.getValue();
    switch (t.getSort()) {
    case ENUM:
        // Treat like a string
        break;
     case PRIMITIVETYPE:
        AST.PrimitiveType pt = (AST.PrimitiveType) t;
        switch (pt.getPrimitiveSort()) {
        case SINT32:
        case SFIXED32:
        case INT32:
            try {
                int i = Integer.parseInt(value);
            } catch (NumberFormatException nfe) {
                typeok = false;
            }
            break;
        case FIXED32:
        case UINT32:
            try {
                long i = Long.parseLong(value);
                if (i < 0 || i > Integer.MAX_VALUE) {
                        typeok = false;
                    }
                } catch (NumberFormatException nfe) {
                    typeok = false;
                }
                break;
            case SINT64:
            case SFIXED64:
            case INT64:
                try {
                    long i = Long.parseLong(value);
                } catch (NumberFormatException nfe) {
                    typeok = false;
                }
                break;
            case FIXED64:
            case UINT64:
                try {
                    double d = Double.parseDouble(value);
                    if (d < 0 || d > Long.MAX_VALUE) {
                        typeok = false;
                    }
                } catch (NumberFormatException nfe) {
                    typeok = false;
                }
                break;
            case FLOAT:
            case DOUBLE:
                try {
                    Double d = Double.parseDouble(value);
                } catch (NumberFormatException nfe) {
                    typeok = false;
                }
                break;
             case BOOL:
                try {
                    long i = Long.parseLong(value);
                    value = (i == 0 ? "false" : "true");
                } catch (NumberFormatException nfe) { /* ignore */}
                 if ("true".equalsIgnoreCase(value)
                    || "yes".equalsIgnoreCase(value)) {
                    value = "true";
                } else if ("false".equalsIgnoreCase(value)
                           || "no".equalsIgnoreCase(value)) {
                    value = "false";
                } else {
                    typeok = false;
                }
                break;
             case BYTES:
                if (value.length() % 2 == 1) {
                    typeok = false;
                } else {
                    for (int i = 0; i < value.length(); i++) {
                        if ("0123456789abcdefABCDEF".indexOf(value.charAt(i)) < 0) {
                            typeok = false;
                            break;
                        }
                    }
                }
                break;
             case STRING: // always ok
                break;
            }
            if (!typeok) {
                return semerror(option,
                                String.format("Option type mismatch: %s=%s",
                                              option.getName(),
                                              value));
            }
            option.setValue(value); // canonical value
            break;
         case MESSAGE:
            return semerror(option, "Message typed options unsupported: " + option.getName());
         default:
            return semerror(option, "Illegal option type: " + option.getName());
         }
    return true;
}


} // class Semantics
