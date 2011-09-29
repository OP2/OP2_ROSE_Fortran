#ifndef SAF_COMMON_H
#define SAF_COMMON_H

#include "rose.h"
#include <list>
using std::list;
#include <map>
using std::map;
using std::pair;
using std::make_pair;
#include <set>
using std::set;
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <iostream>
using std::ostream;
using std::ofstream;
#include <sstream>
using std::stringstream;
using std::ostringstream;
using std::endl;
using std::cout;
using std::cerr;


using namespace VirtualCFG;

const int ZERO = 0;
//const int SPECIAL = 1;
const int INF = 10101010;
const std::string ZEROStr = "0";
//const std::string SPECIALStr = "$";

inline bool XOR(bool x, bool y) { return x != y; }

#define SgDefaultFile Sg_File_Info::generateDefaultFileInfoForTransformationNode()

/* #############################
   ######### T Y P E S #########
   ############################# */

typedef long long quad;
//typedef quad variable;

typedef std::map<quad, quad>                     m_quad2quad;
typedef std::map<quad, std::string>              m_quad2str;
typedef std::map<quad, m_quad2quad>              m_quad2map;
typedef std::pair<quad, quad>                    quadpair;
typedef std::list<quad>                          quadlist;
typedef std::map<quad, quadpair>                 m_quad2quadpair;
typedef std::map<quad, bool>                     m_quad2bool;

class printable
{
	public:
	virtual std::string str(std::string indent="")=0;
	virtual ~printable() {}
};

class dottable
{
	public:
	// Returns a string that containts the representation of the object as a graph in the DOT language
	// that has the given name
	virtual std::string toDOT(std::string graphName)=0;
};

/* #####################################
   ######### F U N C T I O N S #########
   ##################################### */

std::string itostr(int num);

/* #####################################
   ######### D E B U G G I N G #########
   ##################################### */


class dbgStream;

// Adopted from http://wordaligned.org/articles/cpp-streambufs
class dbgBuf: public std::streambuf
{
	friend class dbgStream;
	// True immediately after a new line
	bool synched;
	// True if the owner dbgStream is writing text and false if the user is
	bool ownerAccess;
	std::streambuf* baseBuf;
	std::list<std::string> funcs;
	//std::list<std::string> indents;
	
	// The number of observed '<' characters that have not yet been balanced out by '>' characters.
	// 	numOpenAngles = 1 means that we're inside an HTML tag
	// 	numOpenAngles > 1 implies an error or text inside a comment
	int numOpenAngles;
	
	// The number of divs that have been inserted into the output
	std::list<int> parentDivs;

public:
	
	// Construct a streambuf which tees output to both input
	// streambufs.
	dbgBuf();
	dbgBuf(std::streambuf* baseBuf);
	void init(std::streambuf* baseBuf);
	
private:
	// This dbgBuf has no buffer. So every character "overflows"
	// and can be put directly into the teed buffers.
	virtual int overflow(int c);
	
	// Prints the indent to the stream buffer, returns 0 on success non-0 on failure
	//int printIndent();
	
	// Prints the given string to the stream buffer
	int printString(std::string s);
	
	//virtual int sputc(char c);
	
	virtual std::streamsize xsputn(const char * s, std::streamsize n);
	
	// Sync buffer.
	virtual int sync();
	
	// Switch between the owner class and user code writing text
protected:
	void userAccessing();
	void ownerAccessing();

	// Indicates that the application has entered or exited a function
	void enterFunc(std::string funcName/*, std::string indent="    "*/);
	void exitFunc(std::string funcName);
};

// Stream that uses dbgBuf
class dbgStream : public std::ostream
{
        std::ofstream dbgFile;
	dbgBuf buf;
	std::vector<std::string> colors;
	// The root working directory
	std::string workDir;
	// The directory where all images will be stored
	std::string imgPath;
	// The name of the output debug file
	std::string dbgFileName;
	// The total number of images in the output file
	int numImages;
	
        std::ofstream summaryF;
	
public:
	// Construct an ostream which tees output to the supplied
	// ostreams.
	dbgStream();
	dbgStream(std::string title, std::string dbgFileName, std::string workDir, std::string imgPath);
	void init(std::string title, std::string dbgFileName, std::string workDir, std::string imgPath);
	~dbgStream();
	void printDetailFileHeader(std::string title);
	void printDetailFileTrailer();
		
	// Indicates that the application has entered or exited a function
	void enterFunc(std::string funcName/*, std::string indent="    "*/);
	void exitFunc(std::string funcName);
		
	// Adds an image to the output with the given extension and returns the path of this image
	// so that the caller can write to it.
	std::string addImage(std::string ext=".gif");
	
	// Given a reference to an object that can be represented as a dot graph, create an image from it and add it to the output.
	// Return the path of the image.
	std::string addDOT(dottable& obj);
	// Given a reference to an object that can be represented as a dot graph, create an image of it and return the string
	// that must be added to the output to include this image.
	std::string addDOTStr(dottable& obj);
	// Given a representation of a graph in dot format, create an image from it and add it to the output.
	// Return the path of the image.
	std::string addDOT(std::string dot);
	// The common work code for all the addDOT methods
	void addDOT(std::string imgFName, std::string graphName, std::string dot, std::ostream& ret);
};

// Indicates that the application has entered or exited a function
namespace Dbg {
	extern bool initialized;
	extern dbgStream dbg;
	
	// Initializes the debug sub-system
	void init(std::string title, std::string workDir, std::string fName="debug");
	
	// Indicates that the application has entered or exited a function
	void enterFunc(std::string funcName/*, std::string indent="    "*/);
	void exitFunc(std::string funcName);
		
	// Adds an image to the output with the given extension and returns the path of this image
	// so that the caller can write to it.
	std::string addImage(std::string ext=".gif");
	
	// Given a reference to an object that can be represented as a dot graph,  create an image from it and add it to the output.
	// Return the path of the image.
	std::string addDOT(dottable& obj);
	
	// Given a reference to an object that can be represented as a dot graph, create an image of it and return the string
	// that must be added to the output to include this image.
	std::string addDOTStr(dottable& obj);
	
	// Given a representation of a graph in dot format, create an image from it and add it to the output.
	// Return the path of the image.
	std::string addDOT(std::string dot);
	
	// Given a string, returns a version of the string with all the control characters that may appear in the 
	// string escaped to that the string can be written out to Dbg::dbg with no formatting issues.
	// This function can be called on text that has already been escaped with no harm.
	std::string escape(std::string s);
}

#endif
