#include <fstream>
#include <string>
#include <iomanip>
#include <regex>
#include "caches/lru_variants.h"
#include "request.h"


using namespace std;

int main (int argc, char* argv[])
{

  // output help if insufficient params
  if(argc < 4) {
    cerr << "webcachesim traceFile cacheType cacheSizeBytes [cacheParams]" << endl;
    return 1;
  }

  // trace properties
  const char* path = argv[1];

  // create cache
  const string cacheType = argv[2];
  unique_ptr<Cache> webcache = Cache::create_unique(cacheType);
  if(webcache == nullptr)
    return 1;

  // configure cache size
  const uint64_t cache_size  = std::stoull(argv[3]);
  webcache->setSize(cache_size);

  // parse cache parameters
  regex opexp ("(.*)=(.*)");
  cmatch opmatch;
  string paramSummary;
  for(int i=4; i<argc; i++) {
    regex_match (argv[i],opmatch,opexp);
    if(opmatch.size()!=3) {
      cerr << "each cacheParam needs to be in form name=value" << endl;
      return 1;
    }
    webcache->setPar(opmatch[1], opmatch[2]);
    paramSummary += opmatch[2];
  }

  ifstream infile;
  long long reqs = 0, hits = 0, bytes = 0, bytes_hit = 0;
  long long t, id, size;

  cerr << "running..." << endl;

  infile.open(path);
  SimpleRequest* req = new SimpleRequest(0, 0);
  while (infile >> t >> id >> size)
    {
		webcache->time = t;//add

        reqs++;
		bytes+=req->getSize();
        
        req->reinit(id,size);
        if(webcache->lookup(req)) {
            hits++;
			bytes_hit+=req->getSize();
        } else {
            webcache->admit(req);
        }
    }

  delete req;

  infile.close();


  cout.setf(ios::right);
/*
  cout << setw(10) << "cacheType" << setw(15) <<  "cache_size" << setw(15) << "paramSummary"
	  << setw(10) << "reqs" << setw(10) <<  "hits" << setw(15) << "hits/reqs" << setw(15) << "byte hits/bytes" << endl;

  cout << setw(10) << cacheType << setw(15) << cache_size << setw(15) << paramSummary << setw(10)
       << reqs << setw(10) << hits << setw(15)
       << double(hits)/reqs << setw(15) << double(bytes_hit)/bytes << endl;
*/
  printf("cacheType,cache_size,paramSummary,reqs,hits,hits/reqs,byte_hits/bytes\n");
  cout<<cacheType<<","
	  <<cache_size<<","
	  <<paramSummary<<","
	  <<reqs<<","
	  <<hits<<","
	  <<double(hits)/reqs<<","
	  <<double(bytes_hit)/bytes<<endl;
  return 0;
}
