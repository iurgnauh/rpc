// the extent server implementation

#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#include "lang/verify.h"
#include "kv_server.h"
#include "rpc/slock.h"


kv_server::kv_server() 
{
	VERIFY(pthread_mutex_init(&map_m_, 0) == 0);
}

kv_server::~kv_server()
{
	VERIFY(pthread_mutex_destroy(&map_m_) == 0);
}

/* The RPC reply argument "val" should contain 
 * the retrieved val together with its current version 
 */
int 
kv_server::get(std::string key, kv_protocol::versioned_val &val)
{
	// You fill this in for Lab 1.
	ScopedLock ml(&map_m_);
	if (kv_map.find(key) == kv_map.end() || kv_map[key]->deleted)
		return kv_protocol::NOEXIST;
	val = kv_map[key]->val;
	return kv_protocol::OK;
}


/* the server should store the key-value entry, increment its version if appropriate.
   and put the new version of the stored entry in the RPC reply (i.e. the last argument)
*/
int 
kv_server::put(std::string key, std::string buf, int &new_version)
{
	// You fill this in for Lab 1.
	ScopedLock ml(&map_m_);
	if (kv_map.find(key) == kv_map.end()) {
		kv_protocol::versioned_val val;
		val.buf = buf;
		val.version = 1;
		new_version = 1;
		new_val = value(val, false);
		kv_map[key] = &new_val;
		return kv_protocol::OK;
	}
	value *val_p = key_map[key];
	(val_p->val).buf = buf;
	(val_p->val).version++;
	new_version = (val_p->val).version;
	val_p->deleted = false;
	return kv_protocol::OK;
}

/* "remove" the existing key-value entry
 * do not actually delete it from your storage
 * treat it as a special type of put and 
 * increments the key-value pair's version number
 * like regular puts.
 * Set a delete flag to the "removed" key-value 
 * entry so a later get does not see it.
*/
int 
kv_server::remove(std::string key, int &new_version)
{
	// You fill this in for Lab 1.
	ScopedLock ml(&map_m_);
	if (kv_map.find(key) == kv_map.end()) {
		kv_protocol::versioned_val val;
		val.buf = "";
		val.version = 1;
		new_version = 1;
		value new_val = value(val, true);
		kv_map[key] = &new_val;
		return kv_protocol::NOEXIST;
	}
	value *val_p = key_map[key];
	(val_p->val).version++;
	new_version = (val_p->val).version;
	if (val_p->deleted)
		return kv_protocol::NOEXIST;
	val_p->deleted = true;
	return kv_protocol::OK;
}

int 
kv_server::stat(int x, std::string &msg)
{
	msg = "Server says: I am kicking";	
	return kv_protocol::OK;
}
