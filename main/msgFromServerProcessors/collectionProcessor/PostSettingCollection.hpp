/*
 * SettingCollection.hpp
 *
 *  Created on: 18 апр. 2026 г.
 *      Author: Kirill
 */

#ifndef MAIN_MSGFROMSERVERPROCESSORS_COLLECTIONPROCESSOR_POSTSETTINGCOLLECTION_HPP_
#define MAIN_MSGFROMSERVERPROCESSORS_COLLECTIONPROCESSOR_POSTSETTINGCOLLECTION_HPP_

#include <map>
#include <string>
#include <functional>
#include "StorageTypes.hpp"
#include "ICollectionProcessor.hpp"

// POST class
class PostSettingCollection : public ICollectionProcessor
{
	using setting_type_processor = std::function<void(storage_cmd_t&, cJSON&)>;
	
	public:
	  PostSettingCollection (collection_t collection);
	  virtual ~PostSettingCollection ();
	  
	  void static overrideStorageQueue(QueueHandle_t* queue);
	  
	  void process(const request_t& request) override;
	  
	private: 
	  std::map<std::string, setting_type_processor> processorsMap;
	  static QueueHandle_t* storage_event_queue;
	  
	  void fillProcessorsMap();
};

#endif /* MAIN_MSGFROMSERVERPROCESSORS_COLLECTIONPROCESSOR_POSTSETTINGCOLLECTION_HPP_ */
