#include "hash_table.h"
#include <cstring>
//typedef std::size_t size_t;
extern "C"{
	HASH_TABLE_DECLARE7(htest, size_t, size_t, HASH_TABLE_DEFAULT_HASH, HASH_TABLE_DEFAULT_PRED, malloc, free)
}

#include <cstdlib>
#include <unordered_map>
#include <vector>
#include <iostream>

#define panic(str) \
do{\
	std::cout<<'\n'<<i<<":\t";\
	str;\
	std::cout<<"\n"<<insertCount<<" insert, "<<removeCount<<" remove, "<<searchCount<<" search"<<std::endl;\
	return 0;\
}while(0)
	//std::cout<<"\n"<<insertCount<<" insert, "<<removeCount<<" remove, "<<searchCount<<" search"<<std::endl;
	
const int testCount=1000000;
int main(int argc, char** argv){
	htest* h=new htest;
	ht_init(htest, h);
	/*
	auto h=ht_create(
		[](const void* key)->std::size_t{
			const std::size_t* ptr=static_cast<const std::size_t*>(key);
			return (*ptr);
		},
		[](const void* key1, const void* key2)->int{
			return ((*(static_cast<const std::size_t*>(key1)))==(*(static_cast<const std::size_t*>(key2)))?1:0);
		},
		&malloc,
		&free
	);
	*/
	std::unordered_map<std::size_t, std::size_t> ref;
	//std::vector<std::pair<std::size_t*,std::size_t*>> tmpVec;
	std::size_t insertCount=0;
	std::size_t removeCount=0;
	std::size_t searchCount=0;
	for(int i=0;i<testCount;++i){
		int op=(rand()%5)-2;
		if(op<0) op=((i<testCount/2)?0:1);
		
		if(op==1&&ref.empty()){
			--i;
			continue;
		}
		//std::cout<<"\r"<<insertCount<<" insert, "<<removeCount<<" remove, "<<searchCount<<" search; ";
		//std::cout<<"Test "<<i<<", op "<<op<<"..."<<std::flush;
		switch(op){
			case 0:{
				//insert
				std::size_t key=rand();
				std::size_t value=rand();
				
				
				auto ret=ht_insert(htest, h,key,value);
				if(ret==NULL){
					panic(std::cout<<"insertion failed");
				}
				auto refPair=ref.insert(std::pair<std::size_t,std::size_t>(key,value));
				if((refPair.second!=((*ret)==value))&&(refPair.first->second!=value)){
					panic(std::cout<<"insertion wrong result:"<<refPair.second<<'('<<refPair.first->first<<','<<refPair.first->second<<')'<<((*ret)==value)<<'('<<(*ret)<<')');
				}else if(refPair.first->second!=(*ret)){
					panic(std::cout<<"insertion wrong value");
				}
				++insertCount;
			}break;
			case 1:{
				//remove
				auto ret=ht_erase(htest, h, ref.begin()->first);
				if(!ret){
					panic(std::cout<<"remove failed: "<<(ref.begin()->first));
				}
				ref.erase(ref.begin());
				++removeCount;
			}break;
			case 2:{
				//search
				std::size_t key=rand();
				auto refIter=ref.find(key);
				auto ret=ht_find(htest, h,key);
				if((ret==NULL)!=(refIter==ref.end())){
					panic(std::cout<<"search result wrong"<<(ret==NULL)<<','<<(refIter==ref.end()));
				}else if((refIter!=ref.end())&&((*ret)!=refIter->second)){
					panic(std::cout<<"search value wrong");
				}
				++searchCount;
			}break;
		}
		if(ref.size()!=ht_size(htest, h)){
			panic(std::cout<<"incorrect hash table size:"<<ref.size()<<","<<ht_size(htest,h));
		}
	}
	ht_clear(htest, h);
	delete h;
	std::cout<<"\nTest done!"<<std::endl;
	return 0;
}