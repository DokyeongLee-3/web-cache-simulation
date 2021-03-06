#ifndef __GDSF_H__
#define __GDSF_H__

#include <unordered_map>
#include <limits>
#include <cmath>
#include <cassert>
#include <cmath>
#include "cache.h"
#include "cache_object.h"

typedef std::multimap<long double, CacheObject> priority_object_map_type;	//multimap은 그냥 map과 다르게 key값이 중복가능
typedef priority_object_map_type::iterator priority_object_map_iter;
typedef std::unordered_map<CacheObject, priority_object_map_iter> object_iter_type;
typedef std::unordered_map<CacheObject, long int> frequency_count_type;
typedef std::unordered_map<CacheObject, int> timestamp;



class GD : public Cache{

protected:
	long double clock;
	priority_object_map_type priority_object_map; // <object, key> map
	object_iter_type object_iter_map; // <object, priority_object_map_iter> map
	virtual long double new_H(SimpleRequest* req); // GD, GDS, GDSF 모두 달라야 하는 function

public:

	GD(): Cache(){
		clock = 0;
	}
	virtual ~GD(){
	}

	virtual bool lookup(SimpleRequest* req); // return true if hits false if miss
        virtual void admit(SimpleRequest* req);
	virtual void evict(SimpleRequest* req);
        virtual void evict();
	virtual void update_clock(long double new_clock);
};

static Factory<GD> factoryGD("GD");





class GDS : public GD{

protected:
	virtual long double new_H(SimpleRequest* req);	

public:
	GDS(): GD(){
	}
	virtual ~GDS(){
	}
};
static Factory<GDS> factoryGDS("GDS");


class GDSF : public GD{

protected:
	frequency_count_type frequency_count_map;

public:
	GDSF():GD(){

	}
	virtual ~GDSF(){

	}
	virtual bool lookup(SimpleRequest* req);


};
static Factory<GDSF> factoryGDSF("GDSF");




class GDSF_1 : public GDSF{

protected:
	frequency_count_type frequency_count_map;
	virtual long double new_H(SimpleRequest* req);

public:
	GDSF_1():GDSF(){
		
	}
	virtual ~GDSF_1(){

	}
};
static Factory<GDSF_1> factoryGDSF_1("GDSF_1");




class GDSF_packet: public GDSF{

protected:
	frequency_count_type frequency_count_map;
	virtual long double new_H(SimpleRequest* req);
	
public:
	GDSF_packet():GDSF(){
	}
	virtual ~GDSF_packet(){

	}
};
static Factory<GDSF_packet> factoryGDSF_packet("GDSF_packet");




/*
class WGDSF: public GDSF{

protected:
	virtual long double new_H(SimpleRequest* req);

public:
	WGDSF(): GDSF(){
	
	}
	virtual ~WGDSF(){

	}
	virtual bool lookup(SimpleRequest* req);
	
};

static Factory<WGDSF> factoryWGDSF("WGDSF");
*/


#endif
