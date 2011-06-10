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

/**
 * Adopt the protobuf printer idea of allowing embedded $...$ names
 */

class Printer
{
    String filename = null;
    Writer writer = null;
    PrintWriter pw = null;
    String deltaindent = "    ";

    String indentation = "";

    public Printer(String file) throws IOException
    {
	this(new FileWriter(file));	
    }

    public Printer(Writer writer) 
    {
        this.writer = writer;
	this.pw = new PrintWriter(this.writer);
    }

    // Allow direct access to the writer wrapped as a printwriter
    public PrintWriter getWriter() {return pw;}

    public void flush() {pw.flush();}

    public void close()
    {
	try { pw.close(); writer.close(); pw = null; writer = null; }
	catch (IOException ioe) {};
    }

    public void printf(String fmt, Object... args) throws IOException
    {
	String s = String.format(fmt,args);
        print(s);
    }

    public void println(String text) throws IOException {print(text + "\n");}

    public void print(String text) throws IOException
    {
	pw.print(indentation+text);
    }

    public void printcpp(String text) throws IOException
    {
	pw.print(text);
    }

    public void print(Map<String,String> map, String text)
	throws IOException 
   {
	int len = text.length();
	for(int pos=0;pos<len;) {
	    int c = text.charAt(pos++);
	    if(c == '$' && pos != len) {
		c = text.charAt(pos);
		if(c == '$') {
		    pos++;
      	            pw.append('$');
		} else {// Assume it is an keyword
		    int enddollar = text.indexOf('$',pos);
		    if(enddollar < 0) {
		        System.err.println("Unclosed identifier: "+text.substring(pos-1));
			pw.append('$');
		    } else {
			String identifier = text.substring(pos,enddollar);
			String match = map.get(identifier);
			if(match == null) {
		            System.err.println("Undefined identifier: "+identifier);
			    pw.append("$"+identifier+"$");
			}
			pos = enddollar;
		    }
		}
	    } else
    	        pw.append((char)c);
        }
    }

    public void blankline() {blankline(1);};
    public void blankline(int n) {while(--n >= 0) pw.printf("\n");};

    public void indent() {indentation += deltaindent;}

    public void outdent()
    {
        int newlen = indentation.length() - deltaindent.length();
	if(newlen < 0) {
	    System.err.println("Attempt to Outdent past column 0");
	    newlen = 0;
	}
        indentTo(newlen);
    }

    public void indentTo(int n)
    {
	if(n < 0) n = 0;
	indentation = "";
	while(n-- > 0) indentation += " ";
    }


    public void setIndent(int n)
    {
	if(n <= 0) return;
	String newindent = "";
	while(n-- > 0) newindent += " ";
	deltaindent = newindent;	
    }

    public int getIndent() {return deltaindent.length();}

    // Provide an indent stack
    Stack<Integer> indentstack = new Stack<Integer>();

    public void pushIndent() {pushIndent(0);}
    public void pushIndent(int n)
	{indentstack.push(indentation.length()); indentTo(n);}
    public void popIndent()
    {
	int n = (indentstack.empty()?0:indentstack.pop());
	indentTo(n);
    }

} // class Printer
