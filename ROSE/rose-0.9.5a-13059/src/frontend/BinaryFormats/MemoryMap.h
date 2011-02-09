#ifndef ROSE_MEMORY_MAP_H
#define ROSE_MEMORY_MAP_H

/* Increase ADDR if necessary to make it a multiple of ALMNT */
#define ALIGN_UP(ADDR,ALMNT)       ((((ADDR)+(ALMNT)-1)/(ALMNT))*(ALMNT))

/* Decrease ADDR if necessary to make it a multiple of ALMNT */
#define ALIGN_DN(ADDR,ALMNT)       (((ADDR)/(ALMNT))*(ALMNT))

/** A MemoryMap is an efficient mapping from virtual addresses to source bytes.  The source bytes can be bytes of a file,
 *  bytes stored in some memory buffer, or bytes initialized to zero and are described by the MemoryMap::MapElement class.
 *  The mapping can be built piecemeal and the data structure will coalesce adjacent memory areas when possible. If an attempt
 *  is made to define a mapping from a virtual address to multiple source bytes then an exception is raised. */
class MemoryMap {
public:
    /** Mapping permissions. */
    enum Protection 
	{
        MM_PROT_READ    = 0x1,          /**< Pages can be read. */
        MM_PROT_WRITE   = 0x2,          /**< Pages can be written. */
        MM_PROT_EXEC    = 0x4,          /**< Pages can be executed. */
        MM_PROT_NONE    = 0x0           /**< Pages cannot be accessed. */
    };

    /** Data structure for memory map names.  Often, memory map element names used for debugging are of the form
     *  \code
     *   FILE1(NAME1a+NAME1b+...)+FILE2(NAME2a+NAME2b+...)+...
     *  \endcode
     *
     *  where the file names are all unique. We'd like to be able to operate on these name strings in order to do things like
     *  merging two elements. This data type facilitates these kinds of operations. */
    typedef std::map<std::string, std::set<std::string> > NamePairings;
    
    /** A MemoryMap is composed of zero or more MapElements. Each map element describes a mapping from contiguous virtual
     *  addresses to contiguous file/memory bytes. A map element can point to a buffer supplied by the caller or a buffer
     *  allocated and managed by the MemoryMap itself. MemoryMap-managed buffers are used for anonymous maps where the backing
     *  store is initialized to all zero bytes rather than to values supplied by the caller.
     *
     *  If the user supplies a const pointer base address to the MapElement constructor then any call to MemoryMap::write()
     *  will fail when attempting to write to that map element.  The is_read_only() method returns true if the user supplied a
     *  const base address to the constructor.
     *
     *  The map element also tracks what permissions would be used if the memory were actually mapped for real. These
     *  permissions are bit flags MM_PROT_EXEC, MM_PROT_READ, MM_PROT_WRITE, and MM_PROT_NONE from the Protection enum. The
     *  presence or absence of the MM_PROT_WRITE bit here has no relation to the is_read_only() value -- it is legal for ROSE
     *  to write new values to a memory location that is mapped without MM_PROT_WRITE, but not to a memory location where
     *  is_read_only() is true. */
    class MapElement {
    public:
        MapElement()
            : va(0), size(0), base(NULL), offset(0), read_only(false), mapperms(MM_PROT_READ), anonymous(NULL) {}
        
        MapElement(const MapElement &other) {
            init(other);
        }

        MapElement &operator=(const MapElement &other) {
            nullify();
            init(other);
            return *this;
        }

        ~MapElement() {
            nullify();
        }

        /** Creates a mapping relative to a memory buffer.  The MemoryMap will coalesce adjacent elements having the same base
         *  when possible, but never elements having different bases. */
        MapElement(rose_addr_t va, size_t size, void *base, rose_addr_t offset, unsigned perms=MM_PROT_READ)
            : va(va), size(size), base(base), offset(offset), read_only(false), mapperms(perms), anonymous(NULL) {}

        /** Create a mapping relative to a read-only memory buffer. The MemoryMap will coalesce adjacent elements having the
         *  same base when possible, but never elements having different bases. */
        MapElement(rose_addr_t va, size_t size, const void *base, rose_addr_t offset, unsigned perms=MM_PROT_READ)
            : va(va), size(size), base(const_cast<void*>(base)), offset(offset), read_only(true), mapperms(perms), anonymous(NULL)
            {}

        /** Creates an anonymous mapping where all addresses of the mapping initially contain zero bytes. Note that memory
         *  is not allocated (and the base address is not assigned) until a write attempt is made. The implementation is free
         *  to coalesce compatible adjacent anonymous regions as it sees fit, reallocating memory as necessary. */
        MapElement(rose_addr_t va, size_t size, unsigned perms=MM_PROT_READ)
            : va(va), size(size), base(NULL), offset(0), read_only(false), mapperms(perms), anonymous(new Anonymous)
            {}

        /** Returns the starting virtual address for this map element. */
        rose_addr_t get_va() const {
            return va;
        }

        /** Returns the size in bytes represented by the entire map element. */
        size_t get_size() const {
            return size;
        }

        /** Returns true if the map element is anonymous. */
        bool is_anonymous() const {
            return anonymous!=NULL;
        }

        /** Returns true if the map points to read-only memory. This attribute is orthogonal to the mapping permissions
         *  returned by get_mapperms().  For instance, the underlying storage in ROSE can be a const buffer (is_read_only()
         *  returns true) even though the map element indicates that the storage would be mapped by the loader with write
         *  permission. */
        bool is_read_only() const {
            return read_only;
        }

        /** Returns mapping permissions. The mapping permissions are orthogonal to is_read_only(). For instance, an element
         *  can indicate that memory would be mapped read-only by the loader even when the underlying storage in ROSE is
         *  writable. */
        unsigned get_mapperms() const {
            return mapperms;
        }

        /** Modifies the mapping permissions.  The mapping permissions are orthogonal to is_read_only(). For instance, an
         *  element can indicate that memory would be mapped read-only by the loader even when the underlying storage in ROSE
         *  is writable. */
        void set_mapperms(unsigned new_perms) {
            mapperms = new_perms;
        }

        /** Returns the buffer to which the offset applies.  Invoking this method on an anonymouse map element will cause
         *  memory to be allocated if it hasen't been already (unless allocate_anonymous is false). */
        void *get_base(bool allocate_anonymous=true) const;

        /** Returns the starting offset for this map element. The offset is measured with respect to the value returned by
         *  get_base(). The offset is probably not of interest when the element describes an anonymous mapping. */
        rose_addr_t get_offset() const {
            return offset;
        }

        /** Returns the starting offset of the specified virtual address and verifies that the mapped region contains at least
         *  @p nbytes bytes beyond the specified address. Throws a MemoryMap::NotMapped exception if specified number of bytes
         *  after the virtual address are not represented by this map element. */
        rose_addr_t get_va_offset(rose_addr_t va, size_t nbytes=1) const;

        /** Returns true if this element is consistent with the @p other element. Consistent map elements can be merged when
         *  they are adjacent or overlapping with one another. Elements are not consistent if they have different base
         *  addresses or different permissions.  If the base addresses are the same, elements are not consistent if the
         *  difference in starting virtual addresses is not equal to the difference in offsets. */
        bool consistent(const MapElement &other) const;

        /** Attempts to merge the @p other element with this one.  Returns true if the elements can be merged; false if they
         *  cannot. If the two elements overlap but are inconsistent then a MemoryMap::Inconsistent exception is thrown. */
        bool merge(const MapElement &other);
		
        /** Give the map entry a name. This can be used for debugging, but don't rely too heavily on it because a MemoryMap
         *  may sometimes combine two adjacent elements that have different names. Returns a reference to @p this object so
         *  that it is convenient to use this method in the argument expression for MemoryMap::insert(). */
        MapElement& set_name(const std::string &name);
        
        /** Return the name assigned to this map element.  Names are used primarily for debugging purposes. */
        const std::string &get_name() const {
            return name;
        }

        /** Parses the name assigned to this map element. If the name is of the form described for the NamePairings data type
         *  then load the pairings into the @p pairings argument.  Whatever is not parsed is returned.
         *  argument. */
        std::string get_name_pairings(NamePairings*) const;
        
        /** Sets the name of this map element from the supplied pairings. Two additional strings can be concatenated into the
         *  result. */
        MapElement& set_name(const NamePairings&, const std::string &s1="", const std::string &s2="");

#ifdef _MSC_VER
        /* CH (4/15/2010): Make < operator be its member function instead of non-member function outside to avoid template
         * parameter deduction failure in MSVC */
// tps : commented out because of operatror < is ambigious.
// there is a function at the end of the file that is ambigious
//inline bool operator<(const MemoryMap::MapElement &a, const MemoryMap::MapElement &b) {
//        bool operator<(const MapElement &a) const {
 //           return this->get_va() < a.get_va();
  //      }
#endif

    private:
        friend class MemoryMap;

        /** Initialize this element using data from another element. */
        void init(const MapElement &other) {
            va = other.va;
            size = other.size;
            base = other.base;
            offset = other.offset;
            read_only = other.read_only;
            mapperms = other.mapperms;
            name = other.name;

            anonymous = other.anonymous;
            if (anonymous)
                anonymous->refcount++;
        }

        /** Make this a null mapping, releasing any anonymous memory that might be referenced. */
        void nullify() {
            if (anonymous) {
                if (0==--(anonymous->refcount)) {
                    delete[] anonymous->base;
                    delete anonymous;
                }
            }

            anonymous = NULL;
            va = 0;
            size = 0;
            base = NULL;
            offset = 0;
            read_only = 0;
            mapperms = MM_PROT_NONE;
            name = "";
        }

        /** Adjust the debugging name when merging two map elements. */
        void merge_names(const MapElement &other);

        /** Changes the virtual address of a mapped region.  This is private because changing the starting address can cause
         *  the mapping to become inconsistent. In general, it is safe to split a map element into smaller pieces and this
         *  method can be used to adjust the starting addresses of those pieces. The set_offset() method should also be called
         *  when adjusting the starting address. */
        void set_va(rose_addr_t new_va) {
            va = new_va;
        }

        /** Changes the starting offset with respect to the 'base'.  The starting offset sometimes needs to be adjusted when
         *  splitting a map element into multiple parts. */
        void set_offset(rose_addr_t new_offset) {
            offset = new_offset;
        }

        /** Changes the size of a mapped region. This is private because changing the size can cause the mapping to become
         *  inconsistent. In general, it is safe to split a map element into smaller pieces and this method can be used to
         *  adjust the size of those pieces. */
        void set_size(size_t sz) {
            ROSE_ASSERT(sz>0);
            size = sz;
        }

        rose_addr_t va;                 /**< Virtual address for start of region */
        size_t size;                    /**< Number of bytes in region */
        mutable void *base;             /**< The buffer to which 'offset' applies (access only through get_base()) */
        rose_addr_t offset;             /**< Offset with respect to 'base' */
        bool read_only;                 /**< If set then write() is not allowed */
        unsigned mapperms;              /**< Mapping permissions (MM_PROT_{READ,WRITE,EXEC} from Protection enum) */
        std::string name;               /**< Name used for debugging purposes */

        /** Reference counter for anonymous mappings.  The buffer for anonymous mappings is allocated and freed by this class
         *  rather than the user.  Anonymous mappings are indicated by a non-null pointer. */
        class Anonymous {
        private:
            Anonymous(const Anonymous &other) { abort(); }
            Anonymous &operator=(const Anonymous &other) { abort(); }
        public:
            Anonymous(): refcount(1), base(NULL) {}
            size_t refcount;
            uint8_t *base;
        } *anonymous;
    };

    /** Exceptions for MemoryMap operations. */
    struct Exception {
        Exception(const MemoryMap *map)
            : map(map) {}
        virtual ~Exception() {}
        friend std::ostream& operator<<(std::ostream&, const Exception&);
        virtual void print(std::ostream&) const;
        const MemoryMap *map;           /**< Map that caused the exception if the map is available (null otherwise). */
    };

    /** Exception for an inconsistent mapping. The @p a and @p b are the map elements that are in conflict. For an insert()
     *  operation, the @p a is the element being inserted and @p b is the existing element that's in conflict. Note that the
     *  map may have already been partly modified before the exception is thrown [FIXME: RPM 2009-08-20]. */
    struct Inconsistent : public Exception {
        Inconsistent(const MemoryMap *map, const MapElement &a, const MapElement &b)
            : Exception(map), a(a), b(b) {}
        virtual ~Inconsistent() {}
        friend std::ostream& operator<<(std::ostream&, const Inconsistent&);
        virtual void print(std::ostream&) const;
        MapElement a, b;
    };

    /** Exception for when we try to access a virtual address that isn't mapped. */
    struct NotMapped : public Exception {
        NotMapped(const MemoryMap *map, rose_addr_t va)
            : Exception(map), va(va) {}
        virtual ~NotMapped() {}
        friend std::ostream& operator<<(std::ostream&, const NotMapped&);
        virtual void print(std::ostream&) const;
        rose_addr_t va;
    };

    /** Exception thrown by find_free() when there's not enough free space left. */
    struct NoFreeSpace : public Exception {
        NoFreeSpace(const MemoryMap *map, size_t size)
            : Exception(map), size(size) {}
        virtual ~NoFreeSpace() {}
        friend std::ostream& operator<<(std::ostream&, const NoFreeSpace&);
        virtual void print(std::ostream&) const;
        size_t size;
    };

    /** Exception thrown by load() when there's a syntax error in the index file. */
    struct Syntax: public Exception {
        Syntax(const MemoryMap *map, const std::string &mesg, const std::string &filename, unsigned linenum, int colnum=-1)
            : Exception(map), mesg(mesg), filename(filename), linenum(linenum), colnum(colnum) {}
        virtual ~Syntax() {}
        friend std::ostream& operator<<(std::ostream&, const Syntax&);
        virtual void print(std::ostream&) const;
        std::string mesg;                       /**< Error message. */
        std::string filename;                   /**< Name of index file where error occurred. */
        unsigned linenum;                       /**< Line number (1 origin) where error occurred. */
        int colnum;                             /**< Optional column number (0-origin; negative if unknown). */
    };

    MemoryMap() : sorted(false) {}

    /** Clear the entire memory map by erasing all addresses that are defined. Erasing an address range frees reference
     *  counted anonymous mappings but not user-provided buffers. */
    void clear();

    /** Insert the specified map element. Adjacent elements are coalesced when possible (see MapElement::merge()). */
    void insert(MapElement elmt);

    /** Erase parts of the mapping that correspond to the specified virtual address range. The addresses to be erased don't
     *  necessarily need to correspond to a similar add() call; for instance, it's possible to add a large address space and
     *  then erase parts of it to make holes. */
    void erase(MapElement elmt);

    /** Search for the specified virtual address and return the map element that contains it. Returns null if the
     *  address is not mapped. */
    const MapElement* find(rose_addr_t va) const;

    /** Search for free space in the mapping.  This is done by looking for the lowest possible address not less than @p
     *  start_va and with the specified alignment where there are at least @p size free bytes. Throws a MemoryMap::NoFreeSpace
     *  exception if the search fails to find free space. */
    rose_addr_t find_free(rose_addr_t start_va, size_t size, rose_addr_t mem_alignment=1) const;

    /** Finds the highest area of unmapped addresses.  The return value is the starting address of the highest contiguous
     *  region of unmapped address space that starts at or below the specified maximum.  If no unmapped region exists then a
     *  MemoryMap::NoFreeSpace exception is thrown. */
    rose_addr_t find_last_free(rose_addr_t max=(rose_addr_t)(-1)) const;

    /** Returns the currently defined map elements sorted by virtual address. */
    const std::vector<MapElement> &get_elements() const;

    /** Prunes the map elements by removing those for which @p predicate returns true. */
    void prune(bool(*predicate)(const MapElement&));

    /** Copies data from a contiguous region of the virtual address space into a user supplied buffer. The portion of the
     *  virtual address space to copy begins at @p start_va and continues for @p desired bytes. The data is copied into the
     *  beginning of the @p dst_buf buffer. The return value is the number of bytes that were copied, which might be fewer
     *  than the number of  bytes desired if the mapping does not include part of the address space requested or part of the
     *  address space does not have MM_PROT_READ permission (or the specified permissions). The @p dst_buf bytes that do not
     *  correpond to mapped virtual addresses will be zero filled so that @p desired bytes are always initialized. */
    size_t read(void *dst_buf, rose_addr_t start_va, size_t desired, unsigned req_perms=MM_PROT_READ) const;

    /** Copies data from a supplied buffer into the specified virtual addresses.  If part of the destination address space is
     *  not mapped, then all bytes up to that location are copied and no additional bytes are copied.  The write is also
     *  aborted early if a map element is marked read-only or if its protection lacks the MM_PROT_WRITE bit (or specified
     *  bits).  The return value is the number of bytes copied. */
    size_t write(const void *src_buf, rose_addr_t start_va, size_t size, unsigned req_perms=MM_PROT_WRITE) const;

    /** Returns just the virtual address extents for a memory map. */
    ExtentMap va_extents() const;

    /** Sets protection bits for the specified address range.  The entire address range must already be mapped, but if @p
     *  relax is set then no exception is thrown if part of the range is not mapped (that part is just ignored). */
    void mprotect(const MapElement &elmt, bool relax=false);

    /** Prints the contents of the map for debugging. The @p prefix string is added to the beginning of every line of output
     *  and typically is used to indent the output. */
    void dump(FILE*, const char *prefix="") const;

    /** Dumps the entire map and its contents into a set of files.  The file names are constructed from the @p basename by
     *  appending a hypen and a hexadecimal address (without the leading "0x") and the extension ".data".  The text file whose
     *  name is constructed by appending ".index" to the @p basename contains an index of the memory map. */
    void dump(const std::string &basename) const;

    /** Read a memory map from a set of memory dump files. The argument should be the same basename that was given to an
     *  invocation of the dump() method. The memory map is adjusted according to the contents of the index file. Returns true
     *  if the data was successfully read in its entirety; note that when returning false, this memory map object might
     *  be partially changed (although still in a consistent state).
     *  
     *  This method also understands a more user-friendly dump index format. Each line of the index is either blank
     *  (containing only white space), a comment (introduced with a '#') or a map element specification.  A map element
     *  specification contains the following fields separated by white space (and/or a comma):
     * 
     *  <ul>
     *    <li>The virtual address for the start of this memory area.</li>
     *    <li>The size of this memory area in bytes.</li>
     *    <li>Mapping permissions consisting of the letters "r" (read), "w" (write), or "x" (execute).
     *        Hyphens also be present in this field and do not affect the permissions.</li>
     *    <li>The source of the data. This is the word "anonymous", or the word "base" followed by an ignored name,
     *        or the name of a file.  An "anonymous" mapping indicates that a zero-filled buffer should be allocated
     *        for the data; an "at" mapping generates a file name by appending "-XXXXXXXX.data" to the base name of the
     *        index file where "XXXXXXXX" is the hexadecimal virtual starting address. If a file name is supplied and
     *        it is a relative name, it will be interpreted relative to the index file.</li>
     *    <li>The byte offset of the start of data within the file.  This is ignored for anonymous mappings and is
     *        often zero for files.  It allows a single file to contain multiple memory areas.</li>
     *    <li>An optional comment which will appear as the map element name for debugging.</li>
     *  </ul>
     *
     *  If an error occurs an exception is thrown. */
    bool load(const std::string &basename);

private:
    /* Mutable because some constant methods might sort the elements. */
    mutable bool sorted;                        /**< True if the 'elements' are sorted by virtual address. */
    mutable std::vector<MapElement> elements;   /**< Map elements are only lazily sorted; see 'sorted' data member. */
};

/** Map elements are sorted by virtual address. */
inline bool operator<(const MemoryMap::MapElement &a, const MemoryMap::MapElement &b) {
    return a.get_va() < b.get_va();
}

#endif


