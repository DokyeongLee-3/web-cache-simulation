#include <unordered_map>
#include "gdsf_evict_big.h"



bool GD::lookup(SimpleRequest* req){

	CacheObject obj(req);
	auto iter_ = object_iter_map.find(obj);
	if(iter_ != object_iter_map.end()){	//hit
		LOG("h", 0, obj.id, obj.size); 
			// 원래 hit일때 GD에서는 원래 H value로 restore하는데 그렇게 하지 않고 hit된 file이나 새로 caching된 file들의 priority에 monotonic하게 증가하는 clock을 priority에 더해준다면 recency를 부여할 수 있음 

		long double new_h_val = new_H(req);
		auto iter2 = iter_->second;
		priority_object_map.erase(iter2);
		std::pair<long double, CacheObject> to_insert(new_h_val, obj);
		auto iter_ = priority_object_map.emplace(to_insert);
		assert(object_iter_map.find(obj) != object_iter_map.end()); // object_iter_map에 obj가 당연히 있어야 한다(for debugging)
		object_iter_map[obj] = iter_;
		return true;	
	}

	else{	//miss
		return false;
	}
}


void GD::admit(SimpleRequest* req){
	//std::cerr<<"admit in GD"<<std::endl;
	CacheObject new_obj(req);
	if(_cacheSize < req->getSize()){
		LOG("Can't admit", _cacheSize, req->getId(), req->getSize()); 
		return;
	}

	while(_currentSize + req->getSize() > _cacheSize){
		evict();
	}
		LOG("a", new_H(req), new_obj.id, new_obj.size);
		std::pair<long double, CacheObject> to_insert(new_H(req), new_obj);
		auto iter = priority_object_map.emplace(to_insert);
		std::pair<CacheObject, priority_object_map_iter> to_insert2(new_obj, iter);
		object_iter_map.insert(to_insert2);
		_currentSize = _currentSize + req->getSize();

}

void GD::evict(){ 
	if(priority_object_map.size() > 0){
		priority_object_map_iter min_priority_iter = priority_object_map.begin();
		long double min_priority = min_priority_iter->first;
		priority_object_map_iter second_priority_iter = std::next(min_priority_iter);
		long double second_priority = second_priority_iter->first;
		if(min_priority == second_priority && (min_priority_iter->second).size < (second_priority_iter->second).size){
			//std::cerr<<"evict big one first"<<std::endl;
			min_priority_iter = second_priority_iter;

		}

		update_clock(min_priority);
		CacheObject delobj = min_priority_iter->second; //지금 지우는 file의 priority로 clock을 update -> clock은 monotonic하게 increase, 나중에 cache에admit되는file일수록 높은 clock을 가짐 -> recency
		LOG("e", min_priority_iter->first, delobj.id, delobj.size);
		_currentSize = _currentSize - delobj.size;
		priority_object_map.erase(min_priority_iter);
		
		object_iter_map.erase(min_priority_iter->second);

	}
	else{
		std::cerr<<"no file to evict"<<std::endl;
	}
}

void GD::evict(SimpleRequest* req){ //Do not use

}

void GD::update_clock(long double new_clock){
	clock = new_clock;
}

long double GD::new_H(SimpleRequest* req){
	//std::cerr<<"new H in GD"<<std::endl;
	return clock + 1.0; // GD는 H = L + frequency*(cost/size)에서 frequency, size를 고려하지 않고, 나는 cost를 1로 두는 방법 고수해서 H = L(clock) + 1이 되어서 이런 식이 나옴->cost는 file별로 당연히 다를텐데 이걸  file별로 정확히 수치화할 수 있으면 더 나은 performance를 보일 것
}



long double GDS::new_H(SimpleRequest* req){
	return clock + 1/(double)(req->getSize());
}

bool GDSF_1::lookup(SimpleRequest* req){
	CacheObject obj(req);
	auto it = object_iter_map.find(obj);
	if(it != object_iter_map.end()){ 
		frequency_count_map[obj]++;
		LOG("h", 0, obj.id, obj.size);
		CacheObject cacheObj = it->first;
		priority_object_map_iter iter_ = it->second;
		priority_object_map.erase(iter_);
		long double new_h = new_H(req);
		it->second = priority_object_map.emplace(new_h, cacheObj);
		return true;
	}
	else{
		frequency_count_map[obj] = 1;
		return false;
	}
}

bool GDSF_packet::lookup(SimpleRequest* req){
        CacheObject obj(req);
        auto it = object_iter_map.find(obj);
        if(it != object_iter_map.end()){
                frequency_count_map[obj]++;
                LOG("h", 0, obj.id, obj.size);
                CacheObject cacheObj = it->first;
                priority_object_map_iter iter_ = it->second;
                priority_object_map.erase(iter_);
                long double new_h = new_H(req);
                it->second = priority_object_map.emplace(new_h, cacheObj);
                return true;
        }
        else{
                frequency_count_map[obj] = 1;
                return false;
        }
}


long double GDSF_1::new_H(SimpleRequest* req){
	CacheObject obj(req);
	return clock + (double)frequency_count_map[obj]/(obj.size);
}


long double GDSF_packet::new_H(SimpleRequest* req){
	CacheObject obj(req);
	return clock + ((double)frequency_count_map[obj]*(2+(obj.size/536)))/(obj.size);
}

void WGDSF::admit(SimpleRequest* req){
	//std::cerr<<"admit in WGDSF"<<std::endl;
	CacheObject new_obj(req);
	if(_cacheSize < req->getSize()){
		LOG("Can't admit", _cacheSize, req->getId(), req->getSize());
		return;
	}
	while(_currentSize + req->getSize() > _cacheSize){
		evict();
	}
	long long t = this->time;

	//t_stamp[t][new_obj] = 1;
	t_stamp_[new_obj][t] = 1;

	std::pair<CacheObject, long int> to_insert_frequency_count_map(new_obj, 1);
	frequency_count_map.insert(to_insert_frequency_count_map);
	long double new_hval = new_H(req);
	LOG("a", new_hval, new_obj.id, new_obj.size);
	std::pair<long double, CacheObject> to_insert(new_hval, new_obj);
	auto iter = priority_object_map.emplace(to_insert);
		

	std::pair<CacheObject, priority_object_map_iter> to_insert2(new_obj, iter);
	object_iter_map.insert(to_insert2);
	_currentSize = _currentSize + req->getSize();

}


long double WGDSF::new_H(SimpleRequest* req){
	//std::cerr<<"new H in WGDSF"<<std::endl;
	CacheObject obj(req);
	long double WTF = 0;
	long long t = this->time;
	long int before = t+1;

	
	/*	
	long int count = 0;
	for(long int i = t; i >= 0; i--){
		if((count < frequency_count_map[obj]) && (t_stamp[i][obj] != 0)){
			//std::cerr<<"this time WTF is "<<(double)(t_stamp[i])[obj]/(before-i)<<std::endl;
			WTF += (double)t_stamp[i][obj]/(before-i);
			before = i;
			count++;
		}
		if(count ==  frequency_count_map[obj])
			break;
	}
	*/

	long int now = t;
	for(t_frequency::reverse_iterator riter = t_stamp_[obj].rbegin(); riter != t_stamp_[obj].rend(); ){
		WTF += (double)(riter->second)/(before-now);
		before = (riter->first);
		riter++;
		if(riter == t_stamp_[obj].rend())
			break;
		now = (riter->first);
	}
	

	long double h_val = clock + (WTF/obj.size);
	return h_val;
}

bool WGDSF::lookup(SimpleRequest* req){
	//std::cerr<<"lookup in WGDSF"<<std::endl;
	CacheObject obj(req);
	long long t = this->time;

	//auto iter = t_stamp[t].find(obj);
	auto iter = t_stamp_[obj].find(t);

	auto it = object_iter_map.find(obj);
	if(it != object_iter_map.end()){
		frequency_count_map[obj]++;
		/*
		if(iter != t_stamp[t].end())
			t_stamp[t][obj]++;
		else
			t_stamp[t][obj]=1;
		*/
		
		if(iter != t_stamp_[obj].end())
			t_stamp_[obj][t]++;
		else
			t_stamp_[obj][t] = 1;


		LOG("h", 0, obj.id, obj.size);
		CacheObject cacheObj = it->first;
		priority_object_map_iter iter_ = it->second;
		priority_object_map.erase(iter_);
		long double new_h = new_H(req);
		it->second = priority_object_map.emplace(new_h, cacheObj);
		return true;
	}
	else{	// 없으면 admit에서 frequency count =1, timestamp에 지금 t에 frequency = 1 할당
		//for(int i = 0 ; i<t; i++)
		//	t_stamp[i].erase(obj);
	
		return false;
	}

}

FilterWGDSF::FilterWGDSF()
    : WGDSF(),
      _nParam(2)
{
}

void FilterWGDSF::setPar(std::string parName, std::string parValue){
    if(parName.compare("n") == 0) {
        const uint64_t n = std::stoull(parValue);
        assert(n>0);
        _nParam = n;
    } else {
        std::cerr << "unrecognized parameter: " << parName << std::endl;
    }
}

bool FilterWGDSF::lookup(SimpleRequest* req){
    CacheObject obj(req);
    _filter[obj]++;
    return WGDSF::lookup(req);
}

void FilterWGDSF::admit(SimpleRequest* req){
    CacheObject obj(req);
    if (_filter[obj] <= _nParam) {
        return;
    }
    WGDSF::admit(req);
}

