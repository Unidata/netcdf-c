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

public class ASTFactoryDefault implements ASTFactory
{
    public AST.Root newRoot(String name)
	{return new ASTDefault.Root(name);}
    public AST.File newFile(String name)
	{return new ASTDefault.File(name);}
    public AST.Package newPackage(String name)
	{return new ASTDefault.Package(name);}
    public AST.Enum newEnum(String name)
	{return new ASTDefault.Enum(name);}
    public AST.EnumValue newEnumValue(String name, int value)
	{return new ASTDefault.EnumValue(name, value);}
    public AST.Extend newExtend(String msgname)
	{return new ASTDefault.Extend(msgname);}
    public AST.Extensions newExtensions(String name)
	{return new ASTDefault.Extensions(name);}
    public AST.Field newField(String name, AST.Cardinality cardinality, String fieldtype, int id)
	{return new ASTDefault.Field(name, cardinality, fieldtype, id);}
    public AST.Group newGroup(String name, AST.Cardinality cardinality, int id)
	{return new ASTDefault.Group(name, cardinality, id);}
    public AST.Message newMessage(String name)
	{return new ASTDefault.Message(name);}
    public AST.Option newOption(String name, String value)
	{return new ASTDefault.Option(name, value);}
    public AST.Pair newPair(String name, Object value)
	{return new ASTDefault.Pair(name, value);}
    public AST.CompoundConstant newCompoundConstant(String name)
	{return new ASTDefault.CompoundConstant(name);}
    public AST.RPC newRPC(String name, String argtype, String returntype)
	{return new ASTDefault.RPC(name, argtype, returntype);}
    public AST.Service newService(String name)
	{return new ASTDefault.Service(name);}
    public AST.PrimitiveType newPrimitiveType(AST.PrimitiveSort sort)
	{return new ASTDefault.PrimitiveType(sort);}
}
