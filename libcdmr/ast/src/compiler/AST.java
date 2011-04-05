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

public interface
AST
{
    static final int MAXEXTENSION = 536870911 ; //0x1FFFFFFF

    static final int MAXFIELDID = MAXEXTENSION;

    static final String DEFAULTNAME = "$DEFAULT";

    static final int MINGOOGLERANGE = 19000;
    static final int MAXGOOGLERANGE = 19999;

    public enum Cardinality {
        REQUIRED("required"), OPTIONAL("optional"), REPEATED("repeated");
        private final String name;
        Cardinality(String name) {this.name = name;}
        public String getName()   { return name; }
    }

    // Define the kinds of AST objects to avoid having to do instanceof.
    public enum Sort {
        PACKAGE("package"), ENUM("enum"), ENUMVALUE("enumvalue"),
        EXTEND("extend"), EXTENSIONS("extensions"),
        FIELD("field"), MESSAGE("message"), GROUP("group"),
        OPTION("option"), RPC("rpc"), SERVICE("service"),
        PRIMITIVETYPE("primitivetype"), FILE("file"), ROOT("root"),
        PAIR("pair"), COMPOUNDCONSTANT("compoundconstant")
        ;

        private final String name;
        Sort(String name) {this.name = name;}
        public String getName()   { return name; }
    }

    // Define the kinds or primitive types
    public enum PrimitiveSort {
        DOUBLE("double"), FLOAT("float"),
        INT32("int32"), INT64("int64"),
        UINT32("uint32"), UINT64("uint64"),
        SINT32("sint32"), SINT64("sint64"),
        FIXED32("fixed32"), FIXED64("fixed64"),
        SFIXED32("sfixed32"), SFIXED64("sfixed64"),
        STRING("string"), BYTES("bytes"),
        BOOL("bool"),
        ;

        private final String name;
        PrimitiveSort(String name) {this.name = name;}
        public String getName()   { return name; }
    }

    static public class Position
    {
        int lineno = 0;
        int charno = 0;
        String filename = null;

        public Position() {}
        public Position(int line, int charno, String filename)
        {setLine(line); setChar(charno); setFile(filename);}

        public int getLine() {return lineno;}
        public int getChar() {return charno;}
        public String getFile() {return filename;}

        public void setLine(int lineno) {this.lineno = lineno;}
        public void setChar(int charno) {this.charno = charno;}
        public void setFile(String file) {this.filename = file;}

        public void moveLine(int delta)
            {this.lineno += delta; if(this.lineno < 0) this.lineno = 0;}
        public void moveChar(int delta)
            {this.charno += delta; if(this.charno < 0) this.charno = 0;}

        public boolean equals(Object o)
        {
            if(o instanceof Position) {
                Position p = (Position)o;
                return (p.getLine() == lineno && p.getChar() == charno
                        && p.getFile().equals(filename));
            }
            return false;
        }

        public String toString()
        {
            return String.format("%s:%d.%d",filename,lineno,charno);
        }

        public Position clone()
        {
            return new Position(lineno,charno,filename);
        }

    }

    // Helper class for storing stop-start pairs
    static public class Range
    {
        int stop = 0;
        int start = 0;
        public Range(int start, int stop) {this.start=start; this.stop=stop;}
    }

    // Helper class for options
    static public class OptionDef
    {
	String name;
	String typeref;
	AST.Type typedef;
	boolean userdefined;
	boolean system; // came from System.getProperties()

	public OptionDef() {this(null,null);}

	public OptionDef(String name, String typeref)
        {
	    this.name = name;
	    this.typeref = typeref;
	    this.typedef = null;
	    this.userdefined = false;
        }

	public void setUserDefined(boolean tf) {userdefined = tf;}
	public void setSystem(boolean tf) {system = tf;}
    }

    void setOptionDefs(List<OptionDef> defs);
    List<OptionDef> getOptionDefs();
    OptionDef getOptionDef(String key);

    //////////////////////////////////////////////////

    void setuid();
    int getId();

    int setRefCount(int n);
    void addRefCount(int n);

    List<AST> getChildSet();
    void setChildSet(List<AST> children);

    List<AST> getNodeSet();
    void setNodeSet(List<AST> nodeset);

    Position getPosition();
    void setPosition(Position position);
    Sort getSort();
    void setSort(Sort astclass);

    Root getRoot();
    void setRoot(Root root);

    File getSrcFile();
    void setSrcFile(File srcfile);

    Package getPackage();
    void setPackage(Package srcpackage);

    AST getParent();
    void setParent(AST parent);

    String getName();
    void setName(String name);

    String getScopeName();
    void setScopeName(String name);

    String getQualifiedName();
    void setQualifiedName(String qualifiedname);

    Object getAnnotation();
    void setAnnotation(Object annotation);

    List<Option> getOptions();
    void setOptions(List<Option> options);
    void addOption(Option option);

    Object optionLookup(OptionDef key);
    Object optionLookup(String key);
    Map<OptionDef,Object> getOptionMap();
    void setOptionMap(OptionDef key, Object value);
    void unsetOptionMap(OptionDef key);

    boolean isPacked();
    void setIsPacked(boolean tf);

    String toString();

public interface
Type extends AST
{}

public interface
Root extends AST
{
    List<File> getFileSet();
    void setFileSet(List<File> fileset);

    List<Package> getPackageSet();
    void setPackageSet(List<Package> packageset);

    List<PrimitiveType> getPrimitiveTypes();
    void setPrimitiveTypes(List<PrimitiveType> primitivetypes);

    File getTopFile();
    void setTopFile(File p);
}

public interface
File extends AST
{
    File getParentFile();
    void setParentFile(File f);
    Package getFilePackage();
    void setFilePackage(Package astpackage);
    List<File> getImports();
    void setImports(List<File> imports);

    // The following define the three important sets of decls
    List<Message> getMessages();
    void setMessages(List<Message> sessages);

    List<Extend> getExtends();
    void setExtends(List<Extend> extenders);

    List<Enum> getEnums();
    void setEnums(List<Enum> enums);

    List<Service> getServices();
    void setServices(List<Service> services);
}

public interface
Package extends AST
{
    File getPackageFile();
    void setPackageFile(File astfile);


}

public interface
Enum extends Type
{
    List<EnumValue> getEnumValues();
    void setEnumValues(List<EnumValue> enumvalues);
}

public interface
EnumValue extends AST
{
    int getValue();
    void setValue(int value);
}

public interface
Extend extends AST
{
    Message getMessage();
    void setMessage(Message message);
    List<Field> getFields();
    void setFields(List<Field> fields);
    List<Group> getGroups();
    void setGroups(List<Group> groups);
}

public interface
Extensions extends AST
{
    List<AST.Range> getRanges();
    void setRanges(List<AST.Range> ranges);
}

public interface
Field extends AST
{
    Cardinality getCardinality();
    void setCardinality(Cardinality cardinality);
    Type getType();
    void setType(Type fieldtype);
    int getId();
    void setId(int id);
    AST.Extend getExtend();
    void setExtend(AST.Extend ex);
}

public interface
Group extends AST.Field
{}

public interface
Message extends Type
{
    List<Field> getFields();
    void setFields(List<Field> fields);
    List<Enum> getEnums();
    void setEnums(List<Enum> enums);
    List<Message> getMessages();
    void setMessages(List<Message> messages);
    List<Extend> getExtenders();
    void setExtenders(List<Extend> extenders);
    List<Extensions> getExtensions();
    void setExtensions(List<Extensions> extension);
    List<Group> getGroups();
    void setGroups(List<Group> group);
}

public interface
Option extends AST
{
    OptionDef getOptionDef();
    void setOptionDef(OptionDef od);
    String getValue();
    void setValue(String value);
    // Specifically call out string values
    boolean isStringValued();
    void setStringValued(boolean tf);
}

public interface
CompoundConstant extends AST
{
    List<AST.Pair> getMembers();
    void setMembers(List<AST.Pair> pairs);
}

public interface
Pair extends AST
{
    Object getValue();
    void setValue(Object value);
}

public interface
RPC extends AST
{
    Type getArgType();
    void setArgType(Type argtype);
    Type getReturnType();
    void setReturnType(Type returntype);
}

public interface
Service extends AST
{
    List<RPC> getRPCs();
    void setRPCs(List<RPC> rpcs);
}

public interface
PrimitiveType extends Type
{
    PrimitiveSort getPrimitiveSort();
    void setPrimitiveSort(PrimitiveSort PrimitiveSort);
}

}// interface AST
