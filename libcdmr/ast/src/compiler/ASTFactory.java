/*
 * copyright (c) 1998 - 2010. university corporation for atmospheric research/unidata
 * portions of this software were developed by the unidata program at the
 * university corporation for atmospheric research.
 *
 * access and use of this software shall impose the following obligations
 * and understandings on the user. the user is granted the right, without
 * any fee or cost, to use, copy, modify, alter, enhance and distribute
 * this software, and any derivative works thereof, and its supporting
 * documentation for any purpose whatsoever, provided that this entire
 * notice appears in all copies of the software, derivative works and
 * supporting documentation.  further, ucar requests that the user credit
 * ucar/unidata in any publications that result from the use of this
 * software or in any product that includes this software. the names ucar
 * and/or unidata, however, may not be used in any advertising or publicity
 * to endorse or promote any products or commercial entity unless specific
 * written permission is obtained from ucar/unidata. the user also
 * understands that ucar/unidata is not obligated to provide the user with
 * any support, consulting, training or assistance of any kind with regard
 * to the use, operation and performance of this software nor to provide
 * the user with any updates, revisions, new versions or "bug fixes."
 *
 * this software is provided by ucar/unidata "as is" and any express or
 * implied warranties, including, but not limited to, the implied
 * warranties of merchantability and fitness for a particular purpose are
 * disclaimed. in no event shall ucar/unidata be liable for any special,
 * indirect or consequential damages or any damages whatsoever resulting
 * from loss of use, data or profits, whether in an action of contract,
 * negligence or other tortious action, arising out of or in connection
 * with the access, use or performance of this software.
 */

package unidata.protobuf.compiler;

import java.util.List;

public interface ASTFactory
{
    AST.Root newRoot(String name);
    AST.File newFile(String name);
    AST.Package newPackage(String name);
    AST.Enum newEnum(String name);
    AST.EnumValue newEnumValue(String name, int value);
    AST.Extend newExtend(String msgname);
    AST.Extensions newExtensions(String name);
    AST.Field newField(String name, AST.Cardinality cardinality, String fieldtype, int id);
    AST.Group newGroup(String name, AST.Cardinality cardinality, int id);
    AST.Message newMessage(String name);
    AST.Option newOption(String name, String value);
    AST.Pair newPair(String name, Object value);
    AST.CompoundConstant newCompoundConstant(String name);
    AST.RPC newRPC(String name, String argtype, String returntype);
    AST.Service newService(String name);
    AST.PrimitiveType newPrimitiveType(AST.PrimitiveSort sort);
}
