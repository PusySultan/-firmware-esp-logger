/*
 * ISettingCollection.hpp
 *
 *  Created on: 18 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_MSGFROMSERVERPROCESSORS_COLLECTIONPROCESSOR_ICOLLECTIONPROCESSOR_HPP_
#define MAIN_MSGFROMSERVERPROCESSORS_COLLECTIONPROCESSOR_ICOLLECTIONPROCESSOR_HPP_

#include "collection_t.hpp"
#include "request_t.hpp"

class ICollectionProcessor
{
	public:
	  ICollectionProcessor (collection_t);
	  virtual ~ICollectionProcessor ();
	  virtual collection_t getCollectionType() {return this -> collection;};
	  virtual void process(const request_t&) {};
	  
	private:
	  collection_t collection = UNKNOW_COLLECTION;
};

#endif /* MAIN_MSGFROMSERVERPROCESSORS_COLLECTIONPROCESSOR_ICOLLECTIONPROCESSOR_HPP_ */
