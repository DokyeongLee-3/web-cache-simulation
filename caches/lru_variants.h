#ifndef LRU_VARIANTS_H
#define LRU_VARIANTS_H

#include <unordered_map>
#include <list>
#include <algorithm>
#include <queue>
#include <vector>
#include <random>
#include "cache.h"
#include "cache_object.h"



typedef std::list<CacheObject>::iterator ListIteratorType;
typedef std::unordered_map<CacheObject, ListIteratorType> lruCacheMapType;
typedef std::pair<long int,CacheObject> RRPVobj;


/*
  LRU: Least Recently Used eviction
*/
class LRUCache : public Cache
{
protected:
    // list for recency order
    // std::list is a container, usually, implemented as a doubly-linked list 
    std::list<CacheObject> _cacheList;
    // map to find objects in list
    lruCacheMapType _cacheMap;

    virtual void hit(lruCacheMapType::const_iterator it, uint64_t size);

public:
    LRUCache()
        : Cache()
    {
    }
    virtual ~LRUCache()
    {
    }

    virtual bool lookup(SimpleRequest* req);
    virtual void admit(SimpleRequest* req);
    virtual void evict(SimpleRequest* req);
    virtual void evict();
    virtual SimpleRequest* evict_return();
};

static Factory<LRUCache> factoryLRU("LRU");

/*
  FIFO: First-In First-Out eviction
*/
class FIFOCache : public LRUCache
{
protected:
    virtual void hit(lruCacheMapType::const_iterator it, uint64_t size);

public:
    FIFOCache()
        : LRUCache()
    {
    }
    virtual ~FIFOCache()
    {
    }
};

static Factory<FIFOCache> factoryFIFO("FIFO");

/*
  FilterCache (admit only after N requests)
*/
class FilterCache : public LRUCache
{
protected:
    uint64_t _nParam;
    std::unordered_map<CacheObject, uint64_t> _filter;

public:
    FilterCache();
    virtual ~FilterCache()
    {
    }

    virtual void setPar(std::string parName, std::string parValue);
    virtual bool lookup(SimpleRequest* req);
    virtual void admit(SimpleRequest* req);
};

static Factory<FilterCache> factoryFilter("Filter");

/*
  ThLRU: LRU eviction with a size admission threshold
*/
class ThLRUCache : public LRUCache
{
protected:
    uint64_t _sizeThreshold;

public:
    ThLRUCache();
    virtual ~ThLRUCache()
    {
    }

    virtual void setPar(std::string parName, std::string parValue);
    virtual void admit(SimpleRequest* req);
};

static Factory<ThLRUCache> factoryThLRU("ThLRU");


/*
  SRRIP (M-bits)
*/
class SRRIPCache : public LRUCache
{
public:
struct cmp{
  bool operator()(RRPVobj a, RRPVobj b){
    return a.first < b.first;
  }
};
    long int _mParam;
    std::unordered_map<CacheObject, std::pair<long int, bool>> _RRPVstatus;
    std::priority_queue<RRPVobj, std::deque<RRPVobj>, cmp> _RRPVpq;
  long  int RRPV;
   long int hit_RRPV;
  long  int old_mParam;

   
public:
    SRRIPCache() ;
    virtual ~SRRIPCache()
    {
    }

    virtual void setPar(std::string parName, std::string parValue);
    virtual bool lookup(SimpleRequest* req);
    virtual void admit(SimpleRequest* req);
     virtual void evict();
    virtual SimpleRequest* evict_return();
//   virtual void evict(SimpleRequest* req);
};

static Factory<SRRIPCache> factorySRRIP("SRRIP");


/*
  BRRIP (M-bits)
*/
class BRRIPCache : public SRRIPCache
{
public:
    BRRIPCache() : SRRIPCache() {};
    virtual ~BRRIPCache()
    {
    }

    virtual void admit(SimpleRequest* req);
};

static Factory<BRRIPCache> factoryBRRIP("BRRIP");

#endif