// cc.in28
// smbase/arraymap.h, with one small tweak to make it self-contained


// map: int -> T
template <class T>
class ArrayMap {
private:     // data
  T **map;               // array[0,nextId-1] of owner ptr
  int nextId;            // next id to assign
  int mapSize;           // allocated size of 'map'

private:     // funcs
  void make();
  void del();
  void validate(int index) const;

public:      // data
  ArrayMap() { make(); }
  ~ArrayMap() { del(); }

  // # of elements defined
  int count() const { return nextId; }

  // insert a new element and yield its assigned id
  int insert(T * /*owner*/ t);

  // retrieve by id
  T const *lookupC(int id) const;
  T *lookup(int id) { return const_cast<T*>(lookupC(id)); }
  T *&lookupRef(int id) { validate(id); return map[id]; }

  // throw everything away
  void empty() { del(); make(); }
};

template <class T>
void ArrayMap<T>::make()
{
  mapSize = 100;
  nextId = 0;
  map = new T* [mapSize];
}

template <class T>
void ArrayMap<T>::del()
{
  for (int i=0; i<nextId; i++) {
    delete map[i];
  }
  delete[] map;
}

template <class T>
int ArrayMap<T>::insert(T *t)
{
  if (nextId == mapSize) {
    // make it bigger
    int newMapSize = mapSize * 2;
    T **newMap = new T* [newMapSize];

    // copy the old contents to the new map
    for (int i=0; i<mapSize; i++) {
      newMap[i] = map[i];
    }
    mapSize = newMapSize;

    // blow away the old map
    delete[] map;

    // grab the new map
    map = newMap;
  }

  int ret = nextId++;
  map[ret] = t;
  return ret;
}

void xassert(int c);

template <class T>
void ArrayMap<T>::validate(int id) const
{
  xassert(0 <= id && id < nextId);
}

template <class T>
T const *ArrayMap<T>::lookupC(int id) const
{
  validate(id);
  return map[id];
}
