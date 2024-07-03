const a = require('http')
a.createServer((req, res) => {
    const resText = '"三一重共工,13.41,dddd,xxxxxx,298900,gggg,zzzz"\n'
    res.writeHead(200, 'OK', { 'Content-Type': 'text/html', 'charset': 'utf8' })
    // const resText = '"三一重工,13.41,dddd,xxxxxx,298900,gggg,zzzz"'
    const b = Buffer.from(resText, 'utf8')

    function chunked() {
        console.log(req.headers)
        if(true)
            return res.end()
        let cb = d => {
            const str = d.toString()
            if (str.startsWith('x')) {
                console.log('close')
                res.end()
                process.stdin.off('data', cb)
            } else {
                res.write(str)
                console.log('???', { d: d + '' })

            }
        }
        process.stdin.on('data', cb)
    }
    if (req.method === 'POST') {
        req.on('data', d => {
            console.log('req', req.method, d + '')
            res.write(b)
            chunked()
        })
    } else {
        res.write(b)
        console.log('req', req.method, req.url)
        chunked()

    }

})
    .listen(2222, () => {
        console.log('running at 2222')
    })

