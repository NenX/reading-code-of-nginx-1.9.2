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
static ngx_int_t ngx_http_test_ngx_list_handler(ngx_http_request_t *r);
static ngx_int_t
ngx_http_playgound_handler(ngx_http_request_t *r)
{
    ngx_int_t rc;

    return ngx_http_test_ngx_list_handler(r);
}

static ngx_int_t
ngx_http_dav_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt *h;
    ngx_http_core_main_conf_t *cmcf;

    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL)
    {
        return NGX_ERROR;
    }

    *h = ngx_http_playgound_handler;

    return NGX_OK;
}
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

static void do_test(ngx_http_request_t *r, ngx_buf_t *b)
{
    pid_t pp = getpid();
    char my_buf[1024];

    ngx_str_t *tartget_str;
    ngx_list_part_t *traget_part;
    ngx_list_t *testlist = ngx_list_create(r->pool, 2, sizeof(ngx_str_t));

    set_data(testlist);

    // test
    bzero(my_buf, 1024);
    u_int size_of_short = sizeof(short);
    u_int size_of_short_int = sizeof(short int);
    u_int size_of_int = sizeof(int);
    sprintf(my_buf, "<h1>size_of_short:%d,size_of_short_int:%d,size_of_int%d</h1>", size_of_short, size_of_short_int, size_of_int);

    append_buf(b, my_buf);

    // test
    traget_part = &testlist->part;
    tartget_str = traget_part->elts;
    size_t idx = 0;
    while (1)
    {
        bzero(my_buf, 1024);

        if (idx >= traget_part->nelts)
        {
            if (traget_part->next == NULL)
            {
                break;
            }
            traget_part = traget_part->next;
            tartget_str = traget_part->elts;
            idx = 0;
        }
        tartget_str = tartget_str + idx;
        sprintf(my_buf, "<li> test list xx: %*s, pid:   %d</li>", (int)tartget_str->len, tartget_str->data, pp);
        append_buf(b, my_buf);
        idx++;
    }

    ngx_array_t *testarr;

    testarr = ngx_array_create(r->pool, 3, sizeof(ngx_str_t));

    tartget_str = ngx_array_push(testarr);
    ngx_str_set(tartget_str, "sb1");
    tartget_str = ngx_array_push(testarr);
    ngx_str_set(tartget_str, "sb2");
    tartget_str = ngx_array_push(testarr);
    ngx_str_set(tartget_str, "sb3");
    tartget_str = ngx_array_push(testarr);
    ngx_str_set(tartget_str, "sb444444444444444");

    tartget_str = testarr->elts;

    for (size_t i = 0; i < testarr->nelts; i++)
    {
        bzero(my_buf, 1024);

        sprintf(my_buf, "<li>dynamic array: %*s,%d</li>", (int)tartget_str[i].len, tartget_str[i].data, i);

        append_buf(b, my_buf);
    }

    // ngx_rbtree_t test_rbtree;
    // ngx_rbtree_t * test_rbtree_p;
    // ngx_rbtree_node_t test_sentinal;

    // ngx_rbtree_init(test_rbtree_p,&test_sentinal,ngx_rbtree_insert_value);
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

    append_buf(b, "<h1> Playgound !</h1>");

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
    // clcf->handler = ngx_http_test_ngx_list_handler; // HTTP框架在接收完HTTP请求的头部后，会调用handler指向的方法

    return NGX_CONF_OK;
}

static ngx_command_t ngx_http_test_ngx_list_commands[] = {
    {ngx_string("playground"),
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
    ngx_http_dav_init,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL};

// 定义test_ngx_list模块：
ngx_module_t playground = {
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
