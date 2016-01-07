
#Module ngx_http_status_module

The ngx_http_status_module module provides access to various status information.

This module is available as part of our commercial subscription.

##Example Configuration

~~~
server {
    location = /status {
        status;
    }

    status_zone example_server;
}
~~~
The simple monitoring page is shipped with this distribution, accessible as “/status.html” in the default configuration. It requires the location “/status” to be configured as shown above.

##Directives

~~~
syntax:	status;
default:	 —
context:	location
~~~
The status information will be accessible from the surrounding location.

~~~
syntax:	status_format json;
status_format jsonp [callback];
default:	
status_format json;
context:	http, server, location
~~~
By default, status information is output in the JSON format.

Alternatively, data may be output as JSONP. The callback parameter specifies the name of a callback function. The value can contain variables. If parameter is omitted, or the computed value is an empty string, then “ngx_status_jsonp_callback” is used.

~~~
syntax:	status_zone zone;
default:	 —
context:	server
~~~
Enables collection of virtual server status information in the specified zone. Several virtual servers may share the same zone.

##Data

The following status information is provided:

* version  
Version of the provided data set. The current version is 2.
nginx_version
Version of nginx.
* address  
The address of the server that accepted status request.
* load_timestamp  
Time of the last reload of configuration, in milliseconds since Epoch.
* timestamp  
Current time in milliseconds since Epoch.
* connections  
    * accepted  
The total number of accepted client connections.
    * dropped  
The total number of dropped client connections.
    * active  
The current number of active client connections.
    * idle  
The current number of idle client connections.
* requests  
    * total  
The total number of client requests.
    * current  
The current number of client requests.  
* server_zones  
For each status_zone:
    * processing  
The number of client requests that are currently being processed.
    * requests  
The total number of client requests received from clients.
    * responses  
        * total  
The total number of responses sent to clients.
        * 1xx, 2xx, 3xx, 4xx, 5xx  
The number of responses with status codes 1xx, 2xx, 3xx, 4xx, and 5xx.
    * received  
The total number of bytes received from clients.
    * sent  
The total number of bytes sent to clients.
* upstreams  
For each server in the dynamically configurable group, the following data are provided:
    * server  
An address of the server.
    * backup  
A boolean value indicating whether the “cache loader” process is still loading data from disk into the cache.
    * weight  
Weight of the server.
    * state  
Current state, which may be one of “up”, “down”, “unavail”, or “unhealthy”.
    * active  
The current number of active connections.
    * keepalive  
The current number of idle keepalive connections.
    * requests  
The total number of client requests forwarded to this server.
    * responses  
        * total  
The total number of responses obtained from this server.
        * 1xx, 2xx, 3xx, 4xx, 5xx  
The number of responses with status codes 1xx, 2xx, 3xx, 4xx, and 5xx.
    * sent  
The total number of bytes sent to this server.
    * received  
The total number of bytes received from this server.
    * fails  
The total number of unsuccessful attempts to communicate with the server.
    * unavail  
How many times the server became unavailable for client requests (state “unavail”) due to the number of unsuccessful attempts reaching the max_fails threshold.
    * health_checks  
        * checks  
The total number of health check requests made.
        * fails  
The number of failed health checks.
        * unhealthy  
How many times the server became unhealthy (state “unhealthy”).
        * last_passed  
Boolean indicating if the last health check request was successful and passed tests.
    * downtime    
Total time the server was in the “unavail” and “unhealthy” states.
    * downstart  
The time (in milliseconds since Epoch) when the server became “unavail” or “unhealthy”.
* caches  
For each cache (configured by proxy_cache_path and the likes):
    * size  
The current size of the cache.
    * max_size  
The limit on the maximum size of the cache specified in the configuration.
    * cold  
Boolean indicating if “cache loader” is still loading data into the cache.
    * hits, stale, updating  
        * responses  
The total number of responses read from the cache (hits, or stale responses due to proxy_cache_use_stale and the likes).
        * bytes  
The total number of bytes read from the cache.
    * miss, expired, bypass  
        * responses  
The total number of responses not taken from the cache (misses, expires, or bypasses due to proxy_cache_bypass and the likes).
        * bytes  
The total number of bytes read from the proxied server.
        * responses_written  
The total number of responses written to the cache.
        * bytes_written  
The total number of bytes written to the cache.
