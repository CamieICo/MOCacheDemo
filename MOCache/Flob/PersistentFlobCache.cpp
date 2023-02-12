/*
----
This file is part of SECONDO.

Copyright (C) 2009, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

SECONDO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

SECONDO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

----


*/


#include "PersistentFlobCache.h"
#include "Flob.h"
#include "FlobManager.h"
#include <assert.h>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
/*
1 Class CacheEntry

*/

map<pair<int,int> , int> fileRecIdTomp; //fileid + recid----->grid ID
vector<int> gridsMap; // grid ID -------->hot frequency

//size_t virtualSize = 0;
//
//
//class VirLinkList{
//public:
//    int fileId;
//    int RecId;
//    int size;
//    VirLinkList *next;
//    VirLinkList(){}
//    VirLinkList(int fid, int rid, int s):fileId(fid), RecId(rid), size(s), next(nullptr){}
//};
//VirLinkList *head = new VirLinkList(-1,-1,0);
//class Apoint{
//public:
//    double x, y, t;
//    Apoint(){x = 0; y = 0; t = 0;}
//    Apoint(double _x, double _y, double _t):x(_x),y(_y),t(_t){}
//    void print(){
//        cout << x << " " << y << " " << t ;
//    }
//    Apoint& operator=(const Apoint &p){
//        if(this == &p) return *this;
//        this->x = p.x;
//        this->y = p.y;
//        this->t = p.t;
//        return *this;
//    }
//    bool operator<=(const Apoint &p){
//        if(this->x <= p.x && this->y <= p.y && this->t <= p.t){
//            return true;
//        }
//        return false;
//    }
//    bool operator>=(const Apoint &p){
//        if(this->x >= p.x && this->y >= p.y && this->t >= p.t){
//            return true;
//        }
//        return false;
//    }
//    Apoint(const Apoint& p){
//        this->x = p.x;
//        this->y = p.y;
//        this->t = p.t;
//    }
//    friend ostream& operator<<(ostream &os, const Apoint &p){
//        os << fixed << setprecision(6) <<p.x << " " << p.y << " " << p.t;
//        return os;
//    }
//};

class CacheEntry{
  public:

/*
1.1 Constructor

Creates a new entry for Flob __flob__ and slot number __slotNo__
with given slot size.

*/
    CacheEntry(const Flob& _flob, const size_t _slotNo, size_t _slotSize):
      flob(_flob), slotNo(_slotNo), tableNext(0), tablePrev(0),
      lruPrev(0), lruNext(0){
       size_t offset = _slotNo * _slotSize;
       assert(offset < flob.getSize());
       size = std::min(_slotSize, (flob.getSize() - offset));
       //std::cout <<"slotSize = "<<_slotSize<<", flob.getSize()="<<flob.getSize()<<",offset = "<<offset<<",cacheentry size = "<<size<<std::endl;
       mem = (char*) malloc(size);
    }


/*
1.2 Destructor

Detroys an entry of a Flob.

*/

    virtual ~CacheEntry(){
      free(mem);
    }

/*
1.3 hashValue

Returns a hash value for this entry.

*/
    size_t hashValue(size_t tableSize){
      return (flob.hashValue() + slotNo) % tableSize;
    }

/*
1.4 check for equality / inequality


The flob as well as the slot number unique identify 
a cache entry.

*/
    bool operator==(const CacheEntry& e){
      return flob == e.flob &&
             slotNo == e.slotNo;
    }

    bool operator!=(const CacheEntry& e){
      return !(*this == e);
    }

/*
1.5 Members

all members are private because only the PersistentFlobCache class
known the CacheEntry class.

*/

   Flob flob;     
   size_t slotNo;
   CacheEntry* tableNext;
   CacheEntry* tablePrev;
   CacheEntry* lruPrev;
   CacheEntry* lruNext;
   char* mem;
   size_t size;
};


/*
1.6 Output operator

for simple output an entry.

*/

std::ostream& operator<<(std::ostream& o, const CacheEntry& e){
   o << "[" << e.flob << ", " << e.slotNo << "]" ;
  return o;
}


/*
2 PersistentFlobCache

In the cache for persistent flobs, flobs are splitted to a maximum size,
called slotSize. So a flob may consist of several slots. Each of them holds
memory of size __slotSize__ except the last slot for the flob which may be
smaller (depending on the size of the flob). 

The works in lru manner. It's organized using open hashing. The entries
are additionally linked within a double linked list to hold the order for
lru replacement.

Requested data are taken directly from the cache. If the slot containing the
data is not cached, it is   put into the storage. Thus a flob is read from disk 
slotwise.

When the put operation is called (which should be occur rarely on
 persistent flobs),
The slot stored in the cache is updated and also the disk content is 
written instantly.


2.1 Constructor

Creates a new cache of maximum size and given slotsize.

*/

PersistentFlobCache::PersistentFlobCache(size_t _maxSize, size_t _slotSize,
                                         size_t _avgSize):
    maxSize(_maxSize), slotSize(_slotSize), usedSize(0), first(0), last(0) {


   // compute a good tablesize
   assert(maxSize > slotSize);
   assert(_avgSize <= slotSize);   
 
   tableSize = ((maxSize / _avgSize) * 2);
   if(tableSize < 1u){
      tableSize = 1u; 
   }   
   hashtable = new CacheEntry*[tableSize]; 
   for(unsigned int i=0;i<tableSize; i++){
     hashtable[i] = 0;
   }

   //初始化fileRecIdTomp map
    fstream file("secondo/bin/AboutHotData/mpMap.txt");
    if(!file.is_open()){
        cout << "Read mpMap.txt error!" << endl;
        exit(1);
    }
    string oneLine;
    int a,b,c;
    while(getline(file, oneLine)){
        stringstream words(oneLine);
        words >> a >> b >> c;
        fileRecIdTomp[{a,b}] = c;
    }
    file.close();


    //初始化grids map for hot data statistics
    fstream file3("secondo/bin/AboutHotData/gridsMap.txt");
    if(!file3.is_open()){
        cout << "Read gridsMap.txt error!" << endl;
        exit(1);
    }
    string oneLine3;
    int f;
    while(getline(file3, oneLine3)){
        stringstream words3(oneLine3);
        words3 >> f;
        gridsMap.emplace_back(f);
    }
    file3.close();

///////////////output girdsMap;
//    for(auto it = gridsMap.begin(); it != gridsMap.end(); it++){
//        cout << it->first << it->second;
//    }

    //初始化idBBox map
//    fstream file4("secondo/bin/AboutHotData/idBBox.txt");
//    if(!file4.is_open()){
//        cout << "Read idBBox.txt error!" << endl;
//    }
//    string oneLine4;
//    string h,i,j,k,l,m,n,o,p;
//    while(getline(file4, oneLine4)){
//        stringstream word4(oneLine4);
//        word4 >> h >> i >> j>>k>>l>>m>>n>>o>>p;
//        Apoint a1(atof(k.c_str()), atof(l.c_str()), atof(m.c_str()));
//        Apoint a2(atof(n.c_str()), atof(o.c_str()), atof(p.c_str()));
//        idBBox[{atoi(i.c_str()), atoi(j.c_str())}] = {a1,a2};
//    }
//    file4.close();
//
//
//    //record headlist
//    fstream file5("secondo/bin/AboutHotData/headList.txt");
//    if(!file5.is_open()){
//        cout << "Read headList.txt error!" << endl;
//    }
//    string oneLine5;
//    string q,r,s;
//    VirLinkList *vl = head;
//    cout << head->fileId <<" headhead " <<head->RecId<<endl;
//    while(getline(file5, oneLine5)){
//        stringstream word5(oneLine5);
//        word5 >> q >> r >>s;
//        virtualSize+= atoi(s.c_str());
//        VirLinkList *now = new VirLinkList(atoi(q.c_str()), atoi(r.c_str()), atoi(s.c_str()));
//        vl->next = now;
//        vl = vl->next;
//    }
//    file5.close();
}

/*
2.2 Destructor

destroys the cache.

*/


PersistentFlobCache::~PersistentFlobCache(){
   clear();
   delete[] hashtable;
   hashtable = 0;
}


/*
2.3 clear

removes all entries from the cache.

*/
void PersistentFlobCache::clear(){
  // first, kill entries from hashtable without deleting them from lru

  for(unsigned int i=0;i<tableSize;i++){
    hashtable[i] =0;
  }
  CacheEntry* entry = first;
  size_t killed = 0;

  while(entry){
    CacheEntry* victim = entry;
    entry = entry->lruNext;
    killed += victim->size;
    delete victim;
  }


  first = 0;
  last = 0;
  usedSize = 0;
}


/*
2.4 getData

retrieves data from the cache. If the data are not cached, the
FlobManager is used to access the data.

*/
bool PersistentFlobCache::getData(
             const Flob& flob, 
             char* buffer,
             const SmiSize offset, 
             const SmiSize size){

    if(size==0){  // nothing to do
      return true;
    }
    size_t slotNo = offset / slotSize;
    size_t slotOffset = offset % slotSize; 
    size_t bufferOffset(0);

    while(bufferOffset < size){
       // std::cout << "here 8 "<< std::endl;
        //std::cout <<"PersistentFlobCache slotNo = "<<slotNo<<", slotOffset = "<<slotOffset<<std::endl;
      if(!getDataFromSlot(flob, slotNo, slotOffset, 
                          bufferOffset, size, buffer)){
        std::cerr << "Warning getData failed" << std::endl;
        return false;
      }
       
      slotNo++;
      slotOffset = 0; 
    }
    return true;
}

/*
2.5 putData

updates the data in cache and on disk

*/
bool PersistentFlobCache::putData(
      Flob& flob,
      const char* buffer,
      const SmiSize offset,
      const SmiSize size) {

   size_t slotNo = offset / slotSize; 
   size_t slotOffset = offset % slotSize;
   size_t bufferOffset(0);
   while(bufferOffset < size){
     putDataToFlobSlot(flob, slotNo, slotOffset, bufferOffset, size, buffer);
     slotNo++;
     slotOffset = 0;
   }
   return FlobManager::getInstance().putData(flob, buffer, offset, size, true);
}


/*
2.6 killLastSlot

removes the last slot from the cache if exist. Its required if a resize
function is called.


*/

void PersistentFlobCache::killLastSlot(const Flob& flob){

  size_t slotNo = flob.getSize() / slotSize;
  size_t index = (flob.hashValue() + slotNo) % tableSize;

  if(hashtable[index]==0){
    return;
  } 
  CacheEntry* e = hashtable[index];
  while(e){
    if(e->flob!=flob || e->slotNo!=slotNo){
      e = e->tableNext;
    } else { // victim found
      // remove from table


      if(hashtable[index]==e){
        hashtable[index] = e->tableNext;
        if(e->tableNext){
          e->tableNext->tablePrev = 0; 
        }
      } else {
        e->tablePrev->tableNext = e->tableNext;
        if(e->tableNext){
          e->tableNext->tablePrev = e->tablePrev;
        }
      }
      e->tablePrev=0;
      e->tableNext=0;
      // delete from lru
      if(e==first){
         if(e==last){ // last element 
            first = 0;
            last = 0;
         } else {
            first = first->lruNext;
            first->lruPrev = 0;
            e->lruNext = 0;
         }
      } else{ // e!=first
         if(e==last){
           last = last->lruPrev;
           last->lruNext = 0;
           e->lruPrev=0;
         } else { // remove from the middle of lrulist
           e->lruPrev->lruNext = e->lruNext;
           e->lruNext->lruPrev = e->lruPrev;
           e->lruPrev = 0;
           e->lruNext = 0;
         }
      }
      usedSize -= e->size;
      delete e; 
      e = 0; // stop loop
    }
  } 
}

/*
2.7 getDataFromSlot

retrieves the flob data for a specidied flob.

*/

int hitCount = 0;
int missCount = 0;
int page_In = 0;
int page_Out = 0;
bool PersistentFlobCache::getDataFromSlot(const Flob& flob,
                     const size_t slotNo,
                     const size_t slotOffset,
                     size_t& bufferOffset,
                     const size_t size,
                     char* buffer) {
    //std::cout << "FileId= " << flob.getFileId() <<" RecordId = " << flob.getRecordId() << " Offset= "<<flob.getOffset() << std::endl;
   unsigned int index = (flob.hashValue() + slotNo) % tableSize;
   int flbFID, recID, gridID;
   if(hashtable[index]==0){
       missCount++;
     CacheEntry* newEntry = createEntry(flob, slotNo);
     hashtable[index] = newEntry;
     usedSize += newEntry->size; 
     putAtFront(newEntry);
     flbFID = newEntry->flob.getFileId();
     recID = newEntry->flob.getRecordId();
     //cout << flbFID << " " << recID<<endl;
     if(fileRecIdTomp.find({flbFID, recID}) != fileRecIdTomp.end()){
         gridID = fileRecIdTomp[{flbFID, recID}];
         //cout << "flbFID, recID, gridID = " << flbFID << " " << recID <<" "<< gridID << endl;
         gridsMap[gridID]++;
         //cout << "1----------gridsMap[gridID] = " << gridsMap[gridID] << endl;
     }
     reduce(); // remove entries if too much memory is used
     getData(newEntry, slotOffset, bufferOffset, size, buffer);

     //std::cout << "1 time:" << std::endl;
     return true;
   }
   // hashtable[index] is already used
   CacheEntry* entry = hashtable[index];
   while(entry->tableNext && (entry->flob != flob || entry->slotNo!=slotNo)){
     entry = entry->tableNext;
   }

   if(entry->flob != flob || entry->slotNo!=slotNo){ // no hita
        missCount++;
        CacheEntry* newEntry = createEntry(flob, slotNo);
        newEntry->tablePrev = entry;
        entry->tableNext = newEntry;
        assert(first);
        putAtFront(newEntry);
        usedSize += newEntry->size;
        flbFID = newEntry->flob.getFileId();
        recID = newEntry->flob.getRecordId();
        // cout << flbFID << " " << recID<<endl;
        if(fileRecIdTomp.find({flbFID, recID}) != fileRecIdTomp.end()){
        gridID = fileRecIdTomp[{flbFID, recID}];
        //cout << "flbFID, recID, gridID = " << flbFID << " " << recID <<" "<< gridID << endl;
        gridsMap[gridID]++;
        //cout << "2----------gridsMap[gridID] = " << gridsMap[gridID] << endl;
        }
        reduce();
        getData(newEntry, slotOffset, bufferOffset, size, buffer);

        //std::cout << "2 time:" << std::endl;
        return true;
   } else { // hit, does not access disk data
       hitCount++;
       getData(entry, slotOffset, bufferOffset, size, buffer);
       flbFID = entry->flob.getFileId();
       recID = entry->flob.getRecordId();
       //cout << flbFID << " " << recID<<endl;
       if(fileRecIdTomp.find({flbFID, recID}) != fileRecIdTomp.end()){
           gridID = fileRecIdTomp[{flbFID, recID}];
           //cout << "flbFID, recID, gridID = " << flbFID << " " << recID <<" "<< gridID << endl;
           gridsMap[gridID]++;
           //cout << "3----------gridsMap[gridID] = " << gridsMap[gridID] << endl;
       }
       //std::cout << "3 time:" << std::endl;
       bringToFront(entry);
       return true;
   }
}

/*
2.8 getData

Copies data from a given CacheEntry to a buffer.

*/
void PersistentFlobCache::getData(CacheEntry* entry,  
                              size_t slotOffset, 
                              size_t& bufferOffset, 
                              const size_t size,
                              char* buffer){
  size_t mb = size-bufferOffset; // missing bytes
  size_t ab = entry->size - slotOffset; // bytes available in slot
  size_t pb = std::min(mb,ab); // provided bytes
    //目的地址   原地址  共拷贝字节
  memcpy(buffer+bufferOffset, entry->mem + slotOffset, pb);
  bufferOffset += pb;
}

/*
2.9 putData

puts data to a given CacheEntry

*/

void PersistentFlobCache::putData(CacheEntry* entry,  
                              const size_t slotOffset, 
                              size_t& bufferOffset, 
                              const size_t size,
                              const char* buffer){
  size_t mb = size-bufferOffset; // missing bytes
  size_t ab = entry->size - slotOffset; // bytes available in slot
  size_t pb = std::min(mb,ab); // provided bytes
  memcpy(entry->mem + slotOffset, buffer+bufferOffset, pb);
  bufferOffset += pb;
}

/*
2.10 createEntry

produces a new CacheEntry. If readData are set to be __true__ (default), 
the slot data are load from disk using the FlobManager.

*/
CacheEntry* PersistentFlobCache::createEntry(const Flob& flob, 
                                const size_t slotNo,
                                const bool readData/*=true*/) const{
   CacheEntry* res = new CacheEntry(flob, slotNo, slotSize);
   if(readData){
      FlobManager::getInstance().getData(flob,res->mem, 
                                         slotNo*slotSize, res->size,true);
      //std::cout << "execute here449: " << res->size<< std::endl;
   }
   page_In++;
   return res;
}

/*
2.11 reduce

removes slot from the cache until the size is smaller than the maximum one.

*/

//void quickSort(vector<int>& nums, int k, int i, int j){
//    if(i >= j || j <k ) return;
//    int left = i, right = j;
//    int ran = rand()%(j - i + 1) + i;
//    swap(nums[left], nums[ran]);
//    int key = nums[left];
//    while(left < right){
//        while(left < right && nums[right] >= key){
//            right--;
//        }
//        nums[left] = nums[right];
//        while(left < right && nums[left] <= key){
//            left++;
//        }
//        nums[right] = nums[left];
//    }
//    nums[left] = key;
//    quickSort(nums, k, i, left);
//    quickSort(nums, k, left+1, j);
//}
//
//int findKthLargest(vector<int>& nums, int k) {
//    srand((unsigned)time(nullptr));
//    quickSort(nums, nums.size() - k, 0, nums.size()-1);
//    return nums[nums.size() - k];
//}

void AdjustDown(vector<int>&nums, int k, int len){
    int a0 = nums[k];
    for(int i = 2* k + 1; i < len; i = i * 2 + 1){
        if(i < len-1 && nums[i] < nums[i+1]){
            i++;
        }
        if(a0 >= nums[i]) break;
        else{
            nums[k] = nums[i];
            k = i;
        }
    }
    nums[k] = a0;
}
void HeapSort(vector<int>&nums){
    //建立大根堆
    for(int i = nums.size() / 2 - 1; i >= 0;i--){
        AdjustDown(nums, i, nums.size());
    }
    for(int i = nums.size() -1; i>=0; i--){
        swap(nums[i],nums[0]);
        AdjustDown(nums,0, i);
    }
}
int findKthLargest(vector<int> nums, int k){
    int n = nums.size();
    HeapSort(nums);
    return nums[n-k];
}

void PersistentFlobCache::reduce(){
   while(usedSize > maxSize){
       page_Out++;
       std::cout<<"PresistentFlobCache reduce here."<<std::endl;
       //在gridsMap中找到第k热度最大的元素
       int value = findKthLargest(gridsMap, 10);
       //int value = 8;
       //std::cout << "k = 300, cell heat value= " << value << std::endl;
       CacheEntry* victim = last;
       int flbFID = victim->flob.getFileId();
       int recID = victim->flob.getRecordId();
       //cout << flbFID << " " << recID<<endl;
       int gridID = fileRecIdTomp[{flbFID, recID}]; 
       CacheEntry* minVic = victim;
       int minVal = gridsMap[gridID];
       while(gridsMap[gridID] > value){
           if(victim->lruPrev != 0){
               victim = victim->lruPrev;
           }else{
               victim = 0;
               break;
           }
           flbFID = victim->flob.getFileId();
           recID = victim->flob.getRecordId();
           gridID = fileRecIdTomp[{flbFID, recID}];
           if(minVal > gridsMap[gridID]){
               minVal = gridsMap[gridID];
               minVic = victim;
           }
       }
      // remove from lru
      if(victim != 0){
          if(victim == last){
              last = victim->lruPrev;
              last->lruNext = 0;
              victim->lruPrev = 0;
              assert(victim->lruNext == 0);
          }else if(victim == first){
              first = victim->lruNext;
              first->lruPrev = 0;
              victim->lruNext = 0;
          }else{
              victim->lruPrev->lruNext = victim->lruNext;
              victim->lruNext->lruPrev = victim->lruPrev;
              victim->lruPrev = 0;
              victim->lruNext = 0;
          }
          // remove from hashtable

          if(victim->tablePrev){ // not the first entry in table
              if(victim->tableNext){
                  victim->tablePrev->tableNext = victim->tableNext;
                  victim->tableNext->tablePrev = victim->tablePrev;
                  victim->tablePrev = 0;
                  victim->tableNext = 0;
              } else { // last entry in table
                  victim->tablePrev->tableNext = 0;
                  victim->tablePrev = 0;
              }
          } else {  // first entry in table
              size_t index = victim->hashValue(tableSize);
              assert(hashtable[index] == victim);
              if(victim->tableNext==0){ // the only entry in hashtable
                  hashtable[index] = 0;
              } else {
                  hashtable[index] = victim->tableNext;
                  victim->tableNext->tablePrev = 0;
                  victim->tableNext = 0;
              }
          }
          usedSize -= victim->size;
          delete victim;
          minVic = 0;
      }else{
          if(minVic == last){
              last = minVic->lruPrev;
              last->lruNext = 0;
              minVic->lruPrev = 0;
              assert(minVic->lruNext == 0);
          }else if(minVic == first){
              first = minVic->lruNext;
              first->lruPrev = 0;
              minVic->lruNext = 0;
          }else{
              minVic->lruPrev->lruNext = minVic->lruNext;
              minVic->lruNext->lruPrev = minVic->lruPrev;
              minVic->lruPrev = 0;
              minVic->lruNext = 0;
          }


          // remove from hashtable

          if(minVic->tablePrev){ // not the first entry in table
              if(minVic->tableNext){
                  minVic->tablePrev->tableNext = minVic->tableNext;
                  minVic->tableNext->tablePrev = minVic->tablePrev;
                  minVic->tablePrev = 0;
                  minVic->tableNext = 0;
              } else { // last entry in table
                  minVic->tablePrev->tableNext = 0;
                  minVic->tablePrev = 0;
              }
          } else {  // first entry in table
              size_t index = minVic->hashValue(tableSize);
              assert(hashtable[index] == minVic);
              if(minVic->tableNext==0){ // the only entry in hashtable
                  hashtable[index] = 0;
              } else {
                  hashtable[index] = minVic->tableNext;
                  minVic->tableNext->tablePrev = 0;
                  minVic->tableNext = 0;
              }
          }
          usedSize -= minVic->size;
          delete minVic;
      }

   }
}

/*
2.12 putDataToFlobSlot

puts data to a given slot. If the slot is not present in the
cache, it is created.


*/
bool PersistentFlobCache::putDataToFlobSlot(
        const Flob& flob,
        const size_t slotNo,
        const size_t slotOffset,
        size_t& bufferOffset,
        const size_t size,
        const char* buffer){


  size_t index = (flob.hashValue() + slotNo) % tableSize;
  if(hashtable[index] == 0){ // first entry in hashtable
     bool rd = slotOffset==0 && size >=slotSize;
     CacheEntry* newEntry = createEntry(flob, slotNo, rd);
     putData(newEntry, slotOffset, bufferOffset, size, buffer);
     hashtable[index] = newEntry;
     usedSize += newEntry->size;
     putAtFront(newEntry);
     reduce();
     return true;
  }

  CacheEntry* entry = hashtable[index];
  while(entry->tableNext && (entry->flob!=flob || entry->slotNo!=slotNo)){
     entry = entry->tableNext;
  }

  if(entry->flob==flob && entry->slotNo==slotNo){ // hit in cache
     putData(entry, slotOffset, bufferOffset, size,buffer);
     bringToFront(entry);
     return true;
  }

  // append a new entry at the end of the tablelist
  bool rd = slotOffset==0 && size >=slotSize;
  CacheEntry* newEntry = createEntry(flob, slotNo, rd);
  putData(newEntry, slotOffset, bufferOffset, size, buffer);
  entry->tableNext = newEntry;
  entry->tablePrev = entry;
  usedSize += newEntry->size;
  putAtFront(newEntry);
  reduce();
  return true;
}

/*
2.13 putAtFront

puts an unused CacheEntry to the front of the LRU list.

*/
void PersistentFlobCache::putAtFront(CacheEntry* newEntry){
   assert(newEntry->lruPrev==0);
   assert(newEntry->lruNext==0);
   if(!first){
     assert(!last);
     first = newEntry;
     last = newEntry;
   } else {
     newEntry->lruNext = first;
     first->lruPrev = newEntry;
     first = newEntry;
   }
}

/*
2.14 bringToFront

moves an entry already present in the lrulist to the top of that list.

*/
void PersistentFlobCache::bringToFront(CacheEntry* entry){
  if(first==entry){ // already on front
     return;
  }
  if(last==entry){ // the last element
    assert(last->lruNext==0);
    last= last->lruPrev;
    last->lruNext=0;
    entry->lruPrev=0;
    entry->lruNext = first;
    first->lruPrev = entry;
    first = entry;
    return;
  }
  // entry in in the middle of the list
  assert(entry->lruPrev);
  assert(entry->lruNext);
  entry->lruPrev->lruNext = entry->lruNext;
  entry->lruNext->lruPrev = entry->lruPrev;
  entry->lruPrev = 0;
  entry->lruNext = first;
  first->lruPrev = entry;
  first = entry;
}


/*
2.14 check

Debugging function.

*/
bool PersistentFlobCache::check(){
  if(first==0 || last==0){
    if(first!=last){
      std::cerr << "inconsistence in lru list, first = " << (void*) first 
           << " , last = " << (void*) last << std::endl;
      return false;
    }
    for(unsigned int i=0;i< tableSize;i++){
      if(hashtable[i]){
         std::cerr << "lru is empty, but hashtable[" << i 
              << "] contains an element" << std::endl;
         return false;
      }
    }
    // empty cache
    return true;
  }
  // lru is not empty, check first and last element
  if(first->lruPrev){
    std::cerr << "lru: first has a predecessor" << std::endl;
    return false;
  }
  if(last->lruNext){
    std::cerr << "lru: last has a successor" << std::endl;
    return false;
  }
  // check whether ech element in lru is also an element in hashtable
  CacheEntry* e = first;
  int lrucount = 0;
  while(e){
    lrucount++;
    size_t index = e->hashValue(tableSize); 
    if(!hashtable[index]){
      std::cerr << "element " << (*e) << " stored in lru but hashtable[" 
           << index << " is null" << std::endl;
      return  false;
    }
    CacheEntry* e2 = hashtable[index];
    while(e2 && (*e)!=(*e2)){
      e2 = e2->tableNext;
    }  
    if(!e2){
      std::cerr << "element " << (*e) << " stored in lru but not in hashtable[" 
           << index << "]" << std::endl;
      return false;
    }
    e = e->lruNext;
  }
  // check hashtable
  int tablecount = 0;
  for(unsigned int i=0; i<tableSize;i++){
    if(hashtable[i]){
       if(hashtable[i]->tablePrev){
           std::cerr << " hashtable[" << i << " has a predecessor" << std::endl;
       }
       e = hashtable[i];
       while(e){
         tablecount++;
         if(e->hashValue(tableSize)!=i){
           std::cerr << "element << " << (*e) << " has hashvalue " 
                << e->hashValue(tableSize) << " but is stored at position " 
                << i << std::endl;
           return false;
         }

         if(e->tableNext){
             if(e->tableNext->tablePrev!=e){
                std::cerr << "error in tablelist found" << std::endl;
                return false;
             }
         }
         e = e->tableNext;
       }
    }
  }

  if(lrucount!=tablecount){
    std::cerr << "lrucount = " << lrucount << " #  tablecount = " 
         << tablecount << std::endl;
    return false;
  }

  return true;
}

void PersistentFlobCache::traversePCache() {
    CacheEntry *fir, *las;
    fir = first;
    las = last;
    if(!fir || !las){
        if(!fir) std::cout<<"fir is null"<<std::endl;
        if(!las) std::cout<<"las is null"<<std::endl;
        return;
    }

    std::ofstream PCacheFile("secondo/bin/CacheRecord/PCache.txt",std::ios::trunc);
    if(!PCacheFile.is_open()){
        std::cout<<"PCacheFile open error!"<<std::endl;
    }

    int count = 0;
    //PCacheFile <<"***********PersistentCacheEntry**************"<<std::endl;
    while(fir != las){
        count++;
        PCacheFile <<count<<" ";
        PCacheFile << fir->flob << " " << fir->slotNo <<std::endl;
        fir = fir->lruNext;
    }
    if(fir == las){
        count++;
        PCacheFile << count<<" ";
        PCacheFile << fir->flob << " " << fir->slotNo <<std::endl;
    }
    //PCacheFile <<"***********************************"<<std::endl;
    PCacheFile.close();
}

void PersistentFlobCache::getHitMiss(){
    std::ofstream hitRatio("secondo/bin/CacheRecord/hitRatio.txt",std::ios::app);
    if(!hitRatio.is_open()){
        std::cout<<"hitRatio open error!"<<std::endl;
    }
    hitRatio << hitCount << " " << missCount << " " << hitCount * 1.0 / (hitCount + missCount) << " " << page_In << " " << page_Out<< std::endl;
    hitRatio.close();
}

void PersistentFlobCache::getUsedSize() {
    std::ofstream PCacheUsed("secondo/bin/CacheRecord/PCacheUsed.txt",std::ios::trunc);
    if(!PCacheUsed.is_open()){
        std::cout<<"PCacheUsed open error!"<<std::endl;
    }
    PCacheUsed << usedSize << " " << maxSize << " " << usedSize * 1.0 / maxSize<< std::endl;
    PCacheUsed.close();

}
