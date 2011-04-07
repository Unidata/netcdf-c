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

// An instance of AST serves as the tree root

abstract public class
ASTDefault implements AST
{

//////////////////////////////////////////////////
// Assign indices to all nodes
static int uid = 0;
static int primitiveuid = 0; // separate numbering for primitives
int index;

//////////////////////////////////////////////////
// Instance variables

Position position = null;
Sort sort = null;
AST.Root root = null; // top-level root
AST.File srcfile = null; // immediately containing src file node
AST.Package packageroot = null; // immediately containing package
AST parent = null;
String name = null;
String scopename = null;
String qualifiedname = null;
Object annotation = null;
List<AST> childset = new ArrayList<AST>(); // immediate children
List<AST> nodeset = new ArrayList<AST>(); // All nodes under this node;
// null except for root and packages
List<AST.Option> options = new ArrayList<AST.Option>();
Map<OptionDef, Object> optionmap = new HashMap<OptionDef, Object>();
List<AST.OptionDef> optiondefs = new ArrayList<AST.OptionDef>();

int refcount = 0;

// Track selected common true/false options
boolean ispacked = false;

public ASTDefault(Sort sort)
{
    this.sort = sort;
    setuid();
}

public void setuid()
{
    index = ++uid;
}

public int getId()
{
    return index;
}

public int setRefCount(int n)
{
    int old = refcount;
    refcount = n;
    return old;
}

public void addRefCount(int n)
{
    refcount += n;
}

public List<AST> getChildSet()
{
    return this.childset;
}

public void setChildSet(List<AST> children)
{
    this.childset = children;
}

public List<AST> getNodeSet()
{
    return this.nodeset;
}

public void setNodeSet(List<AST> nodeset)
{
    this.nodeset = nodeset;
}

public Position getPosition()
{
    return position;
}

public void setPosition(Position position)
{
    this.position = position;
}

public Sort getSort()
{
    return this.sort;
}

public void setSort(Sort astclass)
{
    this.sort = astclass;
}

public AST.Root getRoot()
{
    return root;
}

public void setRoot(AST.Root root)
{
    this.root = root;
}

public AST.Package getPackage()
{
    return packageroot;
}

public void setPackage(AST.Package packageroot)
{
    this.packageroot = packageroot;
}

public AST.File getSrcFile()
{
    return srcfile;
}

public void setSrcFile(AST.File srcfile)
{
    this.srcfile = srcfile;
}

public AST getParent()
{
    return parent;
}

public void setParent(AST parent)
{
    this.parent = parent;
}

public String getName()
{
    return name;
}

public void setName(String name)
{
    this.name = name;
}

public String getQualifiedName()
{
    return qualifiedname;
}

public void setQualifiedName(String qualifiedname)
{
    this.qualifiedname = qualifiedname;
}

public String getScopeName()
{
    return scopename;
}

public void setScopeName(String name)
{
    scopename = name;
}

public Object getAnnotation()
{
    return annotation;
}

public void setAnnotation(Object annotation)
{
    this.annotation = annotation;
}

public List<AST.Option> getOptions()
{
    return this.options;
}

public void setOptions(List<AST.Option> options)
{
    this.options = options;
}

public void addOption(AST.Option option)
{
    for(AST.Option opt: options) {
	if(opt.getName().equals(option.getName()))
	    Debug.semerror(option,"Duplicate option names:"+opt.getName());
    }
    this.options.add(option);
}

// Map oriented option access

public Object
optionLookup(String key)
{
    OptionDef def = AuxFcns.getOptionDef(this,key);
    if(def == null)
        Debug.semerror(root,"Unknown option name:"+key);
    return optionLookup(def);
}

public Object
optionLookup(OptionDef key)
{
    return this.optionmap.get(key);
}

public Map<OptionDef,Object>
getOptionMap()
{
    return this.optionmap;
}

public void
setOptionMap(OptionDef key, Object value)
{
    if(key == null)
        Debug.semerror(null,"unknown option:"+key.name);
    else this.optionmap.put(key, value);
}

public void unsetOptionMap(OptionDef key)
{
    this.optionmap.remove(key);
}

public void
setOptionDefs(List<AST.OptionDef> defs)
{
    this.optiondefs = defs;
}

public List<AST.OptionDef> getOptionDefs()
{
    return this.optiondefs;
}

public AST.OptionDef getOptionDef(String key)
{
    for(OptionDef def: this.optiondefs) {
        if(def.name.equals(key)) return def;
    }
    return null;
}


public boolean isPacked()
{
    return ispacked;
}

public void setIsPacked(boolean tf)
{
    ispacked = tf;
}

public String toString()
{
    if (getQualifiedName() != null) return getQualifiedName();
    if (getName() != null) return getName();
    return super.toString();
}

// Convenience grouping class
static public abstract class
Type extends ASTDefault implements AST.Type
{
    public Type(Sort sort)
    {
        super(sort);
    }
}

// An instance of this is the root of the AST tree
static public class
Root extends ASTDefault implements AST.Root
{
    List<AST.Package> packageset = new ArrayList<AST.Package>();
    List<AST.File> fileset = new ArrayList<AST.File>();
    List<AST.PrimitiveType> primitivetypes = null;

    AST.File topfile = null;

    public Root(String name)
    {
        super(Sort.ROOT);
        setName("");
        setPackage(null);
        setSrcFile(null);
    }

    public List<AST.File> getFileSet()
    {
        return this.fileset;
    }

    public void setFileSet(List<AST.File> fileset)
    {
        this.fileset = fileset;
    }

    public List<AST.Package> getPackageSet()
    {
        return this.packageset;
    }

    public void setPackageSet(List<AST.Package> packageset)
    {
        this.packageset = packageset;
    }

    public List<AST.PrimitiveType> getPrimitiveTypes()
    {
        return this.primitivetypes;
    }

    public void setPrimitiveTypes(List<AST.PrimitiveType> primitivetypes)
    {
        this.primitivetypes = primitivetypes;
    }

    public AST.File getTopFile()
    {
        return this.topfile;
    }

    public void setTopFile(AST.File p)
    {
        this.topfile = p;
    }
}

static public class
File extends ASTDefault implements AST.File
{
    AST.Package filepackage = null;
    AST.File parentfile = null;
    List<AST.File> imports = null;
    List<AST> decls = new ArrayList<AST>();
    List<AST.Message> messages = new ArrayList<AST.Message>();
    List<AST.Extend> extenders = new ArrayList<AST.Extend>();
    List<AST.Enum> enums = new ArrayList<AST.Enum>();
    List<AST.Service> services = new ArrayList<AST.Service>();

    public File(String name)
    {
        super(Sort.FILE);
        setName(name);
    }

    public AST.File getParentFile()
    {
        return parentfile;
    }

    public void setParentFile(AST.File f)
    {
        parentfile = f;
    }

    public AST.Package getFilePackage()
    {
        return this.filepackage;
    }

    public void setFilePackage(AST.Package astpackage)
    {
        this.filepackage = astpackage;
    }

    public List<AST.File> getImports()
    {
        return this.imports;
    }

    public void setImports(List<AST.File> imports)
    {
        this.imports = imports;
    }

    public List<AST.Message> getMessages()
    {
        return this.messages;
    }

    public void setMessages(List<AST.Message> sessages)
    {
        this.messages = messages;
    }

    public List<AST.Extend> getExtends()
    {
        return this.extenders;
    }

    public void setExtends(List<AST.Extend> extenders)
    {
        this.extenders = extenders;
    }

    public List<AST.Enum> getEnums()
    {
        return this.enums;
    }

    public void setEnums(List<AST.Enum> enums)
    {
        this.enums = enums;
    }

    public List<AST.Service> getServices()
    {
        return this.services;
    }

    public void setServices(List<AST.Service> services)
    {
        this.services = services;
    }

}

static public class
Package extends ASTDefault implements AST.Package
{
    AST.File packagefile = null; // inverse of AST.File.filepackage

    public Package(String name)
    {
        super(Sort.PACKAGE);
        setName(name);
    }

    public AST.File getPackageFile()
    {
        return this.packagefile;
    }

    public void setPackageFile(AST.File astfile)
    {
        this.packagefile = astfile;
    }


}

static public class
Enum extends ASTDefault implements AST.Enum
{
    List<AST.EnumValue> enumvalues = new ArrayList<AST.EnumValue>();

    public Enum(String name)
    {
        super(Sort.ENUM);
        setName(name);
    }

    public List<AST.EnumValue> getEnumValues()
    {
        return this.enumvalues;
    }

    public void setEnumValues(List<AST.EnumValue> enumvalues)
    {
        this.enumvalues = enumvalues;
    }
}

static public class
EnumValue extends ASTDefault implements AST.EnumValue
{
    int value;

    public EnumValue(String name, int value)
    {
        super(Sort.ENUMVALUE);
        setName(name);
        setValue(value);
    }

    public int getValue()
    {
        return this.value;
    }

    public void setValue(int value)
    {
        this.value = value;
    }
}

static public class
Extend extends ASTDefault implements AST.Extend
{
    AST.Message message = null;
    List<AST.Field> fields = new ArrayList<AST.Field>();
    List<AST.Group> groups = new ArrayList<AST.Group>();

    public Extend(String msgname)
    {
        super(Sort.EXTEND);
        setName(msgname);
    }

    public AST.Message getMessage()
    {
        return this.message;
    }

    public void setMessage(AST.Message message)
    {
        this.message = message;
    }

    public List<AST.Field> getFields()
    {
        return this.fields;
    }

    public void setFields(List<AST.Field> fields)
    {
        this.fields = fields;
    }

    public List<AST.Group> getGroups()
    {
        return this.groups;
    }

    public void setGroups(List<AST.Group> groups)
    {
        this.groups = groups;
    }
}

static public class
Extensions extends ASTDefault implements AST.Extensions
{
    List<AST.Range> ranges = new ArrayList<AST.Range>();

    public Extensions(String name)
    {
        super(Sort.EXTENSIONS);
        setName(name);
    }

    public List<AST.Range> getRanges()
    {
        return this.ranges;
    }

    public void setRanges(List<AST.Range> ranges)
    {
        this.ranges = ranges;
    }
}

static public class
Field extends ASTDefault implements AST.Field
{
    Cardinality cardinality = null;
    AST.Type fieldtype = null;
    int id;
    AST.Extend extend = null;

    public Field(String name, Cardinality cardinality, String fieldtype, int id)
    {
        super(Sort.FIELD);
        setName(name);
        setCardinality(cardinality);
        setAnnotation(fieldtype); // temporary storage
        setId(id);
    }

    public Cardinality getCardinality()
    {
        return this.cardinality;
    }

    public void setCardinality(Cardinality cardinality)
    {
        this.cardinality = cardinality;
    }

    public AST.Type getType()
    {
        return this.fieldtype;
    }

    public void setType(AST.Type fieldtype)
    {
        this.fieldtype = fieldtype;
    }

    public int getId()
    {
        return this.id;
    }

    public void setId(int id)
    {
        this.id = id;
    }

    public AST.Extend getExtend()
    {
        return extend;
    }

    public void setExtend(AST.Extend ex)
    {
        extend = ex;
    }
}

// A group node is a special case of field
static public class
Group extends Field implements AST.Group
{
    public Group(String name, Cardinality cardinality, int id)
    {
        super(name, cardinality, null, id);
        setSort(Sort.GROUP);
        setName(name);
    }
}

static public class
Message extends Type implements AST.Message
{
    // Filled in during Semantic processing
    List<AST.Field> fields = new ArrayList<AST.Field>();
    List<AST.Enum> enums = new ArrayList<AST.Enum>();
    List<AST.Message> messages = new ArrayList<AST.Message>();
    List<AST.Extend> extenders = new ArrayList<AST.Extend>();
    List<AST.Extensions> extensions = new ArrayList<AST.Extensions>();
    List<AST.Group> groups = new ArrayList<AST.Group>();

    public Message(String name)
    {
        super(Sort.MESSAGE);
        setName(name);
    }

    public List<AST.Field> getFields()
    {
        return this.fields;
    }

    public void setFields(List<AST.Field> fields)
    {
        this.fields = fields;
    }

    public List<AST.Enum> getEnums()
    {
        return this.enums;
    }

    public void setEnums(List<AST.Enum> enums)
    {
        this.enums = enums;
    }

    public List<AST.Message> getMessages()
    {
        return this.messages;
    }

    public void setMessages(List<AST.Message> messages)
    {
        this.messages = messages;
    }

    public List<AST.Extend> getExtenders()
    {
        return this.extenders;
    }

    public void setExtenders(List<AST.Extend> extenders)
    {
        this.extenders = extenders;
    }

    public List<AST.Extensions> getExtensions()
    {
        return this.extensions;
    }

    public void setExtensions(List<AST.Extensions> extension)
    {
        this.extensions = extensions;
    }

    public List<AST.Group> getGroups()
    {
        return this.groups;
    }

    public void setGroups(List<AST.Group> groups)
    {
        this.groups = groups;
    }
}

static public class
Option extends ASTDefault implements AST.Option
{
    AST.OptionDef optiondef = null;
    String value;
    boolean isstringvalued = false;

    public Option(String name, String value)
    {
        super(Sort.OPTION);
        setName(name);
        setValue(value);
    }

    public String getValue()
    {
        return this.value;
    }

    public void setValue(String value)
    {
        this.value = value;
    }

    public AST.OptionDef getOptionDef()
    {
        return this.optiondef;
    }

    public void setOptionDef(AST.OptionDef od)
    {
        this.optiondef = od;
    }

    public boolean isStringValued()
    {
        return isstringvalued;
    }

    public void setStringValued(boolean tf)
    {
        isstringvalued = tf;
    }
}

static public class
CompoundConstant extends ASTDefault implements AST.CompoundConstant
{
    List<AST.Pair> members = null;

    public CompoundConstant(String name)
    {
        super(Sort.COMPOUNDCONSTANT);
        setName(name);
    }

    public List<AST.Pair> getMembers()
    {
        return members;
    }

    public void setMembers(List<AST.Pair> members)
    {
        this.members = members;
    }
}

static public class
Pair extends ASTDefault implements AST.Pair
{
    Object value = null;

    public Pair(String name, Object value)
    {
        super(Sort.PAIR);
        setName(name);
        setValue(value);
    }

    public Object getValue()
    {
        return value;
    }

    public void setValue(Object value)
    {
        this.value = value;
    }
}

static public class
RPC extends ASTDefault implements AST.RPC
{
    AST.Type argtype = null;
    AST.Type returntype = null;

    public RPC(String name, String argtype, String returntype)
    {
        super(Sort.RPC);
        setName(name);
        // Use annotation to temporarily store the type names
        String[] names = new String[2];
        names[0] = argtype;
        names[1] = returntype;
        setAnnotation(names);
    }

    public AST.Type getArgType()
    {
        return this.argtype;
    }

    public void setArgType(AST.Type argtype)
    {
        this.argtype = argtype;
    }

    public AST.Type getReturnType()
    {
        return this.returntype;
    }

    public void setReturnType(AST.Type returntype)
    {
        this.returntype = returntype;
    }
}

static public class
Service extends ASTDefault implements AST.Service
{
    // Filled in during semantic processing
    List<AST.RPC> rpcs = new ArrayList<AST.RPC>();

    public Service(String name)
    {
        super(Sort.SERVICE);
        setName(name);
    }

    public List<AST.RPC> getRPCs()
    {
        return this.rpcs;
    }

    public void setRPCs(List<AST.RPC> rpcs)
    {
        this.rpcs = rpcs;
    }
}

static public class
PrimitiveType extends Type implements AST.PrimitiveType
{
    PrimitiveSort primitivesort = null;

    public PrimitiveType(PrimitiveSort primitivesort)
    {
        super(Sort.PRIMITIVETYPE);
        this.primitivesort = primitivesort;
        setName(primitivesort.getName());
    }

    public void setuid()
    {
        index = --primitiveuid;
    }

    public PrimitiveSort getPrimitiveSort()
    {
        return this.primitivesort;
    }

    public void setPrimitiveSort(PrimitiveSort primitivesort)
    {
        this.primitivesort = primitivesort;
    }
}

}// class ASTDefault

