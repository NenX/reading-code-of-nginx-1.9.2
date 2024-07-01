const a = require('http')
a.createServer((req, res) => {
    const resText = '"三一重共工,13.41,dddd,xxxxxx,298900,gggg,zzzz"'
    res.writeHead(200, 'OK', { 'Content-Type': 'text/html', 'charset': 'utf8' })
    // const resText = '"三一重工,13.41,dddd,xxxxxx,298900,gggg,zzzz"'
    const b = Buffer.from(resText, 'utf8')
    console.log('req',req.headers)
    res.write(b)
    res.end();
})
.listen(2222, () => {
    console.log('running at 2222')
})