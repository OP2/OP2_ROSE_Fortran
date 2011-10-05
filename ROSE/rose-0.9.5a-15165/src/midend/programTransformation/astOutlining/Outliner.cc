/**
 *  \file Outliner.cc
 *  \brief An outlining implementation.
 */
// tps (01/14/2010) : Switching from rose.h to sage3.
#include "sage3basic.h"
#include <iostream>
#include <string>
#include <sstream>



#include "NameGenerator.hh"
#include "Outliner.hh"
#include "Preprocess.hh"
//#include "Transform.hh"
#include "commandline_processing.h"
#include "boost/filesystem.hpp"
namespace bfs=boost::filesystem;
// =====================================================================

using namespace std;

namespace Outliner {
  //! A set of flags to control the internal behavior of the outliner
  bool enable_classic=false;
  // use a wrapper for all variables or one parameter for a variable or a wrapper for all variables
  bool useParameterWrapper=false;  // use an array of pointers wrapper for parameters of the outlined function
  bool useStructureWrapper=false;  // use a structure wrapper for parameters of the outlined function
  bool preproc_only_=false;  // preprocessing only
  bool useNewFile=false; // generate the outlined function into a new source file
  bool temp_variable=false; // use temporary variables to reduce pointer dereferencing
  bool enable_liveness =false;
  bool enable_debug=false; // 
  bool exclude_headers=false;
  bool use_dlopen=false; // Outlining the target to a separated file and calling it using a dlopen() scheme. It turns on useNewFile.
  std::string output_path=""; // default output path is the original file's directory
  std::vector<std::string> handles; //  abstract handles of outlining targets, given by command line option -rose:outline:abstract_handle for each
};

// =====================================================================

//! Factory for unique outlined function names.
//prefix+id+suffix
static NameGenerator g_outlined_func_names ("OUT__", 0, "__");
static NameGenerator g_outlined_func_names2 ("OUT_", 0, "");
static NameGenerator g_outlined_arg_names ("__out_argv", 0, "__");

//! Hash a string into an unsigned long integer.
static
unsigned long
hashStringToULong (const string& s)
{
  unsigned long sum = 0;
  for (size_t i = 0; i < s.length (); ++i)
    sum += (unsigned long)s[i];
  return sum;
}

string
Outliner::generateFuncName (const SgStatement* stmt)
{
  // Generate a prefix.
  stringstream s;
  if (use_dlopen)  
    s<< g_outlined_func_names2.next();
  else
    s << g_outlined_func_names.next ();

  // Use the statement's raw filename to come up with a file-specific
  // tag.
  const Sg_File_Info* info = stmt->get_startOfConstruct ();
  ROSE_ASSERT (info);
  if (use_dlopen)
  {
    const string file_name = info->get_raw_filename ();
    const string file_name2 = StringUtility::stripPathFromFileName(file_name);
    string base_name = StringUtility::stripFileSuffixFromFileName(file_name2);
    // base name may contain '-', replace it with '_' to get legal identifier
    for (size_t i=0; i<base_name.size(); i++) 
    {
      //cout<<"file base name:"<<base_name[i]<<endl;
      if (base_name[i]=='-')
        base_name[i]='_';
    }

    s<<"_"<<base_name<<"_"<<info->get_line();
  }
  else
    s << hashStringToULong (info->get_raw_filename ()) << "__";

  return s.str ();
}
string
Outliner::generateFuncArgName (const SgStatement* stmt)
{
  // Generate a prefix.
  stringstream s;
  s << g_outlined_arg_names.next ();

  // Use the statement's raw filename to come up with a file-specific
  // tag.
  const Sg_File_Info* info = stmt->get_startOfConstruct ();
  ROSE_ASSERT (info);
  s << hashStringToULong (info->get_raw_filename ()) << "__";

  return s.str ();
}
// =====================================================================

Outliner::Result
Outliner::outline (SgStatement* s)
{
  string func_name = generateFuncName (s);
  return outline (s, func_name);
}

Outliner::Result
Outliner::outline (SgStatement* s, const std::string& func_name)
{
//cout<<"Debug Outliner::outline() input statement is:"<<s<<endl;  
  SgBasicBlock* s_post = preprocess (s);
//cout<<"Debug Outliner::outline() preprocessed statement is:"<<s_post<<endl;  
  ROSE_ASSERT (s_post);
#if 0
  //Liao, enable -rose:outline:preproc-only, 
  // then any translator can accept it even if their drivers do not handle it individually
  // Internal usage only for debugging
  static bool preproc_only_ = false; 
  SgFile * file= SageInterface::getEnclosingFileNode(s);
  SgStringList argvList = file->get_originalCommandLineArgumentList ();	


  if (CommandlineProcessing::isOption (argvList,"-rose:outline:",
                                     "preproc-only",true))
  {
    cout << "==> Running the outliner's preprocessing phase only." << endl;
    preproc_only_ = true;
  // Avoid passing the option to the backend compiler 
    file->set_originalCommandLineArgumentList(argvList);
  }  
#endif
  if (preproc_only_)
  {
    Outliner::Result fake;
    return fake;
  }  
  else
  {
 // return Transform::outlineBlock (s_post, func_name);

    Outliner::Result returnResult = outlineBlock (s_post, func_name);

#if 0
// This is now done in ASTtools::replaceStatement().
// DQ (2/24/2009): I think that at this point we should delete the subtree represented by "s_post"
// But it might have made more sense to not do a deep copy on "s_post" in the first place.
// Why is there a deep copy on "s_post"?
   SageInterface::deleteAST(s_post);
#else
   if (enable_debug)
     printf ("############ Skipped deletion of AST subtree at s_post = %p = %s \n",s_post,s_post->class_name().c_str());
#endif

   return returnResult;
  }  
}

//! Set internal options based on command line options
void Outliner::commandLineProcessing(std::vector<std::string> &argvList)
{
  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","enable_debug",true))
  {
    cout<<"Enabling debugging mode for outlined functions..."<<endl;
    enable_debug= true;
  }
  //  else // may be set to true by external module directly
  //    enable_debug= false;

  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","preproc-only",true))
  {
    if (enable_debug)
      cout<<"Enabling proprocessing only ..."<<endl;
    preproc_only_ = true;
  }
  //  else 
  //    preproc_only_ = false;

  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","parameter_wrapper",true))
  {
    if (enable_debug)
      cout<<"Enabling parameter wrapping..."<<endl;
    useParameterWrapper= true;
  }
  //  else
  //    useParameterWrapper= false;
  //
  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","structure_wrapper",true))
  {
    if (enable_debug)
      cout<<"Enabling parameter wrapping using a structure..."<<endl;
    useStructureWrapper= true;
  }

  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","new_file",true))
  {
    if (enable_debug)
      cout<<"Enabling new source file for outlined functions..."<<endl;
    useNewFile= true;
  }
  //  else
  //    useNewFile= false;
  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","exclude_headers",true))
  {
    if (enable_debug)
      cout<<"Excluding headers in the new file containing outlined functions.."<<endl;
    exclude_headers= true;
  }
  //  else
  //    exclude_headers= false;

  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","enable_classic",true))
  {
    if (enable_debug)
      cout<<"Enabling a classic way for outlined functions..."<<endl;
    enable_classic = true;
  }
  //  else
  //    enable_classic = false;

  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","temp_variable",true))
  {
    if (enable_debug)
      cout<<"Enabling using temp variables to reduce pointer dereferencing for outlined functions..."<<endl;
    temp_variable = true;
  }
  //  else
  //    temp_variable = false;
 
  if (CommandlineProcessing::isOption (argvList,"-rose:outline:","use_dlopen",true))
  {
    if (enable_debug)
      cout<<"Using dlopen() to find an outilned function saved into a new source file ..."<<endl;
    use_dlopen = true;
  }
  //  else   // this option may be set by other module
   std::string opstr;   
  if (CommandlineProcessing::isOptionWithParameter (argvList,"-rose:outline:","abstract_handle",opstr, true))
  {
    if (enable_debug)
    {
      cout<<"Enabling using abstract handles to specify targets for outlining..."<<endl;
      cout<<"Found a handle:"<<opstr<<endl;
    }
    handles.push_back(opstr);
  }

  if (CommandlineProcessing::isOptionWithParameter (argvList,"-rose:outline:","output_path",output_path, true))
  {
    // remove trailing '/'
    while (output_path[output_path.size()-1]=='/')
      output_path.erase(output_path.end()-1);
    // Create such path if not exists
    bfs::path my_path(output_path); 
    if (!bfs::exists(my_path))
      bfs::create_directory(my_path);
    if (!bfs::is_directory(my_path))  
    {
      cerr<<"output_path:"<<output_path<<" is not a path!"<<endl;
      ROSE_ASSERT(false);
    }

    if (enable_debug)
      cout<<"Using a custom output path:"<<output_path<<endl;
  }
//  else  //reset to NULL if useNewFile is not true
//    output_path="";


  //------------ handle side effects of options-----------
  if (useStructureWrapper) 
   {
     useParameterWrapper = true;
   } 
  //    use_dlopen = false;
  if (use_dlopen)
  {
    // turn on useNewFile as a side effect
    useNewFile= true;
    // also use parameter wrapper to simplify the call
    useParameterWrapper = true;
    temp_variable = true;
    exclude_headers = true;
    if (output_path.empty())
    {
      output_path="/tmp";
    }
  }

 if (temp_variable)    
  {
    if (CommandlineProcessing::isOption (argvList,"-rose:outline:","enable_liveness",true))
      enable_liveness = true;
    //     else
    //        enable_liveness = false;
  }

  // keep --help option after processing, let other modules respond also
  if ((CommandlineProcessing::isOption (argvList,"--help","",false)) ||
      (CommandlineProcessing::isOption (argvList,"-help","",false)))
  {
    cout<<"Outliner-specific options"<<endl;
    cout<<"Usage: outline [OPTION]... FILENAME..."<<endl;
    cout<<"Main operation mode:"<<endl;
    cout<<"\t-rose:outline:preproc-only                     preprocessing only, no actual outlining"<<endl;
    cout<<"\t-rose:outline:abstract_handle handle_string    using an abstract handle to specify an outlining target"<<endl;
    cout<<"\t-rose:outline:parameter_wrapper                use an array of pointers to pack the variables to be passed"<<endl;
    cout<<"\t-rose:outline:structure_wrapper                use a data structure to pack the variables to be passed"<<endl;
    cout<<"\t-rose:outline:enable_classic                   use parameters directly in the outlined function body without transferring statement, C only"<<endl;
    cout<<"\t-rose:outline:temp_variable                    use temp variables to reduce pointer dereferencing for the variables to be passed"<<endl;
    cout<<"\t-rose:outline:enable_liveness                  use liveness analysis to reduce restoring statements if temp_variable is turned on"<<endl;
    cout<<"\t-rose:outline:new_file                         use a new source file for the generated outlined function"<<endl;
    cout<<"\t-rose:outline:output_path                      the path to store newly generated files for outlined functions, if requested by new_file. The original source file's path is used by default."<<endl;
    cout<<"\t-rose:outline:exclude_headers                  do not include any headers in the new file for outlined functions"<<endl;
    cout<<"\t-rose:outline:use_dlopen                       use dlopen() to find the outlined functions saved in new files.It will turn on new_file and parameter_wrapper flags internally"<<endl;
    cout<<"\t-rose:outline:enable_debug                     run outliner in a debugging mode"<<endl;
    cout <<"---------------------------------------------------------------"<<endl;
  }
}


SgBasicBlock *
Outliner::preprocess (SgStatement* s)
{
  // bool b = isOutlineable (s, enable_debug);
  bool b = isOutlineable (s, SgProject::get_verbose () >= 1);
  if (b!= true)
  {
    cerr<<"Outliner::preprocess() Input statement:"<<s->unparseToString()<<"\n is not outlineable!"<<endl;
    ROSE_ASSERT(b);
  //  ROSE_ASSERT (isOutlineable (s, SgProject::get_verbose () >= 1));
  }
  SgBasicBlock* s_post = Preprocess::preprocessOutlineTarget (s);
  ROSE_ASSERT (s_post); 
  return s_post;
}

/* =====================================================================
 *  Container to store the results of one outlining transformation.
 */

Outliner::Result::Result (void)
  : decl_ (0), call_ (0)
{
}

Outliner::Result::Result (SgFunctionDeclaration* decl,
                             SgStatement* call, SgFile* file/*=NULL*/)
  : decl_ (decl), call_ (call), file_(file)
{
}

Outliner::Result::Result (const Result& b)
  : decl_ (b.decl_), call_ (b.call_)
{
}

bool
Outliner::Result::isValid (void) const
{
  return decl_ && call_;
}
// eof
