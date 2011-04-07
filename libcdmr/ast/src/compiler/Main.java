package unidata.protobuf.compiler;

import gnu.getopt.Getopt;
import unidata.protobuf.compiler.*;

import java.util.*;
import java.io.*;

public class Main
{
    // Define a map of -L language tags to class name tags
    static String[][] languagemap = new String[][] {
	// Note, the language tag is tested as lower case
	// while the class name tag is used as is.
	{"java","Java"},
	{"c","C"},
	{"c++","Cpp"}, 
	{"c#","Csharp"},
	{"python","Python"},
	{"ruby","Ruby"}
    };

    static final String DFALTLANGUAGE = "c";
    static final String DFALTPACKAGE = "unidata.protobuf.compiler";

    static List<String> includePaths = new ArrayList<String>();
    static List<String> defines = new ArrayList<String>();
    static List<String> arglist = new ArrayList<String>();
    static String optionLanguage = null;
    static boolean optionVerbose = false;
    static boolean optionParseDebug = false;
    static boolean optionSemanticsDebug = false;
    static boolean optionSemanticStepsDebug = false;
    static boolean optionDuplicate = false;

    static boolean debug = false;

    static public void main(String[] argv) throws Exception
    {
	int c;
	Getopt g = new Getopt("Main",argv,"-:D:I:L:W:V",null);
	while ((c = g.getopt()) != -1) {
            switch (c) {
	    case 1: // intermixed non-option
		arglist.add(g.getOptarg());
		break;		
	    case 'D':
		String def = g.getOptarg();
		if(def.length() > 0) defines.add(def);
		break;	
	    case 'I':
		String path = g.getOptarg();
		if(path.length() > 0) includePaths.add(path);
		break;	
	    case 'L':
		String optionLanguage = g.getOptarg();
		break;
	    case 'W':
		String wvalue = g.getOptarg();
		for(char x: wvalue.toCharArray()) {
		    switch (x) {
		    case 'd': debug = true; break;
		    case 'p': optionParseDebug = true; break;
		    case 't': optionSemanticStepsDebug = true; break;
		    case 's': optionSemanticsDebug = true; break;
		    case 'D': optionDuplicate = true; break;
		    default: break;
		    }
		}
		break;
	    case 'V':
		optionVerbose = true;		
		break;
	    case ':':
	        System.err.println("Command line option requires argument "+g.getOptopt());
	        System.exit(1);
	    case '?':
	        System.err.println("Illegal cmd line option: "+g.getOptopt());
	        System.exit(1);
	    default:
	        System.err.println("Unexpected getopt tag: "+c);
	        System.exit(1);
	    }
        }

	if(optionLanguage == null) optionLanguage = DFALTLANGUAGE;
	// Map the -L language to the class tag
	String classLanguageTag = null;
        for(int i=0;i<languagemap.length;i++) {
	    if(optionLanguage.equalsIgnoreCase(languagemap[i][0])) {
	        classLanguageTag = languagemap[i][1];
		break;
	    }
        }
        if(classLanguageTag == null) {
	    System.err.println("Unknown language: "+optionLanguage);
	    System.exit(1);
        }

	if(arglist.size() == 0) {
	    System.err.println("No input file specified");
	    System.exit(1);
	}

	String rawfilename = arglist.get(0);
	String escapedfilename = AuxFcns.escapedname(rawfilename);
	String inputfilename = AuxFcns.locatefile(rawfilename,includePaths);

	if(inputfilename == null) {
	    System.err.println("Cannot locate input file: "+rawfilename);
	    System.exit(1);
	}
	File inputfile = new File(inputfilename);
	if(!inputfile.canRead()) {
	    System.err.println("Cannot read input file: "+inputfile.toString());
	    System.exit(1);
	}
	FileReader rdr = new FileReader(inputfile);

        ASTFactory factory = new ASTFactoryDefault();
        ProtobufParser parser = new ProtobufParser(factory);
	parser.setIncludePaths(includePaths);
	if(optionParseDebug) parser.setDebugLevel(1);

	boolean pass = parser.parse(rawfilename,rdr);
	if(!pass) {
	    System.err.println("Parse failed");
	    System.exit(1);
	}

	// Compute the post-getopt argv.
        String[] finalargv = arglist.toArray(new String[arglist.size()]);

	// Semantic Processing

	if(optionSemanticStepsDebug) Debug.setTag("trace.semantics.steps");
	if(optionSemanticStepsDebug) Debug.setTag("trace.semantics.steps");
	if(optionSemanticsDebug) Debug.setTag("trace.semantics");
	if(optionDuplicate) {
	    Debug.setTag("trace.duplicate.package");
	}

	// Invoke all the initializers
	Semantics sem = new ProtobufSemantics();
	if(!sem.initialize(parser.getAST(),finalargv,factory)) {
	    System.err.println("Protobuf semantic initialization failure.");
	    System.exit(1);
	}

	// Try to locate language specific classes using reflection
        ClassLoader classLoader = Main.class.getClassLoader();

	// Try to locate the language specific Semantics checking class.
	String semanticsclassname = DFALTPACKAGE
				    + "."
				    + classLanguageTag
				    + "Semantics";
        Semantics langsemantics = null;
        try {
            Class semanticsclass = Class.forName(semanticsclassname);
	    langsemantics = (Semantics)semanticsclass.newInstance();
	    if(!langsemantics.initialize(parser.getAST(),finalargv,factory)) {
	        System.err.println(optionLanguage+": semantic initialization failure.");
	        System.exit(1);
	    }
        } catch (ClassNotFoundException e) {
	    // Ignore
        }

        // Do semantic processing
	pass = sem.process(parser.getAST());
	if(!pass) {
	    System.err.println("Protobuf semantic error detected.");
	    System.exit(1);
	}
	pass = langsemantics.process(parser.getAST());
	if(!pass) {
	    System.err.println(optionLanguage+": semantic errors detected");
	    System.exit(1);
	}

	// Try to locate the language specific Generator class
	String generatorclassname = DFALTPACKAGE
				    + "."
				    + classLanguageTag
				    + "Generator";
        try {
            Class generatorclass = Class.forName(generatorclassname);
	    Generator generator = (Generator)generatorclass.newInstance();
	    generator.generate(parser.getAST(),finalargv);
        } catch (ClassNotFoundException e) {
	    System.err.println("Generator class not found: "+generatorclassname);
	    System.exit(1);
        }
    }
}
