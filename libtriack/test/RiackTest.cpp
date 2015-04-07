#include "RiackTest.h"

//CPPUNIT_TEST_SUITE_REGISTRATION(RiakTest);

void *radi_riack_alloc(void *optional_data, size_t size)
{
	return malloc(size);
}

void radi_riack_free(void *optional_data, void *pointer)
{
	if (pointer)
	{
		free(pointer);
	}
}

void RiakTest::setUp()
{
	printf("setUp\n");
	riack_init();
	m_riak = riack_new_client(NULL);
	int ret = riack_connect(m_riak, "192.168.111.86", 8087, NULL);
	//int ret = riack_connect(m_riak, "192.168.111.104", 8087, NULL);
	//int ret = riack_connect(m_riak, "123.57.207.198", 8087, NULL);
	if (ret != RIACK_SUCCESS)
	{
		riack_free(m_riak);
		m_riak = NULL;
	}
}

void RiakTest::tearDown()
{
	riack_free(m_riak);
	riack_cleanup();
	printf("tearDown\n");
}

void RiakTest::ListBuckets()
{
	riack_string_list* bucket_list;
	riack_list_buckets(m_riak, &bucket_list);

	char temp[_MAX_PATH];
	for (int i = 0; i < bucket_list->string_count; i++)
	{
		riack_string& str = bucket_list->strings[i];
		memset(temp, 0, _MAX_PATH);
		memcpy(temp, str.value, str.len);
		printf("%s\n", temp);
	}

	riack_free_string_list_p(m_riak, &bucket_list);
}

void RiakTest::GetBucketProperty()
{
	const char* id = "f057fbee-d7a4-43a3-b761-163f243730ab";

	riack_string rbucket;
	rbucket.len = strlen(id);
	rbucket.value = (char*)id;

	riack_bucket_properties* props;

	riack_get_bucket_props(m_riak, &rbucket, &props);

	riack_free_bucket_properties_p(m_riak, &props);
}

void RiakTest::CreateBucket()
{

}

void RiakTest::GetBucket()
{
	const char* id = "f057fbee-d7a4-43a3-b761-163f243730ab";

	//riack_client* client = GetConnection();
	//if (client == NULL)
	//{
	//	return NULL;
	//}

	//riack_string rbucket, rkey;
	//riack_get_object *robj = NULL;

	//rbucket.len = strlen(bucket);
	//rbucket.value = (char*)bucket;

	//rkey.len = strlen(key);
	//rkey.value = (char*)key;

	//int ret = riack_get(m_riak, &rbucket, &rkey, NULL, &robj);
	//if (ret != RIACK_SUCCESS)
	//{
	//	return NULL;
	//}

	//int count = robj->object.content_count;
	//if (!count)
	//{
	//	riack_free_get_object_p(m_riak, &robj);
	//	return NULL;
	//}

	//RiakFile* rf = NULL;
	//riack_content& r_content = robj->object.content[0];

	//rf = new RiakFile();
	//rf->Create(this, key, &r_content);

	//riack_free_get_object_p(m_riak, &robj);
	//return rf;
}

void RiakTest::PutFile()
{
	//obj.bucket.value = RIAK_TEST_BUCKET;
	//obj.bucket.len = strlen(RIAK_TEST_BUCKET);
	//obj.key.value = "test_put2";
	//obj.key.len = strlen(obj.key.value);
	//obj.vclock.len = 0;
	//obj.content_count = 1;
	//obj.content = (riack_content*)malloc(sizeof(riack_content));
	//memset(obj.content, 0, sizeof(riack_content));
	//obj.content[0].content_type.value = "application/json";
	//obj.content[0].content_type.len = strlen(obj.content[0].content_type.value);
	//obj.content[0].data = (uint8_t*)data;
	//obj.content[0].data_len = strlen(data);

	const char* bucket = "asdf";
	const char* key = "aaaa";
	const char* type = "application/json";
	const char* data = "{\"testvalue\": \"plappe lappe 2\"}";

	riack_object *robj = NULL;
	robj = riack_object_alloc(m_riak);
	robj->bucket.value = (char*)bucket;
	robj->bucket.len = strlen(bucket);
	robj->key.value = (char*)key;
	robj->key.len = strlen(key);
	robj->content_count = 1;
	robj->content = (riack_content*)malloc(sizeof(riack_content));
	memset(robj->content, 0, sizeof(riack_content));
	robj->content[0].content_type.value = (char*)type;
	robj->content[0].content_type.len = strlen(type);
	robj->content[0].data = (unsigned char*)data;
	robj->content[0].data_len = strlen(data);

	riack_put(m_riak, robj, NULL, NULL);
	

	riack_free_object_p(m_riak, &robj);
}

void RiakTest::GetObject()
{
	const char* bucket = "rfs";
	//const char* key = "f057fbee-d7a4-43a3-b761-163f243730ab";
	//const char* key = "90de2941-bbdf-4a4b-8c0d-14d4dc12e649";	//folder
	//const char* key = "6d31c25e-ddc0-4bc6-8b96-8f190b279a71";
	const char* key = "root";
	riack_string rbucket, rkey;
	riack_get_object *obj;

	rbucket.len = strlen(bucket);
	rbucket.value = (char*)bucket;

	rkey.len = strlen(key);
	rkey.value = (char*)key;

	int ret = riack_get(m_riak, &rbucket, &rkey, NULL, &obj);

	char skey[_MAX_PATH];
	char sval[_MAX_PATH];

	int count = obj->object.content->usermeta_count;
	for (int i = 0; i < count; i++)
	{
		riack_pair& r_meta = obj->object.content->usermetas[i];
		
		memset(skey, 0, _MAX_PATH);
		memset(sval, 0, _MAX_PATH);

		memcpy(skey, r_meta.key.value, r_meta.key.len);
		memcpy(sval, r_meta.value, r_meta.value_len);

		printf("[%s]:%s\n", skey, sval);
	}
	

	riack_free_get_object_p(m_riak, &obj);
}

void RiakTest::AddLink()
{
	const char* bucket = "rfs";
	//const char* key = "f057fbee-d7a4-43a3-b761-163f243730ab";
	//const char* key = "90de2941-bbdf-4a4b-8c0d-14d4dc12e649";	//folder
	//const char* key = "6d31c25e-ddc0-4bc6-8b96-8f190b279a71";
	//const char* key = "root";
	const char* key = "1ca3815e-69cb-4054-99a8-b4b28d57e5b9";
	riack_string rbucket, rkey;
	riack_get_object *obj;

	rbucket.len = strlen(bucket);
	rbucket.value = (char*)bucket;

	rkey.len = strlen(key);
	rkey.value = (char*)key;

	int ret = riack_get(m_riak, &rbucket, &rkey, NULL, &obj);

	riack_link* r_link = obj->object.content->links;
	r_link = (riack_link*)realloc(r_link, sizeof(riack_link)*(obj->object.content->link_count + 1));
	obj->object.content->links = r_link;

	r_link += obj->object.content->link_count;
	memset(r_link, 0, sizeof(riack_link));
	//r_link->bucket.value = "rfs";
	//r_link->bucket.len = strlen("rfs");
	r_link->key.value = "85ad7086-a50d-4ac3-a61f-8987df66966c";
	r_link->key.len = strlen("85ad7086-a50d-4ac3-a61f-8987df66966c");
	r_link->tag.value = "parent";
	r_link->tag.len = strlen("parent");
	obj->object.content->link_count++;

	obj->object.bucket.value = "rfs";
	obj->object.bucket.len = 3;

	ret = riack_put(m_riak, &(obj->object), NULL, NULL);
	ret == RIACK_SUCCESS;

	char skey[_MAX_PATH];
	char sval[_MAX_PATH];

	//int count = obj->object.content->usermeta_count;
	//for (int i = 0; i < count; i++)
	//{
	//	riack_pair& r_meta = obj->object.content->usermetas[i];

	//	memset(skey, 0, _MAX_PATH);
	//	memset(sval, 0, _MAX_PATH);

	//	memcpy(skey, r_meta.key.value, r_meta.key.len);
	//	memcpy(sval, r_meta.value, r_meta.value_len);

	//	printf("[%s]:%s\n", skey, sval);
	//}


}

void RiakTest::ListKeys()
{
	//const char* uuid = "48740512-e7e7-4661-871e-dfa837a2b973";
	const char* uuid = "af926e3d-b855-4d90-990c-db717b0a412d";

	riack_string_linked_list *list;
	riack_string_linked_list *rkey;
	riack_string bucket;
	bucket.value = (char*)uuid;
	bucket.len = strlen(uuid);

	int ret = riack_list_keys(m_riak, &bucket, &list);

	std::ofstream os("g:\\temp\\_aaa.txt");

	char key[RADI_PATH_MAX];
	for (rkey = list; rkey != NULL; rkey = rkey->next)
	{
		memset(key, 0, RADI_PATH_MAX);
		memcpy(key, rkey->string.value, rkey->string.len);
		//printf("[key]:%s\n", key);
		os << key << std::endl;
	}

	os.close();

	riack_free_string_linked_list_p(m_riak, &list);
}

void RiakTest::AddKey_1()
{
	const char* bucket = "test";
	const char* key = "key_111";
	const char* type = "text/plain";
	const char* data = "value_111";

	riack_object *robj = NULL;
	robj = riack_object_alloc(m_riak);
	robj->bucket.value = (char*)strdup(bucket);
	robj->bucket.len = strlen(bucket);
	robj->key.value = (char*)strdup(key);
	robj->key.len = strlen(key);
	robj->content_count = 1;
	robj->content = (riack_content*)malloc(sizeof(riack_content));
	memset(robj->content, 0, sizeof(riack_content));
	robj->content[0].content_type.value = (char*)strdup(type);
	robj->content[0].content_type.len = strlen(type);
	robj->content[0].data = (unsigned char*)strdup(data);
	robj->content[0].data_len = strlen(data);

	riack_put(m_riak, robj, NULL, NULL);


	riack_free_object_p(m_riak, &robj);
}

void RiakTest::AddKey_2()
{
	const char* bucket = "test";
	const char* key = "key_222";
	const char* type = "text/plain";
	const char* data = "value_222";

	riack_object *robj = NULL;
	robj = riack_object_alloc(m_riak);
	robj->bucket.value = (char*)strdup(bucket);
	robj->bucket.len = strlen(bucket);
	robj->key.value = (char*)strdup(key);
	robj->key.len = strlen(key);
	robj->content_count = 1;
	robj->content = (riack_content*)malloc(sizeof(riack_content));
	memset(robj->content, 0, sizeof(riack_content));
	robj->content[0].content_type.value = (char*)strdup(type);
	robj->content[0].content_type.len = strlen(type);
	robj->content[0].data = (unsigned char*)strdup(data);
	robj->content[0].data_len = strlen(data);
	// meta
	riack_pair* rpair = (riack_pair*)malloc(sizeof(riack_pair));
	memset(rpair, 0, sizeof(riack_pair));
	robj->content[0].usermetas = rpair;
	robj->content[0].usermeta_count = 1;
	raick_set_pair(*rpair, "DATA_STORAGE_TYPE", "VALUE");

	riack_put(m_riak, robj, NULL, NULL);

	riack_free_object_p(m_riak, &robj);
}


void RiakTest::AddKey_3()
{
	const char* bucket = "test_333";
	const char* key = "key_333";
	const char* type = "text/plain";
	const char* data = "value_333";

	riack_object *robj = NULL;
	robj = riack_object_alloc(m_riak);
	robj->bucket.value = (char*)strdup(bucket);
	robj->bucket.len = strlen(bucket);
	robj->key.value = (char*)strdup(key);
	robj->key.len = strlen(key);
	robj->content_count = 1;
	robj->content = (riack_content*)malloc(sizeof(riack_content));
	memset(robj->content, 0, sizeof(riack_content));
	robj->content[0].content_type.value = (char*)strdup(type);
	robj->content[0].content_type.len = strlen(type);
	robj->content[0].data = (unsigned char*)strdup(data);
	robj->content[0].data_len = strlen(data);

	riack_put(m_riak, robj, NULL, NULL);

	riack_free_object_p(m_riak, &robj);

	riack_free_object_p(m_riak, &robj);
}



void RiakTest::AddKey_4()
{
	const char* bucket = "test_444";
	const char* key = "key_444";
	const char* type = "text/plain";
	const char* data = "value_444";

	riack_object *robj = NULL;
	robj = riack_object_alloc(m_riak);
	robj->bucket.value = (char*)strdup(bucket);
	robj->bucket.len = strlen(bucket);
	robj->key.value = (char*)strdup(key);
	robj->key.len = strlen(key);
	robj->content_count = 1;
	robj->content = (riack_content*)malloc(sizeof(riack_content));
	memset(robj->content, 0, sizeof(riack_content));
	robj->content[0].content_type.value = (char*)strdup(type);
	robj->content[0].content_type.len = strlen(type);
	robj->content[0].data = (unsigned char*)strdup(data);
	robj->content[0].data_len = strlen(data);

	riack_put(m_riak, robj, NULL, NULL);

	riack_free_object_p(m_riak, &robj);

	riack_free_object_p(m_riak, &robj);
}

void RiakTest::AddKey_1_Link()
{
	const char* bucket = "test";
	const char* key = "key_111";
	const char* type = "text/plain";
	const char* data = "value_111";

	riack_object *robj = NULL;
	robj = riack_object_alloc(m_riak);
	robj->bucket.value = (char*)strdup(bucket);
	robj->bucket.len = strlen(bucket);
	robj->key.value = (char*)strdup(key);
	robj->key.len = strlen(key);
	robj->content_count = 1;
	robj->content = (riack_content*)malloc(sizeof(riack_content));
	memset(robj->content, 0, sizeof(riack_content));
	robj->content[0].content_type.value = (char*)strdup(type);
	robj->content[0].content_type.len = strlen(type);
	robj->content[0].data = (unsigned char*)strdup(data);
	robj->content[0].data_len = strlen(data);
	// link
	const char* l_bucket = "test_333";
	const char* link = "key_333";
	const char* targ = "parent";
	robj->content[0].link_count = 1;
	riack_link* r_link = (riack_link*)malloc(sizeof(riack_link));
	memset(r_link, 0, sizeof(r_link));
	robj->content[0].links = r_link;
	r_link->bucket.value = (char*)strdup(l_bucket);
	r_link->bucket.len = strlen(l_bucket);
	r_link->key.len = strlen(link);
	r_link->key.value = (char*)strdup(link);
	r_link->key.len = strlen(link);
	r_link->tag.value = (char*)strdup(targ);
	r_link->tag.len = strlen(targ);

	riack_put(m_riak, robj, NULL, NULL);
	riack_free_object_p(m_riak, &robj);
}

void RiakTest::GetKey()
{
	const char* bucket = "rfs";
	const char* key = "root";

	riack_string rbucket, rkey;
	riack_get_object *robj = NULL;

	rbucket.len = strlen(bucket);
	rbucket.value = (char*)bucket;

	rkey.len = strlen(key);
	rkey.value = (char*)key;

	int ret = riack_get(m_riak, &rbucket, &rkey, NULL, &robj);


	riack_free_get_object_p(m_riak, &robj);
}

void RiakTest::Add_Link_test_111()
{
	const char* bucket = "test";
	const char* key = "key_111";
	const char* type = "text/plain";
	const char* data = "value_111";


	riack_string rbucket, rkey;
	riack_get_object *robj = NULL;
	riack_object *nobj = NULL;

	rbucket.len = strlen(bucket);
	rbucket.value = (char*)bucket;
	rkey.len = strlen(key);
	rkey.value = (char*)key;

	int ret = riack_get(m_riak, &rbucket, &rkey, NULL, &robj);

	//riack_content& r_content = robj->object.content[0];
	//riack_link* r_link = r_content.links;
	//r_content.links = (riack_link*)realloc(r_link, (r_content.link_count + 1)*sizeof(riack_link));
	//r_link = &(r_content.links[r_content.link_count]);
	//r_content.link_count += 1;

	const char* l_bucket = "test_444";
	const char* link = "key_444";
	const char* targ = "parent";
	//memset(r_link, 0, sizeof(r_link));
	//r_link->bucket.value = (char*)strdup(l_bucket);
	//r_link->bucket.len = strlen(l_bucket);
	//r_link->key.len = strlen(link);
	//r_link->key.value = (char*)strdup(link);
	//r_link->key.len = strlen(link);
	//r_link->tag.value = (char*)strdup(targ);
	//r_link->tag.len = strlen(targ);

	nobj = riack_object_add_link_new(&(robj->object), bucket, key, l_bucket, link, targ);

	//{
	//	
	//	nobj = riack_object_alloc(m_riak);
	//	nobj->bucket.value = (char*)strdup(bucket);
	//	nobj->bucket.len = strlen(bucket);
	//	nobj->key.value = (char*)strdup(key);
	//	nobj->key.len = strlen(key);
	//	nobj->content_count = 1;
	//	nobj->content = (riack_content*)malloc(sizeof(riack_content));
	//	memset(nobj->content, 0, sizeof(riack_content));
	//	nobj->content[0].content_type.value = (char*)strdup(type);
	//	nobj->content[0].content_type.len = strlen(type);
	//	nobj->content[0].data = (unsigned char*)strdup(data);
	//	nobj->content[0].data_len = strlen(data);
	//	// link
	//	const char* l_bucket = "test_333";
	//	const char* link = "key_333";
	//	const char* targ = "parent";
	//	nobj->content[0].link_count = 1;
	//	riack_link* r_link = (riack_link*)malloc(sizeof(riack_link)*nobj->content[0].link_count);
	//	nobj->content[0].links = r_link;

	//	memset(r_link, 0, sizeof(r_link));		
	//	r_link->bucket.value = (char*)strdup(l_bucket);
	//	r_link->bucket.len = strlen(l_bucket);
	//	r_link->key.len = strlen(link);
	//	r_link->key.value = (char*)strdup(link);
	//	r_link->key.len = strlen(link);
	//	r_link->tag.value = (char*)strdup(targ);
	//	r_link->tag.len = strlen(targ);

	//	//l_bucket = "test_444";
	//	//link = "key_444";
	//	//r_link++;
	//	//memset(r_link, 0, sizeof(r_link));
	//	//r_link->bucket.value = (char*)strdup(l_bucket);
	//	//r_link->bucket.len = strlen(l_bucket);
	//	//r_link->key.len = strlen(link);
	//	//r_link->key.value = (char*)strdup(link);
	//	//r_link->key.len = strlen(link);
	//	//r_link->tag.value = (char*)strdup(targ);
	//	//r_link->tag.len = strlen(targ);
	//}

	riack_free_get_object_p(m_riak, &robj);

	riack_put(m_riak, nobj, NULL, NULL);
	
	riack_free_object_p(m_riak, &nobj);
}

void RiakTest::Add_Link_test_111_v2()
{
	const char* bucket = "test";
	const char* key = "key_111";
	const char* type = "text/plain";
	const char* data = "value_111";

	//const char* bucket = "rfs";
	//const char* key = "root";
	//const char* type = "text/plain";
	//const char* data = "value_111";

	riack_string rbucket, rkey;
	riack_get_object *robj = NULL;
	
	rbucket.len = strlen(bucket);
	rbucket.value = (char*)bucket;
	rkey.len = strlen(key);
	rkey.value = (char*)key;

	int ret = riack_get(m_riak, &rbucket, &rkey, NULL, &robj);

	riack_copy_string(robj->object.bucket, rbucket);
	riack_copy_string(robj->object.key, rkey);

	riack_content& r_content = robj->object.content[0];
	riack_link* r_link = r_content.links;
	r_content.links = (riack_link*)realloc(r_link, (r_content.link_count + 1)*sizeof(riack_link));
	r_link = r_content.links + r_content.link_count;
	r_content.link_count += 1;

	const char* l_bucket = "test_444";
	const char* l_key = "key_444";
	const char* l_tag = "parent";
	memset(r_link, 0, sizeof(r_link));
	riack_set_link(*r_link, l_bucket, l_key, l_tag);

	riack_put(m_riak, &(robj->object), NULL, NULL);

	riack_free_get_object_p(m_riak, &robj);
}


/*
typedef struct _riack_content
{
	size_t data_len;
	uint8_t *data;
	riack_string content_type;
	riack_string charset;
	riack_string content_encoding;
	riack_string vtag;
	size_t link_count;
	riack_link* links;
	//
	uint8_t last_modified_present;
	uint32_t last_modified;
	uint8_t last_modified_usecs_present;
	uint32_t last_modified_usecs;
	uint8_t deleted_present;
	uint8_t deleted;
	//
	size_t usermeta_count;
	riack_pair *usermetas;
	size_t index_count;
	riack_pair *indexes;
} riack_content;
*/
riack_object* RiakTest::riack_object_add_link_new(riack_object* sobj, const char* bucket, const char* key, const char* link_bucket, const char* link_key, const char* link_tag)
{
	riack_object* nobj = riack_object_alloc(m_riak);
	memset(nobj, 0, sizeof(riack_object));
	nobj->bucket.value = (char*)strdup(bucket);
	nobj->bucket.len = strlen(bucket);
	nobj->key.value = (char*)strdup(key);
	nobj->key.len = strlen(key);

	nobj->content_count = 1;
	nobj->content = (riack_content*)malloc(sizeof(riack_content));
	memset(nobj->content, 0, sizeof(riack_content));

	riack_content* n_content = nobj->content;
	riack_content* s_content = sobj->content;
	//[data]
	riack_copy_string(n_content->content_type, s_content->content_type);
	//[content_type]
	n_content->data_len = s_content->data_len;
	n_content->data = (uint8_t*)calloc(n_content->data_len, sizeof(uint8_t));
	memcpy(n_content->data, s_content->data, n_content->data_len);
	//[content_encoding]
	if (s_content->content_encoding.len > 0)
	{
		riack_copy_string(n_content->content_encoding, s_content->content_encoding);
	}
	//[vtag]
	if (s_content->vtag.len > 0)
	{
		riack_copy_string(n_content->vtag, s_content->vtag);
	}

	// [last_modified_present]
	// [last_modified]
	// [last_modified_usecs_present]
	// [last_modified_usecs]
	// [deleted_present]
	// [deleted]
	//n_content->last_modified_present = s_content->last_modified_present;
	//n_content->last_modified = s_content->last_modified;
	//n_content->last_modified_usecs_present = s_content->last_modified_usecs_present;
	//n_content->last_modified_usecs = s_content->last_modified_usecs;
	//n_content->deleted_present = s_content->deleted_present;
	//n_content->deleted = s_content->deleted;
	
	// [usermeta_count]
	//n_content->usermeta_count = s_content->usermeta_count;
	//n_content->usermetas = (riack_pair*)calloc(sizeof(riack_pair), n_content->usermeta_count);
	//riack_pair* upair = n_content->usermetas;
	//for (int i = 0; i < n_content->usermeta_count; i++, upair++)
	//{
	//	riack_copy_pair(*upair, s_content->usermetas[i]);
	//}

	//size_t link_count;
	//riack_link* links;
	// [link]
	n_content->link_count = s_content->link_count+1;
	n_content->links = (riack_link*)calloc(sizeof(riack_link), n_content->link_count);
	riack_link* r_link = n_content->links;
	for (int i = 0; i < s_content->link_count; i++, r_link++)
	{
		riack_copy_link(*r_link, s_content->links[i]);
	}
	riack_set_link(*r_link, link_bucket, link_key, link_tag);

	return nobj;
}

//typedef struct _riack_link
//{
//	riack_string bucket;
//	riack_string key;
//	/* Link tag */
//	riack_string tag;
//} riack_link;
void RiakTest::riack_set_link(riack_link& r_link, const char* bucket, const char* key, const char* tag)
{
	r_link.bucket.value = strdup(bucket);
	r_link.bucket.len = strlen(bucket);
	r_link.key.value = strdup(key);
	r_link.key.len = strlen(key);
	r_link.tag.value = strdup(tag);
	r_link.tag.len = strlen(tag);
}


void RiakTest::riack_copy_link(riack_link& obj, riack_link& src)
{
	riack_copy_string(obj.bucket, src.bucket);
	riack_copy_string(obj.key, src.key);
	riack_copy_string(obj.tag, src.tag);
}

void RiakTest::riack_copy_pair(riack_pair& obj, riack_pair& src)
{	
	riack_copy_string(obj.key, src.key);
	obj.value_present = src.value_present;
	obj.value_len = src.value_len;
	obj.value = (uint8_t*)malloc(sizeof(uint8_t)*src.value_len);
	memcpy(obj.value, src.value, src.value_len);
}

void RiakTest::riack_copy_string(riack_string& obj, riack_string& src)
{
	obj.len = src.len;
	obj.value = (char*)calloc(src.len, sizeof(char));
	memcpy(obj.value, src.value, src.len);
}

bool RiakTest::riack_copy_content(riack_content* n_content, riack_content* s_content)
{
	memcpy(n_content, s_content, sizeof(s_content));

	return true;
}

void RiakTest::raick_set_pair(riack_pair& rpair, const char* key, const char* data)
{
	rpair.value_present = 1;
	rpair.key.value = strdup(key);
	rpair.key.len = strlen(key);
	rpair.value_len = strlen(data);
	rpair.value = (uint8_t*)malloc(sizeof(uint8_t)*rpair.value_len);
	memcpy(rpair.value, data, rpair.value_len);
}