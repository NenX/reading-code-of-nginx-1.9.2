#!/bin/sh

./configure --add-module=./src/mytest_config --add-module=./src/my_test_module --add-module=./src/mytest_subrequest --add-module=./src/mytest_upstream --add-module=./src/ngx_http_myfilter_module --with-debug --with-file-aio --add-module=./src/sendfile_test --with-threads  --add-module=./src/nginx-requestkey-module-master/ --with-http_secure_link_module --add-module=./src/redis2-nginx-module-master/ 
# ./configure --add-module=./src/my_test_module --with-debug --with-file-aio