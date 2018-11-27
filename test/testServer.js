const express = require('express');
const app = express();
var bodyParser = require('body-parser');

const router = express.Router();
const port = 3000;

// url: http://localhost:3000/
app.get('/', (request, response) => response.send('Hello World'));

// all routes prefixed with /api
app.use('/api/v1/producer', router);
app.use(bodyParser.json()); // for parsing application/json
app.use(bodyParser.urlencoded({ extended: true })); // for parsing application/x-www-form-

// using router.get() to prefix our path
// url: http://localhost:3000/api/
router.post('/jsonregister', (request, response) => {
    response.json({message: 'REGISTERING:'+request});
    console.log("--> REGISTER req:"+JSON.stringify(request.headers) + " body:"+request.body);
});

router.post('/jsoninsert', (request, response) => {
    response.json({message: 'INSERT:'+request});
    var body="";
    request.on('data', chunk => {
        body += chunk.toString(); // convert Buffer to string
    });
    request.on('end', () => {
	var obj=JSON.parse(body);
        console.log("---->"+JSON.stringify(obj));
//	console.log("--> INSERT req:"+JSON.stringify(request.headers)+" body:"+request.body);
        
    });

});

// set the server to listen on port 3000
app.listen(port, () => console.log(`Listening on port ${port}`));
