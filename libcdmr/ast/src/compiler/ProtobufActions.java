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
import java.util.*;

public abstract class ProtobufActions
{

//////////////////////////////////////////////////
// Class constants

static String GROUPKEYWORD = "group";
static String GOOGLEKEYWORD = "google.protobuf";

// Support classes
static class Constant
{
    int token;
    String lval;
    public Constant(int token, Object lval)
	{this.token=token; this.lval=lval.toString();}
}

static class ImportInfo
{
    String filename;
    public ImportInfo(String filename)
	{this.filename=filename;}
}

//////////////////////////////////////////////////
// Instance State

    ProtobufLexer lexstate = null;
    ProtobufActions state = null; /* Slight kludge */
    ASTFactory astfactory = null;
    AST.Root ast = null; // root node of the AST
    String filename = null;
    List<AST.OptionDef> optiondefs = new ArrayList<AST.OptionDef>();

//////////////////////////////////////////////////
// Constructors

ProtobufActions()
{
    this(null);
}

ProtobufActions(ASTFactory factory)
{
    state = this;
    if(factory == null) factory = new ASTFactoryDefault();
    this.astfactory = factory;
}

public void
reset(String filename, Reader stream)
{
    this.filename = filename;
    lexstate.reset(state);
    lexstate.setStream(filename,stream);
}


//////////////////////////////////////////////////
// Get/set

public AST.Root getAST() {return ast;}

public List<String> getIncludePaths() {return (lexstate==null?null:lexstate.includepaths);}

public void setIncludePaths(List<String> paths)
    {if(lexstate != null) lexstate.includepaths = paths;}

//////////////////////////////////////////////////
// Access into the DapParser for otherwise inaccessible fiels

abstract public boolean parse(String filename,Reader stream) throws IOException;
abstract public Object parseError(String msg);
abstract public Object parseWarning(String msg);
abstract int getDebugLevel();
abstract void setDebugLevel(int level);

//////////////////////////////////////////////////
// Parser actions

// Construct tree root
void
protobufroot(Object file0)
{
    AST.File file = (AST.File)file0;
    file.setName(filename);
    AST.Root root = astfactory.newRoot("");
    this.ast = root;
    root.setTopFile(file);
    root.getChildSet().add(file);
    root.setOptionDefs(optiondefs);
}

Object
protobuffile(Object decllist0)
{
    AST.File f = astfactory.newFile(null);
    f.getChildSet().addAll((List<AST>)decllist0);
    f.setPosition(position());
    return f;
}

Object
packagedecl(Object name0)
{
    Constant name = (Constant)name0;
    AST.Package node = astfactory.newPackage(name.lval);
    node.setPosition(position());
    return node;
}

Object
importlist(Object list0, Object import0)
{
    List<AST> list = (List<AST>)list0;
    if(list == null) list = new ArrayList<AST>();
    if(import0 != null) list.add((AST)import0);
    return list;
}

Object
importstmt(Object importinfo0, Object file0)
{
    ImportInfo info = (ImportInfo)importinfo0;
    AST.File file = (AST.File) file0;
    file.setName(info.filename);
    file.setPosition(position());
    return file;
}

Object
importprefix(Object filename0)
{
    ImportInfo info = new ImportInfo((String)filename0);
    return info;
}

Object
importoptionlist(Object list0, Object decl0)
{
    List<AST.Option> list = (List<AST.Option>)list0;
    if(list == null) list = new ArrayList<AST.Option>();
    if(decl0 != null) list.add((AST.Option)decl0);
    return list;
}

boolean
filepush(Object info0)
{
    ImportInfo info = (ImportInfo)info0;
    // push file stack
    boolean ok = true;
    String errmsg = null;
    try {
        ok = lexstate.pushFileStack(info.filename);
    } catch (Exception e) {  ok = false; errmsg = e.toString();}
    if(!ok) {
        errmsg = "import file failure: "+info.filename + (errmsg==null?"":errmsg);
        parseError(errmsg);
    }
    return ok;
}

boolean
filepop()
{
    return true; //lexstate.popFileStack();
}

Object
decllist(Object list0, Object decl0)
{
    List<AST> list = (List<AST>)list0;
    if(list == null) list = new ArrayList<AST>();
    if(decl0 != null) list.add((AST)decl0);
    return list;
}

Object
option(Object name0, Object constant0)
{
    AST.Option node;
    Constant name = (Constant)name0;
    Constant con = (Constant)constant0;
    node = astfactory.newOption(name.lval,con.lval);
    if(con.token == ProtobufParser.STRINGCONST) node.setStringValued(true);
    node.setPosition(position());
    return node;
}


Object
message(Object name0, Object body0)
{
    Constant name = (Constant)name0;
    AST.Message node = astfactory.newMessage(name.lval);
    node.getChildSet().addAll((List<AST>)body0);
    node.setPosition(position());
    return node;
}

/* The protobuf user defined option mechanism is easily one of the most foolish 
   ideas I have ever seen; some one at Google was trying too clever by half;
   => I do not implement completely
*/
Object
extend(Object msg0, Object fieldlist0)
{
    Constant msg = (Constant)msg0;
    List<AST.Field> fields = (List<AST.Field>)fieldlist0;
    // Check for user defined options
    if(msg.lval.startsWith(GOOGLEKEYWORD)) {
	// Extract the option and store
	for(AST.Field field: fields) {
	     AST.OptionDef od = new AST.OptionDef(field.getName(),
					          (String)field.getAnnotation());
 	     od.setUserDefined(true);
	     optiondefs.add(od); // check for validity later
	}
	return null;
    } else {
        AST.Extend node = astfactory.newExtend(msg.lval);
        node.getChildSet().addAll((List<AST>)fieldlist0);
        node.setPosition(position());
        return node;
    }
}


Object
fieldlist(Object list0, Object decl0)
{
    List<AST.Field> list = (List<AST.Field>)list0;
    if(list == null) list = new ArrayList<AST.Field>();
    if(decl0 != null) list.add((AST.Field)decl0);
    return list;
}

Object
enumtype(Object name0, Object enumlist0)
{
    Constant name = (Constant)name0;
    AST.Enum node = astfactory.newEnum(name.lval);
    node.getChildSet().addAll((List<AST>)enumlist0);
    node.setPosition(position());
    return node;
}


Object
enumlist(Object list0, Object decl0)
{
    List<AST> list = (List<AST>)list0;
    if(list == null) list = new ArrayList<AST>();
    if(decl0 != null) list.add((AST)decl0);
    return list;
}

Object
enumvalue(Object name0, Object intvalue0, Object options0)
{
    Constant name = (Constant)name0;
    Constant con = (Constant)intvalue0;
    int value = 0;
    try {
	value = Integer.parseInt(con.lval);
    } catch (NumberFormatException nfe) {
	return parseError("Illegal enum field value: "+intvalue0);
    }
    AST.EnumValue node = astfactory.newEnumValue(name.lval,value);
    if(options0 == null) options0 = new ArrayList<AST.Option>();
    node.getChildSet().addAll((List<AST>)options0);
    node.setPosition(position());
    return node;
}

Object
enumoptionlist(Object list0, Object decl0)
{
    List<AST.Option> list = (List<AST.Option>)list0;
    if(list == null) list = new ArrayList<AST.Option>();
    if(decl0 != null) list.add((AST.Option)decl0);
    return list;
}

Object
service(Object name0, Object caselist0)
{
    Constant name = (Constant)name0;
    AST.Service node = astfactory.newService(name.lval);
    node.getChildSet().addAll((List<AST>)caselist0);
    node.setPosition(position());
    return node;
}

Object
servicecaselist(Object list0, Object decl0)
{
    List<AST> list = (List<AST>)list0;
    if(list == null) list = new ArrayList<AST>();
    if(decl0 != null) list.add((AST)decl0);
    return list;
}

Object
rpc(Object name0, Object type0, Object returntype0, Object optionlist)
{
    Constant name = (Constant)name0;
    AST.RPC node = astfactory.newRPC(name.lval,(String)type0,(String)returntype0);
    node.setChildSet((List<AST>)optionlist);
    node.setPosition(position());
    return node;
}

Object
optionstmtlist(Object list0, Object optionstmt0)
{
    List<AST.Option> list = (List<AST.Option>)list0;
    if(list == null) list = new ArrayList<AST.Option>();
    if(optionstmt0 != null) list.add((AST.Option)optionstmt0);
    return list;
}

Object
messageelementlist(Object list0, Object decl0)
{
    List<AST> list = (List<AST>)list0;
    if(list == null) list = new ArrayList<AST>();
    if(decl0 != null) list.add((AST)decl0);
    return list;
}

Object
field(Object cardinality0, Object type0, Object name0, Object id0, Object options0)
{
    Constant name = (Constant)name0;
    Constant con = (Constant)id0;
    AST.Cardinality cardinality = null;
    int id;

    if(options0 == null) options0 = new ArrayList<AST.Option>();

    if(type0 instanceof Constant) type0 = ((Constant)type0).lval;

    for(AST.Cardinality card: AST.Cardinality.values()) {
        if(card.getName().equalsIgnoreCase((String)cardinality0))
	    cardinality = card;
    }
    if(cardinality == null)
  	return parseError("Illegal field cardinality: "+cardinality0);

    try {
	id = Integer.parseInt(con.lval);
    } catch (NumberFormatException nfe) {
  	return parseError("Illegal message field id: "+con.lval);
    }
    AST.Field node = astfactory.newField(name.lval,
			cardinality,
			(String)type0,
			id);
    node.getChildSet().addAll((List<AST>)options0);
    node.setPosition(position());
    return node;
}

Object
fieldoptionlist(Object list0, Object decl0)
{
    List<AST.Option> list = (List<AST.Option>)list0;
    if(list == null) list = new ArrayList<AST.Option>();
    if(decl0 != null) list.add((AST.Option)decl0);
    return list;
}

Object
group(Object cardinality0, Object grouptag, Object name0, Object id0, Object msgbody)
{
    Constant name = (Constant)name0;
    Constant con = (Constant)id0;
    if(!GROUPKEYWORD.equals((String)grouptag))
  	return parseError("Illegal group declaration");

    AST.Cardinality cardinality = null;
    for(AST.Cardinality card: AST.Cardinality.values()) {
        if(card.getName().equalsIgnoreCase((String)cardinality0))
	    cardinality = card;
    }
    if(cardinality == null)
  	return parseError("Illegal field cardinality: "+cardinality0);

    int id = -1;
    try {
	id = Integer.parseInt(con.lval);
    } catch (NumberFormatException nfe) {
  	return parseError("Illegal group id: "+con.lval);
    }
    AST.Group node = astfactory.newGroup(name.lval,cardinality,id);
    node.getChildSet().addAll((List<AST>)msgbody);
    node.setPosition(position());
    return node;
}

Object
extensions(Object list0)
{
    // everynode has a name of some sort
    AST.Extensions node = astfactory.newExtensions("$extensions");
    node.setRanges((List<AST.Range>)list0);
    return node;
}

Object
extensionlist(Object list0, Object decl0)
{
    List<AST.Range> list = (List<AST.Range>)list0;
    if(list == null) list = new ArrayList<AST.Range>();
    if(decl0 != null) list.add((AST.Range)decl0);
    return list;
}

Object
extensionrange(Object start0, Object stop0)
{
    int start = 0;
    int stop = 0;
    try {
	start = Integer.parseInt((String)start0);
    } catch (NumberFormatException nfe) {
	return parseError("Illegal extendsion range start value");
    }
    if(stop0 == null)
	stop = AST.MAXEXTENSION;
    else try {
	stop = Integer.parseInt((String)stop0);
    } catch (NumberFormatException nfe) {
	return parseError("Illegal extension range stop value");
    }
    if(start < 0 || start < 0
       || start > AST.MAXFIELDID || stop > AST.MAXFIELDID) {
	return parseError(String.format("Illegal Extension range: %d..%d",
			  start,stop));
    }
    // Make sure that the range does not overlap the google
    // reserved range(s);
    if((start >= AST.MINGOOGLERANGE && start <= AST.MAXGOOGLERANGE)
       || (stop>= AST.MINGOOGLERANGE && stop <= AST.MAXGOOGLERANGE)) {
	return parseError("Extension range overlaps google reserved range");
    }
    AST.Range range = new AST.Range(start,stop);
    return range;
}

boolean
illegalname(Object s)
{
    if(((String)s).indexOf('.') >= 0) {
        parseError("Expected  NAME, found path: "+s.toString());
	return true;
    }
    return false;
}

AST.Position
position()
{
    return lexstate.pos.clone();
}


void
notimplemented(String s)
{
    parseError(s+" not implemented.");
    return;
}

Object
path(Object relpath0, boolean absolute)
{
    String pathstring = ((Constant)relpath0).lval;
    if(absolute) pathstring = "." + pathstring;
    return new Constant(ProtobufParser.STRINGCONST,pathstring);
}

Object
relpath(Object relpath0, Object segment0)
{
    if(relpath0 == null) return segment0;
    return new Constant(ProtobufParser.NAME,((Constant)relpath0).lval + "." + ((Constant)segment0).lval);
}

Object
compound(Object list0)
{
    AST.CompoundConstant node = astfactory.newCompoundConstant("$compound");
    node.setChildSet((List<AST>)list0);
    return node;
}


Object
pairlist(Object list0, Object decl0)
{
    List<AST.Pair> list = (List<AST.Pair>)list0;
    if(list == null) list = new ArrayList<AST.Pair>();
    if(decl0 != null) list.add((AST.Pair)decl0);
    return list;
}

Object
pair(Object name0, Object value0)
{
    Constant name = (Constant)name0;
    Constant value = (Constant)value0;
    AST.Pair node = astfactory.newPair(name.lval,value.lval);
    return node;
}


} // class ProtobufActions
