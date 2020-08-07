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
typedef std::unordered_map<long int, frequency_count_type> timestamp;//timestamp[0]은 t1-t0사이의 object의frequency




class GD : public Cache{

protected:
	long double clock;
	priority_object_map_type priority_object_map; // <object, key> map
	object_iter_type object_iter_map; // <object, priority_object_map_iter> map
	long double new_H(SimpleRequest* req); // GD, GDS, GDSF 모두 달라야 하는 function

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
	long double new_H(SimpleRequest* req);	

public:
	GDS(): GD(){
	}
	virtual ~GDS(){
	}
};
static Factory<GDS> factoryGDS("GDS");

/*
class GDSF : public GDS{

protected:
	frequency_count_type frequency_count_map;

public:
	GDSF():GD(){

	}
	virtual ~GDSF(){

	}
	virtual bool lookup(SimpleRequest* req, long long t);


};
static Factory<GDSF> factoryGDSF("GDSF");
*/



class GDSF_1 : public GD{

protected:
	frequency_count_type frequency_count_map;
	virtual long double new_H(SimpleRequest* req);
	virtual bool lookup(SimpleRequest* req);

public:
	GDSF_1():GD(){
		
	}
	virtual ~GDSF_1(){

	}
};
static Factory<GDSF_1> factoryGDSF_1("GDSF_1");




class GDSF_packet: public GD{

protected:
	frequency_count_type frequency_count_map;
	virtual long double new_H(SimpleRequest* req);
	virtual bool lookup(SimpleRequest* req);
	
public:
	GDSF_packet():GD(){
	}
	virtual ~GDSF_packet(){

	}
};
static Factory<GDSF_packet> factoryGDSF_packet("GDSF_packet");




class WGDSF: public GD{

protected:
	frequency_count_type frequency_count_map;
	virtual long double new_H(SimpleRequest* req);

public:
	WGDSF(): GD(){
	
	}
	virtual ~WGDSF(){

	}
	virtual bool lookup(SimpleRequest* req);
	virtual void admit(SimpleRequest* req);
	//virtual void evict();
	timestamp t_stamp;

	
};

static Factory<WGDSF> factoryWGDSF("WGDSF");


#endif
