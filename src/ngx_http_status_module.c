
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>


struct ngx_http_status_data_zone_s {
    ngx_str_t          name;

    ngx_atomic_int_t   processing;
    ngx_atomic_int_t   requests;

    /* responses */
    ngx_atomic_int_t   total;
    ngx_atomic_int_t   total_1xx;
    ngx_atomic_int_t   total_2xx;
    ngx_atomic_int_t   total_3xx;
    ngx_atomic_int_t   total_4xx;
    ngx_atomic_int_t   total_5xx;

    ngx_atomic_int_t   received;
    ngx_atomic_int_t   sent;
};


struct ngx_http_status_data_cache_s {

};


typedef struct {
    ngx_str_t      version;
    ngx_str_t      nginx_version;
    ngx_str_t      address;
    ngx_uint_t     load_timestamp;
    ngx_uint_t     timestamp;

    /* connections */
    ngx_atomic_int_t    accepted;
    ngx_atomic_int_t    dropped;
    ngx_atomic_int_t    active;
    ngx_atomic_int_t    idle;

    /** requests */
    ngx_atomic_int_t    total;
    ngx_atomic_int_t    current;

} ngx_http_status_data_t;


static ngx_str_t ngx_http_status_version =  ngx_string("4");


static char *ngx_http_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_status_format(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static char *ngx_http_status_zone(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_http_status_commands[] = {

    { ngx_string("status"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_status,
      0,
      0,
      NULL },

    { ngx_string("status_format"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE12,
      ngx_http_status_format,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    { ngx_string("status_zone"),
      NGX_HTTP_SRV_CONF|NGX_CONF_TAKE12,
      ngx_http_status_zone,
      NGX_HTTP_SRV_CONF_OFFSET,
      0,
      NULL },

      ngx_null_command
};


static ngx_http_module_t  ngx_http_status_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,                          /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    NULL,                          /* create location configuration */
    NULL                           /* merge location configuration */
};


ngx_module_t  ngx_http_status_module = {
    NGX_MODULE_V1,
    &ngx_http_status_module_ctx,   /* module context */
    ngx_http_status_commands,      /* module directives */
    NGX_HTTP_MODULE,               /* module type */
    NULL,                          /* init master */
    NULL,                          /* init module */
    NULL,                          /* init process */
    NULL,                          /* init thread */
    NULL,                          /* exit thread */
    NULL,                          /* exit process */
    NULL,                          /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_status_handler(ngx_http_request_t *r)
{
    size_t             size;
    ngx_int_t          rc;
    ngx_buf_t         *b;
    ngx_chain_t        out;
    ngx_atomic_int_t   ap, hn, ac, rq, rd, wr, wa;

    if (r->method != NGX_HTTP_GET && r->method != NGX_HTTP_HEAD) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK) {
        return rc;
    }

    r->headers_out.content_type_len = sizeof("application/json") - 1;
    ngx_str_set(&r->headers_out.content_type, "application/json");
    r->headers_out.content_type_lowcase = NULL;

    if (r->method == NGX_HTTP_HEAD) {
        r->headers_out.status = NGX_HTTP_OK;

        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    size = sizeof("Active connections:  \n") + NGX_ATOMIC_T_LEN
           + sizeof("server accepts handled requests\n") - 1
           + 6 + 3 * NGX_ATOMIC_T_LEN
           + sizeof("Reading:  Writing:  Waiting:  \n") + 3 * NGX_ATOMIC_T_LEN;

    b = ngx_create_temp_buf(r->pool, size);
    if (b == NULL) {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf = b;
    out.next = NULL;

    ap = 1;
    hn = 1;
    ac = 1;
    rq = 1;
    rd = 1;
    wr = 1;
    wa = 1;

    b->last = ngx_sprintf(b->last, "Active connections: %uA \n", ac);

    b->last = ngx_cpymem(b->last, "server accepts handled requests\n",
                         sizeof("server accepts handled requests\n") - 1);

    b->last = ngx_sprintf(b->last, " %uA %uA %uA \n", ap, hn, rq);

    b->last = ngx_sprintf(b->last, "Reading: %uA Writing: %uA Waiting: %uA \n",
                          rd, wr, wa);

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = b->last - b->pos;

    b->last_buf = (r == r->main) ? 1 : 0;
    b->last_in_chain = 1;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out);
}


static char *
ngx_http_status(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;

    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_status_handler;

    return NGX_CONF_OK;
}


static char *
ngx_http_status_format(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    return NGX_CONF_OK;
}


static char *
ngx_http_status_zone(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    return NGX_CONF_OK;
}

