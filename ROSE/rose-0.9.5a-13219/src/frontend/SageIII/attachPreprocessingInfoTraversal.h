#ifndef _ATTACH_PREPROCESSING_INFO_TRAVERSAL_H_
#define _ATTACH_PREPROCESSING_INFO_TRAVERSAL_H_

// DQ (4/5/2006): Andreas has removed this code!

// void printOutComments ( SgLocatedNode* locatedNode );

// Need dummy classes and the actual tree traversal class
// DQ: Renamed classes, can't have DI and DS polluting the global name space (potential for strange errors)
// class DI : public SgInheritedAttribute {};
// class DS : public SgSynthesizedAttribute {};

// DQ (12/12/2008): This is the type use to hold all the CPP directives 
// and comments for each of many files.
typedef std::map<int, ROSEAttributesList*> AttributeMapType;
typedef std::map<int, int> StartingIndexAttributeMapType;
typedef std::map<int, SgLocatedNode*> previousLocatedNodeInFileType;

// DQ (11/29/2008): I don't think these are required to be derived from a special class any more!
// class AttachPreprocessingInfoTreeTraversalInheritedAttrribute  : public AstInheritedAttribute {};
// class AttachPreprocessingInfoTreeTraversalSynthesizedAttribute : public AstSynthesizedAttribute {};
class AttachPreprocessingInfoTreeTraversalInheritedAttrribute
   {
  // DQ (11/30/2008): I want to permit different list of directives and comments to be woven into the AST.
  // Comments and directives from the original source file need to be inserted into the AST for C/C++/Fortran.
  // However, for Fortran we also need to gather and insert the linemarker directives into the AST so that
  // we can support an analysis of the AST that will mark where code has been included from for the case of 
  // Fortran using CPP directives (e.g. #include directives).  To support this the mechanism for weaving
  // the ROSEAttributesList has be be used twice (just for CPP Fortran code) and we need to use this
  // weaving implementat with two different lists of directives.  But moving the ROSEAttributesList
  // into the inherited attribute we can set it differently for the two times we require it to be done.

     public:
       // DQ (12/12/2008): Make this a map to handle the attributes from more than one file (even if we 
       // only handle a single file, this added flexability is easier to support directly than to have 
       // an outer traversal vll an inner traversal).  This more general interface supports the case
       // where we save all comments and CPP directives used from include files in addition to the main 
       // source file.
       // ROSEAttributesList* currentListOfAttributes;
       // AttributeMapType* attributeMapForAllFiles;

#if 0
       // Constructor.
          AttachPreprocessingInfoTreeTraversalInheritedAttrribute(ROSEAttributesList* listOfAttributes)
             : currentListOfAttributes(listOfAttributes)
             {
            // Nothing else to do here.
             }
#else
       // AttachPreprocessingInfoTreeTraversalInheritedAttrribute(AttributeMapType* attributeMap) : attributeMapForAllFiles(attributeMap)
          AttachPreprocessingInfoTreeTraversalInheritedAttrribute()
             {
            // Nothing else to do here.
             }
#endif
   };

// This is an empty class, meaning that we could likely just have implemented a TopDownProcessing traversal.
class AttachPreprocessingInfoTreeTraversalSynthesizedAttribute {};

class AttachPreprocessingInfoTreeTrav 
   : public SgTopDownBottomUpProcessing<AttachPreprocessingInfoTreeTraversalInheritedAttrribute,
                                        AttachPreprocessingInfoTreeTraversalSynthesizedAttribute>
   {
     protected: // Pi-- private:
       //! accumulator attribute
       // SgLocatedNode *previousLocNodePtr;

       // Store the location in the AST of the previous node associated with each file.
          previousLocatedNodeInFileType previousLocatedNodeMap;

       // DQ (11/30/2008): This is now stored in the inherited attribute (so that it can be set external to the traversal).
       // List of all comments and CPP directives
       // ROSEAttributesList *currentListOfAttributes;
          AttributeMapType attributeMapForAllFiles;

       // DQ (12/12/2008): I don't think this is required since it is just the list size!
       // size of list?
       // int sizeOfCurrentListOfAttributes;

       // DQ (12/12/2008): This allows buildCommentAndCppDirectiveList() to get information about what language 
       // and version of language (fixed or free format for Fortran) as required to gather CPP directives and 
       // comments (more for comments than for CPP directives).  This is required even if processing other files
       // (include files).
      //! current source file name id (only handle strings from current file)
       // int currentFileNameId;
         SgSourceFile* sourceFile;

      //! AS(011306) Map of ROSEAttributesLists mapped to filename from Wave
       // DQ (12./12/2008): this should be updated to use int instead of strings.
       // For now I will not touch the Wave specific implementation.
       // std::map<std::string,ROSEAttributesList*>* currentMapOfAttributes;

      //! Use_Wave == true specifies if a wave preprocessor is used
          bool use_Wave;

      //  The mapOfAttributes declaration is specific to wave usage.
      //! Map of filenames to list of attributes as found by WAVE.  
       //   std::map<std::string,ROSEAttributesList*>* mapOfAttributes; 

      //! AS(092107) Optimization variable to avoid n^2 complexity in 
      //! iterateOverListAndInsertPreviouslyUninsertedElementsAppearingBeforeLineNumber()	  
       // int start_index;
          StartingIndexAttributeMapType startIndexMap;

      // DQ (12/16/2008): Added support to collect CPP directives and comments from all 
      // include files (except should specified using exclusion lists via the command line).
         bool processAllIncludeFiles;

     public:
       // DQ (9/24/2007): Moved function definition to source file from header file.
       // AS(011306) Constructor for use of Wave Preprocessor
          AttachPreprocessingInfoTreeTrav( std::map<std::string,ROSEAttributesList*>* attrMap);

     public:

       // DQ (9/24/2007): Moved function definition to source file from header file.
       // Constructor
          AttachPreprocessingInfoTreeTrav( SgSourceFile* file, bool includeDirectivesAndCommentsFromAllFiles );
#if 0
          AttachPreprocessingInfoTreeTrav();
#endif
          void setupPointerToPreviousNode (SgLocatedNode* currentLocNodePtr );

          void iterateOverListAndInsertPreviouslyUninsertedElementsAppearingBeforeLineNumber
             ( SgLocatedNode* locatedNode, int lineNumber, PreprocessingInfo::RelativePositionType location,
               bool reset_start_index, ROSEAttributesList *currentListOfAttributes );

       // Member function to be executed on each node of the AST
       // in the course of its traversal
          AttachPreprocessingInfoTreeTraversalInheritedAttrribute
             evaluateInheritedAttribute( SgNode *n, AttachPreprocessingInfoTreeTraversalInheritedAttrribute inh);

          AttachPreprocessingInfoTreeTraversalSynthesizedAttribute
             evaluateSynthesizedAttribute( SgNode *n, AttachPreprocessingInfoTreeTraversalInheritedAttrribute inh, SubTreeSynthesizedAttributes st);

       // DQ (10/27/2007): Added display function to output information gather durring the collection of 
       // comments and CPP directives across all files.
          void display(const std::string & label) const;

          AttributeMapType & get_attributeMapForAllFiles() { return attributeMapForAllFiles; }
       
       // Access function for elements in the map of attribute lists.
          ROSEAttributesList* getListOfAttributes ( int currentFileNameId );

          void setMapOfAttributes();


       // output for debugging.
       // void display_static_data( const std::string & label ) const;

       // DQ (11/30/2008): Refactored code to isolate this from the inherited attribute evaluation.
       // static ROSEAttributesList* buildCommentAndCppDirectiveList ( SgFile *currentFilePtr, std::map<std::string,ROSEAttributesList*>* mapOfAttributes, bool use_Wave );
          ROSEAttributesList* buildCommentAndCppDirectiveList ( bool use_Wave, std::string currentFilename );
   };

#endif

// EOF
