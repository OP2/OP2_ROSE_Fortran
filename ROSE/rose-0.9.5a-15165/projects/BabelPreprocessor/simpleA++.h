
#define APP_RESTRICT

#define NULL 0L

// This simple class declaration allows us to write transformations that are simple (fast) to compile 
class InternalIndex
   {
     public:
       // member function declarations
          int base;
          int bound;
          int stride;

          InternalIndex();
          InternalIndex(int n);
          InternalIndex(int base, int bound);
          InternalIndex(int base, int bound, int stride);

       // operators for InternalIndex objects
       // This is consistant with A++/P++ InternalIndex (Index and Range) objects
          friend InternalIndex operator+ ( const InternalIndex & lhs, int x );
          friend InternalIndex operator+ ( int x, const InternalIndex & rhs );
          friend InternalIndex operator- ( const InternalIndex & lhs, int x );
          friend InternalIndex operator- ( int x, const InternalIndex & rhs );
   };

// Support for A++/P++ Index and Range objects
class Index : public InternalIndex
   {
     public:
          Index();
          Index(int n);
          Index(int base, int length);
          Index(int base, int length, int stride);
   };
class Range : public InternalIndex
   {
     public:
          Range();
          Range(int n);
          Range(int base, int bound);
          Range(int base, int bound, int stride);
   };

class DomainType
   {
     public:
       // data member declarations
          int Size  [6];
          int Stride[6];
       // member function declarations
          int getBase(int);
          int getBound(int);
          int getLength(int);
   };
class DescriptorType 
   { 
     public:
       // data member declarations 
          DomainType Array_Domain;
          double* APP_RESTRICT Array_View_Pointer1;
          double* APP_RESTRICT Array_View_Pointer2;
          double* APP_RESTRICT Array_View_Pointer3;
          double* APP_RESTRICT Array_View_Pointer4;
          double* APP_RESTRICT Array_View_Pointer5;
          double* APP_RESTRICT Array_View_Pointer6;
   };
class floatArray 
   { 
     public: 
       // data member declarations 
          DescriptorType Array_Descriptor; 
       // member function declarations 
          int getBase(int);  
          int getBound(int); 
   }; 
class intArray 
   { 
     public: 
       // data member declarations 
          DescriptorType Array_Descriptor; 
       // member function declarations 
          int getBase(int);  
          int getBound(int); 
   };
class doubleArray
   {
     public:
       // data member declarations
          DescriptorType Array_Descriptor;
       // member function declarations
          int getBase(int);
          int getBound(int);

       // destructor and constructors
         ~doubleArray ();
          doubleArray ();
          doubleArray (int i);
          doubleArray (int i, int j);
          doubleArray (int i, int j, int k);
          doubleArray (int i, int j, int k, int l);
          doubleArray (int i, int j, int k, int l, int m);
          doubleArray (int i, int j, int k, int l, int m, int n);

       // Indexing operators
          double & operator[](int n);

       // declaration of indexing operators
          doubleArray operator()( const InternalIndex & I );
          doubleArray operator()( const InternalIndex & I, const InternalIndex & J );
          doubleArray operator()( const InternalIndex & I, const InternalIndex & J,
                                  const InternalIndex & K );
          doubleArray operator()( const InternalIndex & I, const InternalIndex & J,
                                  const InternalIndex & K, const InternalIndex & L );

       // example array operators
          doubleArray & operator= ( int x );
          doubleArray & operator= ( doubleArray & X );
          friend doubleArray & operator+ ( doubleArray & lhs, doubleArray & rhs);
          friend doubleArray & operator- ( doubleArray & lhs, doubleArray & rhs);
          friend doubleArray & operator/ ( doubleArray & lhs, double rhs);
          friend doubleArray & operator+ ( doubleArray & lhs, double rhs);

          friend doubleArray & operator+ ( double lhs, doubleArray & rhs);
          friend doubleArray & operator- ( double lhs, doubleArray & rhs);
          friend doubleArray & operator* ( double lhs, doubleArray & rhs);
          friend doubleArray & operator/ ( double lhs, doubleArray & rhs);

          friend doubleArray & operator+ ( doubleArray & lhs, double rhs );
          friend doubleArray & operator- ( doubleArray & lhs, double rhs );
          friend doubleArray & operator* ( doubleArray & lhs, double rhs );
          friend doubleArray & operator/ ( doubleArray & lhs, double rhs );

          friend doubleArray & sin ( doubleArray & rhs);

          double* getAdjustedDataPointer();
          double* getAdjustedDataPointer( const InternalIndex & I );
          double* getAdjustedDataPointer( const InternalIndex & I, const InternalIndex & J );
          double* getAdjustedDataPointer( const InternalIndex & I, const InternalIndex & J,
                                          const InternalIndex & K );
          double* getAdjustedDataPointer( const InternalIndex & I, const InternalIndex & J,
                                          const InternalIndex & K, const InternalIndex & L  );

          doubleArray & redim ( doubleArray & X );
   };














