#include <fstream>
#include <string>
#include <iomanip>
#include <regex>
#include "caches/lru_variants.h"
#include "caches/cache_object.h"
#include "request.h"

using namespace std;

int main(int argc, char *argv[])
{

  // output help if insufficient params
  if (argc < 4)
  {
    cerr << "webcachesim traceFile cacheType cacheSizeBytes [cacheParams]" << endl;
    return 1;
  }

  // trace properties
  const char *path = argv[1];

  // create cache
  const string cacheType = argv[2];
  unique_ptr<Cache> webcache = Cache::create_unique(cacheType);
  if (webcache == nullptr)
    return 1;

  // configure cache size
  const uint64_t cache_size = std::stoull(argv[3]);
  webcache->setSize(cache_size);

  // parse cache parameters
  regex opexp("(.*)=(.*)");
  cmatch opmatch;
  string paramSummary;
  for (int i = 4; i < argc; i++)
  {
    regex_match(argv[i], opmatch, opexp);
    if (opmatch.size() != 3)
    {
      cerr << "each cacheParam needs to be in form name=value" << endl;
      return 1;
    }
    webcache->setPar(opmatch[1], opmatch[2]);
    paramSummary += opmatch[2];
  }

  ifstream infile;
  ofstream outfile;
  long long reqs = 0, hits = 0;
  long long t, id, size;
  long long req_size = 0;
  long long hit_size = 0;
  long long backend_traffic = 0;
  bool hit = true;
  double time =0;
  long double total_transfer_time =0;

  cerr << "running..." << endl;
  
  infile.open(path);
  outfile.open("request_result.csv");
  outfile<<"id,Size,Hit/Miss,Time(ms)\n";
  SimpleRequest *req = new SimpleRequest(0, 0);
  while (infile >> t >> id >> size)
  {
    //std::cerr<<"t is "<<t<<std::endl;
    //std::cerr<<"id is "<<id<<std::endl;
    //std::cerr<<"size is "<<size<<std::endl;
    reqs++;
    
    req->reinit(id, size);
    CacheObject obj(req);
    req_size += req->_size;
    if (webcache->lookup(req))
    {
      hit = true;
      hits++;
      hit_size += obj.size;
      time = (obj.size/12500.0)*2;
    }
    else
    {
      hit = false;
      webcache->admit(req);
      backend_traffic+=req->_size;
      time = (obj.size/12500.0)*2;
      time += (obj.size/3000.0)*2;
    }

    if(hit){
      //std::cerr<<obj.id<<"\t"<<obj.size<<"\t"<<"Hit"<<"\t"<<time<<"\n";
      outfile<<obj.id<<","<< obj.size<< ",Hit,"<< time<<"\n";
      total_transfer_time+=time;
    }else{
      //std::cerr<<obj.id<<"\t"<<obj.size<<"\t"<<"Miss"<<"\t"<<time<<"\n";
      outfile<<obj.id<<","<< obj.size<< ",Miss,"<< time<<"\n";
      total_transfer_time+=time;
    }
  }

  delete req;

  infile.close();
  outfile.close();

  cout.setf(ios::right);

  cout << setw(10) << "cacheType" << setw(15) << "cache_size" << setw(15) << "paramSummary"
       << setw(10) << "reqs" << setw(10) << "hits" << setw(15) << "hits/reqs" << setw(26) << "hit-bytes/req-bytes" << setw(25) << "totalRequestTraffic" << setw(20) << "backendTraffic" << setw(25) << "totalTransferTime(ms)" << endl;

  cout << setw(10) << cacheType << setw(15) << cache_size << setw(15) << paramSummary << setw(10)
       << reqs << setw(10) << hits << setw(15)
       << double(hits) / reqs << setw(26) << double(hit_size) / req_size << setw(25) << req_size << setw(20) << backend_traffic << setw(25) << total_transfer_time << endl;

  return 0;
}
