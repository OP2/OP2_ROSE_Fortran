// vim:sw=4 ts=4 tw=80 et sta fdm=marker:
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <boost/foreach.hpp>

#include "CppRuntimeSystem.h"
#include "rtedsync.h"

using namespace std;

ostream& out = cout;

static const long nondistributed = 0;

static
void beginScope(RuntimeSystem* rs, const std::string& desc)
{
  rs->beginScope(desc);
}

static
void endScope(RuntimeSystem* rs)
{
  rs->endScope(1);
}

static
void freeMemory(RuntimeSystem* rs, Address addr, MemoryType::AllocKind kind = akCxxNew)
{
  rs->freeMemory(addr, kind);
}

static
void createMemory(RuntimeSystem* rs, Address addr, size_t sz, MemoryType::AllocKind kind = akCxxNew, const RsType* t = NULL)
{
  rs->createMemory(addr, sz, kind, nondistributed, t);
}

static
void createArray(RuntimeSystem* rs, Address addr, const char* a, const char* b, const RsArrayType* arr)
{
  rs->createArray(addr, a, b, arr, akStack, nondistributed);
}


static
SourcePosition srcPosition(const char* file, size_t src, size_t rted)
{
  SourceInfo si;

  si.file = file; si.src_line = src; si.rted_line = rted;
  return SourcePosition(si);
}


static
void registerPointerChange( RuntimeSystem* rs,
                            Address src,
                            Address tgt,
                            const RsPointerType* t,
                            bool checkPointerMove = false,
                            bool checkMemLeaks = true
                          )
{
    rs->registerPointerChange(src, tgt, t, checkPointerMove, checkMemLeaks);
}


static
void registerPointerChange( RuntimeSystem* rs,
                            Address src,
                            Address tgt,
                            bool checkPointerMove = false,
                            bool checkMemLeaks = true
                          )
{
    rs->registerPointerChange(src, tgt, checkPointerMove, checkMemLeaks);
}

static
void registerPointerChange( RuntimeSystem* rs,
                            const std::string& mangledName,
                            Address tgt,
                            bool checkPointerMove = false,
                            bool checkMemLeaks = true
                          )
{
    Address src = rs->getStackManager()->getVariableByMangledName(mangledName)->getAddress();

    registerPointerChange(rs, src, tgt, checkPointerMove, checkMemLeaks);
}

static
void checkMemRead(RuntimeSystem* rs, Address addr, size_t sz)
{
  rs->checkMemRead(addr, sz);
}

static
void registerMemType(MemoryType& mt, size_t ofs, const RsType* t)
{
  Address addr = mt.beginAddress();

  addr.local += ofs;
  mt.registerMemType(addr, ofs, t);
}


static
void checkMemWrite(RuntimeSystem* rs, Address addr, size_t sz, const RsType* t = NULL)
{
  rs->checkMemWrite(addr, sz, t);
}

inline
Address asAddr(size_t sysaddr)
{
  return rted_Addr(reinterpret_cast<char*>(sysaddr));
}

template <class T>
inline
Address memAddr(const T* t)
{
  return rted_Addr(t);
}


template <class T>
inline
const T* point_to(const Address& addr)
{
  return reinterpret_cast<const T*>(addr.local);
}

static
void checkIfSameChunk(MemoryManager& mm, const Address& base, const Address& addr, size_t sz)
{
  mm.checkIfSameChunk(base, addr, sz);
}


/// \note operator+= is only safe in non distributed environments
inline
Address& operator+=(Address& lhs, long offset)
{
  lhs.local += offset;
  return lhs;
}

/// \note operator+ is only safe in non distributed environments
inline
Address operator+(const Address& lhs, long offset)
{
  Address tmp(lhs);

  tmp.local += offset;
  return tmp;
}

/// \note operator- is only safe in non distributed environments
inline
Address operator-(const Address& lhs, long offset)
{
  return lhs + (-offset);
}


#define TEST_INIT( MSG)                               \
    out << "-- " << (MSG) << endl ;         \
    bool errorFound=false;                            \
    RuntimeSystem * rs = RuntimeSystem::instance();   \
    rs->checkpoint(srcPosition( (MSG), __LINE__, __LINE__ ));              \


//#ASR 07/07/10
//Fixed RuntimeViolation object creation for Cxx Standard
#define TEST_CATCH( TYPE)                                          \
    catch (RuntimeViolation & e)  {                                \
        if( e.getType() != ( TYPE) ) {                             \
            out << "Wrong error detected: "                        \
                << e.getShortDesc() << " instead of "              \
                << RuntimeViolation(TYPE).getShortDesc()           \
                << " : " <<  __LINE__                              \
                << endl;                                           \
            exit(1);                                               \
        }                                                          \
        errorFound=true;                                           \
    }                                                              \
    if(!errorFound)                                                \
    {                                                              \
      out   << "Failed to detect error "                           \
            << RuntimeViolation(TYPE).getShortDesc()           \
                << " : " <<  __LINE__                              \
            << endl;                                               \
      exit(1);                                                     \
    }                                                              \
    errorFound=false;                                              \

#define CHECKPOINT rs -> checkpoint( srcPosition( "", __LINE__, __LINE__ ));


#define CLEANUP  {  rs->doProgramExitChecks();  rs->clearStatus(); }



// ------------------- cstdlib string test macros ----------------------------

// each test must define TRY_BODY, as a call to the method being tested, e.g.
//      rs->check_strlen(  (const char*) str)
#define TRY_BODY


#define DEST_SOURCE                                                 \
    char *dest = NULL, *source = NULL;
#define STR1_STR2                                                   \
    char *str1 = NULL, *str2 = NULL;
#define TEST_INIT_STRING( ... )                                     \
    size_t N = 9;                                                   \
    size_t SZ = (N + 1) * sizeof(char);                             \
    __VA_ARGS__;

// Tests that try body will complain if var isn't allocated or if it isn't
// initialized.
//
// As a side effect, this will also initialize var.
#define TEST_STRING_READ( var)                                      \
    /* haven't allocated var yet */                                 \
    try { TRY_BODY }                                                \
    TEST_CATCH( RuntimeViolation::INVALID_READ);                    \
                                                                    \
    var = (char*) malloc( SZ);                                      \
    strcpy( var, "         ");                                      \
    var[ N] = ' ';                                                  \
    createMemory(rs,  memAddr(var), SZ);                         \
                                                                    \
    /* haven't initialized var yet */                               \
    try { TRY_BODY }                                                \
    TEST_CATCH( RuntimeViolation::INVALID_READ);                    \
                                                                    \
    checkMemWrite(rs,  memAddr(var), SZ);

// Does TEST_STRING_READ, but also checks that try body complains if var doesn't
// have a null terminator in allocated memory.
#define TEST_STRING_READ_TERMINATED( var)                           \
    TEST_STRING_READ( var )                                         \
                                                                    \
    /* no null-terminator in var */                                 \
    try { TRY_BODY }                                                \
    TEST_CATCH( RuntimeViolation::INVALID_READ);                    \
    var[ N] = '\0';

#define TEST_STRING_WRITE( var, badsz, oksz)                        \
    /* haven't allocated var yet */                                 \
    try { TRY_BODY }                                                \
    TEST_CATCH( RuntimeViolation::INVALID_WRITE);                   \
    var = (char*) malloc( badsz );                                  \
    createMemory(rs,  memAddr(var), badsz );                     \
    /* var isn't large enough */                                    \
    try { TRY_BODY }                                                \
    TEST_CATCH( RuntimeViolation::INVALID_WRITE);                   \
                                                                    \
    freeMemory(rs,  memAddr(var));                               \
    var = (char*) realloc( var, oksz );                             \
    createMemory(rs,  memAddr(var), oksz );

#define TEST_STRING_CAT( var, oksz)                                 \
    /* haven't allocated var yet */                                 \
    try { TRY_BODY }                                                \
    TEST_CATCH( RuntimeViolation::INVALID_READ);                    \
    var = (char*) malloc( oksz );                                   \
    createMemory(rs,  memAddr(var), oksz );                      \
    var[ 0 ] = ' ';                                                 \
    var[ 1 ] = '\0';                                                \
    checkMemWrite(rs,  memAddr(var), 2);                         \
    /* var is large enough to hold source, but not large enough */  \
    /* for source to be appended */                                 \
    try { TRY_BODY }                                                \
    TEST_CATCH( RuntimeViolation::INVALID_WRITE);                   \
    var[ 0 ] = '\0';

// Tests that try body complains if either str1 or str2 lacks a null terminator
// in allocated memory, or if either isn't allocated yet.
//
// Tests that try body does not complain when both of these conditions are
// satisfied.
#define TEST_STR1_STR2                                              \
    TEST_INIT_STRING( STR1_STR2)                                    \
                                                                    \
    TEST_STRING_READ_TERMINATED( str1)                              \
    TEST_STRING_READ_TERMINATED( str2)                              \
    TRY_BODY                                                        \
    TEST_STRING_CLEANUP( str1)                                      \
    TEST_STRING_CLEANUP( str2)                                      \
                                                                    \
    TEST_STRING_READ_TERMINATED( str2)                              \
    TEST_STRING_READ_TERMINATED( str1)                              \
    TRY_BODY                                                        \
    TEST_STRING_CLEANUP( str2)                                      \
    TEST_STRING_CLEANUP( str1)

#define TEST_STRING_CLEANUP( var)                                   \
    free( var );                                                    \
    freeMemory(rs,  memAddr(var) );                              \
    var = NULL;

// -------------------------------------- Memory Checker ------------------------------------------

void testSuccessfulMallocFree()
{
    TEST_INIT("Testing Successful Malloc Free");
    errorFound=false; /*tps unused variable - removing warning */

    createMemory(rs, asAddr(42),10);

    rs->log() << "After creation" << endl;
    rs->printMemStatus();
    freeMemory(rs, asAddr(42));

    rs->log() << "After free" << endl;
    rs->printMemStatus();

    CLEANUP
}

void testFreeInsideBlock()
{
    TEST_INIT("Testing Invalid Free in allocated Block");

    createMemory(rs, asAddr(42),10);

    try  {  freeMemory(rs, asAddr(44)); }
    TEST_CATCH(RuntimeViolation::INVALID_FREE);

    //free the right one
    freeMemory(rs, asAddr(42));

    CLEANUP


}

void testInvalidFree()
{
    TEST_INIT("Testing invalid Free outside allocated Block");

    createMemory(rs, asAddr(42),10);

    try  {  freeMemory(rs, asAddr(500));   }
    TEST_CATCH(RuntimeViolation::INVALID_FREE)

    //free the right one
    freeMemory(rs, asAddr(42));

    CLEANUP
}

void testInvalidStackFree()
{
    TEST_INIT("Testing invalid Free of stack memory");

    // freeing heap memory should be fine
    createMemory(rs, asAddr(42),10);
    freeMemory(rs, asAddr(42));

    // but freeing stack memory is not okay
    rs->createStackMemory(asAddr(42),sizeof(int),"SgTypeInt");
    try  {  freeMemory(rs, asAddr(42));   }
    TEST_CATCH(RuntimeViolation::INVALID_FREE)

    // test cleanup
    freeMemory(rs, asAddr(42), akStack);

    CLEANUP
}

void testDoubleFree()
{
    TEST_INIT("Testing Double Free");

    createMemory(rs, asAddr(42),10);
    freeMemory(rs, asAddr(42));

    try  {  freeMemory(rs, asAddr(42));  }
    TEST_CATCH(RuntimeViolation::INVALID_FREE)

    CLEANUP
}

void testDoubleAllocation()
{
    TEST_INIT("Testing Double Allocation");


    createMemory(rs, asAddr(42),10);

    try{  createMemory(rs, asAddr(45),10);  }
    TEST_CATCH(RuntimeViolation::DOUBLE_ALLOCATION)


    freeMemory(rs, asAddr(42));

    CLEANUP
}


void testMemoryLeaks()
{
    TEST_INIT("Testing detection of memory leaks");

    createMemory(rs, asAddr(42),10);
    createMemory(rs, asAddr(60),10);
    createMemory(rs, asAddr(0),10);

    freeMemory(rs, asAddr(60));

    try{ rs->doProgramExitChecks(); }
    TEST_CATCH(RuntimeViolation::MEMORY_LEAK)

    freeMemory(rs, asAddr(0));
    freeMemory(rs, asAddr(42));

    CLEANUP
}

void testEmptyAllocation()
{
    TEST_INIT("Testing detection of empty allocation");

    try { createMemory(rs, asAddr(12),0); }
    TEST_CATCH(RuntimeViolation::EMPTY_ALLOCATION)

    CLEANUP
}


void testMemAccess()
{
    TEST_INIT("Testing memory access checks");

    createMemory(rs, asAddr(0),10);

    checkMemWrite(rs, asAddr(9),1);
    try { checkMemWrite(rs, asAddr(9),2); }
    TEST_CATCH(RuntimeViolation::INVALID_WRITE)

    try { checkMemRead(rs, asAddr(0), 4); }
    TEST_CATCH(RuntimeViolation::INVALID_READ)

    checkMemWrite(rs, asAddr(0), 5);
    checkMemRead(rs, asAddr(3), 2);

    freeMemory(rs, asAddr(0));

    CLEANUP
}

void testMallocDeleteCombinations()
{
    TEST_INIT("Testing malloc/delete, new/free and similar combinations");

    // memory created via malloc
    createMemory(rs,  asAddr(0x42), sizeof( long ), akCHeap );
    // can't be freed via non-free (e.g. delete)
    try { freeMemory(rs,  asAddr(0x42), akCxxNew ); }
    TEST_CATCH( RuntimeViolation::INVALID_FREE )
    // but can be freed via free
    freeMemory(rs,  asAddr(0x42), akCHeap );

    // memory created via new
    createMemory(rs,  asAddr(0x42), sizeof( long ), akCxxNew );
    // can't be freed via free
    try { freeMemory(rs,  asAddr(0x42), akCHeap ); }
    TEST_CATCH( RuntimeViolation::INVALID_FREE )
    // but can be freed via delete
    freeMemory(rs,  asAddr(0x42), akCxxNew );

    CLEANUP
}





// -------------------------------------- Stack and Variables Tests ------------------------------------------


/*
void testStack()
{
    Address addr=0;

    RuntimeSystem * rs = RuntimeSystem::instance();
    rs->createVariable(addr+=4,"GlobalVar1","MangledGlobal1","SgInt");
    rs->createVariable(addr+=4,"GlobalVar2","MangledGlobal2","SgDouble");

    cout << endl << endl << "After Globals" << endl;

    rs->printStack(cout);
    rs->printMemStatus(cout);

    beginScope(rs, "Function1");
    rs->createVariable(addr+=4,"Function1Var1","Mangled","SgInt");
    rs->createVariable(addr+=4,"Fucntion1Var2","Mangled","SgDouble");

    checkMemWrite(rs, 2348080,4);
    checkMemRead(rs, addr-4,4);

    cout << endl << endl << "After Function1" << endl;
    rs->printStack(cout);
    rs->printMemStatus(cout);


    endScope(rs);
    cout << endl << endl << "After return of function" << endl;
    rs->printStack(cout);
    rs->printMemStatus(cout);
}
*/


// -------------------------------------- File Monitoring Tests ------------------------------------------


void testFileDoubleClose()
{
    TEST_INIT("Testing double file close");

    rs->registerFileOpen((FILE*)42,"MyFileName.txt",READ);
    rs->registerFileClose((FILE*)42);

    try { rs->registerFileClose((FILE*)42); }
    TEST_CATCH(RuntimeViolation::INVALID_FILE_CLOSE)

    CLEANUP
}


void testFileDoubleOpen()
{
    TEST_INIT("Testing double file open");

    rs->registerFileOpen((FILE*)42,"MyFileName.txt",READ);
    try {  rs->registerFileOpen((FILE*)42,"Other.txt",READ); }
    TEST_CATCH(RuntimeViolation::DOUBLE_FILE_OPEN)

    rs->registerFileClose((FILE*)42);

    CLEANUP
}

void testFileInvalidClose()
{
    TEST_INIT("Testing invalid file close");

    rs->registerFileOpen((FILE*)42,"MyFileName.txt",READ);
    try {  rs->registerFileClose((FILE*)43); }
    TEST_CATCH(RuntimeViolation::INVALID_FILE_CLOSE)


    rs->registerFileClose((FILE*)42);

    CLEANUP
}

void testFileUnclosed()
{
    TEST_INIT("Testing detection of unclosed files");

    rs->registerFileOpen((FILE*)42,"MyFileName.txt",READ);
    try {  rs->doProgramExitChecks(); }
    TEST_CATCH(RuntimeViolation::UNCLOSED_FILES)

    rs->registerFileClose((FILE*)42);

    CLEANUP
}

void testFileInvalidAccess()
{
    TEST_INIT("Testing invalid file-access");


    try{ rs->registerFileOpen(NULL,"name",READ ); }
    TEST_CATCH(RuntimeViolation::INVALID_FILE_OPEN)


    FILE * fh = (FILE*)42;
    // some allowed operations
    rs->registerFileOpen(fh,"MyFileName.txt",READ | WRITE);
    rs->checkFileAccess(fh,true);
    rs->checkFileAccess(fh,false);
    rs->registerFileClose(fh);


    // check if illegal write is detected
    rs->registerFileOpen(fh,"MyFileName.txt",READ);
    rs->checkFileAccess(fh,true);
    try {  rs->checkFileAccess(fh,false); } //invalid write
    TEST_CATCH(RuntimeViolation::INVALID_FILE_ACCESS)
    rs->registerFileClose(fh);

    // check if illegal read is detected
    rs->registerFileOpen(fh,"MyFileName.txt",WRITE);
    rs->checkFileAccess(fh,false);
    try {  rs->checkFileAccess(fh,true); } //invalid read
    TEST_CATCH(RuntimeViolation::INVALID_FILE_ACCESS)
    rs->registerFileClose(fh);

    try {  rs->checkFileAccess((FILE*)43,true); }
    TEST_CATCH(RuntimeViolation::INVALID_FILE_ACCESS)

    CLEANUP
}

void testScopeFreesStack()
{
    TEST_INIT("Testing that exiting a scope frees stack variables")
    errorFound=false; /*tps unused variable - removing warning */


    beginScope(rs, "main");
    rs->createVariable(
        asAddr(4),
        "my_var",
        "mangled_my_var",
        "SgTypeInt",
        akStack,
        nondistributed
    );
    endScope(rs);

    CLEANUP
}

// Tests that an implicit scope exists, i.e. main's scope.  Calling
// createVariable without ever calling beginScope or endScope should not result
// in memory errors.Address
void testImplicitScope()
{
    TEST_INIT("Testing that an implicit scope exists for globals/main")
    errorFound=false; /*tps unused variable - removing warning */

    rs->createVariable(
        asAddr(4),
        "my_var",
        "mangled_my_var",
        "SgTypeInt",
        akStack,
        nondistributed
    );

    CLEANUP
}


// -------------------------------------- Pointer Tracking Tests ------------------------------------------

 void testLostMemRegion()
{
    //
    TEST_INIT("Testing detection of lost mem-regions");
    TypeSystem * ts = rs->getTypeSystem();

    createMemory(rs, asAddr(10), 2*sizeof(int));
    createMemory(rs, asAddr(18), 2*sizeof(int));

    Address       addr = asAddr(100);
    int           ptrSize = sizeof(void*);

    beginScope(rs, "Scope1");
        rs->createVariable(addr+=ptrSize,"p1_to_10","mangled_p1_to_10",ts->getPointerType("SgTypeInt"), akStack, nondistributed);
        registerPointerChange(rs, "mangled_p1_to_10", asAddr(10));

        rs->createVariable(addr+=ptrSize,"p1_to_18","mangled_p1_to_18",ts->getPointerType("SgTypeInt"), akStack, nondistributed);
        registerPointerChange(rs, "mangled_p1_to_18", asAddr(18));


        beginScope(rs, "Scope2");
            rs->createVariable(addr+=ptrSize,"p2_to_10","mangled_p2_to_10",ts->getPointerType("SgTypeInt"), akStack, nondistributed);
            registerPointerChange(rs, "mangled_p2_to_10", asAddr(10));
        endScope(rs);

        try{ registerPointerChange(rs, "mangled_p1_to_10", asAddr(0)); }
        TEST_CATCH(RuntimeViolation::MEM_WITHOUT_POINTER)

        try{ registerPointerChange(rs, "mangled_p1_to_18", asAddr(0)); }
        TEST_CATCH(RuntimeViolation::MEM_WITHOUT_POINTER)

    endScope(rs);

    freeMemory(rs, asAddr(10));
    freeMemory(rs, asAddr(18));

    CLEANUP
}

void testLostMemRegionFromDoublePointer()
{
    TEST_INIT(  "Testing detection of lost mem region, which was previously "
                "pointed to by heap ptr")
    TypeSystem * ts = rs -> getTypeSystem();

    AddressDesc ptr_desc = rted_ptr();
const RsType*     int_ptr = ts -> getPointerType( "SgTypeInt", ptr_desc );
    AddressDesc ptr_ptr_desc = rted_address_of(ptr_desc);
const RsType*     int_ptr_ptr = ts -> getPointerType( "SgTypeInt", ptr_ptr_desc );

    Address var_addr = asAddr(0x7ffb0);
    Address heap_addr_outer = asAddr(0x42);
    Address heap_addr_inner = asAddr(0x24601);

    createMemory(rs,  heap_addr_outer, 2 * sizeof( int* ));
    createMemory(rs,  heap_addr_inner, 2 * sizeof( int ));

    // int** ptr;
    rs -> createVariable( var_addr, "int**", "mangled_int**", int_ptr_ptr, akStack, nondistributed );
    // ptr = (int**) malloc( 2 * sizeof( int* ));
    registerPointerChange( rs, var_addr, heap_addr_outer, false );

    // ptr[ 0 ] = (int*) malloc( 2 * sizeof( int ));
    checkMemWrite( rs, heap_addr_outer, sizeof( int* ), int_ptr );
    rs->checkpoint( SourcePosition() );
    registerPointerChange( rs, heap_addr_outer, heap_addr_inner, false );


    try{ freeMemory(rs,  heap_addr_outer ); }
    TEST_CATCH( RuntimeViolation::MEM_WITHOUT_POINTER )

    freeMemory(rs,  heap_addr_inner );
    freeMemory(rs,  heap_addr_outer );

    CLEANUP
}

void testPointerChanged()
{
    TEST_INIT("Pointer Tracking test: Pointer changes chunks");

    createMemory(rs, asAddr(10), 2*sizeof(int));
    createMemory(rs, asAddr(18), 2*sizeof(int));

    TypeSystem * ts = rs->getTypeSystem();

    std::cout << "--- Scope 1" << std::endl;

    rs -> setViolationPolicy( RuntimeViolation::INVALID_PTR_ASSIGN, ViolationPolicy::Exit );
    // Case1: change of allocation chunk
    Address addr = asAddr(100);
    int ptrSize = sizeof(void*);
    beginScope(rs, "Scope1");

        rs->createVariable(addr+=ptrSize,"p1_to_10","mangled_p1_to_10", ts->getPointerType("SgTypeInt"), akStack, nondistributed);
        registerPointerChange(rs, "mangled_p1_to_10", asAddr(10));

        rs->createVariable(addr+=ptrSize,"p2_to_10","mangled_p2_to_10",ts->getPointerType("SgTypeInt"), akStack, nondistributed);
        registerPointerChange(rs, "mangled_p2_to_10", asAddr(10));

        rs->createVariable(addr+=ptrSize,"p1_to_18","mangled_p1_to_18",ts->getPointerType("SgTypeInt"), akStack, nondistributed);
        registerPointerChange(rs, "mangled_p1_to_18", asAddr(18));

        try{ registerPointerChange(rs, "mangled_p1_to_10", asAddr(18), true); }
        TEST_CATCH(RuntimeViolation::INVALID_PTR_ASSIGN )

        rs->checkpoint(SourcePosition());
        registerPointerChange(rs, "mangled_p1_to_18", asAddr(18+sizeof(int)));

        freeMemory(rs, asAddr(10));
        freeMemory(rs, asAddr(18));
    endScope(rs);


    std::cout << "--- Scope 2" << std::endl;

    // Case2: change of "type-chunk"
    beginScope(rs, "Scope2");
        struct A { int arr[10]; int behindArr; };
        RsClassType * typeA = &ts->getClassType("A",sizeof(A),false);
        typeA->addMember("arr",ts->getArrayType("SgTypeInt",10 * sizeof(int)), offsetof(A,arr));
        typeA->addMember("behindArr",ts->getTypeInfo("SgTypeInt"),offsetof(A,behindArr));
        assert(typeA->isComplete());

        // Create an instance of A on stack
        rs->createVariable(asAddr(0x42),"instanceOfA","mangled", typeA, akStack, nondistributed);

        rs->createVariable(asAddr(0x100),"intPtr","mangled_intPtr",ts->getPointerType("SgTypeInt"), akStack, nondistributed);
        registerPointerChange(rs, "mangled_intPtr",asAddr(0x42));

        try{ registerPointerChange(rs, "mangled_intPtr", asAddr(0x42) + 10*sizeof(int),true); }
        TEST_CATCH(RuntimeViolation::INVALID_PTR_ASSIGN )
    endScope(rs);

    std::cout << "--- Scope 3" << std::endl;

    // use default policy for case 3
    rs -> setViolationPolicy( RuntimeViolation::INVALID_PTR_ASSIGN, ViolationPolicy::InvalidatePointer );
    // Case3: into non-allocated memory
    rs -> beginScope( "Scope3" );

        CHECKPOINT
        int *ptr = (int*) (&ptr);
        rs->createVariable( memAddr(&ptr),"s3_ptr","mangled_s3_ptr",ts->getPointerType("SgTypeInt"), akStack, nondistributed);

        CHECKPOINT
        // default policy is to invalidate pointers
        registerPointerChange(rs, memAddr(&ptr), memAddr(ptr), ts->getPointerType("SgTypeInt"), true, false);

        CHECKPOINT
        try{ checkMemRead(rs,  memAddr(ptr), sizeof(ptr)); }
        TEST_CATCH(RuntimeViolation::INVALID_READ )


    endScope(rs);


    CLEANUP
}

void testInvalidPointerAssign()
{
    TEST_INIT("Testing Invalid Pointer assign");

    TypeSystem * ts = rs->getTypeSystem();

    beginScope(rs, "Scope2");
        // Create an instance of A on stack
        rs->createVariable(asAddr(0x42), "instanceOfA","mangled","SgTypeDouble", akStack, nondistributed);
        rs->createVariable(asAddr(0x100), "intPtr","mangled_intPtr",ts->getPointerType("SgTypeInt"), akStack, nondistributed);
        // Try to access double with an int ptr
        try { registerPointerChange(rs, "mangled_intPtr", asAddr(0x42)); }
        TEST_CATCH ( RuntimeViolation::INVALID_TYPE_ACCESS )

    endScope(rs);
    CLEANUP
}

void testPointerTracking()
{
    TEST_INIT("Testing Pointer tracking")
    errorFound=false; /*tps unused variable - removing warning */

    TypeSystem * ts = rs->getTypeSystem();

    // class A { int arr[2]; int intBehindArr; }
    RsClassType * type = &ts->getClassType("A",3*sizeof(int),false);
    type->addMember("arr",ts->getArrayType("SgTypeInt",2 * sizeof(int)));
    type->addMember("intBehindArr",ts->getTypeInfo("SgTypeInt"));

    beginScope(rs, "TestScope");
    rs->createVariable(asAddr(42), "instanceOfA","mangled","A", akStack, nondistributed);

    rs->createVariable(asAddr(100), "pointer","mangledPointer",ts->getPointerType("A"), akStack, nondistributed);

    //rs->setQtDebuggerEnabled(true);
    //rs->checkpoint(SourcePosition());


    endScope(rs);

    CLEANUP
}

// Note, this doesn't test for non-aligned access of the outer array, which
// doesn't work correctly.  e.g.
//
//  checkIfSameChunk( 0x100, 0x100 + sizeof( int ), 3 * sizeof( int ))
//
// which checks for access of an int[3] at x[ 0 ][ 1 ], which is within the same
// memory chunk, but not really legal.
void testMultidimensionalStackArrayAccess()
{
    TEST_INIT("Testing Multidimensional Array Access")

    TypeSystem * ts = rs -> getTypeSystem();
    MemoryManager* mm = rs -> getMemManager();

    size_t intsz = sizeof( int );

    // int x[ 2 ][ 3 ]
    //  type array of array of int
    const RsArrayType* type = ts->getArrayType( ts->getArrayType( "SgTypeInt", 3 * intsz), 2 * 3 * sizeof(int) );

    beginScope(rs, "TestScope");

    createArray( rs, asAddr(0x100), "array[2][3]", "mangled_array[2][3]", type );

    // check legal memory read from same memory region, but out of bounds on
    // inner array, i.e check
    //  x[ 0 ][ 3 ]     // actually x[ 1 ][ 0 ]
    try { checkIfSameChunk( *mm, asAddr(0x100), asAddr(0x100) + 3 * intsz, intsz); }
    TEST_CATCH ( RuntimeViolation::POINTER_CHANGED_MEMAREA )
    // as above, but out of bounds in the other direction
    try { checkIfSameChunk( *mm, asAddr(0x100) + 3 * intsz, asAddr(0x100) + 2 * intsz, intsz); }
    TEST_CATCH ( RuntimeViolation::POINTER_CHANGED_MEMAREA )

    CHECKPOINT
    // as above, but this time legally access the sub array
    checkIfSameChunk( *mm, asAddr(0x100) + 3 * intsz, asAddr(0x100) + 3 * intsz, intsz);

    endScope(rs);

    CLEANUP
}

void testArrayAccess()
{
    TEST_INIT("Testing Heap Array")
    TypeSystem * ts = rs->getTypeSystem();

    Address heapAddr = asAddr(0x42);
    createMemory(rs, heapAddr,10*sizeof(int));
    createMemory(rs, heapAddr+10*sizeof(int),10); //allocate second chunk directly afterwards

    rs -> setViolationPolicy( RuntimeViolation::INVALID_PTR_ASSIGN, ViolationPolicy::Exit );
    beginScope(rs, "Scope");

    Address pointerAddr = asAddr(0x100);
    rs->createVariable(asAddr(0x100),"intPointer","mangled_intPointer",ts->getPointerType("SgTypeInt"), akStack, nondistributed);
    CHECKPOINT
    registerPointerChange(rs, pointerAddr,heapAddr,false);

    CHECKPOINT
    //simulate iteration over array
    for(int i=0; i<10 ; i++)
        registerPointerChange(rs, pointerAddr,heapAddr+ i*sizeof(int),true);


    // write in second allocation ( not allowed to changed mem-chunk)
    try { registerPointerChange(rs, pointerAddr,heapAddr+ 10*sizeof(int),true); }
    TEST_CATCH ( RuntimeViolation::INVALID_PTR_ASSIGN )

    // write in illegal mem region before
    try { registerPointerChange(rs, pointerAddr, heapAddr - sizeof(int),true); }
    TEST_CATCH ( RuntimeViolation::INVALID_READ )

    freeMemory(rs, heapAddr);
    freeMemory(rs, heapAddr+10*sizeof(int));

    endScope(rs);
    rs -> setViolationPolicy( RuntimeViolation::INVALID_PTR_ASSIGN, ViolationPolicy::InvalidatePointer );

    CLEANUP
}

void testDoubleArrayHeapAccess()
{
    TEST_INIT("Testing Heap Double Array (e.g. int**)")
    errorFound=false; /*tps unused variable - removing warning */
    TypeSystem * ts = rs -> getTypeSystem();

    AddressDesc ptr_desc = rted_ptr();
const RsType*     int_ptr = ts -> getPointerType( "SgTypeInt", ptr_desc );
    AddressDesc ptr_ptr_desc = rted_address_of(ptr_desc);
const RsType*     int_ptr_ptr = ts -> getPointerType( "SgTypeInt", ptr_ptr_desc );

    Address var_addr = asAddr(0x7ffb0);
    Address heap_addr_outer = asAddr(0x42);
    Address heap_addr_inner = asAddr(0x24601);

    createMemory(rs,  heap_addr_outer, 2 * sizeof( int* ));
    createMemory(rs,  heap_addr_inner, 2 * sizeof( int ));

    // int** ptr;
    rs -> createVariable( var_addr, "int**", "mangled_int**", int_ptr_ptr, akStack, nondistributed );
    // ptr = (int**) malloc( 2 * sizeof( int* ));
    registerPointerChange( rs, var_addr, heap_addr_outer, false );

    // ptr[ 0 ] = (int*) malloc( 2 * sizeof( int ));
    checkMemWrite( rs, heap_addr_outer, sizeof( int* ), int_ptr );
    rs->checkpoint(SourcePosition());
    registerPointerChange( rs, heap_addr_outer, heap_addr_inner );


    freeMemory(rs,  heap_addr_inner );
    freeMemory(rs,  heap_addr_outer );

    CLEANUP
}

// -------------------------------------- CStdLib Tests ------------------------------------------

void test_memcpy()
{
    TEST_INIT("Testing calls to memcpy");

    Address address = asAddr(0);

    createMemory(rs,  address += 4, 16);
    Address ptr1 = address;

    createMemory(rs,  address += 16, 16);
    checkMemWrite(rs,  address, 16);
    Address ptr2 = address;


    // 4..20 doesn't overlap 20..36
    rs->check_memcpy( point_to<void>(ptr1), point_to<void>(ptr2), 16);

    // but 4..20 overlaps 16..32
    try { rs->check_memcpy( point_to<void>(ptr1), point_to<void>(ptr2 - 4), 16);}
    TEST_CATCH( RuntimeViolation::INVALID_MEM_OVERLAP)

    freeMemory(rs,  ptr1);
    freeMemory(rs,  ptr2);

    CLEANUP
}

void test_memmove()
{
    TEST_INIT("Testing calls to memmove");
    Address address = asAddr(0);

    createMemory(rs,  address += 4, 16);
    Address ptr1 = address;

    createMemory(rs,  address += 16, 16);
    Address ptr2 = address;


    try { rs->check_memmove( point_to<void>(ptr1), point_to<void>(ptr2 - 4), 16);}
    TEST_CATCH( RuntimeViolation::INVALID_READ)

    checkMemWrite(rs,  address, 16);
    rs->check_memmove( point_to<void>(ptr1), point_to<void>(ptr2), 16);

    freeMemory(rs,  ptr1);
    freeMemory(rs,  ptr2);

    CLEANUP
}

void test_strcpy()
{
    //~ TEST_INIT("Testing calls to strcpy");
//~
    //~ #undef TRY_BODY
    //~ #define TRY_BODY rs->check_strcpy(  dest, (const char*) source);
    //~ TEST_INIT_STRING( DEST_SOURCE)
    //~ TEST_STRING_READ_TERMINATED( source)
    //~ TEST_STRING_WRITE( dest, SZ / 2, SZ)
//~
    //~ // also, it's not legal for the strings to overlap
    //~ try { rs->check_strcpy( source + (N/2), source); }
    //~ TEST_CATCH( RuntimeViolation::INVALID_MEM_OVERLAP);
//~
    //~ TRY_BODY
    //~ TEST_STRING_CLEANUP( dest)
    //~ TEST_STRING_CLEANUP( source)
//~
    //~ CLEANUP
}

void test_strncpy()
{
    //~ TEST_INIT("Testing calls to strncpy");
//~
    //~ #undef TRY_BODY
    //~ #define TRY_BODY rs->check_strncpy(  dest, (const char*) source, N / 2);
    //~ TEST_INIT_STRING( DEST_SOURCE)
    //~ TEST_STRING_READ( source)
    //~ TEST_STRING_WRITE( dest, SZ / 4, SZ / 2)
//~
    //~ // also it's not legal for the strings to overlap
    //~ try { rs->check_strncpy(  source + (N/2),  source, N); }
    //~ TEST_CATCH( RuntimeViolation::INVALID_MEM_OVERLAP);
//~
    //~ TRY_BODY
    //~ TEST_STRING_CLEANUP( dest)
    //~ TEST_STRING_CLEANUP( source)
//~
    //~ CLEANUP
}

void test_strcat()
{
    TEST_INIT("Testing calls to strcat");

    #undef TRY_BODY
    #define TRY_BODY rs->check_strcat(  dest, (const char*) source);
    TEST_INIT_STRING( DEST_SOURCE)
    TEST_STRING_READ_TERMINATED( source)
    TEST_STRING_READ_TERMINATED( dest)
    TEST_STRING_CLEANUP( dest)
    TEST_STRING_CAT( dest, SZ)

    TRY_BODY
    TEST_STRING_CLEANUP( dest)
    TEST_STRING_CLEANUP( source)

    CLEANUP
}

void test_strncat()
{
    TEST_INIT("Testing calls to strncat");

    #undef TRY_BODY
    #define TRY_BODY rs->check_strncat(  dest, (const char*) source, N / 2);
    TEST_INIT_STRING( DEST_SOURCE)
    TEST_STRING_READ( source)
    TEST_STRING_READ( dest)
    TEST_STRING_CLEANUP( dest)
    TEST_STRING_CAT( dest, SZ / 2)

    TRY_BODY
    TEST_STRING_CLEANUP( dest)
    TEST_STRING_CLEANUP( source)

    CLEANUP
}

void test_strchr()
{
    TEST_INIT("Testing calls to strchr");

    #undef TRY_BODY
    #define TRY_BODY rs->check_strchr(  (const char*) str, 'x');
    TEST_INIT_STRING( char *str = NULL)
    TEST_STRING_READ_TERMINATED( str)

    TRY_BODY
    TEST_STRING_CLEANUP( str)

    CLEANUP
}

void test_strpbrk()
{
    TEST_INIT("Testing calls to strpbrk");

    #undef TRY_BODY
    #define TRY_BODY rs->check_strpbrk(  (const char*) str1, (const char*) str2);
    TEST_STR1_STR2

    CLEANUP
}

void test_strspn()
{
    TEST_INIT("Testing calls to strspn");

    #undef TRY_BODY
    #define TRY_BODY rs->check_strspn(  (const char*) str1, (const char*) str2);
    TEST_STR1_STR2

    CLEANUP
}

void test_strstr()
{
    TEST_INIT("Testing calls to strstr");

    #undef TRY_BODY
    #define TRY_BODY rs->check_strstr(  (const char*) str1, (const char*) str2);
    TEST_STR1_STR2

    CLEANUP
}

void test_strlen()
{
    TEST_INIT("Testing calls to strlen");

    #undef TRY_BODY
    #define TRY_BODY rs->check_strlen(  (const char*) str);
    TEST_INIT_STRING( char* str = NULL)
    TEST_STRING_READ_TERMINATED( str)

    TRY_BODY
    TEST_STRING_CLEANUP( str)

    CLEANUP
}

void test_memcpy_strict_overlap()
{
    TEST_INIT(  "Testing that memcpy complains about overlap for allocated "
                "ranges, not merely for the copied ranges.")

    const char* s1;
    const char* s2 = "here is string 2";


    createMemory(rs,  memAddr(s2), 16 * sizeof( char ));
    checkMemWrite(rs,  memAddr(s2), 16 * sizeof( char ));

    s1 = &s2[ 7 ];

    // even though there's no overlap with the copied ranges, memcpy shouldn't
    // be called when the allocated blocks overlap
    try { rs->check_memcpy( s1, s2, 3);}
    TEST_CATCH( RuntimeViolation::INVALID_MEM_OVERLAP)

    freeMemory(rs,  memAddr(s2));

    CLEANUP
}

void test_meminit_nullterm_included()
{
    TEST_INIT(  "Testing that strcpy et. al set the full destination "
                "initialized, including the null terminator");
    errorFound=false; /*tps unused variable - removing warning */
    size_t n = 9; // sizeof("a string") + 1 for \0
    char s1[ 9 ];
    char s2[ 9 ] = "a string";
    char s3[ 9 ];

    createMemory(rs,  memAddr(s1), n);
    createMemory(rs,  memAddr(s2), n);
    createMemory(rs,  memAddr(s3), n);

    checkMemWrite(rs,  memAddr(s2), n);

    rs->check_strcpy( (char*) s1, (const char*) s2);
    strcpy( s1, s2);
    rs->check_strcpy( (char*) s3, (const char*) s1);

    freeMemory(rs,  memAddr(s1));
    freeMemory(rs,  memAddr(s2));
    freeMemory(rs,  memAddr(s3));
    CLEANUP
}

void test_range_overlap()
{
    TEST_INIT("Testing that overlap checks cover ranges")

    char* s1;
    char s2[ 20 ];
    char s3[ 9 ] = "a string";


    createMemory(rs,  memAddr(s2), sizeof(s2));
    createMemory(rs,  memAddr(s3), sizeof(s3));
    checkMemWrite(rs,  memAddr(s3), sizeof(s3));
    rs->check_strcpy( s2, s3);

    strcpy( s2, s3);
    s1 = &s2[ 3 ];

    try { rs->check_strcat( s1, s2);}
    TEST_CATCH( RuntimeViolation::INVALID_MEM_OVERLAP)

    rs->check_strcpy( &s2[ 10 ], s3);
    strcpy( &s2[ 10 ], s3);

    // this should be fine -- the cat doesn't reach s2[ 10 ]
    s2[ 0 ] = '\0';
    s1 = &s2[0];
    rs->check_strcat( s1, &s2[ 10 ]);

    s1[ 0 ] = ' ';
    s1[ 3 ] = '\0';

    // now the cat will reach and an overlap occurs
    try { rs->check_strcat( s1, &s2[ 10 ]);}
    TEST_CATCH( RuntimeViolation::INVALID_MEM_OVERLAP)


    freeMemory(rs,  memAddr(s2));
    freeMemory(rs,  memAddr(s3));

    CLEANUP
}



void testTypeSystemDetectNested()
{
    TEST_INIT("Testing TypeSystem: nested type detection")

    TypeSystem * ts = rs->getTypeSystem();

    // Register Struct A
    struct A { int a1; char a2; double a3; };
    RsClassType * typeA = &ts->getClassType("A",sizeof(A),false);
    typeA->addMember("a1",ts->getTypeInfo("SgTypeInt"),   offsetof(A,a1));
    typeA->addMember("a2",ts->getTypeInfo("SgTypeChar"),  offsetof(A,a2));
    typeA->addMember("a3",ts->getTypeInfo("SgTypeDouble"),offsetof(A,a3));
    assert(typeA->isComplete());

    // Register Struct B
    struct B { A arr[10]; char b1; int b2; };
    RsClassType * typeB = &ts->getClassType("B",sizeof(B),false);
    typeB->addMember("arr",ts->getArrayType("A",10 * sizeof(struct A)),    offsetof(B,arr));
    typeB->addMember("b1",ts->getTypeInfo("SgTypeChar"),offsetof(B,b1) );
    typeB->addMember("b2",ts->getTypeInfo("SgTypeInt"), offsetof(B,b2));
    assert(typeB->isComplete());


    CHECKPOINT
    // Create Memory with first an A and then a B
    const Address ADDR = asAddr(42);
    createMemory(rs, ADDR, sizeof(A)+sizeof(B));
    MemoryType* mt = rs->getMemManager()->getMemoryType(ADDR);
    registerMemType(*mt, 0, ts->getTypeInfo("A"));
    CHECKPOINT
    registerMemType(*mt, 0 + sizeof(A), ts->getTypeInfo("B"));
    CHECKPOINT
    registerMemType(*mt, 0 + sizeof(A) + offsetof(A,a3), ts->getTypeInfo("SgTypeDouble"));


    //rs->setQtDebuggerEnabled(true);
    //rs->checkpoint(SourcePosition());
    //rs->setQtDebuggerEnabled(false);
    /*
    RsType * type1 = mt->getTypeAt(sizeof(A)+offsetof(B,arr),sizeof(A));
    RsType * type2 = mt->getTypeAt(sizeof(A)+offsetof(B,arr) + 5*sizeof(A)+1,sizeof(A));
    cout << "Type1 " << (type1 ? type1->getName() : "NULL") << endl;
    cout << "Type2 " << (type2 ? type2->getName() : "NULL") << endl;
    */

    CHECKPOINT
    //Access to padded area
    try { registerMemType(*mt, 0 + offsetof(A,a2)+1,ts->getTypeInfo("SgTypeChar")); }
    TEST_CATCH( RuntimeViolation::INVALID_TYPE_ACCESS)

    CHECKPOINT
    //Access to padded area
    //Wrong type
    try { registerMemType(*mt, 0, ts->getTypeInfo("B")); }
    TEST_CATCH( RuntimeViolation::INVALID_TYPE_ACCESS)

    CHECKPOINT
    //Access to padded area
    //Wrong basic type
    registerMemType(*mt, 0 + offsetof(A,a2), ts->getTypeInfo("SgTypeChar"));
    try { registerMemType(*mt, 0 + offsetof(A,a2), ts->getTypeInfo("SgTypeInt")); }
    TEST_CATCH( RuntimeViolation::INVALID_TYPE_ACCESS)

    rs->log() << "Type System Status after test" << endl;
    ts->print(rs->log());

    CHECKPOINT
    freeMemory(rs, asAddr(42));

    CLEANUP
}

void testTypeSystemMerge()
{
    TEST_INIT("Testing TypeSystem: Merging basic types into a struct ")
    TypeSystem * ts = rs->getTypeSystem();

    struct A { int a1; int a2; float a3; };
    RsClassType * typeA = &ts->getClassType("A",sizeof(A),false);
    typeA->addMember("a1", ts->getTypeInfo("SgTypeInt"),   offsetof(A,a1));
    typeA->addMember("a2", ts->getTypeInfo("SgTypeInt"),   offsetof(A,a2));
    typeA->addMember("a3", ts->getTypeInfo("SgTypeFloat"), offsetof(A,a3));
    assert(typeA->isComplete());

    rs -> checkpoint( SourcePosition() );
    const Address ADDR = asAddr(42);
    createMemory(rs, ADDR, 100);

        MemoryType* mt = rs->getMemManager()->getMemoryType(ADDR);
        //first part in mem is a double
        registerMemType(*mt, 0, ts->getTypeInfo("SgTypeDouble"));

        //then two ints are accessed
        registerMemType(*mt, 0 + sizeof(double)+offsetof(A,a1),ts->getTypeInfo("SgTypeInt"));
        registerMemType(*mt, 0 + sizeof(double)+offsetof(A,a2),ts->getTypeInfo("SgTypeInt") );
        //then the same location is accessed with an struct of two int -> has to merge
        registerMemType(*mt, 0 + sizeof(double),typeA);

        // because of struct access it is known that after the two ints a float follows -> access with int failes
        try { registerMemType(*mt, 0 + sizeof(double) + offsetof(A,a3), ts->getTypeInfo("SgTypeInt")); }
        TEST_CATCH( RuntimeViolation::INVALID_TYPE_ACCESS)


    rs -> checkpoint( SourcePosition() );
    freeMemory(rs, ADDR);

    CLEANUP
}

void testPartialTypeSystemArrayAccess() {
    TEST_INIT("Testing types: array access of type with subtype overlap")
    errorFound=false; /*tps unused variable - removing warning */
    MemoryManager *mm = rs -> getMemManager();
    TypeSystem *ts = rs -> getTypeSystem();

    // register type(s)
    struct Typ { char a; int b; };
    RsClassType *typ = &ts->getClassType( "Typ", sizeof( Typ ),false);
    typ -> addMember( "a", ts->getTypeInfo( "SgTypeInt" ), offsetof( Typ, a ));
    typ -> addMember( "b", ts->getTypeInfo( "SgTypeInt" ), offsetof( Typ, b ));
    assert( typ -> isComplete() );

    Address Addr = asAddr(0x42);
    size_t el2_offset = sizeof( Typ );
  //  size_t el2_a_offset = el2_offset + offsetof( Typ, a );
    size_t el2_b_offset = el2_offset + offsetof( Typ, b );

    createMemory(rs, Addr, 2 * sizeof( Typ ) );
    // register known memory
    //
    //  0       8   12  16
    //  ////[int]////[int]
    //  [  Typ  ]/////////
    MemoryType*mt = mm -> getMemoryType( Addr );
    registerMemType( *mt, 0, typ );
    registerMemType( *mt, 0 + el2_b_offset, ts->getTypeInfo( "SgTypeInt" ));

    // Check array access of larger type.  So far we have no reason to complain:
    // we don't know the full type at 8..16, but the part we do know is
    // consistent with the requested type.
    //
    //  0       8   12  16
    //          [  Typ?  ]
    CHECKPOINT
    checkIfSameChunk( *mm, Addr, Addr + el2_offset, (size_t)sizeof( Typ ));

    freeMemory(rs,  Addr );
    CLEANUP
}

void testTypeSystemSubtypes() {
    TEST_INIT("Testing types: createObject should handle subtypes")
    errorFound=false; /*tps unused variable - removing warning */
    MemoryManager* mm = rs -> getMemManager();
    TypeSystem* ts = rs -> getTypeSystem();

    // register types
    class Base { public: int x; };
    class Sub : public Base { public: int y[ 200 ]; };
    RsClassType* rs_base = new RsClassType( "Base", sizeof( Base ), false );
    rs_base -> addMember( "x", ts->getTypeInfo( "SgTypeInt" ), offsetof( Base, x ));
    assert( rs_base -> isComplete() );

    RsClassType* rs_sub = new RsClassType( "Sub", sizeof( Sub ), false );
    rs_sub -> addMember(
        "y",
        ts -> getArrayType( "SgTypeInt", sizeof( int[ 200 ])),
        offsetof( Sub, y ));
    assert( rs_sub -> isComplete() );

    // we should be able to call createObject for the same address in either
    // order and end up with the more specific (larger) type.
    rs -> createObject( asAddr(0x42), rs_base );
    rs -> createObject( asAddr(0x42), rs_sub );
    MemoryType* mt = mm -> findContainingMem( asAddr(0x42), 1 );
    assert( mt );
    assert( rs_sub == mt -> getTypeAt( 0, mt -> getSize() ));

    freeMemory(rs, asAddr(0x42) );

    // same test, but we call createObject in the reverse order
    rs -> createObject( asAddr(0x42), rs_sub );
    rs -> createObject( asAddr(0x42), rs_base );
    mt = mm -> findContainingMem( asAddr(0x42), 1 );
    assert( mt );
    assert( rs_sub == mt -> getTypeAt( 0, mt -> getSize() ));

    freeMemory(rs, asAddr(0x42) );

    CLEANUP
}

void testTypeSystemNested() {
    TEST_INIT("Testing types: createObject should ignore nested types")
    errorFound=false; /*tps unused variable - removing warning */
    MemoryManager* mm = rs -> getMemManager();
    TypeSystem* ts = rs -> getTypeSystem();

    // register types
    class Base { public: int x; };
    class Composite { public: int p; Base y[ 200 ]; };
    RsClassType* rs_base = new RsClassType( "Base", sizeof( Base ), false );
    rs_base -> addMember( "x", ts->getTypeInfo( "SgTypeInt" ), offsetof( Base, x ));
    assert( rs_base -> isComplete() );

    RsClassType* rs_composite = new RsClassType( "Composite", sizeof( Composite ), false );
    rs_composite -> addMember( "p", ts->getTypeInfo( "SgTypeInt" ), offsetof( Composite, p ));
    rs_composite -> addMember(
        "y",
        ts -> getArrayType( rs_base, sizeof( Base[ 200 ])),
        offsetof( Composite, y ));
    assert( rs_composite -> isComplete() );


    // Once we create the larger class, we should ignore calls to the composite
    // types.  If we did this in the reverse order the types would be merged.
    rs -> createObject( asAddr(0x42), rs_composite );
    MemoryType* mt = mm -> findContainingMem( asAddr(0x42), 1 );
    assert( mt );
    assert( rs_composite == mt -> getTypeAt( 0, mt -> getSize() ));

    rs -> createObject( asAddr(0x42) + offsetof( Composite, y ), rs_base );
    assert( mt );
    assert( rs_composite == mt -> getTypeAt( 0, mt -> getSize() ));

    freeMemory(rs,  asAddr(0x42) );

    CLEANUP
}

void testTypeConsistencyChecking() {
    TEST_INIT("Testing type consistency checking")
    errorFound=false; /*tps unused variable - removing warning */
    TypeSystem *ts = rs -> getTypeSystem();

    // register user types
    struct Typ { char a; int b; };
    RsClassType *typ = &ts->getClassType( "TypA", sizeof( Typ ), false);
    typ -> addMember( "a", ts->getTypeInfo( "SgTypeChar" ), offsetof( Typ, a ));
    typ -> addMember( "b", ts->getTypeInfo( "SgTypeInt" ), offsetof( Typ, b ));
    assert( typ -> isComplete() );

    typ = &ts->getClassType( "TypB", sizeof( Typ ),false);
    typ -> addMember( "a", ts->getTypeInfo( "SgTypeChar" ), offsetof( Typ, a ));
    typ -> addMember( "b", ts->getTypeInfo( "SgTypeInt" ), offsetof( Typ, b ));
    assert( typ -> isComplete() );

    // gather types
    const RsType & int_typ = *(ts->getTypeInfo( "SgTypeInt" ));
    const RsType & typA = *(ts->getTypeInfo( "TypA" ));
    const RsType & typB = *(ts->getTypeInfo( "TypB" ));

    vector< RsType* > to_delete;
    // char, 4 unknown
    RsCompoundType* compound = new RsCompoundType( sizeof( Typ ));
    RsType &compound_char = *compound;
    to_delete.push_back( compound );
    compound -> addMember( "", ts->getTypeInfo( "SgTypeChar" ), offsetof( Typ, a ));

    // 1 unknown, int
    compound = new RsCompoundType( sizeof( Typ ));
    RsType &compound_int = *compound;
    to_delete.push_back( compound );
    compound -> addMember( "", ts->getTypeInfo( "SgTypeInt" ), offsetof( Typ, b ));

    // 5 char
    compound = new RsCompoundType( sizeof( Typ ));
    RsType &compound_array = *compound;
    to_delete.push_back( compound );
    compound
        -> addMember(
            "",
            ts -> getArrayType( "SgTypeChar", 5 * sizeof( char )),
            offsetof( Typ, a ));

    assert( !( int_typ.isConsistentWith( typA )));
    assert( !( typA.isConsistentWith( int_typ )));
    assert( !( int_typ.isConsistentWith( typB )));

    // even though typeA and typB have the same members, if we know something is
    // a "TypB" then it cannot be a "TypA"
    assert( !( typA.isConsistentWith( typB )));

    assert( typA.isConsistentWith( compound_char ));
    assert( typA.isConsistentWith( compound_int ));
    assert( !(typA.isConsistentWith( compound_array )));
    assert( compound_char.isConsistentWith( typA ));
    assert( compound_int.isConsistentWith( typA ));

    assert( compound_char.isConsistentWith( compound_int ));
    assert( compound_char.isConsistentWith( compound_array ));

    assert( compound_int.isConsistentWith( compound_char ));
    assert( !( compound_int.isConsistentWith( compound_array )));

    // FIXME 3: we don't handle this case (see
    // RsCompoundType::checkConsistencyWith )
    // [ int ]
    // ? [int ]
    // not consistent b/c of space in the type for the char
    // assert( !(compound_int.isConsistentWith( int_typ )));
    assert( !(compound_char.isConsistentWith( int_typ )));

    BOOST_FOREACH( RsType* target, to_delete ) {
        delete target;
    }
    CLEANUP
}


extern "C"
{
  int upc_main(int, char**, char**)
  {
      std::cout << "Start RTED single thread tests " << std::endl;

      rted_UpcAllInitWorkzone();
      rted_UpcEnterWorkzone();

      try
      {
          RuntimeSystem * rs = RuntimeSystem::instance();
          rs->setTestingMode(true);
          rs->setOutputFile("test_output.txt");

          testTypeConsistencyChecking();
//~ //~
          testTypeSystemDetectNested();
          testTypeSystemMerge();
          testPartialTypeSystemArrayAccess();
//~ //~
          testTypeSystemSubtypes();
          testTypeSystemNested();
  //~ //~
          testSuccessfulMallocFree();
  //~ //~
          testFreeInsideBlock();
          testInvalidFree();
          testInvalidStackFree();
          testDoubleFree();
          testDoubleAllocation();
          testMemoryLeaks();
          testEmptyAllocation();
          testMemAccess();
          testMallocDeleteCombinations();
  //~ //~
          testFileDoubleClose();
          testFileDoubleOpen();
          testFileInvalidClose();
          testFileUnclosed();
          testFileInvalidAccess();
  //~ //~
          testScopeFreesStack();
          testImplicitScope();
  //~
          testLostMemRegion();
          testLostMemRegionFromDoublePointer();
          testPointerChanged();
          testInvalidPointerAssign();
          testPointerTracking();
          testArrayAccess();
          testDoubleArrayHeapAccess();
          testMultidimensionalStackArrayAccess();
  //~ //~
          test_memcpy();
          test_memmove();
          test_strcpy();
          test_strncpy();
          test_strcat();
          test_strncat();
          test_strchr();
          test_strpbrk();
          test_strspn();
          test_strstr();
          test_strlen();
  //~ //~
          test_memcpy_strict_overlap();
          test_meminit_nullterm_included();
          test_range_overlap();


    rs->doProgramExitChecks();
    printf("All tests passed. \n");
      }
      catch( RuntimeViolation& e)
      {
          out << "Unexpected Error: " << endl << e;
          exit( 1);
      }

      return 0;
  }

#ifdef WITH_UPC
  // \hack see comment in CppRuntimeSystem/ptrops.upc
  // \note since this is a single threaded test, we just provide the name
  char rted_base_hack[0];
#else
  int main(int noargs, char** args, char** envp)
  {
    upc_main(noargs, args, envp);
  }
#endif /* !WITH_UPC */
}
