#include "memory_object_impl.h"
#include <map>
#include <typeinfo>

using namespace std;

namespace AbstractMemoryObject {

  //there are at least three levels resolution for modeling memory for labeled aggregates (structures, classes, etc)
  //
  //Think the following example:
  //-------------------
  //struct A { int i; int j;} ;
  //struct A a1;
  //struct A a2;
  //
  //int x = a1.i + a2.i + a2.j + a2.j;
  //-------------------
  //
  //1. most coarse: treat any references to struct A's instance as the same to struct A.
  //    This means to treat a1.i, a2.i, a2.j, a2.j as the same access to struct A.
  //    There is only one memory object for all a1.i, a2.i, a2.j and a2.j.
  //
  //2. finer: distinguish between different fields of A. But the same field to all instances will be treated as the same.
  //
  //  Now a1.i and a2.i will be modeled as A.i
  //      a2.j and a2.j will be modeled as A.j
  //  There are two memory objects for all a1.i, a2.i, a2.j and a2.j.
  //
  //3. Finest : distinguish between not only the fields, but also the instances.
  //
  //    a1.i, a2.i, a2.j, a2.j will be represented as four different objects.
  
  // We decide the resolution of this implementation is to distinguish between both fields and instances of aggregated objects.
  // This means type declarations (e.g. struct A{}) without declared instances (struct A a1; )will not have the corresponding memory objects.
  //
  // Naively creating NamedObj from symbol won't work since a single symbol can be shared by different NamedObj,
  // e.g.,  a1.i, and a2.i are different. But the field i will share the same symbol
  // So we have to traverse the SgVarRef to generate NamedObj
  // three level case: a1.mag.size  the AST will look like
  //    (a1 <-- SgDotExp --> mag ) <-- SgDotExp --> size
  //
  // To create all NamedObj, one has to search both symbols and SgVarRef and skip the overlapped objects.
  //
  // For all symbols, there are two categories
  // 1. symbols corresponding to real top level instances of types. Create NamedObj as we see each of them, NULL as parent
  //    They are the symbols with declarations not under SgClassDefinition
  // 2. symbols within type declarations: does not correspond NamedObj by itself. We should ignore them until we see the instance  
  //    They are the symbols with declarations under SgClassDefinition
  //   
  // For all SgVarRef, find the corresponding symbol 
  // 1. if is a instance symbol. skip since we already created it
  // 2. if it is a symbol within a declaration, two cases
  //    a. the symbol has a pointer type, we don't track pointer aliasing, so we create AliasedObj for it
  //    b. other types: a child of an instance, check if is the rhs of SgDotExp/SgArrowExp, if not assert
  //        use lhs of SgDotExp/SgArrowExp as parent
  //            lhs could be SgVarRefExp: find the corresponding NamedObj as parent (top level object, labeled aggregate)
  //            lhs could be another SgDotExp: find its rhs's NamedObj as parent
  //  
  //  To avoid duplicated NamedObj, map[parent][symbol] -> NamedObj
  //
  // TODO: build the connection between SgVarRefExp and the created NamedObj and support fast lookup!

  // the top level builder for ObjSet from any SgNode
   ObjSet* ObjSetFactory::createObjSet (SgNode* n)
   {
     ObjSet* rt = NULL;

     assert  (n!= NULL);
     if (isSgVarRefExp (n))
     {
       SgVarRefExp* exp = isSgVarRefExp (n);
       assert (exp != NULL);
       rt = createNamedOrAliasedObjSet (exp);
     }
     else if (isSgExpression(n)) // the order matters !! Must put after V_SgVarRefExp
     {
       SgExpression* exp = isSgExpression (n);
       assert (exp != NULL);
       rt = createExpressionObjSet (exp, exp->get_type());
     }
     else if (isSgType(n))
     {
       SgType* t = isSgType(n);
       assert (t != NULL);
       rt = createAliasedObjSet (t);
     }
     else if (isSgSymbol(n))
     {
       SgSymbol* s = isSgSymbol (n);
       assert (s != NULL);

       if (!isMemberVariableDeclarationSymbol (s))
         rt  = createNamedObjSet (s, s->get_type(), NULL); // parent should be NULL since it is not a member variable symbol
     }
     return rt;

   }



  // They are all single object
  size_t Scalar_Impl::objCount() {return 1;}
  size_t LabeledAggregate_Impl::objCount() {return 1;}
  size_t Array_Impl::objCount() {return 1;}
  size_t Pointer_Impl::objCount() {return 1;}

  bool Scalar_Impl::maySet() { return false; }
  bool LabeledAggregate_Impl::maySet() { return false; }
  bool Array_Impl::maySet() { return false; }
  bool Pointer_Impl::maySet() { return false; }

  bool Scalar_Impl::mustSet() { return true; }
  bool LabeledAggregate_Impl::mustSet() { return true; }
  bool Array_Impl::mustSet() { return true; }
  bool Pointer_Impl::mustSet() { return true; }

  std::string LabeledAggregateField_Impl::getName()
  {
    ObjSet* f = getField();
    NamedObj * nn = dynamic_cast <NamedObj*> (f);
    assert (nn != NULL);
    return nn->getName(); 
  }

  size_t LabeledAggregateField_Impl::getIndex()
  {
    LabeledAggregate* parent = getParent();
    std::vector<LabeledAggregateField *> elements = parent->getElements();
    size_t i =0;
    for (i=0; i++; i< elements.size())
    {
      if (this == elements[i])
        break;
    }
    assert (i !=  elements.size()); // must find it!
    return i;
  }
  std::string LabeledAggregateField_Impl::toString()
  {
    string rt;
    rt = "LabeledAggregateField_Impl: parent @ " + StringUtility::numberToString(parent)  + " field " 
      + field->toString();
    return rt;
  }

  // --------------------- Expression Object --------------------
  bool ExprObj::operator== (ExprObj& o2)
  {
    if (o2.anchor_exp == anchor_exp)
      return true;
    else
      return false;
  }

  bool ExprObj::operator== (ObjSet& o2)
  {
    bool is_expr_obj = true;
    // There might be better way to code this
    try
    {
      ExprObj & expr_o2 = dynamic_cast <ExprObj&> (o2);
    }
    catch (bad_cast & bc)
    {
      is_expr_obj= false;
    }

    if (is_expr_obj)
    {
      ExprObj & expr_o2 = dynamic_cast <ExprObj&> (o2);
      return (*this == expr_o2);
    }
    else
      return false;
  }
  std::string ExprObj::toString()
  {
    string rt;

    if (anchor_exp!= NULL)
      rt += "expression: " + anchor_exp->unparseToString() + " @ " + StringUtility::numberToString (anchor_exp);
    else
      rt += "expression: NULL";

    if (type != NULL )
      rt += "  type:" + type->unparseToString() + " @ " + StringUtility::numberToString(type);
    else
      rt += "  type: NULL";
    return rt;
  }
  //------------------
  std::set<SgType*> ScalarExprObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (ExprObj::getType());
    return rt;
  }

  bool ScalarExprObj::operator == (ObjSet& o2)
  {
   ExprObj& o1 = dynamic_cast<ExprObj&> (*this);
    return (o1==o2);
  } 

  std::string ScalarExprObj::toString()
  {
    string rt = "ScalarExprObj @" + StringUtility::numberToString(this)+ " "+ ExprObj::toString();
    return rt;
  }

  //------------------
  std::set<SgType*> PointerExprObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (ExprObj::getType());
    return rt;
  }

  ObjSet* PointerExprObj::getDereference() 
  {
    // simplest type-based implementation
    SgType* t = ExprObj::getType();
    SgPointerType* p_t = isSgPointerType(t);
    assert (p_t != NULL);
    return createAliasedObjSet (p_t);
  }

  ObjSet* PointerExprObj::getElements() // in case it is a pointer to array
  {
    ObjSet* rt = NULL;
    //TODO
    assert (false);
    return rt;
  }

  bool  PointerExprObj::equalPoints (Pointer & that)
  {
    // type based: same base type, sa
    SgType* this_type = *(this->getType().begin()); 
    SgType* that_type = *(that.getType().begin());
    return (this_type == that_type);
  }

  bool PointerExprObj::operator == (ObjSet& o2)
  {
   ExprObj& o1 = dynamic_cast<ExprObj&> (*this);
    return (o1==o2);
  } 


  std::string PointerExprObj::toString()
  {
    string rt = "PointerExprObj @" + StringUtility::numberToString(this)+ " "+ ExprObj::toString() ;
    return rt;
  }

   //---------------------
    LabeledAggregateExprObj::LabeledAggregateExprObj (SgExpression* e, SgType* t): ExprObj (e,t) 
  {
    assert (e != NULL);
    assert (t != NULL);

    assert (e->get_type() == t);
    SgClassType * c_t = isSgClassType(t);
    assert (c_t != NULL);
    fillUpElements (this, LabeledAggregate_Impl::getElements(), c_t);
  }

  std::set<SgType*> LabeledAggregateExprObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (ExprObj::getType());
    return rt;
  }

   std::string LabeledAggregateExprObj::toString()
   {
     std::string rt = "LabeledAggregateExprObj @ " + StringUtility::numberToString (this);
     rt += " "+ ExprObj::toString();
     rt += "   with " + StringUtility::numberToString(fieldCount()) + " fields:\n";
     for (int i =0; i< fieldCount(); i++)
     {
       rt += "\t" + (getElements())[i]->toString() + "\n";
     }
     return rt; 
   }


  // --------------------- Named Object --------------------
  
  bool NamedObj::operator== (NamedObj & o2)
  {
    bool rt = false;
    NamedObj & o1 = *this;
    if (o1.anchor_symbol == o2.anchor_symbol)
    {
      if (o1.parent == NULL && o2.parent == NULL)
        rt = true ;
      else if (o1.parent != NULL && o2.parent != NULL)  
      {
        //TODO when labeled object is ready
      }
    }
    return rt;
  }

  bool NamedObj::operator== (ObjSet & o2)
  {
    // three cases:
//    bool is_exp_obj = true; // always return false whenever expression obj is involved
    bool is_named_obj = true;
    bool is_aliased_obj = true;

    // There might be better way to code this
    try
    {
      AliasedObj & aliased_o2 = dynamic_cast <AliasedObj&> (o2);
    }
    catch (bad_cast & bc)
    {
      is_aliased_obj = false;
    }

    /*
       try
       {
       ExpressionObj & exp_o2 = dynamic_cast <ExpressionObj&> (o2); 
       } 
       catch (bad_cast & bc)
       {
       is_exp_obj = false;
       }
       */
    try
    {
      NamedObj& named_o2 = dynamic_cast <NamedObj&> (o2);
    }
    catch (bad_cast & bc)
    {
      is_named_obj = false;
    }

    NamedObj& o1 = *this;
    if (is_named_obj)
    {
      NamedObj & named_o2 = dynamic_cast <NamedObj&> (o2);
      return o1 == named_o2;
    }

    if (is_aliased_obj)
    {
      AliasedObj & aliased_o2 = dynamic_cast <AliasedObj&> (o2);
      return isAliased (o1.getType(), aliased_o2.getType());
    }

    return false;
    
  }

  std::string NamedObj::toString()
  {
    string rt;

    if (anchor_symbol != NULL)
      rt += "symbol: " + anchor_symbol->get_name().getString() + " @ " + StringUtility::numberToString (anchor_symbol);
    else
      rt += "symbol: NULL";

    if (type != NULL )
      rt += "  type:" + type->unparseToString() + " @ " + StringUtility::numberToString(type);
    else
      rt += "  type: NULL";

    if (parent != NULL )
//      rt += "  parent:" + parent->toString() + " @ " + StringUtility::numberToString(parent); // Cannot do this since it will cause infinite recursion
      rt += "  parent: @ " + StringUtility::numberToString(parent); // use address is sufficient
     else
       rt += "  parent: NULL";

    return rt;
  }

  //------------------
  std::set<SgType*> ScalarNamedObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (NamedObj::getType());
    return rt;
  }

  bool ScalarNamedObj::operator == (ObjSet& o2)
  {
   NamedObj& o1 = dynamic_cast<NamedObj&> (*this);
    return (o1==o2);
  } 

  std::string ScalarNamedObj::toString()
  {
    string rt = "ScalarNamedObj @" + StringUtility::numberToString(this)+ " "+ NamedObj::toString();
    return rt;
  }

  //------------------
  std::set<SgType*> PointerNamedObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (NamedObj::getType());
    return rt;
  }

  ObjSet* PointerNamedObj::getDereference() 
  {
    // simplest type-based implementation
    SgType* t = NamedObj::getType();
    SgPointerType* p_t = isSgPointerType(t);
    assert (p_t != NULL);
    return createAliasedObjSet (p_t);
  }

  ObjSet* PointerNamedObj::getElements() // in case it is a pointer to array
  {
    ObjSet* rt = NULL;
    //TODO
    assert (false);

    return rt;
    
  }

  bool  PointerNamedObj::equalPoints (Pointer & that)
  {
    // type based: same base type, sa
    SgType* this_type = *(this->getType().begin()); 
    SgType* that_type = *(that.getType().begin());
    return (this_type == that_type);
  }

  bool PointerNamedObj::operator == (ObjSet& o2)
  {
   NamedObj& o1 = dynamic_cast<NamedObj&> (*this);
    return (o1==o2);
  } 


  std::string PointerNamedObj::toString()
  {
    string rt = "PointerNamedObj @" + StringUtility::numberToString(this)+ " "+ NamedObj::toString() ;
    return rt;
  }

  // a helper function to fill up std::vector<LabeledAggregateField*>  from a class/structure type
  // TODO handle static members,they should be treated as global variables , not instances
  void fillUpElements (ObjSet* p, std::vector<LabeledAggregateField*> & elements, SgClassType* c_t)
  {
    assert (p!= NULL);
    LabeledAggregate* lp = dynamic_cast <LabeledAggregate* > (p);
    assert (lp != NULL);

    assert (c_t != NULL);
    SgDeclarationStatement * decl = c_t ->get_declaration();
    assert (decl != NULL);
    SgClassDeclaration* c_decl = isSgClassDeclaration(decl);
    assert (c_decl != NULL);
    SgClassDeclaration* def_decl = isSgClassDeclaration(c_decl->get_definingDeclaration()); 
    if (def_decl != NULL )
    {   
      SgClassDefinition * c_def = def_decl->get_definition();
      assert (c_def != NULL);
      // get members and insert LabeledAggregateField_Impl
      SgDeclarationStatementPtrList stmt_list = c_def->get_members();
      SgDeclarationStatementPtrList::iterator iter;
      for (iter = stmt_list.begin(); iter != stmt_list.end(); iter ++)
      {
        SgDeclarationStatement * decl_stmt = *iter;
        SgVariableDeclaration * var_decl = isSgVariableDeclaration (decl_stmt);
        if (var_decl)
        {
          SgVariableSymbol * s = SageInterface::getFirstVarSym(var_decl);
          ObjSet* field_obj = createNamedObjSet (s, s->get_type(), lp);
          LabeledAggregateField_Impl * f = new LabeledAggregateField_Impl (field_obj, lp);
          elements.push_back(f);
        }  
      }
    }
  }

   //---------------------
    LabeledAggregateNamedObj::LabeledAggregateNamedObj (SgSymbol* s, SgType* t, ObjSet* p): NamedObj (s,t,p) 
  {
    assert (s != NULL);
    assert (t != NULL);

    assert (s->get_type() == t);
    SgClassType * c_t = isSgClassType(t);

    fillUpElements (this, LabeledAggregate_Impl::getElements(), c_t);
  }

  std::set<SgType*> LabeledAggregateNamedObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (NamedObj::getType());
    return rt;
  }

   std::string LabeledAggregateNamedObj::toString()
   {
     std::string rt = "LabeledAggregateNamedObj @ " + StringUtility::numberToString (this);
     rt += " "+ NamedObj::toString();
     rt += "   with " + StringUtility::numberToString(fieldCount()) + " fields:\n";
     for (int i =0; i< fieldCount(); i++)
     {
       rt += "\t" + (getElements())[i]->toString() + "\n";
     }
     return rt; 
   }

  // --------------------- Aliased Object --------------------
  std::string AliasedObj::toString()  
  {
    string rt;
    if (type != NULL )
      rt += type->unparseToString() + " @ " + StringUtility::numberToString(type); 
    return rt;
  } 

  bool isAliased (SgType *t1, SgType* t2)
  {
     // Simplest alias analysis: same type ==> aliased
    // TODO : consider subtype, if type1 is a subtype of type2, they are aliased to each other also
    if (t1 == t2)
      return true;
    else
      return false;
    
  }

  bool AliasedObj::operator == ( AliasedObj & o2) 
  {
    AliasedObj& o1 = *this;
    SgType* own_type = o1.getType();
    SgType* other_type = o2.getType();
    return isAliased (own_type, other_type);
 }

  // TODO AliasedObj and NamedObj,  
  // TODO NamedObj and AliasedObj 
  bool AliasedObj::operator < ( AliasedObj& o2)
  {
    return (this->getType() < o2.getType());
  }

  bool AliasedObj::operator == (ObjSet& o2) 
  {
    // three cases
    // 1. o2 is  ExpressionObj: always return false
    // 2. o2 is Named Obj: return operator == (AliasedObj&o1, NamedObj & o2)
    // 3. o2 is AliasedObj:
    // bool is_exp_obj = true;
    bool is_named_obj = true;
    bool is_aliased_obj = true;

    // There might be better way to code this
    try
    {
      AliasedObj & aliased_o2 = dynamic_cast <AliasedObj&> (o2); 
    } 
    catch (bad_cast & bc)
    {
      is_aliased_obj = false;
    }

    /*
       try
       {
       ExpressionObj & exp_o2 = dynamic_cast <ExpressionObj&> (o2); 
       } 
       catch (bad_cast & bc)
       {
       is_exp_obj = false;
       }
       */
    try
    {
      NamedObj& named_o2 = dynamic_cast <NamedObj&> (o2); 
    } 
    catch (bad_cast & bc)
    {
      is_named_obj = false;
    }

    AliasedObj& o1 = *this;
    if (is_aliased_obj)
    {
      AliasedObj & aliased_o2 = dynamic_cast <AliasedObj&> (o2);
      return o1 == aliased_o2;
    }  

    if (is_named_obj)
    {
      NamedObj & named_o2 = dynamic_cast <NamedObj&> (o2);
      return isAliased (o1.getType(), named_o2.getType());
      
    }

    return false;
  }

  bool ScalarAliasedObj::operator == (ObjSet& o2)
  {
    AliasedObj& o1 = dynamic_cast<AliasedObj&> (*this);
    //  return (o1.operator==( o2));
    return (o1==o2);
  }

  bool LabeledAggregateAliasedObj::operator == (ObjSet& o2)
  {
    AliasedObj& o1 = dynamic_cast<AliasedObj&> (*this);
    //  return (o1.operator==( o2));
    return (o1==o2);
  }

  bool ArrayAliasedObj::operator == (ObjSet& o2)
  {
    AliasedObj& o1 = dynamic_cast<AliasedObj&> (*this);
    //  return (o1.operator==( o2));
    return (o1==o2);
  }

  bool PointerAliasedObj::operator == (ObjSet& o2)
  {
    AliasedObj& o1 = dynamic_cast<AliasedObj&> (*this);
    //  return (o1.operator==( o2));
    return (o1==o2);
  }



  std::set<SgType*> ScalarAliasedObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (AliasedObj::getType());
    return rt;
  }

  std::set<SgType*> LabeledAggregateAliasedObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (AliasedObj::getType());
    return rt;
  }

  std::set<SgType*> ArrayAliasedObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (AliasedObj::getType());
    return rt;
  }

  std::set<SgType*> PointerAliasedObj::getType()
  {
    std::set<SgType*> rt;
    rt.insert (AliasedObj::getType());
    return rt;
  }

  // toString()
  string ScalarAliasedObj::toString()
  {
    string rt = "ScalarAliasedObj @ " + StringUtility::numberToString(this)+ " "+ AliasedObj::toString();
    return rt;
  }
  string LabeledAggregateAliasedObj::toString()
  {
    string rt = "LabeledAggregateAliasedObj @ " + StringUtility::numberToString(this)+ " "+ AliasedObj::toString();
    return rt;
  }
  string ArrayAliasedObj::toString()
  {
    string rt = "ArrayAliasedObj @ " + StringUtility::numberToString(this)+ " "+ AliasedObj::toString();
    return rt;
  }
  string PointerAliasedObj::toString()
  {
    string rt = "PointerAliasedObj @ " + StringUtility::numberToString(this)+ " "+ AliasedObj::toString();
    return rt;
  }

  /*
     bool operator == ( ScalarAliasedObj & o1, ObjSet& o2) 
     {
     return dynamic_cast<ObjSet>
     }
     */

  // A map to store aliased obj set
  // This can provide quick lookup for existing aliased objset to avoid duplicated creation
  map<SgType*, ObjSet*> aliased_objset_map; 

  void dump_aliased_objset_map ()
  {
    cout<<"Not yet implemented."<<endl;
    assert (false);

  }
  // builder for different objects
  ObjSet* createAliasedObjSet(SgType*t)  // One object per type, Type based alias analysis
  {
    bool assert_flag = true; 
    assert (t!= NULL);
    ObjSet* rt = NULL;
    map<SgType*, ObjSet*>::const_iterator iter;
    iter = aliased_objset_map.find(t);
    if (iter == aliased_objset_map.end())
    { // None found, create a new one and update the map
      if (SageInterface::isScalarType(t))
        // We define the following SgType as scalar types: 
        // char, short, int, long , void, Wchar, Float, double, long long, string, bool, complex, imaginary 
      {
        rt = new ScalarAliasedObj(t);
        assert (rt != NULL); 
      }
      else if (isSgPointerType(t))
      {
        rt = new PointerAliasedObj(t);
        assert (rt != NULL); 
      }
      else if (isSgArrayType(t))
      {  
        rt = new ArrayAliasedObj (t);
        assert (rt != NULL); 
      }  
      else if (isSgClassType(t))
      {  
        rt = new   LabeledAggregateAliasedObj (t);
        assert (rt != NULL); 
      }  
      else
      {
        cerr<<"Warning: createAliasedObjSet(): unhandled type:"<<t->class_name()<<endl;
        assert_flag = false;
      }  

      // update the map  only if something has been created
      if (rt != NULL) 
        aliased_objset_map[t]= rt;
    }
    else // Found one, return it directly
    {
      rt = (*iter).second; 
    }  

    if (assert_flag)
      assert (rt != NULL); // we cannot assert this since not all SgType are supported now
    return rt;
  } 

  // A map to store named obj set
  // This can provide quick lookup for existing named objset to avoid duplicated creation
  // SgSymbol associated with class/struct data member is shared among all class/struct instances
  // so we have to use two keys (parent ObjSet and SgSymbol) to ensure the uniqueness of named objects
  map<ObjSet*,  map<SgSymbol*, ObjSet*> > named_objset_map; 

  // variables that are explicitly declared/named in the source code
  // local, global, static variables,
  // formal and actual function parameters
  //  Scalar
  //  Labeled aggregate
  //  Pointer
  //  Array
  ObjSet* createNamedObjSet(SgSymbol* anchor_symbol, SgType* t, ObjSet* parent)
  {
    ObjSet* rt = NULL;
    
    // check parameters
    assert (anchor_symbol != NULL);
    assert (anchor_symbol->get_type() == t);
    bool assert_flag = true; 

//    map<SgSymbol*, ObjSet*>::const_iterator iter;
//    iter = named_objset_map.find(anchor_symbol);
//    if (iter == named_objset_map.end())
    if (named_objset_map[parent][anchor_symbol] == NULL)
    { // None found, create a new one depending on its type and update the map
      if (SageInterface::isScalarType(t))
        // We define the following SgType as scalar types: 
        // char, short, int, long , void, Wchar, Float, double, long long, string, bool, complex, imaginary 
      { 
        rt = new ScalarNamedObj(anchor_symbol, t, parent);
        assert (rt != NULL);
      }
      else if (isSgPointerType(t))
      {
        rt = new PointerNamedObj(anchor_symbol,t, parent);
        assert (rt != NULL);
      }
      else if (isSgClassType(t))
      {
        rt = new LabeledAggregateNamedObj (anchor_symbol,t, parent);
        assert (rt != NULL);
      }
/* //TODO
      else if (isSgArrayType(t))
      {
        rt = new ArrayNamedObj (t);
        assert (rt != NULL);
      }
*/
      else
      {
        cerr<<"Warning: createNamedObjSet(): unhandled symbol:"<<anchor_symbol->class_name() << 
          " name: " <<  anchor_symbol->get_name().getString() << " type: "<< t->class_name()<< " @ "<<StringUtility::numberToString(anchor_symbol) <<endl;
        assert_flag = false;
      }

      // update the map  only if something has been created
      if (rt != NULL)
        named_objset_map[parent][anchor_symbol]= rt;
    }
    else // Found one, return it directly
    {
      //rt = (*iter).second;
      rt = named_objset_map[parent][anchor_symbol];
    }
  
    if (assert_flag)
      assert (rt != NULL); // we cannot assert this since not all SgType are supported now
    return rt;
  }

  // For a SgVarRef, find the corresponding symbol first
  // 1. if is a instance symbol. It corresponding to real top level instances of types. Create NamedObj as we see each of them, NULL as parent
  //     They are the symbols with declarations not under SgClassDefinition
  // 2. if it is a symbol within a class definition, two cases
  //    a. the symbol has a pointer type, we don't track pointer aliasing, so we create AliasedObj for it
  //    b. other types: a child of an instance, check if is the rhs of SgDotExp/SgArrowExp, if not assert
  //        use lhs of SgDotExp/SgArrowExp as parent
  //            lhs could be SgVarRefExp: find the corresponding NamedObj as parent (top level object, labeled aggregate)
  //            lhs could be another SgDotExp: find its rhs's NamedObj as parent

  ObjSet* createNamedOrAliasedObjSet (SgVarRefExp* r) // create NamedObjSet or aliased object from a variable reference 
  {
    assert (r!=NULL);
    SgVariableSymbol * s = r->get_symbol();
    assert (s != NULL);
    SgType * t = s->get_type();
    assert (t != NULL);
    //TODO think again about if we want to break the chain here   
#if 0  
    if (isSgPointerType(t)) // pointer type is taken care already by iterating through SgType from memory pools
    {
      //we don't track pointer aliasing, so we create AliasedObj for it
      return createAliasedObjSet (t);
    }
#endif    
    if (isMemberVariableDeclarationSymbol (s))
    { // symbol within SgClassDefinition
      //I think a reference to a data memory can only happen through . or -> operator
      SgExpression* parent = isSgExpression(r->get_parent());
      assert (parent != NULL);
      SgDotExp * d_e = isSgDotExp (parent);
      SgArrowExp * a_e = isSgArrowExp (parent);
      assert (d_e != NULL || a_e != NULL);
      SgBinaryOp* b_e = isSgBinaryOp (parent);
      assert (b_e != NULL);
      assert (b_e->get_rhs_operand_i() == r);

      // First, get ObjSet for its parent part
      ObjSet* p_obj = NULL; 
      SgExpression * lhs = b_e ->get_lhs_operand_i();
      assert (lhs != NULL);
      if (isSgVarRefExp(lhs))
      {
        p_obj = createNamedOrAliasedObjSet (isSgVarRefExp(lhs)); // recursion here
      }
      else if (isSgBinaryOp (lhs)) // another SgDotExp or SgArrowExp
      { // find its rhs's NamedObj as parent
        SgDotExp * d_e2 = isSgDotExp (lhs);
        SgArrowExp * a_e2 = isSgArrowExp (lhs);
        assert (d_e2 != NULL || a_e2 != NULL);
        SgExpression* rhs = isSgBinaryOp (lhs) -> get_rhs_operand_i();
        assert (isSgVarRefExp (rhs) != NULL); // there might be some more cases!!
        p_obj = createNamedOrAliasedObjSet (isSgVarRefExp(rhs));
      }
      // now create the child mem obj
      ObjSet* mem_obj = createNamedObjSet (s, s->get_type(), p_obj); 
      // assert (mem_obj != NULL); // we may return NULL for cases not yet handled
      return mem_obj;

    }
    else // other symbols
    {
      ObjSet* mem_obj = createNamedObjSet (s, s->get_type(), NULL); 
      // assert (mem_obj != NULL); // We may return NULL for cases not yet handled
      return mem_obj;
    }

  }

  
  // A map to avoid duplicated creation of ExprObj
  // SgExpression here excludes SgVarRef, which should be associated with a named memory object
  map<SgExpression*, ObjSet*> expr_objset_map; 

  ObjSet* createExpressionObjSet(SgExpression* anchor_exp, SgType*t)
  {
    ObjSet* rt = NULL;
    assert (anchor_exp != NULL);
    assert (t!= NULL);
    assert (anchor_exp->get_type() == t);

    bool assert_flag = true; 
    SgVarRefExp* var_exp = isSgVarRefExp (anchor_exp);
    if (var_exp)
    {
      cerr<<"Error. Trying to create an expression object when anchor_exp is a SgVarRefExp, which should be associated with a named object. "<<endl;
      assert (false);
    }  

    if (expr_objset_map[anchor_exp] == NULL)
    { // None found, create a new one depending on its type and update the map
      if (SageInterface::isScalarType(t))
        // We define the following SgType as scalar types: 
        // char, short, int, long , void, Wchar, Float, double, long long, string, bool, complex, imaginary 
      { 
        rt = new ScalarExprObj(anchor_exp, t);
        assert (rt != NULL);
      }
      else if (isSgPointerType(t))
      {
        rt = new PointerExprObj(anchor_exp,t);
        assert (rt != NULL);
      }
      else if (isSgClassType(t))
      {
        rt = new LabeledAggregateExprObj (anchor_exp,t);
        assert (rt != NULL);
      }
/* //TODO
      else if (isSgArrayType(t))
      {
        rt = new ArrayNamedObj (t);
        assert (rt != NULL);
      }
*/
      else
      {
        cerr<<"Warning: createExprObjSet(): unhandled expression:"<<anchor_exp->class_name() << 
          " string : " <<  anchor_exp->unparseToString() << " type: "<< t->class_name()<< " @ "<<StringUtility::numberToString(anchor_exp) <<endl;
        assert_flag = false;
      }

      // update the map  only if something has been created
      if (rt != NULL)
        expr_objset_map[anchor_exp]= rt;
    }
    else // Found one, return it directly
    {
      rt = expr_objset_map[anchor_exp];
    }
  
    if (assert_flag)
      assert (rt != NULL); // we cannot assert this since not all SgType are supported now
    return rt;
  }

  // a helper function to check if a symbol is corresponding to a member variable declaration within SgClassDefinition or not
  bool isMemberVariableDeclarationSymbol(SgSymbol * s)
  {
    bool rt = false; 
    assert (s!=NULL);
    // Only relevant for SgVariableSymbol for now
    SgVariableSymbol* vs = isSgVariableSymbol (s); 
    if (vs != NULL)
    {
      SgInitializedName* i_name = vs->get_declaration();
      assert  (i_name != NULL);
      if (isSgClassDefinition(i_name->get_scope()))
        rt = true;
    }
    return rt; 
  }


} // end namespace
