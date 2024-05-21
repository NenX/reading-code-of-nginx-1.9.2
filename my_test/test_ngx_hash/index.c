#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

/*
在ngx_http_test_ngx_list_handler处理方法传来的ngx_http_request_t对象中就有这个请求的内存池管理对象，我们对内存池的操作都可以基于
它来进行，这样，在这个请求结束的时候，内存池分配的内存也都会被释放。
*/

/*
在ngx_http_test_ngx_list_handler的返回值中，如果是正常的HTTP返回码，Nginx就会按照规范构造合法的响应包发送给用户。
例如，假设对于PUT方法暂不支持，那么，在处理方法中发现方法名是PUT时，返回NGX_HTTP_NOT_ALLOWED，这样Nginx也就会构造类似下面的响应包给用户。
*/

/*
Nginx在调用例子中的ngx_http_test_ngx_list_handler方法时是阻塞了整个Nginx
进程的，所以ngx_http_test_ngx_list_handler或类似的处理方法中是不能有耗时很长的操作的。
*/
static struct user_data_s
{
    ngx_str_t str;
    size_t num;
};
static char server_name1[1024];
static char server_name2[1024];
static char server_name3[1024];
static char server_name_find[1024];
typedef struct user_data_s user_data;
static void set_data(ngx_list_t *testlist)
{
    ngx_str_t *tartget_str;

    tartget_str = ngx_list_push(testlist);
    tartget_str->len = sizeof("1");
    tartget_str->data = "1";

    tartget_str = ngx_list_push(testlist);
    tartget_str->len = sizeof("1");
    tartget_str->data = "3";

    tartget_str = ngx_list_push(testlist);
    tartget_str->len = sizeof("1");
    tartget_str->data = "7";

    tartget_str = ngx_list_push(testlist);

    tartget_str->len = sizeof("sbb123");
    tartget_str->data = "sbb123";
}
static void append_buf(ngx_buf_t *b, char *str)
{
    ngx_memcpy(b->last, str, strlen(str));
    b->last = b->last + strlen(str);
}
ngx_module_t test_ngx_hash;
static void do_test(ngx_http_request_t *r, ngx_buf_t *b)
{

    ngx_memcpy(server_name1, "*.test.com", strlen("*.test.com"));
    ngx_memcpy(server_name2, "www.test.*", strlen("www.test.*"));
    ngx_memcpy(server_name3, "www.test.com", strlen("www.test.com"));

    ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_get_module_loc_conf(r, test_ngx_hash);

    char my_buf[1024];

    ngx_str_t *tartget_str;
    ngx_list_part_t *traget_part;

    ngx_hash_init_t hash;
    ngx_hash_keys_arrays_t ha;
    ngx_hash_combined_t combined_hash;

    ngx_memzero(&ha, sizeof(ngx_hash_keys_arrays_t));

    ha.temp_pool = ngx_create_pool(16384, r->connection->log);
    ha.pool = r->pool;

    ngx_hash_keys_array_init(&ha, NGX_HASH_LARGE);

    user_data users[3] = {
        strlen(server_name1),
        server_name1,
        0,
        strlen(server_name2),
        server_name2,
        1,
        strlen(server_name3),
        server_name3,
        2,
    };

    ngx_hash_add_key(&ha, &users[0].str, users + 0, NGX_HASH_WILDCARD_KEY);
    ngx_hash_add_key(&ha, &users[1].str, users + 1, NGX_HASH_WILDCARD_KEY);
    ngx_hash_add_key(&ha, &users[2].str, users + 2, NGX_HASH_WILDCARD_KEY);

    hash.key = ngx_hash_key_lc;
    hash.max_size = 100;
    hash.bucket_size = 48;
    hash.name = "test hash";
    hash.pool = r->pool;

    if (ha.keys.nelts)
    {
        hash.hash = &combined_hash.hash;
        hash.temp_pool = NULL;
        ngx_hash_init(&hash, ha.keys.elts, ha.keys.nelts);
    }
    if (ha.dns_wc_head.nelts)
    {
        hash.hash = NULL;
        hash.temp_pool = ha.temp_pool;
        ngx_hash_wildcard_init(&hash, ha.dns_wc_head.elts, ha.dns_wc_head.nelts);
        combined_hash.wc_head = hash.hash;
    }
    if (ha.dns_wc_tail.nelts)
    {
        hash.hash = NULL;
        hash.temp_pool = ha.temp_pool;
        ngx_hash_wildcard_init(&hash, ha.dns_wc_tail.elts, ha.dns_wc_tail.nelts);
        combined_hash.wc_tail = hash.hash;
    }
    ngx_destroy_pool(ha.temp_pool);

    ngx_memcpy(server_name_find, "www.test.org", strlen("www.test.org"));

    user_data user_find = {
        strlen(server_name_find),
        server_name_find,
        99,
    };


    user_data *target = ngx_hash_find_combined(&combined_hash, ngx_hash_key_lc(user_find.str.data, user_find.str.len), user_find.str.data, user_find.str.len);
    if (target != NULL)
    {
        bzero(my_buf, 1024);

        sprintf(my_buf, "<li> hash find !!!!!!!!!!!: %*s,%d</li>", (int)target->str.len, target->str.data, target->num);
        append_buf(b, my_buf);
    }
}
static ngx_int_t ngx_http_test_ngx_list_handler(ngx_http_request_t *r)
{
    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
        return NGX_HTTP_NOT_ALLOWED;

    ngx_int_t rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK)
        return rc;

    ngx_buf_t *b;
    b = ngx_create_temp_buf(r->pool, 1024);

    if (b == NULL)
        return NGX_HTTP_INTERNAL_SERVER_ERROR;

    append_buf(b, "<h1> test_ngx_hash !</h1>");

    do_test(r, b);
    // char *strs1[] = {"aa", "bb", "cc", "dd", "ee"};
    // char **strs2 = {"aa", "bb", "cc", "dd", "ee"};
    // printf("!!!!!!!!!!!!~~ strs1 -> %p\n", strs1[1]);
    // printf("!!!!!!!!!!!!~~ strs2 -> %p\n", strs2[1]);
    b->last_buf = 1;

    r->headers_out.content_length_n = b->last - b->start;

    r->headers_out.status = NGX_HTTP_OK;
    ngx_str_set(&r->headers_out.content_type, "text/html");

    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
        return rc;

    ngx_chain_t out;
    out.buf = b;
    out.next = NULL;

    return ngx_http_output_filter(r, &out);
}

static char *ngx_http_test_ngx_list(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf;

    /*
    首先找到test_ngx_list配置项所属的配置块，clcf看上去像是location块内的数据结构，其实不然，它可以是main、srv或者loc级别配置项，也就是说，
    在每个http{}和server{}内也都有一个ngx_http_core_loc_conf_t结构体
    */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

    /*HTTP框架在处理用户请求进行到NGX_HTTP_CONTENT_PHASE阶段时，如果请求的主机域名、URI与test_ngx_list配置项所在的配置块相匹配，就将调用我们实现的ngx_http_test_ngx_list_handler方法处理这个请求*/
    // 该函数在ngx_http_core_content_phase中的ngx_http_finalize_request(r, r->content_handler(r));里面的r->content_handler(r)执行
    clcf->handler = ngx_http_test_ngx_list_handler; // HTTP框架在接收完HTTP请求的头部后，会调用handler指向的方法

    return NGX_CONF_OK;
}

static ngx_command_t ngx_http_test_ngx_list_commands[] = {
    {ngx_string("test_ngx_hash"),
     NGX_HTTP_MAIN_CONF | NGX_HTTP_SRV_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_NOARGS,

     /*
     ngx_http_test_ngx_list是ngx_command_t结构体中的set成员（完整定义为char *(*set)(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);），
     当在某个配置块中出现test_ngx_list配置项时，Nginx将会调用ngx_http_test_ngx_list方法。下面看一下如何实现ngx_http_test_ngx_list方法。
     */
     ngx_http_test_ngx_list,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL},
    ngx_null_command};

// 暂且不管如何实现处理请求的ngx_http_test_ngx_list_handler方法，如果没有什么工作是必须在HTTP框架初始化时完成的，那就不必实现ngx_http_module_t的8个回调方法，可以像下面这样定义ngx_http_module_t接口。
static ngx_http_module_t playground_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL};

// 定义test_ngx_list模块：
ngx_module_t test_ngx_hash = {
    NGX_MODULE_V1,
    &playground_ctx,
    ngx_http_test_ngx_list_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING};
