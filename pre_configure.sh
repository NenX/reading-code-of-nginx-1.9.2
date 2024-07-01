#!/bin/sh

./configure --add-module=./src/mytest_config \
--add-module=./src/my_test_module \
--add-module=./my_test/test_ngx_list \
--add-module=./my_test/playground \
--add-module=./my_test/test_ngx_hash \
--add-module=./my_test/test_ngx_rbtree \
--add-module=./my_test/test_ngx_queue`` \
--add-module=./src/mytest_subrequest \
--add-module=./src/mytest_upstream \
--add-module=./src/ngx_http_myfilter_module \
--add-module=./src/sendfile_test \
--add-module=./src/nginx-requestkey-module-master/ \
--add-module=./src/redis2-nginx-module-master/ \
--with-debug \
--with-file-aio \
--with-threads  \
--with-http_secure_link_module
# ./configure --add-module=./src/my_test_module --with-debug --with-file-aio