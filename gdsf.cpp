#include "gdsf.h"
#include <unordered_map>




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
		//frequency_count_map.insert(new_obj, 1);
		//////Map<CacheObject, priority_object ma 에서의 iterator> 에도 삽입
		std::pair<CacheObject, priority_object_map_iter> to_insert2(new_obj, iter);
		object_iter_map.insert(to_insert2);
		////////////////////////////////////////////////////
		_currentSize = _currentSize + req->getSize();

}

void GD::evict(){ // priority가 같은 object에 대해서는 size가 큰걸 evict하는게 이득? 작은게 이득?
	if(priority_object_map.size() > 0){
		priority_object_map_iter min_priority_iter = priority_object_map.begin();
		long double min_priority = min_priority_iter->first;
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
	return clock + 1.0; // GD는 H = L + frequency*(cost/size)에서 frequency, size를 고려하지 않고, 나는 cost를 1로 두는 방법 고수해서 H = L(clock) + 1이 되어서 이런 식이 나옴->cost는 file별로 당연히 다를텐데 이걸  file별로 정확히 수치화할 수 있으면 더 나은 performance를 보일 것
}



long double GDS::new_H(SimpleRequest* req){
	//std::cerr<<"new_H in the GDS"<<std::endl;
	return clock + 1/(double)(req->getSize());
}

bool GDSF::lookup(SimpleRequest* req){
	CacheObject obj(req);
	if(GD::lookup(req)){
		frequency_count_map[obj]++;
		return true;
	}
	else{
		frequency_count_map[obj] = 1;
		return false;
	}
}

long double GDSF::new_H(SimpleRequest* req){
	//std::cerr<<"new_H in the GDSF"<<std::endl;
	CacheObject obj(req);
	return clock + frequency_count_map[req]/(req->getSize());
}



