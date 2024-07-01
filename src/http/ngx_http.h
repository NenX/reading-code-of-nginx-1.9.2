
/*11111111111111111111111111
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_HTTP_H_INCLUDED_
#define _NGX_HTTP_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

typedef struct ngx_http_request_s     ngx_http_request_t;
typedef struct ngx_http_upstream_s    ngx_http_upstream_t;
typedef struct ngx_http_cache_s       ngx_http_cache_t;
typedef struct ngx_http_file_cache_s  ngx_http_file_cache_t;
typedef struct ngx_http_log_ctx_s     ngx_http_log_ctx_t;
typedef struct ngx_http_chunked_s     ngx_http_chunked_t;

#if (NGX_HTTP_V2)
typedef struct ngx_http_v2_stream_s   ngx_http_v2_stream_t;
#endif

typedef ngx_int_t (*ngx_http_header_handler_pt)(ngx_http_request_t *r,
    ngx_table_elt_t *h, ngx_uint_t offset);
typedef u_char *(*ngx_http_log_handler_pt)(ngx_http_request_t *r,
    ngx_http_request_t *sr, u_char *buf, size_t len);


#include <ngx_http_variables.h>
#include <ngx_http_config.h>
#include <ngx_http_request.h>
#include <ngx_http_script.h>
#include <ngx_http_upstream.h>
#include <ngx_http_upstream_round_robin.h>
#include <ngx_http_core_module.h>

#if (NGX_HTTP_V2)
#include <ngx_http_v2.h>
#endif
#if (NGX_HTTP_CACHE)
#include <ngx_http_cache.h>
#endif
#if (NGX_HTTP_SSI)
#include <ngx_http_ssi_filter_module.h>
#endif
#if (NGX_HTTP_SSL)
#include <ngx_http_ssl_module.h>
#endif


struct ngx_http_log_ctx_s { //在ngx_http_init_connection中创建空间
    ngx_connection_t    *connection; //赋值见ngx_http_init_connection
    ngx_http_request_t  *request; //赋值见ngx_http_create_request
    ngx_http_request_t  *current_request; //赋值见ngx_http_create_request
};

/*
格式:

十六进制ea5表明这个暑假块有3749字节
          这个块为3749字节，块数结束后\r\n表明这个块已经结束               这个块为3752字节，块数结束后\r\n表明这个块已经结束 
                                                                                                                                 0表示最后一个块，最后跟两个\r\n
ea5\r\n........................................................\r\n ea8\r\n..................................................\r\n 0\r\n\r\n

参考:http://blog.csdn.net/zhangboyj/article/details/6236780
*/
struct ngx_http_chunked_s {
    ngx_uint_t           state;
    off_t                size;
    off_t                length;
};

/*
upstream模块每次接收到一段TCP流时都会回调mytest模块实现的process_header方法解析，达样就需要有一个上下文保存解析状态,因为获取上游服务器
的相应头部的时候，可能需要读取多次。参考ngx_http_upstream_s中的process_header解释
*/ //头部行解析过程见ngx_http_parse_status_line
typedef struct {
    ngx_uint_t           http_version; // HTTP/1.1 对应的为1001
    ngx_uint_t           code; //// HTTP/1.1 200 OK 对应中的200
    ngx_uint_t           count; //响应码数字位数，例如200，则为3
    u_char              *start;//执行http 1/1 200 ok中的2的地址
    u_char              *end; //end实际上指向的是\r的地址
} ngx_http_status_t;

/*
ngx_http_get_module_ctx和ngx_http_set_ctx这两个宏可以完成HTTP上下文的设置和
使用。r的结构类型为ngx_http_request_t

ngx_http_get_module ctx接受两个参数，其中第1个参数是ngx_http_request_t指针，
第2个参数则是当前的HTTP模块对象。例如，在mytest模块中使用的就是在ngx_module_t类型的ngx_http_mytest_module结构体。ngx_http_get_module- ctx返回值
就是某个HTTP模块的上下文结构体指针，如果这个HTTP模块没有设置过上下文，那么将
会返回NULL空指针。因此，在任何一个HTTP模块中，都可以使用ngx_http_get_module_
ctx获取所有HTTP模块为该请求创建的上下文结构体。
*/ 
//ngx_http_get_module_ctx存储运行过程中的各种状态(例如读取后端数据，可能需要多次读取)  ngx_http_get_module_loc_conf获取该模块在local{}中的配置信息
//注意ngx_http_get_module_main_conf ngx_http_get_module_loc_conf和ngx_http_get_module_ctx的区别
#define ngx_http_get_module_ctx(r, module)  (r)->ctx[module.ctx_index]  //主要是http upstream上下文

/*
 ngx_http_set ctx接受3个参数，其中第1个参数是ngx_http_request_t指针，第2个参
数是准备设置的上下文结构体的指针，第3个参数则是HTTP模块对象。  参考4.5节
*/ //注意和conf->ctx的区别，参考ngx_http_get_module_loc_conf
#define ngx_http_set_ctx(r, c, module)      r->ctx[module.ctx_index] = c; //主要是http upstream相关的fastcgi proxy memcached等上下文 


ngx_int_t ngx_http_add_location(ngx_conf_t *cf, ngx_queue_t **locations,
    ngx_http_core_loc_conf_t *clcf);
ngx_int_t ngx_http_add_listen(ngx_conf_t *cf, ngx_http_core_srv_conf_t *cscf,
    ngx_http_listen_opt_t *lsopt);


void ngx_http_init_connection(ngx_connection_t *c);
void ngx_http_close_connection(ngx_connection_t *c);

#if (NGX_HTTP_SSL && defined SSL_CTRL_SET_TLSEXT_HOSTNAME)
int ngx_http_ssl_servername(ngx_ssl_conn_t *ssl_conn, int *ad, void *arg);
#endif

ngx_int_t ngx_http_parse_request_line(ngx_http_request_t *r, ngx_buf_t *b);
ngx_int_t ngx_http_parse_uri(ngx_http_request_t *r);
ngx_int_t ngx_http_parse_complex_uri(ngx_http_request_t *r,
    ngx_uint_t merge_slashes);
ngx_int_t ngx_http_parse_status_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_http_status_t *status);
ngx_int_t ngx_http_parse_unsafe_uri(ngx_http_request_t *r, ngx_str_t *uri,
    ngx_str_t *args, ngx_uint_t *flags);
ngx_int_t ngx_http_parse_header_line(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_uint_t allow_underscores);
ngx_int_t ngx_http_parse_multi_header_lines(ngx_array_t *headers,
    ngx_str_t *name, ngx_str_t *value);
ngx_int_t ngx_http_parse_set_cookie_lines(ngx_array_t *headers,
    ngx_str_t *name, ngx_str_t *value);
ngx_int_t ngx_http_arg(ngx_http_request_t *r, u_char *name, size_t len,
    ngx_str_t *value);
void ngx_http_split_args(ngx_http_request_t *r, ngx_str_t *uri,
    ngx_str_t *args);
ngx_int_t ngx_http_parse_chunked(ngx_http_request_t *r, ngx_buf_t *b,
    ngx_http_chunked_t *ctx);


ngx_http_request_t *ngx_http_create_request(ngx_connection_t *c);
ngx_int_t ngx_http_process_request_uri(ngx_http_request_t *r);
ngx_int_t ngx_http_process_request_header(ngx_http_request_t *r);
void ngx_http_process_request(ngx_http_request_t *r);
void ngx_http_update_location_config(ngx_http_request_t *r);
void ngx_http_handler(ngx_http_request_t *r);
void ngx_http_run_posted_requests(ngx_connection_t *c);
ngx_int_t ngx_http_post_request(ngx_http_request_t *r,
    ngx_http_posted_request_t *pr);
void ngx_http_finalize_request(ngx_http_request_t *r, ngx_int_t rc);
void ngx_http_free_request(ngx_http_request_t *r, ngx_int_t rc);

void ngx_http_empty_handler(ngx_event_t *wev);
void ngx_http_request_empty_handler(ngx_http_request_t *r);


#define NGX_HTTP_LAST   1
#define NGX_HTTP_FLUSH  2

ngx_int_t ngx_http_send_special(ngx_http_request_t *r, ngx_uint_t flags);


ngx_int_t ngx_http_read_client_request_body(ngx_http_request_t *r,
    ngx_http_client_body_handler_pt post_handler);
ngx_int_t ngx_http_read_unbuffered_request_body(ngx_http_request_t *r);

ngx_int_t ngx_http_send_header(ngx_http_request_t *r);
ngx_int_t ngx_http_special_response_handler(ngx_http_request_t *r,
    ngx_int_t error);
ngx_int_t ngx_http_filter_finalize_request(ngx_http_request_t *r,
    ngx_module_t *m, ngx_int_t error);
void ngx_http_clean_header(ngx_http_request_t *r);


ngx_int_t ngx_http_discard_request_body(ngx_http_request_t *r);
void ngx_http_discarded_request_body_handler(ngx_http_request_t *r);
void ngx_http_block_reading(ngx_http_request_t *r);
void ngx_http_test_reading(ngx_http_request_t *r);


char *ngx_http_types_slot(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
char *ngx_http_merge_types(ngx_conf_t *cf, ngx_array_t **keys,
    ngx_hash_t *types_hash, ngx_array_t **prev_keys,
    ngx_hash_t *prev_types_hash, ngx_str_t *default_types);
ngx_int_t ngx_http_set_default_types(ngx_conf_t *cf, ngx_array_t **types,
    ngx_str_t *default_type);

#if (NGX_HTTP_DEGRADATION)
ngx_uint_t  ngx_http_degraded(ngx_http_request_t *);
#endif


extern ngx_module_t  ngx_http_module;

extern ngx_str_t  ngx_http_html_default_types[];


/*
ngx_http_output_header_filter_pt  ngx_http_top_header_filter;
ngx_http_output_body_filter_pt    ngx_http_top_body_filter;
ngx_http_request_body_filter_pt   ngx_http_top_request_body_filter;
*/
extern ngx_http_output_header_filter_pt  ngx_http_top_header_filter;
extern ngx_http_output_body_filter_pt    ngx_http_top_body_filter;
extern ngx_http_request_body_filter_pt   ngx_http_top_request_body_filter;

typedef struct {
    //proxy_pass  http://10.10.0.103:8080/tttxx;中的http://10.10.0.103:8080
    ngx_str_t                      key_start; // 初始状态plcf->vars.key_start = plcf->vars.schema;
    /*  "http://" 或者 "https://"  */ //指向的是uri，当时schema->len="http://" 或者 "https://"字符串长度7或者8，参考ngx_http_proxy_pass
    ngx_str_t                      schema; ////proxy_pass  http://10.10.0.103:8080/tttxx;中的http://
    
    ngx_str_t                      host_header;//proxy_pass  http://10.10.0.103:8080/tttxx; 中的10.10.0.103:8080
    ngx_str_t                      port; //"80"或者"443"，见ngx_http_proxy_set_vars
    //proxy_pass  http://10.10.0.103:8080/tttxx; 中的/tttxx         uri不带http://http://10.10.0.103:8080 url带有http://10.10.0.103:8080
    ngx_str_t                      uri; //ngx_http_proxy_set_vars里面赋值   uri是proxy_pass  http://10.10.0.103:8080/xxx中的/xxx，如果
    //proxy_pass  http://10.10.0.103:8080则uri长度为0，没有uri
} ngx_http_proxy_vars_t;


typedef struct {
    ngx_array_t                   *flushes;
     /* 
    把proxy_set_header与ngx_http_proxy_headers合在一起，然后把其中的key:value字符串和长度信息添加到headers->lengths和headers->values中
    把ngx_http_proxy_cache_headers合在一起，然后把其中的key:value字符串和长度信息添加到headers->lengths和headers->values中
     */
    //1. 里面存储的是ngx_http_proxy_headers和proxy_set_header设置的头部信息头添加到该lengths和values 存入ngx_http_proxy_loc_conf_t->headers
    //2. 里面存储的是ngx_http_proxy_cache_headers设置的头部信息头添加到该lengths和values  存入ngx_http_proxy_loc_conf_t->headers_cache
    ngx_array_t                   *lengths; //创建空间和赋值见ngx_http_proxy_init_headers
    ngx_array_t                   *values;  //创建空间和赋值见ngx_http_proxy_init_headers
    ngx_hash_t                     hash;
} ngx_http_proxy_headers_t;

typedef struct {
    ngx_http_upstream_conf_t       upstream;

    /* 下面这几个和proxy_set_body XXX配置相关 proxy_set_body设置包体后，就不会传送客户端发送来的数据到后端服务器(只传送proxy_set_body设置的包体)，
    如果没有设置，则会发送客户端包体数据到后端， 参考ngx_http_proxy_create_request */
    ngx_array_t                   *body_flushes; //从proxy_set_body XXX配置中获取，见ngx_http_proxy_merge_loc_conf
    ngx_array_t                   *body_lengths; //从proxy_set_body XXX配置中获取，见ngx_http_proxy_merge_loc_conf
    ngx_array_t                   *body_values;  //从proxy_set_body XXX配置中获取，见ngx_http_proxy_merge_loc_conf
    ngx_str_t                      body_source; //proxy_set_body XXX配置中的xxx

    //数据来源是ngx_http_proxy_headers  proxy_set_header，见ngx_http_proxy_init_headers
    ngx_http_proxy_headers_t       headers;//创建空间和赋值见ngx_http_proxy_merge_loc_conf
#if (NGX_HTTP_CACHE)
    //数据来源是ngx_http_proxy_cache_headers，见ngx_http_proxy_init_headers
    ngx_http_proxy_headers_t       headers_cache;//创建空间和赋值见ngx_http_proxy_merge_loc_conf
#endif
    //通过proxy_set_header Host $proxy_host;设置并添加到该数组中
    ngx_array_t                   *headers_source; //创建空间和赋值见ngx_http_proxy_init_headers

    //解析uri中的变量的时候会用到下面两个，见ngx_http_proxy_pass  proxy_pass xxx中如果有变量，下面两个就不为空
    ngx_array_t                   *proxy_lengths;
    ngx_array_t                   *proxy_values;
    //proxy_redirect [ default|off|redirect replacement ]; 
    //成员类型ngx_http_proxy_rewrite_t
    ngx_array_t                   *redirects; //和配置proxy_redirect相关，见ngx_http_proxy_redirect创建空间
    ngx_array_t                   *cookie_domains;
    ngx_array_t                   *cookie_paths;

    /* 此配置项表示转发时的协议方法名。例如设置为：proxy_method POST;那么客户端发来的GET请求在转发时方法名也会改为POST */
    ngx_str_t                      method;
    ngx_str_t                      location; //当前location的名字 location xxx {} 中的xxx

    //proxy_pass  http://10.10.0.103:8080/tttxx; 中的http://10.10.0.103:8080/tttxx
    ngx_str_t                      url; //proxy_pass url名字，见ngx_http_proxy_pass

#if (NGX_HTTP_CACHE)
    ngx_http_complex_value_t       cache_key;//proxy_cache_key为缓存建立索引时使用的关键字；见ngx_http_proxy_cache_key
#endif

    ngx_http_proxy_vars_t          vars;//里面保存proxy_pass uri中的uri信息
    //默认1
    ngx_flag_t                     redirect;//和配置proxy_redirect相关，见ngx_http_proxy_redirect
    //proxy_http_version设置，
    ngx_uint_t                     http_version; //到后端服务器采用的http协议版本，默认NGX_HTTP_VERSION_10

    ngx_uint_t                     headers_hash_max_size;
    ngx_uint_t                     headers_hash_bucket_size;

#if (NGX_HTTP_SSL)
    ngx_uint_t                     ssl;
    ngx_uint_t                     ssl_protocols;
    ngx_str_t                      ssl_ciphers;
    ngx_uint_t                     ssl_verify_depth;
    ngx_str_t                      ssl_trusted_certificate;
    ngx_str_t                      ssl_crl;
    ngx_str_t                      ssl_certificate;
    ngx_str_t                      ssl_certificate_key;
    ngx_array_t                   *ssl_passwords;
#endif
} ngx_http_proxy_loc_conf_t;

extern ngx_module_t  ngx_http_proxy_module;

#endif /* _NGX_HTTP_H_INCLUDED_ */
