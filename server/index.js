const mysql = require('mysql')

const db = mysql.createConnection({
    host: "localhost",
    user: "root",
    password: "",
    database:"weatherdata" 
});

db.connect( err => {
    if (err){
        console.log('error db')
        return err;
    }
    else{
        console.log("Database connected");
    }
});

const express = require('express')
const app = express()
const port = 5000
const sensorsCount = 2

app.use(function(req, res, next){
    res.header('Access-Control-Allow-Origin', '*');
    res.header("Access-Control-Allow-Headers", 'Content-Type, Access-Control-Allow-Headers, X-Requested-With');
    next()
})

app.get('/', (req, res) => {
    res.send('Hello World!')
})

app.get('/get', (req, res)=>{
    db.query( "SELECT * FROM `data`", (err, result, field) =>{
        let data = []
        for (let i = 0; i < result.length; ++i){
            if (result[i]["device"] == req.query.id || req.query.id == "last"){
                data.push({"value": result[i]["value"],
                        "device": result[i]["device"],
                        "time": result[i]["time"],
                        "type": result[i]["type"]
                })
            }
            else if (req.query.id == "all"){
                let f = true
                for (let x = 0; x < data.length; ++x){
                    if (data[x]["device"] == result[i]["device"]){
                        f = false
                    }
                }
                if (f){
                    data.push({
                        "device": result[i]["device"]
                    })
                }
            }
        }
        if (req.query.id != "all" && req.query.id != "last"){
            data.sort(function(a,b){
                let tA = Number(a["time"].split(':')[0])*60 + Number(a["time"].split(':')[1])
                let tB = Number(b["time"].split(':')[0])*60 + Number(b["time"].split(':')[1])
                if (tA > tB){
                    return 1
                }
                else if (tA < tB){
                    return -1
                }
                else{
                    return 0
                }
            })
            let elems = []
            for (let j = 0; j + sensorsCount <= data.length; j += sensorsCount){
                let temp = {}
                for (let x = j; x < j + sensorsCount; ++x){
                    temp[data[x]["type"]] = data[x]["value"]
                    temp["time"] = data[x]["time"]
                    temp["device"] = data[x]["device"]
                }
                elems.push(temp)
            }
            data = elems
        }
        if (req.query.id == "last"){
            let strng = {}
            for (let i = 1; i <= sensorsCount; ++i){
                strng[data[data.length-i]["type"]] = data[data.length-i]["value"]
            }
            res.send({data: strng})
        }
        else{ 
            res.send({data: data})
        }
    });
})

app.post('/upload', (req, res) => {
    try{
        let body = ''
        req.on("data", (data)=>{
            body += data
        })
        req.on("end", ()=>{
            while (body.includes("'")){
                body = body.replace("'", '"', 1)
            }
            let params = JSON.parse(body)
            console.log(params)
            db.query("INSERT INTO `data`(`type`, `time`, `device`, `value`) VALUES ('" + params["type"] + "','" + params["time"] + "','" + params["device"] + "','" + params["value"]+ "')", (err, result, field) =>{ ////not sure
            });
            res.send({ok: true})
        })
    } catch (err){
        console.log(err)
    }
})

app.listen(port, ()=>{
    console.log("work")
})