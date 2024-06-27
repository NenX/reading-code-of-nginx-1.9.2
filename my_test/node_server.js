const http = require('http')

http.createServer((req,res)=>{
    res.setHeader('Say-Hello','hello')
    res.write('okok')
    res.end()

}).listen(8899,()=>console.log('runing'))