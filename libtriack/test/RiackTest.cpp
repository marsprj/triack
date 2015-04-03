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
	//int ret = riack_connect(m_riak, "192.168.111.86", 8087, NULL);
	int ret = riack_connect(m_riak, "192.168.111.104", 8087, NULL);
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