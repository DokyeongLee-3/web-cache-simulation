#include <unordered_map>
#include <limits>
#include <cmath>
#include <cassert>
#include <cmath>
#include <cassert>
#include "lru_variants.h"
#include "../random_helper.h"

// golden section search helpers
#define SHFT2(a,b,c) (a)=(b);(b)=(c);
#define SHFT3(a,b,c,d) (a)=(b);(b)=(c);(c)=(d);

// math model below can be directly copiedx
// static inline double oP1(double T, double l, double p) {
static inline double oP1(double T, double l, double p) {
    return (l * p * T * (840.0 + 60.0 * l * T + 20.0 * l*l * T*T + l*l*l * T*T*T));
}

static inline double oP2(double T, double l, double p) {
    return (840.0 + 120.0 * l * (-3.0 + 7.0 * p) * T + 60.0 * l*l * (1.0 + p) * T*T + 4.0 * l*l*l * (-1.0 + 5.0 * p) * T*T*T + l*l*l*l * p * T*T*T*T);
}

/*
  LRU: Least Recently Used eviction
*/
bool LRUCache::lookup(SimpleRequest* req)
{
    // CacheObject: defined in cache_object.h 
    CacheObject obj(req);
    // _cacheMap defined in class LRUCache in lru_variants.h 
    auto it = _cacheMap.find(obj);
    if (it != _cacheMap.end()) {
        // log hit
        LOG("h", 0, obj.id, obj.size);
        hit(it, obj.size);
        return true;
    }
    return false;
}

void LRUCache::admit(SimpleRequest* req)
{
    const uint64_t size = req->getSize();
    // object feasible to store?
    if (size > _cacheSize) {   //_cacheSize??cache??total ?¬ì´××ˆë? ê° ë¦¬í‚¤ê³? ë¶ Òï¨í´?˜ìŠ¤??Cache??×¼¤ë²„ë³ ?˜ë¡œ ?•ì˜?˜ì–´?ˆìŒ
        LOG("L", _cacheSize, req->getId(), size);
        return;
    }
    // check eviction needed
    while (_currentSize + size > _cacheSize) {
        evict();
    }
    // admit new object
    CacheObject obj(req); // object?˜ID? sizeë¥?ì´ˆê¸°?”ì‹œì¼œì„œ ?•ì˜
    _cacheList.push_front(obj); // list???¤ë“œ??object ?½ìž…
    _cacheMap[obj] = _cacheList.begin(); // map??obj ì¶”ê?
    _currentSize += size;
    LOG("a", _currentSize, obj.id, obj.size);
}

void LRUCache::evict(SimpleRequest* req)
{
    CacheObject obj(req);
    auto it = _cacheMap.find(obj);
    if (it != _cacheMap.end()) {
        ListIteratorType lit = it->second;
        LOG("e", _currentSize, obj.id, obj.size);
        _currentSize -= obj.size;
        _cacheMap.erase(obj);
        _cacheList.erase(lit);
    }
}


SimpleRequest* LRUCache::evict_return()
{
    // evict least popular (i.e. last element)
    if (_cacheList.size() > 0) {
        ListIteratorType lit = _cacheList.end();
        lit--;
        CacheObject obj = *lit;
        LOG("e", _currentSize, obj.id, obj.size);
        SimpleRequest* req = new SimpleRequest(obj.id, obj.size);
        _currentSize -= obj.size;
        _cacheMap.erase(obj);
        _cacheList.erase(lit);
        return req;
    }
    return NULL;
}

void LRUCache::evict()
{
    evict_return();
}

// const_iterator: a forward iterator to const value_type, where 
// value_type is pair<const key_type, mapped_type>
void LRUCache::hit(lruCacheMapType::const_iterator it, uint64_t size)
{
    // transfers it->second (i.e., ObjInfo) from _cacheList into 
    //    *this. The transferred it->second is to be inserted before 
    //    the element pointed to by _cacheList.begin()
    //
    // _cacheList is defined in class LRUCache in lru_variants.h 
    _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
}

/*
  FIFO: First-In First-Out eviction
*/
void FIFOCache::hit(lruCacheMapType::const_iterator it, uint64_t size)
{
}

/*
  FilterCache (admit only after N requests)
*/
FilterCache::FilterCache()
    : LRUCache(),
      _nParam(2)
{
}

void FilterCache::setPar(std::string parName, std::string parValue) {
    if(parName.compare("n") == 0) {
        const uint64_t n = std::stoull(parValue);
        assert(n>0);
        _nParam = n;
    } else {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}


bool FilterCache::lookup(SimpleRequest* req)
{
    CacheObject obj(req);
    _filter[obj]++;
    return LRUCache::lookup(req);
}

void FilterCache::admit(SimpleRequest* req)
{
    CacheObject obj(req);
    if (_filter[obj] <= _nParam) {
        return;
    }
    LRUCache::admit(req);
}


/*
  ThLRU: LRU eviction with a size admission threshold
*/
ThLRUCache::ThLRUCache()
    : LRUCache(),
      _sizeThreshold(524288)
{
}

void ThLRUCache::setPar(std::string parName, std::string parValue) {
    if(parName.compare("t") == 0) {
        const double t = stof(parValue);
        assert(t>0);
        _sizeThreshold = pow(2.0,t);
    } else {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}


void ThLRUCache::admit(SimpleRequest* req)
{
    const uint64_t size = req->getSize();
    // admit if size < threshold
    if (size < _sizeThreshold) {
        LRUCache::admit(req);
    }
}

/*
  SRRIP (M-bits)
*/
SRRIPCache::SRRIPCache()
    : LRUCache(),
      _mParam(3)
{
  hit_RRPV = 0;
}

void SRRIPCache::setPar(std::string parName, std::string parValue) {
    if(parName.compare("m") == 0) {
        const uint64_t m = std::stoull(parValue);
        assert(m>0);
        _mParam = pow(2.0,m)-1;
        old_mParam = _mParam;
//        std::cout<<"M is "<<m<<" _mParam is "<<_mParam<<std::endl;
    } else {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}

bool SRRIPCache::lookup(SimpleRequest* req)
{
//   std::cout<<"here?"<<std::endl;
    CacheObject obj(req);
    auto it = _cacheMap.find(obj);
    if (it != _cacheMap.end()) {
        // log hit
        LOG("h", 0, obj.id, obj.size);
        hit(it, obj.size);
          _RRPVstatus[obj].first = hit_RRPV;
          _RRPVstatus[obj].second = false;
//      for(auto lit = _RRPVstatus.begin(); lit != _RRPVstatus.end(); ++lit){
//         std::cout<<"ID: "<<lit->first.id<<"-"<<"RRPV: "<<lit->second<<" ";
//      }
//      std::cout<<std::endl;
        return true;
    }
    return false;
}




void SRRIPCache::admit(SimpleRequest* req)
{
    const uint64_t size = req->getSize();
    // object feasible to store?
    if (size > _cacheSize) {
        LOG("L", _cacheSize, req->getId(), size);
        return;
    }
    // check eviction needed
    while (_currentSize + size > _cacheSize) {
        evict();
    }
    // admit new object
    CacheObject obj(req);
     _RRPVstatus[obj] = std::make_pair(_mParam-1, true);
     _RRPVpq.push(std::make_pair(_mParam-1,obj));
    _cacheList.push_front(obj);
    _cacheMap[obj] = _cacheList.begin();
    _currentSize += size;
    LOG("a", _currentSize, obj.id, obj.size);
    //std::cout<<"pqSIZE: "<<_RRPVpq.size()<<" pqTOP.RRPV: "<<_RRPVpq.top().first<<" pqTOP.size: "<<_RRPVpq.top().second.size<<" pqTOP.id: "<<_RRPVpq.top().second.id<<std::endl;
    
//   for(auto lit = _RRPVstatus.begin(); lit != _RRPVstatus.end(); ++lit){
//      std::cout<<"ID:"<<lit->first.id<<"-"<<"RRPV:"<<lit->second<<" ++ ";
//   }
//   std::cout<<std::endl;
}

void SRRIPCache::evict()
{
  //std::cout<<"Get ready to evict"<<std::endl;
   //value°ªÀÌ 2^M-1 ÀÎkey°ªÀ» Ã£¾Æ¼­ ±× ÇØ´ç ¿ÀºêÁ§Æ®¸¦ evict(req)·Î ³Ñ°ÜÁÖ±â
   SRRIPCache::evict_return();
}

SimpleRequest* SRRIPCache::evict_return()
{


  if(_cacheList.size() > 0) {
    //»©¾ßÇÏ´Â °æ¿ì, ¿À·¡µµ·Ï Á¢±ÙµÇÁö ¾ÊÀºÄ£±¸
    CacheObject obj;
    //std::cout<<"error where"<<std::endl;
    while(1){
      auto check = _RRPVpq.top();
      //std::cout<<"check get?"<<std::endl;
      
      if(_RRPVstatus[check.second].second == true){
        //std::cout<<"In if"<<std::endl;
    
        obj = check.second;
        if(_RRPVstatus[check.second].first != old_mParam){
          old_mParam = _RRPVstatus[check.second].first;
          _mParam--;
          hit_RRPV--;
          
          //std::cout<<" "<<_mParam;
        }
        break;
      }
      else{
      //std::cout<<"In else"<<std::endl;
    
        _RRPVpq.push(std::make_pair(_RRPVstatus[check.second].first, check.second));
        _RRPVstatus[check.second].second = true;
        _RRPVpq.pop();
      }
      
   } 
   LOG("e", _currentSize, obj.id, obj.size);
    SimpleRequest* req = new SimpleRequest(obj.id, obj.size);
    LRUCache::evict(req);
    _RRPVstatus.erase(obj);
    _RRPVpq.pop();
    return req;
  
  }

  return NULL;

}


void BRRIPCache::admit(SimpleRequest* req)
{
//   std::cout<<"BRRIP admit and cache size is "<<_cacheSize<<std::endl;
    const uint64_t size = req->getSize();
    // object feasible to store?
    
    if (size > _cacheSize) {
        LOG("L", _cacheSize, req->getId(), size);
        return;
    }
    // check eviction needed
    while (_currentSize + size > _cacheSize) {
        evict();
    }
    // admit new object
    CacheObject obj(req);
    if(rand()%100==0){
      _RRPVstatus[obj] = std::make_pair(_mParam-1, true);
   }else{
       _RRPVstatus[obj] = std::make_pair(_mParam, true);
    }
     _RRPVpq.push(std::make_pair(_mParam-1,obj));
   _cacheList.push_front(obj);
    _cacheMap[obj] = _cacheList.begin();
    _currentSize += size;
//    std::cout<<"BRRIP ongoing>>>>>>"<<std::endl;
    LOG("a", _currentSize, obj.id, obj.size);
//   for(auto lit = _RRPVstatus.begin(); lit != _RRPVstatus.end(); ++lit){
//      std::cout<<"ID:"<<lit->first.id<<"-"<<"RRPV:"<<lit->second<<" ++ ";
//   }
//   std::cout<<std::endl;
}